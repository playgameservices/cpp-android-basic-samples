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
    .SetOnQuestCompleted([this](gpg::Quest quest){
      LOGI("Completed a quest!!");
      CompleteQuest(quest);
    })
    .Create(platform_configuration);
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
 * Claim quest milestone
 */
void Engine::ClaimMilestone(gpg::QuestMilestone milestone)
{
  EnableUI(false);
  service_->Quests().ClaimMilestone(milestone, [this,milestone](gpg::QuestManager::ClaimMilestoneResponse milestoneResponse){
    if( gpg::IsSuccess(milestoneResponse.status) )
    {
      LOGI("Claiming reward");
      auto begin = milestone.CompletionRewardData().begin();
      auto end = milestone.CompletionRewardData().end();
      std::string reward = "You got " + std::string(begin, end);
      ndk_helper::JNIHelper::GetInstance()->RunOnUiThread([reward]() {
        jui_helper::JUIToast toast(reward.c_str());
        toast.Show();
      });
    }
    EnableUI(true);
  });
}

/*
 * Show quest UI
 */
void Engine::ShowQuestUI() {
  EnableUI(false);
  service_->Quests().ShowAllUI([this](
      gpg::QuestManager::QuestUIResponse const & response) {
    if (IsSuccess(response.status)) {
      if (response.accepted_quest.Valid())
      {
        LOGI("Accepted a quest");
        EnableUI(true);
      }
      if (response.milestone_to_claim.Valid())
      {
        LOGI("Claimed a milestone");
        ClaimMilestone(response.milestone_to_claim);
      }
    } else {
      LOGI("Invalid response status");
      EnableUI(true);
    }
  });
}

/*
 * Show toast when completing quest
 */
void Engine::CompleteQuest(gpg::Quest quest)
{
  ndk_helper::JNIHelper::GetInstance()->RunOnUiThread([quest]() {
    std::string str = std::string("Quest completed:");
    str += quest.Name();
    jui_helper::JUIToast toast(str.c_str());
    toast.Show();
  });
}

/*
 * Show current event status
 */
void Engine::ShowEventStatus()
{
  EnableUI(false);
  service_->Events().FetchAll([this](gpg::EventManager::FetchAllResponse const &response)
                              {
    if (IsSuccess(response.status))
    {
      ndk_helper::JNIHelper::GetInstance()->RunOnUiThread([this,response]() {
        LOGI("Showing event status");
        if (dialog_)
          delete dialog_;

        dialog_ = new jui_helper::JUIDialog(app_->activity);

        std::ostringstream str;
        auto begin = response.data.begin();
        auto end = response.data.end();
        while( begin != end)
        {
          str <<begin->second.Name() << ": " << begin->second.Count() << "\n";
          begin++;
        }

        // Setting up message
        jui_helper::JUITextView* text = new jui_helper::JUITextView(
            str.str().c_str());

        text->AddRule(jui_helper::LAYOUT_PARAMETER_ALIGN_PARENT_TOP,
                              jui_helper::LAYOUT_PARAMETER_TRUE);
        text->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                              jui_helper::LAYOUT_PARAMETER_TRUE);
        text->SetAttribute("TextSize", jui_helper::ATTRIBUTE_UNIT_SP, 18.f);

        // OK Button
        jui_helper::JUIButton *button = new jui_helper::JUIButton("OK");
        button->SetCallback(
            [this](jui_helper::JUIView * view, const int32_t message) {
          switch (message) {
          case jui_helper::JUICALLBACK_BUTTON_UP: {
            dialog_->Close();
          }
          }
        });
        button->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                        jui_helper::LAYOUT_PARAMETER_TRUE);
        button->AddRule(jui_helper::LAYOUT_PARAMETER_BELOW, text);

        dialog_->SetCallback(
            jui_helper::JUICALLBACK_DIALOG_CANCELLED,
            [this](jui_helper::JUIDialog * dialog, const int32_t message) {
          LOGI("Dialog cancelled");
          dialog_->Close();
          EnableUI(true);
        });
        dialog_->SetCallback(
            jui_helper::JUICALLBACK_DIALOG_DISMISSED,
            [this](jui_helper::JUIDialog * dialog, const int32_t message) {
          LOGI("Dialog dismissed");
          dialog_->Close();
          EnableUI(true);
        });

        dialog_->SetAttribute("Title", "Event status");
        dialog_->AddView(text);
        dialog_->AddView(button);
        dialog_->Show();
        LOGI("Showing dialog");
      });

    }
    else
      EnableUI(true);
  });
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
  // Setup Buttons
  //
  status_text_ = new jui_helper::JUITextView(
      service_->IsAuthorized() ? "Signed In." : "Signed Out.");

  status_text_->AddRule(jui_helper::LAYOUT_PARAMETER_ALIGN_PARENT_BOTTOM,
                        jui_helper::LAYOUT_PARAMETER_TRUE);
  status_text_->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                        jui_helper::LAYOUT_PARAMETER_TRUE);

  // Sign in button.
  button_sign_in_ = new jui_helper::JUIButton(
      service_->IsAuthorized() ? "Sign Out" : "Sign In");
  button_sign_in_->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                           jui_helper::LAYOUT_PARAMETER_TRUE);
  button_sign_in_->AddRule(jui_helper::LAYOUT_PARAMETER_ABOVE, status_text_);
  button_sign_in_->SetCallback(
      [this](jui_helper::JUIView * view, const int32_t message) {
    if (message == jui_helper::JUICALLBACK_BUTTON_UP) {
      if (service_->IsAuthorized()) {
        service_->SignOut();
      } else {
        service_->StartAuthorizationUI();
      }
    }
  });
  button_sign_in_->SetAttribute("MinimumWidth", buttonWidth);
  button_sign_in_->SetMargins(0, 50, 0, 0);

  button_quests_ = new jui_helper::JUIButton("Show quests");
  button_quests_->AddRule(jui_helper::LAYOUT_PARAMETER_ABOVE, button_sign_in_);
  button_quests_->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                           jui_helper::LAYOUT_PARAMETER_TRUE);
  button_quests_->SetAttribute("MinimumWidth", buttonWidth);
  button_quests_->SetCallback(
      [this](jui_helper::JUIView * view, const int32_t message) {
    if (message == jui_helper::JUICALLBACK_BUTTON_UP) {
      ShowQuestUI();
    }
  });

  button_events_ = new jui_helper::JUIButton("Show event status");
  button_events_->AddRule(jui_helper::LAYOUT_PARAMETER_ABOVE,
                          button_quests_);
  button_events_->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                          jui_helper::LAYOUT_PARAMETER_TRUE);
  button_events_->SetAttribute("MinimumWidth", buttonWidth);
  button_events_->SetCallback(
      [this](jui_helper::JUIView * view, const int32_t message) {
    if (message == jui_helper::JUICALLBACK_BUTTON_UP) {
      ShowEventStatus();
    }
  });
  button_events_->SetMargins(0, 50, 0, 0);

  button_green_ = new jui_helper::JUIButton("Green event");
  button_green_->AddRule(jui_helper::LAYOUT_PARAMETER_ABOVE,
                         button_events_);
  button_green_->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                               jui_helper::LAYOUT_PARAMETER_TRUE);
  button_green_->SetAttribute("MinimumWidth", buttonWidth);
  button_green_->SetCallback(
      [this](jui_helper::JUIView * view, const int32_t message) {
    if (message == jui_helper::JUICALLBACK_BUTTON_UP) {
      std::string id = ndk_helper::JNIHelper::GetInstance()->GetStringResource("event_green");
      if( id == "" || id == "ReplaceMe")
      {
        LOGI("Invalid Event ID!, please check res/values/ids.xml");
        return;
      }
      service_->Events().Increment(id.c_str());
      ndk_helper::JNIHelper::GetInstance()->RunOnUiThread([]() {
        jui_helper::JUIToast toast("Got Green event");
        toast.Show();
      });
    }
  });

  button_yellow_ = new jui_helper::JUIButton("Yellow event");
  button_yellow_->AddRule(jui_helper::LAYOUT_PARAMETER_ABOVE,
                          button_green_);
  button_yellow_->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                               jui_helper::LAYOUT_PARAMETER_TRUE);
  button_yellow_->SetAttribute("MinimumWidth", buttonWidth);
  button_yellow_->SetCallback(
      [this](jui_helper::JUIView * view, const int32_t message) {
    if (message == jui_helper::JUICALLBACK_BUTTON_UP) {
      std::string id = ndk_helper::JNIHelper::GetInstance()->GetStringResource("event_yellow");
      if( id == "" || id == "ReplaceMe")
      {
        LOGI("Invalid Event ID!, please check res/values/ids.xml");
        return;
      }
      service_->Events().Increment(id.c_str());
      ndk_helper::JNIHelper::GetInstance()->RunOnUiThread([]() {
        jui_helper::JUIToast toast("Got Yellow event");
        toast.Show();
      });
    }
  });

  button_blue_ = new jui_helper::JUIButton("Blue event");
  button_blue_->AddRule(jui_helper::LAYOUT_PARAMETER_ABOVE,
                        button_yellow_);
  button_blue_->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                               jui_helper::LAYOUT_PARAMETER_TRUE);
  button_blue_->SetAttribute("MinimumWidth", buttonWidth);
  button_blue_->SetCallback(
      [this](jui_helper::JUIView * view, const int32_t message) {
    if (message == jui_helper::JUICALLBACK_BUTTON_UP) {
      std::string id = ndk_helper::JNIHelper::GetInstance()->GetStringResource("event_blue");
      if( id == "" || id == "ReplaceMe")
      {
        LOGI("Invalid Event ID!, please check res/values/ids.xml");
        return;
      }
      service_->Events().Increment(id.c_str());
      ndk_helper::JNIHelper::GetInstance()->RunOnUiThread([]() {
        jui_helper::JUIToast toast("Got Blue event");
        toast.Show();
      });
    }
  });

  button_red_ = new jui_helper::JUIButton("Red event");
  button_red_->AddRule(jui_helper::LAYOUT_PARAMETER_ABOVE,
                       button_blue_);
  button_red_->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                               jui_helper::LAYOUT_PARAMETER_TRUE);
  button_red_->SetAttribute("MinimumWidth", buttonWidth);
  button_red_->SetCallback(
      [this](jui_helper::JUIView * view, const int32_t message) {
    if (message == jui_helper::JUICALLBACK_BUTTON_UP) {
      std::string id = ndk_helper::JNIHelper::GetInstance()->GetStringResource("event_red");
      if( id == "" || id == "ReplaceMe")
      {
        LOGI("Invalid Event ID!, please check res/values/ids.xml");
        return;
      }
      service_->Events().Increment(id.c_str());
      ndk_helper::JNIHelper::GetInstance()->RunOnUiThread([]() {
        jui_helper::JUIToast toast("Got Red event");
        toast.Show();
      });
    }
  });

  //Add to screen
  jui_helper::JUIWindow::GetInstance()->AddView(button_red_);
  jui_helper::JUIWindow::GetInstance()->AddView(button_blue_);
  jui_helper::JUIWindow::GetInstance()->AddView(button_yellow_);
  jui_helper::JUIWindow::GetInstance()->AddView(button_green_);

  jui_helper::JUIWindow::GetInstance()->AddView(button_events_);
  jui_helper::JUIWindow::GetInstance()->AddView(button_quests_);
  jui_helper::JUIWindow::GetInstance()->AddView(button_sign_in_);
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
    button_blue_->SetAttribute("Enabled", b);
    button_red_->SetAttribute("Enabled", b);
    button_green_->SetAttribute("Enabled", b);
    button_yellow_->SetAttribute("Enabled", b);
    button_events_->SetAttribute("Enabled", b);
    button_quests_->SetAttribute("Enabled", b);
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
Java_com_google_example_games_tq_TrivialQuestNativeActivity_nativeOnActivityResult(
    JNIEnv *env, jobject thiz, jobject activity, jint requestCode,
    jint resultCode, jobject data) {
  gpg::AndroidSupport::OnActivityResult(env, activity, requestCode, resultCode,
                                        data);
}

JNIEXPORT void
Java_com_google_example_games_tq_TrivialQuestNativeActivity_nativeOnActivityCreated(
    JNIEnv *env, jobject thiz, jobject activity, jobject saved_instance_state) {
  gpg::AndroidSupport::OnActivityCreated(env, activity, saved_instance_state);
}

JNIEXPORT void
Java_com_google_example_games_tq_TrivialQuestNativeActivity_nativeOnActivityDestroyed(
    JNIEnv *env, jobject thiz, jobject activity) {
  gpg::AndroidSupport::OnActivityDestroyed(env, activity);
}

JNIEXPORT void
Java_com_google_example_games_tq_TrivialQuestNativeActivity_nativeOnActivityPaused(
    JNIEnv *env, jobject thiz, jobject activity) {
  gpg::AndroidSupport::OnActivityPaused(env, activity);
}

JNIEXPORT void
Java_com_google_example_games_tq_TrivialQuestNativeActivity_nativeOnActivityResumed(
    JNIEnv *env, jobject thiz, jobject activity) {
  gpg::AndroidSupport::OnActivityResumed(env, activity);
}

JNIEXPORT void
Java_com_google_example_games_tq_TrivialQuestNativeActivity_nativeOnActivitySaveInstanceState(
    JNIEnv *env, jobject thiz, jobject activity, jobject out_state) {
  gpg::AndroidSupport::OnActivitySaveInstanceState(env, activity, out_state);
}

JNIEXPORT void
Java_com_google_example_games_tq_TrivialQuestNativeActivity_nativeOnActivityStarted(
    JNIEnv *env, jobject thiz, jobject activity) {
  gpg::AndroidSupport::OnActivityStarted(env, activity);
}

JNIEXPORT void
Java_com_google_example_games_tq_TrivialQuestNativeActivity_nativeOnActivityStopped(
    JNIEnv *env, jobject thiz, jobject activity) {
  gpg::AndroidSupport::OnActivityStopped(env, activity);
}

}
