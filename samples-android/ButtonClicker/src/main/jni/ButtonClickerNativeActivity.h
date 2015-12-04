//  Copyright (c) 2014 Google. All rights reserved.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

#ifndef TBMPSLEKETONNATIVIACTIVITY_H_
#define TBMPSLEKETONNATIVIACTIVITY_H_

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
#include <thread>

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
#define HELPER_CLASS_SONAME "ButtonClickerNativeActivity"

//
// Also set "com.google.android.gms.games.APP_ID" in AndrdoiManifest.xml
//

const int32_t MIN_PLAYERS = 1;
const int32_t MAX_PLAYERS = 3;
const double GAME_DURATION = 20.0;

enum NEXT_PARTICIPANT {
  NEXT_PARTICIPANT_AUTOMATCH = -1,
  NEXT_PARTICIPANT_NONE = -2,
};

struct PLAYER_STATUS {
  int32_t score;
  bool finished;
};
/*
 * Engine class of the sample
 */
struct android_app;
class Engine : public gpg::IRealTimeEventListener {
 public:
  // GPG-related methods
  void InitGooglePlayGameServices();
  void InviteFriend();
  void ShowRoomInbox();

  void InitializeGame();
  void PlayGame();
  void LeaveGame();
  void QuickMatch();

  void BroadcastScore(bool bFinal);

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

  // IRealTimeEventListener members
  virtual void OnRoomStatusChanged(gpg::RealTimeRoom const &room);

  virtual void OnParticipantStatusChanged(
      gpg::RealTimeRoom const &room,
      gpg::MultiplayerParticipant const &participant);

  virtual void OnDataReceived(
      gpg::RealTimeRoom const &room,
      gpg::MultiplayerParticipant const &from_participant,
      std::vector<uint8_t> data, bool is_reliable);

  // We are not using these callbacks below
  // because the app just waits for the room to become active,
  // no need to inspect individual changes
  virtual void OnConnectedSetChanged(gpg::RealTimeRoom const &room) {}

  virtual void OnP2PConnected(gpg::RealTimeRoom const &room,
                              gpg::MultiplayerParticipant const &participant) {}
  virtual void OnP2PDisconnected(
      gpg::RealTimeRoom const &room,
      gpg::MultiplayerParticipant const &participant) {}

 private:
  // Callbacks for GPG authentication.
  void OnAuthActionStarted(gpg::AuthOperation op);
  void OnAuthActionFinished(gpg::AuthOperation op, gpg::AuthStatus status);

  void UpdateScore();
  bool UpdateTime();

  void EnableUI(bool enable);
  void InitUI();
  void TransformPosition(ndk_helper::Vec2 &vec);
  void UpdateFPS(float fFPS);
  void SetParameters(jui_helper::JUIButton *button, jui_helper::JUIView *below);

  std::unique_ptr<gpg::GameServices> service_;  // gpg service instance
  std::unordered_map<std::string, PLAYER_STATUS> players_score_;  // hashmap to
                                                                  // keep
                                                                  // tracking of
                                                                  // player
                                                                  // scores
  gpg::RealTimeRoom room_;  // room status. This variable is updated each time
                            // the room status is updated
                            // in OnRoomStatusChanged()
  int32_t score_counter_;   // Score counter of local player
  bool playing_;            // Am I playing a game?
  std::string self_id_;     // Local player's ID
  double start_time_;       // Game start time

  // synchronization primitive to synchronize
  // UIThread, Timer thread and gpg callback thread
  mutable std::mutex mutex_;

  // Renderer of a teapot
  TeapotRenderer renderer_;

  // GLContext instance
  ndk_helper::GLContext *gl_context_;

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

  // JUI dialog-related UI stuff here
  jui_helper::JUIDialog *dialog_;

  jui_helper::JUIButton *button_sign_in_;
  jui_helper::JUIButton *button_quick_match_;
  jui_helper::JUIButton *button_invite_;
  jui_helper::JUIButton *button_matches_;

  jui_helper::JUITextView *status_text_;

  jui_helper::JUITextView *time_text_;
  jui_helper::JUITextView *my_score_text_;
  jui_helper::JUITextView *scores_text_;
  jui_helper::JUIButton *button_play_;
};

#endif  // TBMPSLEKETONNATIVIACTIVITY_H_
