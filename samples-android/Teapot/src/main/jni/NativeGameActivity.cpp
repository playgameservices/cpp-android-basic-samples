//
// Copyright 2013 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http:// www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

//------------------------------------------------------------------------------
// Include files
//------------------------------------------------------------------------------
#include <android/log.h>
#include <android/native_window_jni.h>
#include <android_native_app_glue.h>
#include <cpu-features.h>
#include <errno.h>
#include <jni.h>

// For GPGS
#include "gpg/android_platform_configuration.h"
#include "gpg/android_initialization.h"
#include "gpg/android_support.h"

#include "JavaUI.h"
#include "NDKHelper.h"
#include "StateManager.h"
#include "TeapotRenderer.h"


//------------------------------------------------------------------------------
// Preprocessor
//------------------------------------------------------------------------------
// Class name of helper function
#define HELPER_CLASS_NAME "com.sample.helper.NDKHelper"
// Class name of JUIhelper function
#define JUIHELPER_CLASS_NAME "com.sample.helper.JUIHelper"
// Share object name of helper function library
#define HELPER_CLASS_SONAME "NativeGameActivity"

//------------------------------------------------------------------------------
// Shared state for our app.
//------------------------------------------------------------------------------
class Engine {
 public:
  static void HandleCmd(android_app *app, int32_t cmd);
  static int32_t HandleInput(android_app *app, AInputEvent *event);

  Engine();
  ~Engine();
  void SetState(android_app *state);
  int InitDisplay(const int32_t cmd);
  void LoadResources();
  void UnloadResources();
  void DrawFrame();
  void TermDisplay();
  void TrimMemory();
  bool IsReady();

  // Callbacks from GPG.
  void OnAuthActionStarted(gpg::AuthOperation op);
  void OnAuthActionFinished(gpg::AuthOperation op, gpg::AuthStatus status);

 private:
  TeapotRenderer renderer_;

  ndk_helper::GLContext *gl_context_;

  bool initialized_resources_;
  bool has_focus_;

  ndk_helper::DoubletapDetector doubletap_detector_;
  ndk_helper::PinchDetector pinch_detector_;
  ndk_helper::DragDetector drag_detector_;
  ndk_helper::PerfMonitor monitor_;

  jui_helper::JUIButton *button_sign_in_;
  jui_helper::JUITextView *status_text_;

  ndk_helper::TapCamera tap_camera_;

  android_app *app_;
  int current_score_ = 0;

  void UpdateFPS(float fps);
  void ShowUI();
  void TransformPosition(ndk_helper::Vec2 *vec);

  void InitUI();
};

Engine g_engine;

Engine::Engine()
    : initialized_resources_(false), has_focus_(false), app_(nullptr),
      button_sign_in_(nullptr), status_text_(nullptr) {
  gl_context_ = ndk_helper::GLContext::GetInstance();
}

Engine::~Engine() {
  jui_helper::JUIWindow::GetInstance()->Close();
}

void Engine::InitUI() {
  // The window is being shown, get it ready.
  jui_helper::JUIWindow::Init(app_->activity, JUIHELPER_CLASS_NAME);

  // Show toast with app label
  ndk_helper::JNIHelper::GetInstance()->RunOnUiThread([]() {
    jui_helper::JUIToast toast(
      ndk_helper::JNIHelper::GetInstance()->GetAppLabel());
    toast.Show();
  });

  //
  // Buttons
  //
  // Sign in button.
  button_sign_in_ = new jui_helper::JUIButton(
      StateManager::GetGameServices()->IsAuthorized() ? "Sign Out" : "Sign In");
  button_sign_in_->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                           jui_helper::LAYOUT_PARAMETER_TRUE);
  button_sign_in_->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                           jui_helper::LAYOUT_PARAMETER_TRUE);
  button_sign_in_->SetCallback(
      [this](jui_helper::JUIView *view, const int32_t message) {
        LOGI("button_sign_in_ click: %d", message);
        if (message == jui_helper::JUICALLBACK_BUTTON_UP) {
          if (StateManager::GetGameServices()->IsAuthorized()) {
            StateManager::SignOut();
          } else {
            StateManager::BeginUserInitiatedSignIn();
          }
        }
      });
  jui_helper::JUIWindow::GetInstance()->AddView(button_sign_in_);

  // Achievement button
  jui_helper::JUIButton *button_achievement =
      new jui_helper::JUIButton("Unlock Achievement");
  button_achievement->AddRule(jui_helper::LAYOUT_PARAMETER_BELOW,
                              button_sign_in_);
  button_achievement->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                              jui_helper::LAYOUT_PARAMETER_TRUE);
  button_achievement->SetCallback(
      [this](jui_helper::JUIView *view, const int32_t message) {
        LOGI("Button click: %d", message);
        if (message == jui_helper::JUICALLBACK_BUTTON_UP) {
            std::string id = ndk_helper::JNIHelper::GetInstance()->GetStringResource("achievement_prime");
            if( id == "" || id == "ReplaceMe") {
                LOGI("Invalid achievement ID!, please check res/values/ids.xml");
                return;
            }
          StateManager::UnlockAchievement(id.c_str());
        }
      });
  jui_helper::JUIWindow::GetInstance()->AddView(button_achievement);

  // Show Achievements button
  jui_helper::JUIButton *button_show_achievements_ui =
      new jui_helper::JUIButton("Show Achievements UI!");
  button_show_achievements_ui->AddRule(jui_helper::LAYOUT_PARAMETER_BELOW,
                                       button_achievement);
  button_show_achievements_ui->AddRule(
      jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
      jui_helper::LAYOUT_PARAMETER_TRUE);
  button_show_achievements_ui->SetCallback(
      [this](jui_helper::JUIView *view, const int32_t message) {
        LOGI("Button click: %d", message);
        if (message == jui_helper::JUICALLBACK_BUTTON_UP) {
          StateManager::ShowAchievements();
        }
      });
  jui_helper::JUIWindow::GetInstance()->AddView(button_show_achievements_ui);

  // High score UI
  //
  // Setup SeekBar
  //
  jui_helper::JUISeekBar *seek_bar_high_score = new jui_helper::JUISeekBar();
  seek_bar_high_score->SetCallback(
      jui_helper::JUICALLBACK_SEEKBAR_PROGRESSCHANGED,
      [this](jui_helper::JUIView *view, const int32_t mes, const int32_t p1,
          const int32_t p2) {
        LOGI("Seek progress %d", p1);
        current_score_ = p1;
      });

  // Configure relative layout parameter
  seek_bar_high_score->AddRule(jui_helper::LAYOUT_PARAMETER_BELOW,
                               button_show_achievements_ui);
  seek_bar_high_score->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                               jui_helper::LAYOUT_PARAMETER_TRUE);
  seek_bar_high_score->SetLayoutParams(jui_helper::ATTRIBUTE_SIZE_MATCH_PARENT,
                                       jui_helper::ATTRIBUTE_SIZE_WRAP_CONTENT);
  jui_helper::JUIWindow::GetInstance()->AddView(seek_bar_high_score);

  // Achievement Button
  jui_helper::JUIButton *button_submit_high_score =
      new jui_helper::JUIButton("Submit High Score!!");
  button_submit_high_score->AddRule(jui_helper::LAYOUT_PARAMETER_BELOW,
                                    seek_bar_high_score);
  button_submit_high_score->AddRule(
      jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
      jui_helper::LAYOUT_PARAMETER_TRUE);
  button_submit_high_score->SetCallback(
      [this](jui_helper::JUIView *view, const int32_t message) {
        LOGI("Button click: %d", message);
        if (message == jui_helper::JUICALLBACK_BUTTON_UP) {
            std::string id = ndk_helper::JNIHelper::GetInstance()->GetStringResource("leaderboard_easy");
            if( id == "" || id == "ReplaceMe") {
                LOGI("Invalid Leaderboard ID!, please check res/values/ids.xml");
                return;
            }
          StateManager::SubmitHighScore(id.c_str(), current_score_);
        }
      });
  jui_helper::JUIWindow::GetInstance()->AddView(button_submit_high_score);

  // Show Leaderboard UI
  jui_helper::JUIButton *button_show_leaderboard_ui =
      new jui_helper::JUIButton("Show Leaderboard UI!");
  button_show_leaderboard_ui->AddRule(jui_helper::LAYOUT_PARAMETER_BELOW,
                                      button_submit_high_score);
  button_show_leaderboard_ui->AddRule(
      jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
      jui_helper::LAYOUT_PARAMETER_TRUE);
  button_show_leaderboard_ui->SetCallback(
      [this](jui_helper::JUIView *view, const int32_t message) {
        LOGI("Button click: %d", message);
        if (message == jui_helper::JUICALLBACK_BUTTON_UP) {
            std::string id = ndk_helper::JNIHelper::GetInstance()->GetStringResource("leaderboard_easy");
            if( id == "" || id == "ReplaceMe") {
                LOGI("Invalid Leaderboard ID!, please check res/values/ids.xml");
                return;
            }
          StateManager::ShowLeaderboard(id.c_str());
        }
      });
  jui_helper::JUIWindow::GetInstance()->AddView(button_show_leaderboard_ui);

  status_text_ = new jui_helper::JUITextView(
      StateManager::IsAuthInProgress()
      ? "Signing In..."
      : StateManager::GetGameServices()->IsAuthorized()
        ? "Signed In."
        : "Signed Out.");

  status_text_->AddRule(jui_helper::LAYOUT_PARAMETER_ALIGN_PARENT_BOTTOM,
                        jui_helper::LAYOUT_PARAMETER_TRUE);
  status_text_->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                        jui_helper::LAYOUT_PARAMETER_TRUE);
  jui_helper::JUIWindow::GetInstance()->AddView(status_text_);
  return;
}

void Engine::LoadResources() {
  renderer_.Init();
  renderer_.Bind(&tap_camera_);
}

void Engine::UnloadResources() {
  renderer_.Unload();
}

// Initialize an EGL context for the current display.
int Engine::InitDisplay(const int32_t cmd) {
  if (!initialized_resources_) {
    gl_context_->Init(app_->window);
    InitUI();
    gl_context_->SetSwapInterval(0);  // Set interval of 0 for a benchmark
    LoadResources();
    initialized_resources_ = true;
  } else {
    // initialize OpenGL ES and EGL
    if (EGL_SUCCESS != gl_context_->Resume(app_->window)) {
      UnloadResources();
      LoadResources();
    }
    jui_helper::JUIWindow::GetInstance()->Resume(app_->activity, cmd);
  }

  ShowUI();

  // Initialize GL state.
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  // Note that screen size might have been changed
  glViewport(0, 0, gl_context_->GetScreenWidth(),
             gl_context_->GetScreenHeight());
  renderer_.UpdateViewport();

  tap_camera_.SetFlip(1.f, -1.f, -1.f);
  tap_camera_.SetPinchTransformFactor(2.f, 2.f, 8.f);

  return 0;
}

// Just the current frame in the display.
void Engine::DrawFrame() {
  float fps;
  if (monitor_.Update(fps)) {
    UpdateFPS(fps);
  }
  renderer_.Update(monitor_.GetCurrentTime());

  // Just fill the screen with a color.
  glClearColor(0.5f, 0.5f, 0.5f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  renderer_.Render();

  // Swap
  if (EGL_SUCCESS != gl_context_->Swap()) {
    UnloadResources();
    LoadResources();
  }
}

// Tear down the EGL context currently associated with the display.
void Engine::TermDisplay() {
  gl_context_->Suspend();
}

void Engine::TrimMemory() {
  LOGI("Trimming memory");
  gl_context_->Invalidate();
}

// Process the next input event.
int32_t Engine::HandleInput(android_app *app, AInputEvent *event) {
  Engine *eng = reinterpret_cast<Engine*>(app->userData);
  if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
    ndk_helper::GESTURE_STATE double_tap_state =
        eng->doubletap_detector_.Detect(event);
    ndk_helper::GESTURE_STATE drag_state = eng->drag_detector_.Detect(event);
    ndk_helper::GESTURE_STATE pinch_state = eng->pinch_detector_.Detect(event);

    // Double tap detector has a priority over other detectors
    if (double_tap_state == ndk_helper::GESTURE_STATE_ACTION) {
      // Detect double tap
      eng->tap_camera_.Reset(true);
    } else {
      // Handle drag state
      if (drag_state & ndk_helper::GESTURE_STATE_START) {
        // Otherwise, start dragging
        ndk_helper::Vec2 v;
        eng->drag_detector_.GetPointer(v);
        eng->TransformPosition(&v);
        eng->tap_camera_.BeginDrag(v);
      } else if (drag_state & ndk_helper::GESTURE_STATE_MOVE) {
        ndk_helper::Vec2 v;
        eng->drag_detector_.GetPointer(v);
        eng->TransformPosition(&v);
        eng->tap_camera_.Drag(v);
      } else if (drag_state & ndk_helper::GESTURE_STATE_END) {
        eng->tap_camera_.EndDrag();
      }

      // Handle pinch state
      if (pinch_state & ndk_helper::GESTURE_STATE_START) {
        // Start new pinch
        ndk_helper::Vec2 v1;
        ndk_helper::Vec2 v2;
        eng->pinch_detector_.GetPointers(v1, v2);
        eng->TransformPosition(&v1);
        eng->TransformPosition(&v2);
        eng->tap_camera_.BeginPinch(v1, v2);
      } else if (pinch_state & ndk_helper::GESTURE_STATE_MOVE) {
        // Multi touch
        // Start new pinch
        ndk_helper::Vec2 v1;
        ndk_helper::Vec2 v2;
        eng->pinch_detector_.GetPointers(v1, v2);
        eng->TransformPosition(&v1);
        eng->TransformPosition(&v2);
        eng->tap_camera_.Pinch(v1, v2);
      }
    }
    return 1;
  }
  return 0;
}

// Process the next main command.
void Engine::HandleCmd(struct android_app *app, int32_t cmd) {
  Engine *eng = reinterpret_cast<Engine*>(app->userData);
  switch (cmd) {
    case APP_CMD_INIT_WINDOW: {
      // The window is being shown, get it ready.
      if (app->window != nullptr) {
        eng->InitDisplay(cmd);
        eng->DrawFrame();
      }
      break;
    }
    case APP_CMD_RESUME: {
      jui_helper::JUIWindow::GetInstance()->Resume(app->activity,
                                                   APP_CMD_RESUME);
      break;
    }
    case APP_CMD_GAINED_FOCUS: {
      // Start animation
      eng->has_focus_ = true;
      jui_helper::JUIWindow::GetInstance()->Resume(app->activity,
                                                   APP_CMD_GAINED_FOCUS);
      break;
    }
    case APP_CMD_TERM_WINDOW: {
      // The window is being hidden or closed, clean it up.
      eng->TermDisplay();
      eng->has_focus_ = false;
      break;
    }
    case APP_CMD_LOST_FOCUS: {
      // Also stop animating.
      eng->has_focus_ = false;
      eng->DrawFrame();
      break;
    }
    case APP_CMD_LOW_MEMORY: {
      // Free up GL resources
      eng->TrimMemory();
      break;
    }
    default: {
      // Ignore other messages.
      break;
    }
  }
}

//-------------------------------------------------------------------------
// Misc
//-------------------------------------------------------------------------
void Engine::SetState(android_app *state) {
  app_ = state;
  doubletap_detector_.SetConfiguration(app_->config);
  drag_detector_.SetConfiguration(app_->config);
  pinch_detector_.SetConfiguration(app_->config);
}

bool Engine::IsReady() {
  return has_focus_;
}

void Engine::TransformPosition(ndk_helper::Vec2 *vec) {
  *vec = ndk_helper::Vec2(2.0f, 2.0f) * *vec
      / ndk_helper::Vec2(gl_context_->GetScreenWidth(),
                         gl_context_->GetScreenHeight())
      - ndk_helper::Vec2(1.f, 1.f);
}

void Engine::ShowUI() {
  JNIEnv *jni;
  app_->activity->vm->AttachCurrentThread(&jni, nullptr);

  // Default class retrieval
  jclass clazz = jni->GetObjectClass(app_->activity->clazz);
  jmethodID methodID = jni->GetMethodID(clazz, "showUI", "()V");
  jni->CallVoidMethod(app_->activity->clazz, methodID);

  app_->activity->vm->DetachCurrentThread();
}

void Engine::UpdateFPS(float fps) {
  JNIEnv *jni;
  app_->activity->vm->AttachCurrentThread(&jni, nullptr);

  // Default class retrieval
  jclass clazz = jni->GetObjectClass(app_->activity->clazz);
  jmethodID methodID = jni->GetMethodID(clazz, "updateFPS", "(F)V");
  jni->CallVoidMethod(app_->activity->clazz, methodID, fps);

  app_->activity->vm->DetachCurrentThread();
}

void Engine::OnAuthActionStarted(gpg::AuthOperation op) {
  if (!initialized_resources_) {
    return;
  }
  ndk_helper::JNIHelper::GetInstance()->RunOnUiThread([this, op]() {
    if (status_text_) {
      if (op == gpg::AuthOperation::SIGN_IN) {
        status_text_->SetAttribute("Text", "Signing In...");
      } else {
        status_text_->SetAttribute("Text", "Signing Out...");
      }
    }
  });
}

void Engine::OnAuthActionFinished(gpg::AuthOperation op,
                                  gpg::AuthStatus status) {
  if (!initialized_resources_) {
    return;
  }

  ndk_helper::JNIHelper::GetInstance()->RunOnUiThread([this, status]() {
    if (status == gpg::AuthStatus::VALID) {
      jui_helper::JUIToast toast("Signed In.");
      toast.Show();
    } else {
      jui_helper::JUIToast toast("Signed Out.");
      toast.Show();
    }

    if (button_sign_in_) {
      button_sign_in_->SetAttribute("Text", status == gpg::AuthStatus::VALID
                                            ? "Sign Out"
                                            : "Sign In");
    }

    if (status_text_) {
      status_text_->SetAttribute("Text", status == gpg::AuthStatus::VALID
                                         ? "Signed In"
                                         : "Signed Out");
    }
  });
}

// This is the main entry point of a native application that is using
// android_native_app_glue.  It runs in its own thread, with its own
// event loop for receiving input events and doing other things.
void android_main(android_app *state) {
  app_dummy();

  g_engine.SetState(state);

  // Init helper functions
  ndk_helper::JNIHelper::Init(state->activity, HELPER_CLASS_NAME,
                              HELPER_CLASS_SONAME);

  state->userData = &g_engine;
  state->onAppCmd = Engine::HandleCmd;
  state->onInputEvent = Engine::HandleInput;

#ifdef USE_NDK_PROFILER
  monstartup("libTeapotNativeActivity.so");
#endif

  // We could initialize in the JNI_OnLoad, but it's also valid here.
  gpg::AndroidInitialization::android_main(state);
  if (state->savedState == nullptr) {
    // We aren't resuming, create a new GameServices.
    gpg::AndroidPlatformConfiguration platform_configuration;
    platform_configuration.SetActivity(state->activity->clazz);
    StateManager::InitServices(
        platform_configuration,
        [](gpg::AuthOperation op) {
          g_engine.OnAuthActionStarted(op);
        },
        [](gpg::AuthOperation op, gpg::AuthStatus status) {
          g_engine.OnAuthActionFinished(op, status);
        });
  }

  // loop waiting for stuff to do.
  while (1) {
    // Read all pending events.
    int id;
    int events;
    android_poll_source *source;

    // If not animating, we will block forever waiting for events.
    // If animating, we loop until all events are read, then continue
    // to draw the next frame of animation.
    while ((id = ALooper_pollAll(g_engine.IsReady() ? 0 : -1, nullptr, &events,
                                 reinterpret_cast<void**>(&source))) >= 0) {
      // Process this event.
      if (source != nullptr) {
        source->process(state, source);
      }

      // Check if we are exiting.
      if (state->destroyRequested != 0) {
        g_engine.TermDisplay();
        return;
      }
    }

    if (g_engine.IsReady()) {
      // Drawing is throttled to the screen update rate, so there
      // is no need to do timing here.
      g_engine.DrawFrame();
    }
  }
}

extern "C" {
JNIEXPORT void
Java_com_google_example_nativegame_NativeGameActivity_OnPauseHandler(
    JNIEnv *env, jobject thiz) {
  // This call is to suppress 'E/WindowManager(): android.view.WindowLeaked...'
  // errors. Since orientation change events in NativeActivity comes later than
  // expected, we can not dismiss popupWindow gracefully from NativeActivity. So
  // we are releasing popupWindows explicitly triggered from Java callback
  // through JNI call.
  jui_helper::JUIWindow::GetInstance()->Suspend(APP_CMD_PAUSE);
}

JNIEXPORT void
Java_com_google_example_nativegame_NativeGameActivity_nativeOnActivityResult(
        JNIEnv *env, jobject thiz, jobject activity, jint requestCode,
        jint resultCode, jobject data) {
  gpg::AndroidSupport::OnActivityResult(env, activity, requestCode, resultCode,
                                        data);
}
}  // extern "C"

