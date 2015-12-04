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

#ifndef TRIVIALQUESTNATIVIACTIVITY_H_
#define TRIVIALQUESTNATIVIACTIVITY_H_

/*
 * Include files
 */
#include <jni.h>
#include <errno.h>

#include <android/log.h>
#include <android_native_app_glue.h>
#include <android/native_window_jni.h>
#include <cpu-features.h>
#include <sstream>
#include <algorithm>
#include <locale>
#include <codecvt>

// For GPGS
#include "gpg/gpg.h"

#include "TeapotRenderer.h"
#include "NDKHelper.h"
#include "JavaUI.h"

/*
 * Preprocessors
 */

// Class name of helper function
#define HELPER_CLASS_NAME "com.sample.helper.NDKHelper"
// Class name of JUIhelper function
#define JUIHELPER_CLASS_NAME "com.sample.helper.JUIHelper"
// Share object name of helper function library
#define HELPER_CLASS_SONAME "TrivialQuestNativeActivity"

//
// Also set "com.google.android.gms.games.APP_ID" in AndrdoiManifest.xml
//

/*
 * Engine class of the sample
 */
struct android_app;
class Engine {
public:
  // GPG related methods
  void InitGooglePlayGameServices();
  void ShowQuestUI();
  void ShowEventStatus();
  void CompleteQuest(gpg::Quest quest);
  void ClaimMilestone(gpg::QuestMilestone milestone);

  // Event handling
  static void HandleCmd(struct android_app *app, int32_t cmd);
  static int32_t HandleInput(android_app *app, AInputEvent *event);
  void UpdatePosition(AInputEvent *event, int32_t iIndex, float &fX, float &fY);

  // Engine life cycles
  Engine();
  ~Engine();
  void SetState(android_app *state);
  int InitDisplay(const int32_t cmd);
  void LoadResources();
  void UnloadResources();
  void DrawFrame();
  void TermDisplay(const int32_t cmd);
  void TrimMemory();
  bool IsReady();

  // Sensor managers
  void ProcessSensors(const int32_t id) { sensroManager_.Process(id); }
  void ResumeSensors() { sensroManager_.Resume(); }
  void SuspendSensors() { sensroManager_.Suspend(); }

private:
  // Callbacks from GPG.
  void OnAuthActionStarted(gpg::AuthOperation op);
  void OnAuthActionFinished(gpg::AuthOperation op, gpg::AuthStatus status);

  std::unique_ptr<gpg::GameServices> service_;

  bool authorizing_;
  //
  void EnableUI(bool enable);
  void InitUI();
  void TransformPosition(ndk_helper::Vec2 &vec);
  void UpdateFPS(float fFPS);

  // Renderer of a teapot
  TeapotRenderer renderer_;

  // GLContext instance
  ndk_helper::GLContext *gl_context_;

  // Sensor manager
  ndk_helper::SensorManager sensroManager_;

  bool initialized_resources_;
  bool has_focus_;

  // Helpers for touch control
  ndk_helper::DoubletapDetector doubletap_detector_;
  ndk_helper::PinchDetector pinch_detector_;
  ndk_helper::DragDetector drag_detector_;
  ndk_helper::PerfMonitor monitor_;
  ndk_helper::TapCamera tap_camera_;

  // JUI text view to show FPS
  jui_helper::JUITextView *textViewFPS_;

  // Native acitivity app instance
  android_app *app_;

  // JUI dialog
  jui_helper::JUIDialog *dialog_;

  jui_helper::JUIButton *button_sign_in_;
  jui_helper::JUIButton *button_blue_;
  jui_helper::JUIButton *button_red_;
  jui_helper::JUIButton *button_yellow_;
  jui_helper::JUIButton *button_green_;
  jui_helper::JUIButton *button_events_;
  jui_helper::JUIButton *button_quests_;

  jui_helper::JUITextView *status_text_;

};

#endif //TRIVIALQUESTNATIVIACTIVITY_H_
