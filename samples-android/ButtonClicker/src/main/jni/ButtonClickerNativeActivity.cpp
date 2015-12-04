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

/*
 * This file demonstrates,
 * - How to use RTMP features in C++ code with gpg native client, including
 *   - Sign in to gpg service
 *   - How to handle RTMP callbacks
 *   - How to initiate RTMP matchs via several ways (QuickMatch, Invitation etc)
 *   - How to send reliable/unreliable packets to peers
 * - Setup management UI and game UI
 */

/*
 * Include files
 */
#include "ButtonClickerNativeActivity.h"

/*
 * Initialize GooglePlayGameServices via gpg::GameServices::Builder
 * In the build, it's setting several callbacks such as auth status changes,
 * receiving invitations etc.
 */
void Engine::InitGooglePlayGameServices() {
  if (service_ != nullptr) {
    return;
  }

  gpg::AndroidInitialization::android_main(app_);

  // Game Services have not been initialized, create a new Game Services.
  gpg::AndroidPlatformConfiguration platform_configuration;
  platform_configuration.SetActivity(app_->activity->clazz);

  gpg::GameServices::Builder builder;
  service_ =
      builder.SetOnAuthActionStarted([this](gpg::AuthOperation op) {
                                       // This callback is invoked when auth
                                       // action started
                                       // While auth action is going on, disable
                                       // auth and related UI
                                       OnAuthActionStarted(op);
                                     })
          .SetOnAuthActionFinished([this](gpg::AuthOperation op,
                                          gpg::AuthStatus status) {
             // This callback is invoked when auth action finished
             // Check status code and update UI to signed-in state
             OnAuthActionFinished(op, status);
           })
          .SetOnMultiplayerInvitationEvent([this](
               gpg::MultiplayerEvent event, std::string match_id,
               gpg::MultiplayerInvitation invitation) {
             // Invoked when invitation has been received
             // It can be received from the Play Game app, a notification, or
             // live while the app is running.
             // (e.g. Though PlayGam app, while the app is running)
             LOGI("MultiplayerInvitationEvent callback");

             if (event ==
                 gpg::TurnBasedMultiplayerEvent::UPDATED_FROM_APP_LAUNCH) {

               // In this case, an invitation has been accepted already
               // in notification or in Play game app
               gpg::RealTimeMultiplayerManager::RealTimeRoomResponse result =
                   service_->RealTimeMultiplayer().AcceptInvitationBlocking(
                       invitation, this);
               if (gpg::IsSuccess(result.status)) {
                 room_ = result.room;
                 service_->RealTimeMultiplayer().ShowWaitingRoomUI(
                     room_, MIN_PLAYERS,
                     [this](gpg::RealTimeMultiplayerManager::
                                WaitingRoomUIResponse const &waitResult) {
                       EnableUI(true);
                       if (gpg::IsSuccess(waitResult.status)) {
                         PlayGame();
                       }
                     });
               } else {
                 LeaveGame();
               }
             } else {
               // Otherwise, show default inbox and let players to accept an
               // invitation
               ShowRoomInbox();
             }
           })
          .Create(platform_configuration);
}

/*
 * Callback: Authentication action started
 *
 * gpg::AuthOperation op : SIGN_IN = 1, SIGN_OUT = 2
 *
 */
void Engine::OnAuthActionStarted(gpg::AuthOperation op) {
  if(!initialized_resources_) {
    return;
  }

  ndk_helper::JNIHelper::GetInstance()->RunOnUiThread([this, op]() {
    EnableUI(false);
    if (op == gpg::AuthOperation::SIGN_IN) {
      status_text_->SetAttribute("Text", "Signing In...");
    } else {
      status_text_->SetAttribute("Text", "Signing Out...");
    }
  });
}

/*
 * Callback: Authentication action finishes
 *
 * gpg::AuthOperation op : SIGN_IN = 1, SIGN_OUT = 2
 *
 */
void Engine::OnAuthActionFinished(gpg::AuthOperation op,
                                  gpg::AuthStatus status) {
  if (gpg::IsSuccess(status)) {
    service_->Players().FetchSelf([this](
        gpg::PlayerManager::FetchSelfResponse const &response) {
      if (gpg::IsSuccess(response.status)) {
        self_id_ = response.data.Id();
      }
    });
  }

  if(!initialized_resources_) {
    return;
  }

  ndk_helper::JNIHelper::GetInstance()->RunOnUiThread([this, status]() {
    EnableUI(true);
    button_sign_in_->SetAttribute(
        "Text", gpg::IsSuccess(status) ? "Sign Out" : "Sign In");

    status_text_->SetAttribute(
        "Text", gpg::IsSuccess(status) ? "Signed In" : "Signed Out");
  });
}

/*
 * Show room inbox
 */
void Engine::ShowRoomInbox() {
  service_->RealTimeMultiplayer().ShowRoomInboxUI([this](
      gpg::RealTimeMultiplayerManager::RoomInboxUIResponse const &response) {
    if (gpg::IsSuccess(response.status)) {
      gpg::RealTimeMultiplayerManager::RealTimeRoomResponse result =
          service_->RealTimeMultiplayer().AcceptInvitationBlocking(
              response.invitation, this);
      if (gpg::IsSuccess(result.status)) {
        room_ = result.room;
        service_->RealTimeMultiplayer().ShowWaitingRoomUI(
            room_, MIN_PLAYERS,
            [this](
                gpg::RealTimeMultiplayerManager::WaitingRoomUIResponse const &
                    waitResult) {
              EnableUI(true);
              if (gpg::IsSuccess(waitResult.status)) {
                PlayGame();
              }
            });
      } else
        EnableUI(true);  // Go back to original state for retry
    } else {
      LOGI("Invalid response status");
      EnableUI(true);  // Go back to original state for retry
    }
  });
  EnableUI(false);
}

/*
 * Quick match
 * - Create a match with minimal setting and play the game
 */
void Engine::QuickMatch() {
  gpg::RealTimeRoomConfig config =
      gpg::RealTimeRoomConfig::Builder()
          .SetMinimumAutomatchingPlayers(MIN_PLAYERS)
          .SetMaximumAutomatchingPlayers(MAX_PLAYERS)
          .Create();

  service_->RealTimeMultiplayer().CreateRealTimeRoom(
      config, this,
      [this](gpg::RealTimeMultiplayerManager::RealTimeRoomResponse const &
                 response) {
        LOGI("created a room %d", response.status);
        if (gpg::IsSuccess(response.status)) {
          room_ = response.room;
          service_->RealTimeMultiplayer().ShowWaitingRoomUI(
              room_, MIN_PLAYERS,
              [this](
                  gpg::RealTimeMultiplayerManager::WaitingRoomUIResponse const &
                      waitResult) {
                EnableUI(true);
                if (gpg::IsSuccess(waitResult.status)) {
                  PlayGame();
                }
              });
        } else
          EnableUI(true);  // Go back to original state for retry
      });
  EnableUI(false);
}

/*
 * Invite friends
 * - Show Player Select UI via ShowPlayerSelectUI,
 * - When the UI is finished, create match and show game UI
 */
void Engine::InviteFriend() {
  service_->RealTimeMultiplayer().ShowPlayerSelectUI(
      MIN_PLAYERS, MAX_PLAYERS, true,
      [this](gpg::RealTimeMultiplayerManager::PlayerSelectUIResponse const &
                 response) {
        LOGI("inviting friends %d", response.status);
        if (gpg::IsSuccess(response.status)) {
          // Create room
          gpg::RealTimeRoomConfig config =
              gpg::RealTimeRoomConfig::Builder()
                  .PopulateFromPlayerSelectUIResponse(response)
                  .Create();

          auto roomResponse =
              service_->RealTimeMultiplayer().CreateRealTimeRoomBlocking(config,
                                                                         this);
          if (gpg::IsSuccess(roomResponse.status)) {
            room_ = roomResponse.room;
            service_->RealTimeMultiplayer().ShowWaitingRoomUI(
                room_, MIN_PLAYERS,
                [this](gpg::RealTimeMultiplayerManager::
                           WaitingRoomUIResponse const &waitResult) {
                  EnableUI(true);
                  if (gpg::IsSuccess(waitResult.status)) {
                    PlayGame();
                  }
                });
          } else
            EnableUI(true);  // Go back to original state for retry
        } else
          EnableUI(true);  // Go back to original state for retry
      });
  EnableUI(false);
}

/*
 * Broadcast my score to peers
 */
void Engine::BroadcastScore(bool bFinal) {
  std::vector<uint8_t> v;
  if (!bFinal) {
    v.push_back('U');
    v.push_back(static_cast<uint8_t>(score_counter_));
    service_->RealTimeMultiplayer().SendUnreliableMessageToOthers(room_, v);
  } else {
    v.push_back('F');
    v.push_back(static_cast<uint8_t>(score_counter_));

    const std::vector<gpg::MultiplayerParticipant> participants =
        room_.Participants();
    for (gpg::MultiplayerParticipant participant : participants) {
      service_->RealTimeMultiplayer().SendReliableMessage(
          room_, participant, v, [](gpg::MultiplayerStatus const &) {});
    }
  }
}

/*
 * Got message from peers
 * room : The room which from_participant is in.
 * from_participant : The participant who sent the data.
 * data : The data which was recieved.
 * is_reliable : Whether the data was sent using the unreliable or
 *                    reliable mechanism.
 * In this app, packet format is defined as:
 * 1 byte: indicate score type 'F': final score 'U' updating score
 * 1 byte: score
 */
void Engine::OnDataReceived(gpg::RealTimeRoom const &room,
                            gpg::MultiplayerParticipant const &from_participant,
                            std::vector<uint8_t> data, bool is_reliable) {
  if (data[0] == 'F' && is_reliable) {
    // Got final score
    players_score_[from_participant.Id()].score = data[1];
    players_score_[from_participant.Id()].finished = true;
    LOGI("Got final data from Dispname:%s ID:%s",
         from_participant.DisplayName().c_str(), from_participant.Id().c_str());
  } else if (data[0] == 'U' && !is_reliable) {
    // Got current score
    uint8_t score = players_score_[from_participant.Id()].score;
    players_score_[from_participant.Id()].score = std::max(score, data[1]);
    LOGI("Got data from Dispname:%s ID:%s",
         from_participant.DisplayName().c_str(), from_participant.Id().c_str());
  }
  UpdateScore();
}

/*
 * Room status change callback
 */
void Engine::OnRoomStatusChanged(gpg::RealTimeRoom const &room) {
  room_ = room;
}

/*
 * Invoked when participant status changed
 */
void Engine::OnParticipantStatusChanged(
    gpg::RealTimeRoom const &room,
    gpg::MultiplayerParticipant const &participant) {

  // Update participant status
  LOGI("Participant %s status changed: %d", participant.Id().c_str(),
       participant.Status());

  if (participant.Status() != gpg::ParticipantStatus::JOINED) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (players_score_.find(participant.Id()) != players_score_.end()) {
        players_score_[participant.Id()].finished = true;
      }
    }
    UpdateScore();
  }
}

/*
 * Play games UI that is in your turn
 */
void Engine::PlayGame() {
  ndk_helper::JNIHelper::GetInstance()->RunOnUiThread([this]() {
    LOGI("Playing match");
    if (dialog_) delete dialog_;

    // Start game
    InitializeGame();

    //
    // Using jui_helper, a support library, to create and bind gameplay buttons.
    //
    dialog_ = new jui_helper::JUIDialog(app_->activity);

    // Setting up labels
    time_text_ = new jui_helper::JUITextView("0:00");
    time_text_->AddRule(jui_helper::LAYOUT_PARAMETER_ALIGN_PARENT_TOP,
                        jui_helper::LAYOUT_PARAMETER_TRUE);
    time_text_->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                        jui_helper::LAYOUT_PARAMETER_TRUE);
    time_text_->SetAttribute("TextSize", jui_helper::ATTRIBUTE_UNIT_SP, 18.f);
    time_text_->SetAttribute("Padding", 10, 10, 10, 10);

    my_score_text_ = new jui_helper::JUITextView("000");
    my_score_text_->AddRule(jui_helper::LAYOUT_PARAMETER_BELOW, time_text_);
    my_score_text_->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                            jui_helper::LAYOUT_PARAMETER_TRUE);
    my_score_text_->SetAttribute("TextSize", jui_helper::ATTRIBUTE_UNIT_SP,
                                 24.f);
    my_score_text_->SetAttribute("Padding", 10, 10, 10, 10);

    // Take Turn Button
    button_play_ = new jui_helper::JUIButton("Click me!");
    button_play_->SetCallback([this](jui_helper::JUIView *view,
                                     const int32_t message) {
      switch (message) {
        case jui_helper::JUICALLBACK_BUTTON_UP: {
          if (!playing_) return;
          score_counter_++;
          UpdateScore();
          UpdateTime();

          // Broadcast my score to others via unreliable protocol
          BroadcastScore(false);
        }
      }
    });

    button_play_->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                          jui_helper::LAYOUT_PARAMETER_TRUE);
    button_play_->AddRule(jui_helper::LAYOUT_PARAMETER_BELOW, my_score_text_);

    const int32_t labelWidth = 600;
    const int32_t labelHeight = 300;
    scores_text_ = new jui_helper::JUITextView("0:00");
    scores_text_->AddRule(jui_helper::LAYOUT_PARAMETER_BELOW, button_play_);
    scores_text_->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                          jui_helper::LAYOUT_PARAMETER_TRUE);
    scores_text_->SetAttribute("TextSize", jui_helper::ATTRIBUTE_UNIT_SP, 18.f);
    scores_text_->SetAttribute("MinimumWidth", labelWidth);
    scores_text_->SetAttribute("MinimumHeight", labelHeight);
    scores_text_->SetAttribute("Padding", 10, 10, 10, 10);

    UpdateScore();

    dialog_->AddView(my_score_text_);
    dialog_->AddView(button_play_);
    dialog_->AddView(time_text_);
    dialog_->AddView(scores_text_);

    dialog_->SetAttribute("Title", "Click the button");
    dialog_->SetCallback(
        jui_helper::JUICALLBACK_DIALOG_DISMISSED,
        [this](jui_helper::JUIDialog *dialog, const int32_t message) {
          LOGI("Dialog dismissed");
          LeaveGame();
          dialog_ = nullptr;
        });

    dialog_->Show();
    LOGI("Showing dialog");

    //
    // Invoke time counter periodically
    //
    std::thread([this]() {
                  ndk_helper::JNIHelper &helper =
                      *ndk_helper::JNIHelper::GetInstance();
                  helper.AttachCurrentThread();
                  while (UpdateTime()) {
                    std::chrono::milliseconds d(100);
                    std::this_thread::sleep_for(d);
                  }
                  // Broadcast my score to others via reliable protocol
                  BroadcastScore(true);

                  UpdateScore();
                  helper.DetachCurrentThread();
                }).detach();
  });
}

/*
 * Update game UI when some player's score is updated
 */
void Engine::UpdateScore() {
  // Lock mutex since this one can be called from multiple thread,
  // gpg callback tread and UI callback thread
  std::lock_guard<std::mutex> lock(mutex_);

  int32_t SIZE = 64;
  char str[SIZE];
  snprintf(str, SIZE, "%03d", score_counter_);
  std::string str_myscore(str);

  snprintf(str, SIZE, "My score: %03d %s\n", score_counter_,
           playing_ ? "" : "*");
  std::string allstr(str);

  // Append other player
  std::vector<gpg::MultiplayerParticipant> participants = room_.Participants();
  for (gpg::MultiplayerParticipant participant : participants) {
    LOGI("Participant Dispname:%s ID:%s", participant.DisplayName().c_str(),
         participant.Id().c_str());
    if (participant.HasPlayer())
      LOGI("self:%s PlayerID:%s", self_id_.c_str(),
           participant.Player().Id().c_str());

    if (participant.HasPlayer() &&
        participant.Player().Id().compare(self_id_) == 0)
      continue;  // Skip local player

    int32_t score = 0;
    bool finished = false;
    if (players_score_.find(participant.Id()) != players_score_.end()) {
      score = players_score_[participant.Id()].score;
      finished = players_score_[participant.Id()].finished;
    }

    LOGI("Status %d", participant.Status());
    snprintf(str, SIZE, "%s: %03d %s\n", participant.DisplayName().c_str(),
             score, finished ? "*" : "");
    allstr += str;
  }

  // Update game UI, UI update needs to be performed in UI thread
  ndk_helper::JNIHelper::GetInstance()
      ->RunOnUiThread([this, str_myscore, allstr]() {
          my_score_text_->SetAttribute(
              "Text", const_cast<const char *>(str_myscore.c_str()));
          scores_text_->SetAttribute("Text",
                                     const_cast<const char *>(allstr.c_str()));
        });
}

/*
 * Update game timer and game UI
 */
bool Engine::UpdateTime() {
  // UpdateTime() is invoked from other thread asynchrnously
  // So need to Lock mutex
  std::lock_guard<std::mutex> lock(mutex_);

  if (!playing_) return false;

  double current_time = monitor_.GetCurrentTime();
  current_time -= start_time_ + 1.0;

  if (current_time >= GAME_DURATION) {
    // finish game
    playing_ = false;
    current_time = GAME_DURATION;
    ndk_helper::JNIHelper::GetInstance()->RunOnUiThread([this, current_time]() {
      button_play_->SetAttribute("Enabled", false);
    });
  }

  // Update game UI, UI update needs to be performed in UI thread
  ndk_helper::JNIHelper::GetInstance()->RunOnUiThread([this, current_time]() {
    // LOGI("Updating time %f", current_time);
    int32_t SIZE = 64;
    char str[SIZE];
    snprintf(str, SIZE, "0:%02.0f", GAME_DURATION - current_time);
    time_text_->SetAttribute("Text", const_cast<const char *>(str));
  });

  return true;
}

/*
 * Initialize game state
 */
void Engine::InitializeGame() {
  // Lock mutex
  std::lock_guard<std::mutex> lock(mutex_);

  playing_ = true;

  start_time_ = monitor_.GetCurrentTime();
  score_counter_ = 0;
  players_score_.clear();
}

/*
 * Leave game
 */
void Engine::LeaveGame() {
  // Lock mutex
  std::lock_guard<std::mutex> lock(mutex_);

  service_->RealTimeMultiplayer().LeaveRoom(
      room_, [](gpg::ResponseStatus const &status) {});

  LOGI("Game is over");
  playing_ = false;
}

/*
 * Initialize game management UI,
 * invoking jui_helper functions to create java UIs
 */
void Engine::InitUI() {
  // The window initialization
  jui_helper::JUIWindow::Init(app_->activity, JUIHELPER_CLASS_NAME);

  // Show toast with app label
  ndk_helper::JNIHelper::GetInstance()->RunOnUiThread([]() {
    if(NULL == jui_helper::JUIWindow::GetInstance()->GetContext()) {
        return;
    }
    jui_helper::JUIToast toast(
        ndk_helper::JNIHelper::GetInstance()->GetAppLabel());
    toast.Show();
  });

  //
  // Using jui_helper, a support library, to create and bind game management
  // UIs.
  //

  //
  // Buttons
  //
  // Sign in button.
  button_sign_in_ = new jui_helper::JUIButton(
      service_->IsAuthorized() ? "Sign Out" : "Sign In");
  SetParameters(button_sign_in_, nullptr);
  button_sign_in_->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                           jui_helper::LAYOUT_PARAMETER_TRUE);
  button_sign_in_->SetCallback([this](jui_helper::JUIView *view,
                                      const int32_t message) {
    if (message == jui_helper::JUICALLBACK_BUTTON_UP) {
      if (service_->IsAuthorized()) {
        service_->SignOut();
      } else {
        service_->StartAuthorizationUI();
      }
    }
  });
  button_sign_in_->SetMargins(0, 0, 0, 50);

  jui_helper::JUIWindow::GetInstance()->AddView(button_sign_in_);

  button_quick_match_ = new jui_helper::JUIButton("Quick match");
  SetParameters(button_quick_match_, button_sign_in_);
  button_quick_match_->SetCallback([this](jui_helper::JUIView *view,
                                          const int32_t message) {
    if (message == jui_helper::JUICALLBACK_BUTTON_UP) {
      QuickMatch();
    }
  });
  jui_helper::JUIWindow::GetInstance()->AddView(button_quick_match_);

  button_invite_ = new jui_helper::JUIButton("Invite Friends!");
  SetParameters(button_invite_, button_quick_match_);
  button_invite_->SetCallback([this](jui_helper::JUIView *view,
                                     const int32_t message) {
    if (message == jui_helper::JUICALLBACK_BUTTON_UP) {
      InviteFriend();
    }
  });
  jui_helper::JUIWindow::GetInstance()->AddView(button_invite_);

  button_matches_ = new jui_helper::JUIButton("All my invitations");
  SetParameters(button_matches_, button_invite_);
  button_matches_->SetCallback([this](jui_helper::JUIView *view,
                                      const int32_t message) {
    if (message == jui_helper::JUICALLBACK_BUTTON_UP) {
      ShowRoomInbox();
    }
  });
  jui_helper::JUIWindow::GetInstance()->AddView(button_matches_);

  status_text_ = new jui_helper::JUITextView(
      service_->IsAuthorized() ? "Signed In." : "Signed Out.");
  status_text_->AddRule(jui_helper::LAYOUT_PARAMETER_ALIGN_PARENT_BOTTOM,
                        jui_helper::LAYOUT_PARAMETER_TRUE);
  status_text_->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                        jui_helper::LAYOUT_PARAMETER_TRUE);
  jui_helper::JUIWindow::GetInstance()->AddView(status_text_);

  textViewFPS_ = new jui_helper::JUITextView("0.0FPS");
  textViewFPS_->SetAttribute("Gravity", jui_helper::ATTRIBUTE_GRAVITY_LEFT);
  textViewFPS_->SetAttribute("TextColor", 0xffffffff);
  textViewFPS_->SetAttribute("TextSize", jui_helper::ATTRIBUTE_UNIT_SP, 18.f);
  textViewFPS_->SetAttribute("Padding", 10, 10, 10, 10);
  jui_helper::JUIWindow::GetInstance()->AddView(textViewFPS_);

  // Init play game services
  InitGooglePlayGameServices();

  return;
}

/*
 * Set common parameters to button
 */
void Engine::SetParameters(jui_helper::JUIButton *button,
                           jui_helper::JUIView *below) {
  const int32_t buttonWidth = 600;
  button->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                  jui_helper::LAYOUT_PARAMETER_TRUE);
  button->SetAttribute("MinimumWidth", buttonWidth);

  if (below != nullptr)
    button->AddRule(jui_helper::LAYOUT_PARAMETER_BELOW, below);
}

/*
 * Enable/Disable management UI
 */
void Engine::EnableUI(bool enable) {
  ndk_helper::JNIHelper::GetInstance()->RunOnUiThread([this, enable]() {
    button_sign_in_->SetAttribute("Enabled", enable);

    bool b = enable;
    if (service_->IsAuthorized() == false) b = false;
    button_quick_match_->SetAttribute("Enabled", b);
    button_invite_->SetAttribute("Enabled", b);
    button_matches_->SetAttribute("Enabled", b);
  });
}

/*
 * JNI functions those manage activity lifecycle
 */
extern "C" {
/*
 * These callbacks are necessary to work Google Play Game Services UIs properly
 *
 * For apps which target Android 2.3 or 3.x devices (API Version prior to 14),
 * Play Game Services has no way to automatically receive Activity lifecycle
 * callbacks. In these cases, Play Game Services relies on the owning Activity
 * to notify it of lifecycle events. Any Activity which owns a GameServices
 * object should call the AndroidSupport::* functions from within their own
 * lifecycle callback functions. The arguments in these functions match those
 * provided by Android, so no additional processing is necessary.
 */
JNIEXPORT void
Java_com_google_example_games_ButtonClicker_ButtonClickerNativeActivity_nativeOnActivityResult(
    JNIEnv *env, jobject thiz, jobject activity, jint requestCode,
    jint resultCode, jobject data) {
  gpg::AndroidSupport::OnActivityResult(env, activity, requestCode, resultCode,
                                        data);
}

JNIEXPORT void
Java_com_google_example_games_ButtonClicker_ButtonClickerNativeActivity_nativeOnActivityCreated(
    JNIEnv *env, jobject thiz, jobject activity, jobject saved_instance_state) {
  gpg::AndroidSupport::OnActivityCreated(env, activity, saved_instance_state);
}

JNIEXPORT void
Java_com_google_example_games_ButtonClicker_ButtonClickerNativeActivity_nativeOnActivityDestroyed(
    JNIEnv *env, jobject thiz, jobject activity) {
  gpg::AndroidSupport::OnActivityDestroyed(env, activity);
}

JNIEXPORT void
Java_com_google_example_games_ButtonClicker_ButtonClickerNativeActivity_nativeOnActivityPaused(
    JNIEnv *env, jobject thiz, jobject activity) {
  gpg::AndroidSupport::OnActivityPaused(env, activity);
}

JNIEXPORT void
Java_com_google_example_games_ButtonClicker_ButtonClickerNativeActivity_nativeOnActivityResumed(
    JNIEnv *env, jobject thiz, jobject activity) {
  gpg::AndroidSupport::OnActivityResumed(env, activity);
}

JNIEXPORT void
Java_com_google_example_games_ButtonClicker_ButtonClickerNativeActivity_nativeOnActivitySaveInstanceState(
    JNIEnv *env, jobject thiz, jobject activity, jobject out_state) {
  gpg::AndroidSupport::OnActivitySaveInstanceState(env, activity, out_state);
}

JNIEXPORT void
Java_com_google_example_games_ButtonClicker_ButtonClickerNativeActivity_nativeOnActivityStarted(
    JNIEnv *env, jobject thiz, jobject activity) {
  gpg::AndroidSupport::OnActivityStarted(env, activity);
}

JNIEXPORT void
Java_com_google_example_games_ButtonClicker_ButtonClickerNativeActivity_nativeOnActivityStopped(
    JNIEnv *env, jobject thiz, jobject activity) {
  gpg::AndroidSupport::OnActivityStopped(env, activity);
}
}
