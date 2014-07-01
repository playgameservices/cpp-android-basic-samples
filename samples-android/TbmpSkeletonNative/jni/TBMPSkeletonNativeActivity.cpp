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
#include "TBMPSkeletonNativeActivity.h"

/*
 * Key for JSON object
 */
const char *TurnCounterKey = "turnCounter";
const char *StringToPassKey = "data";

/*
 * Initialize GooglePlayGameServices
 */
void Engine::InitGooglePlayGameServices() {
  gpg::AndroidInitialization::android_main(app_);

  if (service_ == nullptr) {
    // Game Services have not been initialized, create a new Game Services.
    gpg::AndroidPlatformConfiguration platform_configuration;
    platform_configuration.SetActivity(app_->activity->clazz);

    gpg::GameServices::Builder builder;
    service_ = builder.SetOnAuthActionStarted([this](gpg::AuthOperation op) {
      OnAuthActionStarted(op);
    })
        .SetOnAuthActionFinished(
            [this](gpg::AuthOperation op, gpg::AuthStatus status) {
      OnAuthActionFinished(op, status);
    })
        .SetOnTurnBasedMatchEvent(
            [this](gpg::TurnBasedMultiplayerEvent event, std::string match_id,
                   gpg::TurnBasedMatch match) {
      LOGI("TurnBasedMultiplayerEvent callback");
      //Show default inbox
      ShowMatchInbox();
    })
        .SetOnMultiplayerInvitationEvent(
            [this](gpg::TurnBasedMultiplayerEvent event, std::string match_id,
                   gpg::MultiplayerInvitation invitation) {
      LOGI("MultiplayerInvitationEvent callback");
      //Show default inbox
      ShowMatchInbox();
    }).Create(platform_configuration);
  }
}

/*
 * Callback: Authentication started
 */
void Engine::OnAuthActionStarted(gpg::AuthOperation op) {
  ndk_helper::JNIHelper::GetInstance()->RunOnUiThread([this, op]() {
    EnableUI(false);
    authorizing_ = true;
    if (status_text_) {
      if (op == gpg::AuthOperation::SIGN_IN) {
        LOGI("Signing in to GPG");
        status_text_->SetAttribute("Text", "Signing In...");
      } else {
        LOGI("Signing out from GPG");
        status_text_->SetAttribute("Text", "Signing Out...");
      }
    }
  });
}

/*
 * Callback: Authentication finishes
 */
void Engine::OnAuthActionFinished(gpg::AuthOperation op,
                                  gpg::AuthStatus status) {
  ndk_helper::JNIHelper::GetInstance()->RunOnUiThread([this, status]() {
    EnableUI(true);
    authorizing_ = false;
    if (button_sign_in_) {
      button_sign_in_->SetAttribute(
          "Text", gpg::IsSuccess(status) ? "Sign Out" : "Sign In");
    }

    if (status_text_) {
      status_text_->SetAttribute(
          "Text",
          gpg::IsSuccess(status) ? "Signed In" : "Signed Out");
    }
  });
}

/*
 * Show match inbox
 */
void Engine::ShowMatchInbox() {
  service_->TurnBasedMultiplayer().ShowMatchInboxUI([this](
      gpg::TurnBasedMultiplayerManager::MatchInboxUIResponse const & response) {
    if (gpg::IsSuccess(response.status)) {
      //Show game based on the user's selection
      switch (response.match.Status()) {
      case gpg::MatchStatus::THEIR_TURN:
        //Manage match with dismiss, leave and cancel options
        ManageGame(response.match, true, true, false);
        break;
      case gpg::MatchStatus::MY_TURN:
        //Play selected game
        PlayGame(response.match);
        break;
      case gpg::MatchStatus::COMPLETED:
        //Manage match with dismiss, rematch options
        ManageGame(response.match, false, false, true);
        break;
      case gpg::MatchStatus::EXPIRED:
      default:
        //Manage match with dismiss option
        ManageGame(response.match, false, false, false);
        break;
      }
    } else {
      LOGI("Invalid response status");
    }
  });
}

/*
 * Quick match
 * - Create a match with minimal setting and play the game
 */
void Engine::QuickMatch() {
  gpg::TurnBasedMultiplayerManager &manager = service_->TurnBasedMultiplayer();
  gpg::TurnBasedMatchConfig config =
      gpg::TurnBasedMatchConfig::Builder().SetMinimumAutomatchingPlayers(1)
          .SetMaximumAutomatchingPlayers(2).Create();

  EnableUI(false);
  manager.CreateTurnBasedMatch(
      config,
      [this](gpg::TurnBasedMultiplayerManager::TurnBasedMatchResponse const &
             matchResponse) {
    EnableUI(true);
    if (gpg::IsSuccess(matchResponse.status)) {
      PlayGame(matchResponse.match);
    }
  });
}

/*
 * Invite friends
 * - Show Player Select UI via ShowPlayerSelectUI,
 * - When the UI is finished, create match and show game UI
 */
void Engine::InviteFriend() {
  service_->TurnBasedMultiplayer().ShowPlayerSelectUI(
      MIN_PLAYERS, MAX_PLAYERS, true,
      [this](gpg::TurnBasedMultiplayerManager::PlayerSelectUIResponse const &
             response) {
    LOGI("selected match %d", response.status);
    if (gpg::IsSuccess(response.status)) {
      // Create new match with the config
      gpg::TurnBasedMatchConfig config = gpg::TurnBasedMatchConfig::Builder()
          .SetMinimumAutomatchingPlayers(response.minimum_automatching_players)
          .SetMaximumAutomatchingPlayers(response.maximum_automatching_players)
          .AddAllPlayersToInvite(response.player_ids).Create();
      EnableUI(false);
      service_->TurnBasedMultiplayer().CreateTurnBasedMatch(
          config,
          [this](
              gpg::TurnBasedMultiplayerManager::TurnBasedMatchResponse const &
              matchResponse) {
        EnableUI(true);
        if (gpg::IsSuccess(matchResponse.status)) {
          PlayGame(matchResponse.match);
        }
      });
    }
  });
}

/*
 * Cancel current match
 */
void Engine::CancelMatch() {
  EnableUI(false);
  gpg::TurnBasedMultiplayerManager &manager = service_->TurnBasedMultiplayer();
  manager.CancelMatch(current_match_, [this](gpg::MultiplayerStatus status) {
    EnableUI(true);
    LOGI("Canceled the game");
  });
}

/*
 * Dismiss current match
 */
void Engine::DismissMatch() {
  gpg::TurnBasedMultiplayerManager &manager = service_->TurnBasedMultiplayer();
  manager.DismissMatch(current_match_);
}

/*
 * Rematch selected match
 */
void Engine::Rematch() {
  EnableUI(false);
  gpg::TurnBasedMultiplayerManager &manager = service_->TurnBasedMultiplayer();
  manager.Rematch(
      current_match_,
      [this](gpg::TurnBasedMultiplayerManager::TurnBasedMatchResponse
                 matchResponse) {
    LOGI("Remathing the game");
    EnableUI(true);
    if (gpg::IsSuccess(matchResponse.status)) {
      PlayGame(matchResponse.match);
    }
  });
}

/*
 * Leave current match
 * Invoking different APIs based on match state
 */
void Engine::LeaveMatch() {
  gpg::TurnBasedMultiplayerManager &manager = service_->TurnBasedMultiplayer();
  if (current_match_.Status() == gpg::MatchStatus::MY_TURN) {
      //Leave a game
    manager.LeaveMatchDuringMyTurn(current_match_,
                                   current_match_.SuggestedNextParticipant(),
                                   [this](gpg::MultiplayerStatus status) {
      LOGI("Left the game");
    });
  } else {
    manager.LeaveMatchDuringTheirTurn(current_match_,
                                      [this](gpg::MultiplayerStatus status) {
      LOGI("Left the game");
    });
  }
  return;
}

/*
 * Take turn based on playGame UI parameters
 */
void Engine::TakeTurn(const bool winning, const bool losing) {
  gpg::TurnBasedMultiplayerManager &manager = service_->TurnBasedMultiplayer();

  // When it is MY_TURN, localParticipant is always PendingParticipant().
  gpg::MultiplayerParticipant localParticipant =
      current_match_.PendingParticipant();

  LOGI("Taking my turn. local participant id:%s",
       localParticipant.Id().c_str());

  gpg::MultiplayerParticipant nextParticipant =
      current_match_.SuggestedNextParticipant();

  if (!nextParticipant.Valid()) {
    //Error case
    manager.DismissMatch(current_match_);
    return;
  }

  turn_counter_++;
  std::vector<uint8_t> match_data = SetupMatchData();

  //By default, passing through existing participatntResults
  gpg::ParticipantResults results = current_match_.ParticipantResults();

  if (winning) {
    //Create winning participants result
    results = current_match_.ParticipantResults()
        .WithResult(localParticipant.Id(), // local player ID
                    0,                     // placing
                    gpg::MatchResult::WIN  // status
                    );
  } else if (losing) {
    //Create losing participants result
    results = current_match_.ParticipantResults()
        .WithResult(localParticipant.Id(), // local player ID
                    0,                     // placing
                    gpg::MatchResult::LOSS // status
                    );
  }

    //Take normal turn
  manager.TakeMyTurn(
      current_match_, match_data, results, nextParticipant,
      [this](gpg::TurnBasedMultiplayerManager::TurnBasedMatchResponse const &
             response) {
    LOGI("Took turn");
  });
}

/*
 * Play games UI that is in your turn
 */
void Engine::PlayGame(gpg::TurnBasedMatch const &match) {
  current_match_ = match;
  ParseMatchData();

  ndk_helper::JNIHelper::GetInstance()->RunOnUiThread([this]() {
    LOGI("Playing match");
    if (dialog_)
      delete dialog_;

    dialog_ = new jui_helper::JUIDialog(app_->activity);

    // Setting up CheckBox
    checkBoxWinning_ = new jui_helper::JUICheckBox("Win the game");
    checkBoxLosing_ = new jui_helper::JUICheckBox("I have lost");
    checkBoxQuit_ = new jui_helper::JUICheckBox("Leave");

    // Setting up linear layout
    jui_helper::JUILinearLayout *layout = new jui_helper::JUILinearLayout();
    layout->SetLayoutParams(jui_helper::ATTRIBUTE_SIZE_MATCH_PARENT,
                            jui_helper::ATTRIBUTE_SIZE_WRAP_CONTENT);
    layout->SetMargins(0, 50, 0, 50);
    layout->SetAttribute("Orientation",
                         jui_helper::LAYOUT_ORIENTATION_VERTICAL);
    layout->AddView(checkBoxWinning_);
    layout->AddView(checkBoxLosing_);
    layout->AddView(checkBoxQuit_);

    // Take Turn Button
    jui_helper::JUIButton *button = new jui_helper::JUIButton("Take Turn");
    button->SetCallback(
        [this](jui_helper::JUIView * view, const int32_t message) {
      switch (message) {
      case jui_helper::JUICALLBACK_BUTTON_UP: {
        if (checkBoxQuit_->IsChecked())
          LeaveMatch();
        else
          TakeTurn(checkBoxWinning_->IsChecked(), checkBoxLosing_->IsChecked());
        dialog_->Close();
      }
      }
    });
    button->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                    jui_helper::LAYOUT_PARAMETER_TRUE);
    button->AddRule(jui_helper::LAYOUT_PARAMETER_BELOW, layout);

    jui_helper::JUIButton *buttonCancel = new jui_helper::JUIButton("Cancel");
    buttonCancel->SetCallback(
        [this](jui_helper::JUIView * view, const int32_t message) {
      switch (message) {
      case jui_helper::JUICALLBACK_BUTTON_UP: {
        CancelMatch();
        dialog_->Close();
      }
      }
    });
    buttonCancel->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                          jui_helper::LAYOUT_PARAMETER_TRUE);
    buttonCancel->AddRule(jui_helper::LAYOUT_PARAMETER_BELOW, button);

    dialog_->SetCallback(
        jui_helper::JUICALLBACK_DIALOG_CANCELLED,
        [this](jui_helper::JUIDialog * dialog, const int32_t message) {
      LOGI("Dialog cancelled");
      CancelMatch();
      dialog_->Close();
    });
    dialog_->SetCallback(
        jui_helper::JUICALLBACK_DIALOG_DISMISSED,
        [this](jui_helper::JUIDialog * dialog, const int32_t message) {
      LOGI("Dialog dismissed");
      dialog_->Close();
    });

    int32_t size = 64;
    char str[size];
    snprintf(str, size, "Turn %d", turn_counter_);
    dialog_->SetAttribute("Title", (const char *)str);
    dialog_->AddView(layout);
    dialog_->AddView(button);
    dialog_->Show();
    LOGI("Showing dialog");
  });
}

/*
 * Manage games UI when finished/their turn game is selected in match inbox UI
 */
void Engine::ManageGame(gpg::TurnBasedMatch const &match, const bool leave,
                        const bool cancel, const bool rematch) {
  current_match_ = match;
  ParseMatchData();

  ndk_helper::JNIHelper::GetInstance()->RunOnUiThread(
      [this, leave, cancel, rematch]() {
    LOGI("Managing match");
    if (dialog_)
      delete dialog_;
    dialog_ = new jui_helper::JUIDialog(app_->activity);

    jui_helper::JUIButton *currentButton;
    // Take Turn Button
    jui_helper::JUIButton *button = new jui_helper::JUIButton("Dismiss");
    button->SetCallback(
        [this](jui_helper::JUIView * view, const int32_t message) {
      switch (message) {
      case jui_helper::JUICALLBACK_BUTTON_UP: {
        DismissMatch();
        dialog_->Close();
      }
      }
    });
    button->AddRule(jui_helper::LAYOUT_PARAMETER_ALIGN_PARENT_TOP,
                    jui_helper::LAYOUT_PARAMETER_TRUE);
    button->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                    jui_helper::LAYOUT_PARAMETER_TRUE);
    dialog_->AddView(button);
    currentButton = button;

    if (leave) {
      LOGI("Adding leave button");
      // Leave Button
      jui_helper::JUIButton *button = new jui_helper::JUIButton("Leave");
      button->SetCallback(
          [this](jui_helper::JUIView * view, const int32_t message) {
        switch (message) {
        case jui_helper::JUICALLBACK_BUTTON_UP: {
          LeaveMatch();
          dialog_->Close();
        }
        }
      });
      button->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                      jui_helper::LAYOUT_PARAMETER_TRUE);
      button->AddRule(jui_helper::LAYOUT_PARAMETER_BELOW, currentButton);
      dialog_->AddView(button);
      currentButton = button;
    }

    if (cancel) {
      LOGI("Adding cancel button");
      // Leave Button
      jui_helper::JUIButton *button = new jui_helper::JUIButton("Cancel");
      button->SetCallback(
          [this](jui_helper::JUIView * view, const int32_t message) {
        switch (message) {
        case jui_helper::JUICALLBACK_BUTTON_UP: {
          CancelMatch();
          dialog_->Close();
        }
        }
      });
      button->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                      jui_helper::LAYOUT_PARAMETER_TRUE);
      button->AddRule(jui_helper::LAYOUT_PARAMETER_BELOW, currentButton);
      dialog_->AddView(button);
      currentButton = button;
    }

    if (rematch) {
      LOGI("Adding rematch button");
      // Leave Button
      jui_helper::JUIButton *button = new jui_helper::JUIButton("Rematch");
      button->SetCallback(
          [this](jui_helper::JUIView * view, const int32_t message) {
        switch (message) {
        case jui_helper::JUICALLBACK_BUTTON_UP: {
          Rematch();
          dialog_->Close();
        }
        }
      });
      button->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                      jui_helper::LAYOUT_PARAMETER_TRUE);
      button->AddRule(jui_helper::LAYOUT_PARAMETER_BELOW, currentButton);
      dialog_->AddView(button);
      currentButton = button;
    }

    dialog_->SetCallback(
        jui_helper::JUICALLBACK_DIALOG_CANCELLED,
        [this](jui_helper::JUIDialog * dialog, const int32_t message) {
      LOGI("Dialog cancelled");
      dialog_->Close();
    });
    dialog_->SetCallback(
        jui_helper::JUICALLBACK_DIALOG_DISMISSED,
        [this](jui_helper::JUIDialog * dialog, const int32_t message) {
      LOGI("Dialog dismissed");
      dialog_->Close();
    });

    int32_t size = 64;
    char str[size];
    snprintf(str, size, "Turn %d", turn_counter_);
    dialog_->SetAttribute("Title", (const char *)str);
    dialog_->Show();
    LOGI("Showing dialog");
  });
}

/*
 * Parse JSON match data
 */
void Engine::ParseMatchData() {
  LOGI("Parsing match data %d", current_match_.Data().size());
  turn_counter_ = 1;
  if (current_match_.HasData() == false && current_match_.Data().size() == 0) {
    LOGI("Game data not found");
    return;
  }

  //UTF16 to UTF8 conversion
  //Removing BOM for first 2 bytes
  std::u16string source;
  auto it = current_match_.Data().begin();
  auto end = current_match_.Data().end();
  //Skip BOM
  it++;
  it++;
  while (it != end) {
    uint16_t i = *it++;
    i |= (*it++) << 8;
    source.push_back(i);
  }

  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
  std::string str = convert.to_bytes(source);

  Json::Value root;
  Json::Reader reader;
  bool parsingSuccessful = reader.parse(str, root);
  if (!parsingSuccessful) {
    LOGI("Data parse error");
    return;
  }

  turn_counter_ = root[TurnCounterKey].asInt();
  LOGI("Got turn counter %d", turn_counter_);
}

/*
 * Create JSON data to store
 */
std::vector<uint8_t> Engine::SetupMatchData() {
  Json::StyledWriter writer;
  Json::Value root;
  root[TurnCounterKey] = turn_counter_;
  root[StringToPassKey] = "test";
  std::string source = writer.write(root);

  //Converting back from UTF8 to UTF16
  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
  std::u16string dest = convert.from_bytes(source);

  std::vector<uint8_t> v;
  v.push_back(0xff); //Adding BOM
  v.push_back(0xfe); //Adding BOM
  auto it = dest.begin();
  auto end = dest.end();
  while (it != end) {
    uint16_t i = *it++;
    v.push_back(i & 0xff);
    v.push_back(i >> 8);
  }

  LOGI("Created Game Data: size: %d", v.size());
  return v;
}

/*
 * Initialize main sample UI,
 * invoking jui_helper functions to create java UIs
 */
void Engine::InitUI() {
  const int32_t buttonWidth = 600;
    // Show toast with app label
  ndk_helper::JNIHelper::GetInstance()->RunOnUiThread([]() {
    jui_helper::JUIToast toast(
        ndk_helper::JNIHelper::GetInstance()->GetAppLabel());
    toast.Show();
  });

  // The window initialization
  jui_helper::JUIWindow::Init(app_->activity, JUIHELPER_CLASS_NAME);

  //
  // Buttons
  //
  // Sign in button.
  button_sign_in_ = new jui_helper::JUIButton(
      service_->IsAuthorized() ? "Sign Out" : "Sign In");
  button_sign_in_->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                           jui_helper::LAYOUT_PARAMETER_TRUE);
  button_sign_in_->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                           jui_helper::LAYOUT_PARAMETER_TRUE);
  button_sign_in_->SetCallback(
      [this](jui_helper::JUIView * view, const int32_t message) {
    LOGI("button_sign_in_ click: %d", message);
    if (message == jui_helper::JUICALLBACK_BUTTON_UP) {
      if (service_->IsAuthorized()) {
        service_->SignOut();
      } else {
        service_->StartAuthorizationUI();
      }
    }
  });
  button_sign_in_->SetAttribute("MinimumWidth", buttonWidth);
  button_sign_in_->SetMargins(0, 0, 0, 50);

  jui_helper::JUIWindow::GetInstance()->AddView(button_sign_in_);

  button_quick_match_ = new jui_helper::JUIButton("Quick match");
  button_quick_match_->AddRule(jui_helper::LAYOUT_PARAMETER_BELOW,
                               button_sign_in_);
  button_quick_match_->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                               jui_helper::LAYOUT_PARAMETER_TRUE);
  button_quick_match_->SetAttribute("MinimumWidth", buttonWidth);
  button_quick_match_->SetCallback(
      [this](jui_helper::JUIView * view, const int32_t message) {
    if (message == jui_helper::JUICALLBACK_BUTTON_UP) {
      QuickMatch();
    }
  });
  jui_helper::JUIWindow::GetInstance()->AddView(button_quick_match_);

  button_invite_ = new jui_helper::JUIButton("Invite Friends!");
  button_invite_->AddRule(jui_helper::LAYOUT_PARAMETER_BELOW,
                          button_quick_match_);
  button_invite_->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                          jui_helper::LAYOUT_PARAMETER_TRUE);
  button_invite_->SetAttribute("MinimumWidth", buttonWidth);
  button_invite_->SetCallback(
      [this](jui_helper::JUIView * view, const int32_t message) {
    if (message == jui_helper::JUICALLBACK_BUTTON_UP) {
      InviteFriend();
    }
  });
  jui_helper::JUIWindow::GetInstance()->AddView(button_invite_);

  button_matches_ = new jui_helper::JUIButton("All my matches");
  button_matches_->AddRule(jui_helper::LAYOUT_PARAMETER_BELOW, button_invite_);
  button_matches_->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                           jui_helper::LAYOUT_PARAMETER_TRUE);
  button_matches_->SetAttribute("MinimumWidth", buttonWidth);
  button_matches_->SetCallback(
      [this](jui_helper::JUIView * view, const int32_t message) {
    if (message == jui_helper::JUICALLBACK_BUTTON_UP) {
      ShowMatchInbox();
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

  if (authorizing_)
    EnableUI(false);
  return;
}

void Engine::EnableUI(bool enable) {
  LOGI("Updating UI:%d", enable);
  ndk_helper::JNIHelper::GetInstance()->RunOnUiThread([this, enable]() {
    button_sign_in_->SetAttribute("Enabled", enable);

    bool b = enable;
    if (service_->IsAuthorized() == false)
      b = false;
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
Java_com_google_example_games_tbmpskel_TBMPSkeletonNativeActivity_nativeOnActivityResult(
    JNIEnv *env, jobject thiz, jobject activity, jint requestCode,
    jint resultCode, jobject data) {
  gpg::AndroidSupport::OnActivityResult(env, activity, requestCode, resultCode,
                                        data);
}

JNIEXPORT void
Java_com_google_example_games_tbmpskel_TBMPSkeletonNativeActivity_nativeOnActivityCreated(
    JNIEnv *env, jobject thiz, jobject activity, jobject saved_instance_state) {
  gpg::AndroidSupport::OnActivityCreated(env, activity, saved_instance_state);
}

JNIEXPORT void
Java_com_google_example_games_tbmpskel_TBMPSkeletonNativeActivity_nativeOnActivityDestroyed(
    JNIEnv *env, jobject thiz, jobject activity) {
  gpg::AndroidSupport::OnActivityDestroyed(env, activity);
}

JNIEXPORT void
Java_com_google_example_games_tbmpskel_TBMPSkeletonNativeActivity_nativeOnActivityPaused(
    JNIEnv *env, jobject thiz, jobject activity) {
  gpg::AndroidSupport::OnActivityPaused(env, activity);
}

JNIEXPORT void
Java_com_google_example_games_tbmpskel_TBMPSkeletonNativeActivity_nativeOnActivityResumed(
    JNIEnv *env, jobject thiz, jobject activity) {
  gpg::AndroidSupport::OnActivityResumed(env, activity);
}

JNIEXPORT void
Java_com_google_example_games_tbmpskel_TBMPSkeletonNativeActivity_nativeOnActivitySaveInstanceState(
    JNIEnv *env, jobject thiz, jobject activity, jobject out_state) {
  gpg::AndroidSupport::OnActivitySaveInstanceState(env, activity, out_state);
}

JNIEXPORT void
Java_com_google_example_games_tbmpskel_TBMPSkeletonNativeActivity_nativeOnActivityStarted(
    JNIEnv *env, jobject thiz, jobject activity) {
  gpg::AndroidSupport::OnActivityStarted(env, activity);
}

JNIEXPORT void
Java_com_google_example_games_tbmpskel_TBMPSkeletonNativeActivity_nativeOnActivityStopped(
    JNIEnv *env, jobject thiz, jobject activity) {
  gpg::AndroidSupport::OnActivityStopped(env, activity);
}

}
