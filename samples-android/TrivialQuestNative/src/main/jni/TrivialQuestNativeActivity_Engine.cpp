/*
 * Copyright 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Include files
 */
#include "TrivialQuestNativeActivity.h"

/*
 * Ctor
 */
Engine::Engine()
    : initialized_resources_(false), has_focus_(false), authorizing_(false),
      app_(nullptr), dialog_(nullptr), textViewFPS_(nullptr),
      button_sign_in_(nullptr), status_text_(nullptr), button_events_(nullptr) {
  gl_context_ = ndk_helper::GLContext::GetInstance();
}

/*
 * Dtor
 */
Engine::~Engine() {
  jui_helper::JUIWindow::GetInstance()->Close();
  delete dialog_;
}

/**
 * Load resources
 */
void Engine::LoadResources() {
  renderer_.Init();
  renderer_.Bind(&tap_camera_);
}

/**
 * Unload resources
 */
void Engine::UnloadResources() { renderer_.Unload(); }

/**
 * Initialize an EGL context for the current display.
 */
int Engine::InitDisplay(const int32_t cmd) {
  if (!initialized_resources_) {
    gl_context_->Init(app_->window);
    InitUI();
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

  // Enable culling OpenGL state
  glEnable(GL_CULL_FACE);

  // Enabled depth test OpenGL state
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

/**
 * Just the current frame in the display.
 */
void Engine::DrawFrame() {
  float fFPS;
  if (monitor_.Update(fFPS)) {
    UpdateFPS(fFPS);
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

/**
 * Tear down the EGL context currently associated with the display.
 */
void Engine::TermDisplay(const int32_t cmd) {
  gl_context_->Suspend();
  jui_helper::JUIWindow::GetInstance()->Suspend(cmd);
}

void Engine::TrimMemory() {
  LOGI("Trimming memory");
  gl_context_->Invalidate();
}

/**
 * Process the next input event.
 * In the handler, it passes touch events to gesture detector helper class
 * Based on gesture detection, the app moves camera, model view accordingly
 */
int32_t Engine::HandleInput(android_app *app, AInputEvent *event) {
  Engine *eng = (Engine *)app->userData;
  if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
    ndk_helper::GESTURE_STATE doubleTapState =
        eng->doubletap_detector_.Detect(event);
    ndk_helper::GESTURE_STATE dragState = eng->drag_detector_.Detect(event);
    ndk_helper::GESTURE_STATE pinchState = eng->pinch_detector_.Detect(event);

    // Double tap detector has a priority over other detectors
    if (doubleTapState == ndk_helper::GESTURE_STATE_ACTION) {
      // Detect double tap
      eng->tap_camera_.Reset(true);
    } else {
      // Handle drag state
      if (dragState & ndk_helper::GESTURE_STATE_START) {
        // Otherwise, start dragging
        ndk_helper::Vec2 v;
        eng->drag_detector_.GetPointer(v);
        eng->TransformPosition(v);
        eng->tap_camera_.BeginDrag(v);
      } else if (dragState & ndk_helper::GESTURE_STATE_MOVE) {
        ndk_helper::Vec2 v;
        eng->drag_detector_.GetPointer(v);
        eng->TransformPosition(v);
        eng->tap_camera_.Drag(v);
      } else if (dragState & ndk_helper::GESTURE_STATE_END) {
        eng->tap_camera_.EndDrag();
      }

      // Handle pinch state
      if (pinchState & ndk_helper::GESTURE_STATE_START) {
        // Start new pinch
        ndk_helper::Vec2 v1;
        ndk_helper::Vec2 v2;
        eng->pinch_detector_.GetPointers(v1, v2);
        eng->TransformPosition(v1);
        eng->TransformPosition(v2);
        eng->tap_camera_.BeginPinch(v1, v2);
      } else if (pinchState & ndk_helper::GESTURE_STATE_MOVE) {
        // Multi touch
        // Start new pinch
        ndk_helper::Vec2 v1;
        ndk_helper::Vec2 v2;
        eng->pinch_detector_.GetPointers(v1, v2);
        eng->TransformPosition(v1);
        eng->TransformPosition(v2);
        eng->tap_camera_.Pinch(v1, v2);
      }
    }
    return 1;
  }
  return 0;
}

/**
 * Process the next main command.
 */
void Engine::HandleCmd(struct android_app *app, int32_t cmd) {
  Engine *eng = (Engine *)app->userData;
  LOGI("message %d", cmd);
  switch (cmd) {
  case APP_CMD_SAVE_STATE:
    break;
  case APP_CMD_INIT_WINDOW:
    if (app->window != NULL) {
      eng->InitDisplay(APP_CMD_INIT_WINDOW);
      eng->DrawFrame();
    }

    break;
  case APP_CMD_TERM_WINDOW:
    // Note that JUI helper needs to know if a window has been terminated
    eng->TermDisplay(APP_CMD_TERM_WINDOW);

    eng->has_focus_ = false;
    break;
  case APP_CMD_START:
    break;
  case APP_CMD_STOP:
    break;
  case APP_CMD_RESUME:
    jui_helper::JUIWindow::GetInstance()->Resume(app->activity, APP_CMD_RESUME);
    break;
  case APP_CMD_GAINED_FOCUS:
    // Start animation
    eng->ResumeSensors();
    eng->has_focus_ = true;
    jui_helper::JUIWindow::GetInstance()->Resume(app->activity,
                                                 APP_CMD_GAINED_FOCUS);
    break;
  case APP_CMD_LOST_FOCUS:
    // Also stop animating.
    eng->SuspendSensors();
    eng->has_focus_ = false;
    eng->DrawFrame();
    break;
  case APP_CMD_LOW_MEMORY:
    // Free up GL resources
    eng->TrimMemory();
    break;
  case APP_CMD_CONFIG_CHANGED:
    // Configuration changes
    eng->TermDisplay(APP_CMD_CONFIG_CHANGED);
    eng->InitDisplay(APP_CMD_CONFIG_CHANGED);
    break;
  case APP_CMD_DESTROY:
    ndk_helper::JNIHelper::GetInstance()->DetachCurrentThread();
    break;
  }
}

/*
 * Misc
 */
void Engine::SetState(android_app *state) {
  app_ = state;
  doubletap_detector_.SetConfiguration(app_->config);
  drag_detector_.SetConfiguration(app_->config);
  pinch_detector_.SetConfiguration(app_->config);
  sensroManager_.Init(state);
}

bool Engine::IsReady() {
  if (has_focus_)
    return true;

  return false;
}

void Engine::TransformPosition(ndk_helper::Vec2 &vec) {
  vec = ndk_helper::Vec2(2.0f, 2.0f) * vec /
            ndk_helper::Vec2(gl_context_->GetScreenWidth(),
                             gl_context_->GetScreenHeight()) -
        ndk_helper::Vec2(1.f, 1.f);
}

void Engine::UpdateFPS(float fFPS) {
  ndk_helper::JNIHelper::GetInstance()->RunOnUiThread([fFPS, this]() {
    const int32_t count = 64;
    char str[count];
    snprintf(str, count, "%2.2f FPS", fFPS);
    textViewFPS_->SetAttribute("Text", (const char *)str);
  });

  return;
}

Engine g_engine;

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(android_app *state) {
  app_dummy();

  g_engine.SetState(state);

  // Init helper functions
  ndk_helper::JNIHelper::Init(state->activity, HELPER_CLASS_NAME,
                              HELPER_CLASS_SONAME);

  // Init play game services
  g_engine.InitGooglePlayGameServices();

  state->userData = &g_engine;
  state->onAppCmd = Engine::HandleCmd;
  state->onInputEvent = Engine::HandleInput;

  // loop waiting for stuff to do.
  while (1) {
    // Read all pending events.
    int id;
    int events;
    android_poll_source *source;

    // If not animating, we will block forever waiting for events.
    // If animating, we loop until all events are read, then continue
    // to draw the next frame of animation.
    while ((id = ALooper_pollAll(g_engine.IsReady() ? 0 : -1, NULL, &events,
                                 (void **)&source)) >= 0) {
      // Process this event.
      if (source != NULL)
        source->process(state, source);

      g_engine.ProcessSensors(id);

      // Check if we are exiting.
      if (state->destroyRequested != 0) {
        g_engine.TermDisplay(APP_CMD_TERM_WINDOW);
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
Java_com_google_example_games_tq_TrivialQuestNativeActivity_OnPauseHandler(
    JNIEnv *env) {
  // This call is to suppress 'E/WindowManager(): android.view.WindowLeaked...'
  // errors.
  // Since orientation change events in NativeActivity comes later than
  // expected, we can not dismiss
  // popupWindow gracefully from NativeActivity.
  // So we are releasing popupWindows explicitly triggered from Java callback
  // through JNI call.
  jui_helper::JUIWindow::GetInstance()->Suspend(APP_CMD_PAUSE);
}
}
