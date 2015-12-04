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
#include "CollectAllTheStarsNativeActivity.h"

/*
 * Key for JSON object
 */
const char *JSON_VERSION = "1.1";
const char *JSON_KEY_VERSION = "version";
const char *JSON_KEY_LEVELS = "levels";

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
        .EnableSnapshots(). //Enable Snapshot
        Create(platform_configuration);
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
 * Show snapshot select UI
 */
void Engine::ShowSnapshotSelectUI() {
  service_->Snapshots().ShowSelectUIOperation(
      ALLOW_CREATE_SNAPSHOT_INUI,
      ALLOW_DELETE_SNAPSHOT_INUI,
      MAX_SNAPSHOTS,
      SNAPSHOT_UI_TITLE,
      [this](gpg::SnapshotManager::SnapshotSelectUIResponse const & response) {
    LOGI("Snapshot selected");
    if (IsSuccess(response.status)) {
      if (response.data.Valid()) {
        LOGI("Description: %s", response.data.Description().c_str());
        LOGI("FileName %s", response.data.FileName().c_str());

        //Opening the snapshot data
        current_snapshot_ = response.data;
        LoadFromSnapshot();
      } else {
        LOGI("Creating new snapshot");
        SaveSnapshot();
      }
    } else {
      LOGI("ShowSelectUIOperation returns an error %d", response.status);
      EnableUI(true);
    }
  });
}

/*
 * Save Snapshot
 */
void Engine::SaveSnapshot() {
  EnableUI(false);
  std::string fileName;
  if (current_snapshot_.Valid() == false) {
    fileName = GenerateSaveFileName();
    LOGI("Creating new snapshot %s", fileName.c_str());
  } else {
    fileName = current_snapshot_.FileName();
  }

  service_->Snapshots().Open(
      fileName,
      gpg::SnapshotConflictPolicy::MANUAL,
      [this](gpg::SnapshotManager::OpenResponse const & response) {
    LOGI("Opened file");
    if (IsSuccess(response.status)) {
      gpg::SnapshotMetadata metadata = response.data;
      if (response.conflict_id != "") {
        //Conflict detected
        LOGI("Snapshot conflict detected going to resolve that");
        bool b = ResolveConflicts(response, 0);
        if (!b) {
          LOGI("Failed resolving conflicts");
          EnableUI(true);
          return;
        }
      }

      //Create image to represent the snapshot
      //Reading png from asset for now
      std::vector<uint8_t> pngData;
      if (!ndk_helper::JNIHelper::GetInstance()->ReadFile("gps.png",
                                                          &pngData)) {
        LOGI("Can not open a file:%s", "gps.png");
        EnableUI(true);
        return;
      }

      gpg::SnapshotMetadataChange::Builder builder;
      gpg::SnapshotMetadataChange metadata_change =
          builder.SetDescription("CollectAllTheStar savedata ")
              .SetCoverImageFromPngData(pngData).Create();
      // Save the snapshot.
      gpg::SnapshotManager::CommitResponse commitResponse =
          service_->Snapshots()
              .CommitBlocking(metadata, metadata_change, SetupSnapshotData());

      if (IsSuccess(commitResponse.status)) {
        LOGI("Saved game");
      } else {
        LOGI("Saved game failed error: %d", commitResponse.status);
      }
    }
    EnableUI(true);
  });
}

bool
Engine::ResolveConflicts(gpg::SnapshotManager::OpenResponse const &openResponse,
                         const int32_t retry) {
  if (openResponse.conflict_id == "") {
    LOGI("No conflict detected");
    return true;
  }
  //
  // Note: Expecting ResolveConflicts() is called within callback, we are using
  // blocking read here
  //
  gpg::SnapshotManager &manager = service_->Snapshots();
  gpg::SnapshotManager::OpenResponse responseBase =
      manager.OpenBlocking(openResponse.conflict_original.FileName(),
                           gpg::SnapshotConflictPolicy::MANUAL);
  gpg::SnapshotManager::ReadResponse responseReadBase =
      manager.ReadBlocking(responseBase.data);
  ParseSnapshotData(responseReadBase.data);

  // Temporary store data,
  int32_t temp_scores[NUM_GAME_WORLD][NUM_GAME_STAGES]; //960 bytes
  for (int32_t i = 0; i < NUM_GAME_WORLD; ++i)
    for (int32_t j = 0; j < NUM_GAME_STAGES; ++j) {
      temp_scores[i][j] = scores_[i][j];
    }

  gpg::SnapshotManager::OpenResponse responseRemote =
      manager.OpenBlocking(openResponse.conflict_unmerged.FileName(),
                           gpg::SnapshotConflictPolicy::MANUAL);
  gpg::SnapshotManager::ReadResponse responseReadRemote =
      manager.ReadBlocking(responseRemote.data);
  ParseSnapshotData(responseReadRemote.data);

  // Merging them
  for (int32_t i = 0; i < NUM_GAME_WORLD; ++i)
    for (int32_t j = 0; j < NUM_GAME_STAGES; ++j) {
      if (temp_scores[i][j] > scores_[i][j])
        scores_[i][j] = temp_scores[i][j];
    }

  //Create image to represent the snapshot
  //Reading png from asset for now
  std::vector<uint8_t> pngData;
  if (!ndk_helper::JNIHelper::GetInstance()->ReadFile("gps.png", &pngData)) {
    LOGI("Can not open a file:%s", "gps.png");
    return false;
  }

  gpg::SnapshotMetadataChange::Builder builder;
  gpg::SnapshotMetadataChange metadata_change =
      builder.SetDescription("CollectAllTheStar savedata ")
          .SetCoverImageFromPngData(pngData).Create();

  //Resolve conflict
  gpg::SnapshotManager::CommitResponse commitResponse =
      manager.ResolveConflictBlocking(openResponse.data, metadata_change,
                                      openResponse.conflict_id);
  if (IsSuccess(commitResponse.status)) {
    LOGI("Conflict resolution succeeded");
  } else {
    LOGI("Conflict resolution failed error: %d", commitResponse.status);
  }

  //Try to re-open,
  gpg::SnapshotManager::OpenResponse retryResponse =
      manager.OpenBlocking(current_snapshot_.FileName(),
                         gpg::SnapshotConflictPolicy::MANUAL);
  if (IsSuccess(retryResponse.status)) {
    if (retryResponse.conflict_id != "") {
      if (retry > MAX_RETRY)
        return false;
      LOGI("Need conflict resolution again..");
      return ResolveConflicts(retryResponse, retry + 1);
    }
  }

  SaveSnapshot();  //Currently, we need to save data again.
                          //We will have convenient API to resolve conflicts and save data
                          //In upcoming release
  return true;
}

/*
 * Load snapshot data
 */
void Engine::LoadFromSnapshot() {
  if (!current_snapshot_.Valid()) {
    LOGI("Snapshot is not valid!");
    EnableUI(true);
    return;
  }
  LOGI("Opening file");
  service_->Snapshots()
      .Open(current_snapshot_.FileName(),
            gpg::SnapshotConflictPolicy::MANUAL,
            [this](gpg::SnapshotManager::OpenResponse const & response) {
    LOGI("Opened file");
    if (IsSuccess(response.status)) {
      //Do need conflict resolution?
      if (response.data.Valid() == false) {
        if (response.conflict_id != "") {
          LOGI("Need conflict resolution");
          bool b = ResolveConflicts(response, 0);
          if (!b) {
            LOGI("Failed resolving conflicts");
            EnableUI(true);
            return;
          }
        } else {
          EnableUI(true);
          return;
        }
      }

      LOGI("Reading file");
      gpg::SnapshotManager::ReadResponse responseRead =
          service_->Snapshots().ReadBlocking(response.data);
      if (IsSuccess(responseRead.status)) {
        LOGI("Parsing data");
        ParseSnapshotData(responseRead.data);
        ndk_helper::JNIHelper::GetInstance()->RunOnUiThread([this]() {
          UpdateGameUI();
        });
      }
    }

    EnableUI(true);
  });
}

/*
 * Parse JSON match data
 */
void Engine::ParseSnapshotData(const std::vector<uint8_t> &data) {
  LOGI("Parsing snapshot data");
  if (data.size() == 0) {
    LOGI("Game data not found");
    return;
  }

  std::string str(data.begin(), data.end());
  LOGI("Data: %s", str.c_str());

  Json::Value root;
  Json::Reader reader;
  bool parsingSuccessful = reader.parse(str, root);
  if (!parsingSuccessful) {
    LOGI("Data parse error");
    return;
  }

  std::string version = root[JSON_KEY_VERSION].asString();
  if (version != JSON_VERSION) {
    LOGI("Data version mismatch");
    return;
  }

  //Reset existing data
  for (int32_t i = 0; i < NUM_GAME_WORLD; ++i)
    for (int32_t j = 0; j < NUM_GAME_STAGES; ++j)
      scores_[i][j] = 0;

  //Level data
  const Json::Value levels = root[JSON_KEY_LEVELS];
  Json::ValueIterator it = levels.begin();
  Json::ValueIterator itEnd = levels.end();
  while (it != itEnd) {
    std::string stage = it.key().asString();
    int32_t score = (*it).asInt();
    LOGI("stage:%s score:%d", stage.c_str(), score);

    if (map_scores_.find(stage) != map_scores_.end())
      *map_scores_[stage] = score;
    ++it;
  }
}

/*
 * Create JSON data to store
 */
std::vector<uint8_t> Engine::SetupSnapshotData() {
  Json::StyledWriter writer;
  Json::Value root;

  root[JSON_KEY_VERSION] = JSON_VERSION;
  std::string strKey = std::string(JSON_KEY_LEVELS);
  for (int32_t i = 0; i < NUM_GAME_WORLD; ++i)
    for (int32_t j = 0; j < NUM_GAME_STAGES; ++j) {
      if (scores_[i][j]) {
        std::ostringstream str;
        str << i + 1 << "-" << j + 1;
        root[strKey][str.str()] = scores_[i][j];
      }
    }

  std::string source = writer.write(root);

  std::vector<uint8_t> v;
  auto it = source.begin();
  auto end = source.end();
  while (it != end) {
    uint8_t i = *it++;
    v.push_back(i);
  }

  LOGI("Created Game Data: size: %d", v.size());
  return v;
}

/*
 * Generate a unique filename
 */
std::string Engine::GenerateSaveFileName() {
  // Generates random filename
  timeval Time;
  gettimeofday(&Time, NULL);

  std::ostringstream str;
  str << "snapshotTemp-" << Time.tv_usec;

  return str.str();
}

/*
 * Update gameUI
 */
void Engine::UpdateGameUI() {
  for (int32_t i = 0; i < NUM_GAME_STAGES; ++i) {
    std::ostringstream str;
    str << current_world_ + 1 << "-" << i + 1 << "\n";
    int32_t j = 0;
    for (; j < scores_[current_world_][i]; ++j)
      str << "★";
    for (; j < MAX_STARS; ++j)
      str << "☆";

    button_games_[i]->SetAttribute("Text", str.str().c_str());
  }

  std::ostringstream str;
  str << "World " << current_world_ + 1;
  labelWorld_->SetAttribute("Text", str.str().c_str());
}

jui_helper::JUIButton *Engine::CreateButton(int32_t i) {
  jui_helper::JUIButton *button =
      new jui_helper::JUIButton("1-1\n☆☆☆☆☆");
  button->SetAttribute("TextSize", jui_helper::ATTRIBUTE_UNIT_SP, 12.f);
  button->SetCallback(
      [this, i](jui_helper::JUIView * view, const int32_t message) {
    if (message == jui_helper::JUICALLBACK_BUTTON_UP) {
      //Update game data
      scores_[current_world_][i] =
          (scores_[current_world_][i] + 1) % (MAX_STARS + 1);
      UpdateGameUI();
    }
  });
  return button;
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
  // Setting up game UI
  int32_t index = 0;
  jui_helper::JUILinearLayout *masterLayout = new jui_helper::JUILinearLayout();
  masterLayout->SetAttribute("Orientation",
                             jui_helper::LAYOUT_ORIENTATION_VERTICAL);
  masterLayout->SetAttribute("Gravity",
                             jui_helper::ATTRIBUTE_GRAVITY_CENTER_VERTICAL);
  masterLayout->SetLayoutParams(jui_helper::ATTRIBUTE_SIZE_MATCH_PARENT,
                                jui_helper::ATTRIBUTE_SIZE_WRAP_CONTENT);

  //Select stage
  jui_helper::JUILinearLayout *layout = new jui_helper::JUILinearLayout();
  layout->SetLayoutParams(jui_helper::ATTRIBUTE_SIZE_MATCH_PARENT,
                          jui_helper::ATTRIBUTE_SIZE_WRAP_CONTENT);
  layout->SetMargins(0, 50, 0, 50);
  layout->SetAttribute("Orientation",
                       jui_helper::LAYOUT_ORIENTATION_HORIZONTAL);
  layout->SetAttribute("Gravity", jui_helper::ATTRIBUTE_GRAVITY_CENTER);
  buttonLeft_ = new jui_helper::JUIButton("<<");
  buttonRight_ = new jui_helper::JUIButton(">>");
  labelWorld_ = new jui_helper::JUITextView("Stage 1");
  buttonLeft_->SetCallback(
      [this](jui_helper::JUIView * view, const int32_t message) {
    if (message == jui_helper::JUICALLBACK_BUTTON_UP) {
      //Change game world
      if (current_world_ > 0) {
        current_world_--;
        UpdateGameUI();
      }
    }
  });
  buttonRight_->SetCallback(
      [this](jui_helper::JUIView * view, const int32_t message) {
    if (message == jui_helper::JUICALLBACK_BUTTON_UP) {
      //Change game world
      if (current_world_ < NUM_GAME_WORLD - 1) {
        current_world_++;
        UpdateGameUI();
      }
    }
  });

  layout->AddView(buttonLeft_);
  layout->AddView(labelWorld_);
  layout->AddView(buttonRight_);
  masterLayout->AddView(layout);

  for (int32_t i = 0; i < 4; ++i) {
    button_games_[index + 0] = CreateButton(index + 0);
    button_games_[index + 1] = CreateButton(index + 1);
    button_games_[index + 2] = CreateButton(index + 2);

    //Center element
    button_games_[index + 1]
        ->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                  jui_helper::LAYOUT_PARAMETER_TRUE);
    button_games_[index + 1]->SetMargins(50, 50, 50, 50);

    jui_helper::JUILinearLayout *layout = new jui_helper::JUILinearLayout();
    layout->SetLayoutParams(jui_helper::ATTRIBUTE_SIZE_MATCH_PARENT,
                            jui_helper::ATTRIBUTE_SIZE_WRAP_CONTENT);
    layout->SetMargins(0, 50, 0, 50);
    layout->SetAttribute("Orientation",
                         jui_helper::LAYOUT_ORIENTATION_HORIZONTAL);
    layout->SetAttribute("Gravity", jui_helper::ATTRIBUTE_GRAVITY_CENTER);
    layout->AddView(button_games_[index + 0]);
    layout->AddView(button_games_[index + 1]);
    layout->AddView(button_games_[index + 2]);
    masterLayout->AddView(layout);

    index += 3;
  }
  jui_helper::JUIWindow::GetInstance()->AddView(masterLayout);

  status_text_ = new jui_helper::JUITextView(
      service_->IsAuthorized() ? "Signed In." : "Signed Out.");

  status_text_->AddRule(jui_helper::LAYOUT_PARAMETER_ALIGN_PARENT_BOTTOM,
                        jui_helper::LAYOUT_PARAMETER_TRUE);
  status_text_->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                        jui_helper::LAYOUT_PARAMETER_TRUE);
  jui_helper::JUIWindow::GetInstance()->AddView(status_text_);

  // Sign in button.
  button_sign_in_ = new jui_helper::JUIButton(
      service_->IsAuthorized() ? "Sign Out" : "Sign In");
  button_sign_in_->AddRule(jui_helper::LAYOUT_PARAMETER_ABOVE, status_text_);
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
  button_sign_in_->SetMargins(0, 50, 0, 0);

  // Select button.
  button_select_ = new jui_helper::JUIButton("Select");
  button_select_->AddRule(jui_helper::LAYOUT_PARAMETER_ABOVE, button_sign_in_);
  button_select_->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                          jui_helper::LAYOUT_PARAMETER_TRUE);
  button_select_->SetCallback(
      [this](jui_helper::JUIView * view, const int32_t message) {
    LOGI("button_select_ click: %d", message);
    if (message == jui_helper::JUICALLBACK_BUTTON_UP) {
      EnableUI(false);
      ShowSnapshotSelectUI();
    }
  });
  button_select_->SetAttribute("MinimumWidth", buttonWidth);

  // Save button.
  button_save_ = new jui_helper::JUIButton("Save");
  button_save_->AddRule(jui_helper::LAYOUT_PARAMETER_ABOVE, button_select_);
  button_save_->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                        jui_helper::LAYOUT_PARAMETER_TRUE);
  button_save_->SetCallback(
      [this](jui_helper::JUIView * view, const int32_t message) {
    LOGI("button_save_ click: %d", message);
    if (message == jui_helper::JUICALLBACK_BUTTON_UP) {
      //Save snapshot
      SaveSnapshot();
    }
  });
  button_save_->SetAttribute("MinimumWidth", buttonWidth);

  // Load button.
  button_load_ = new jui_helper::JUIButton("Load");
  button_load_->AddRule(jui_helper::LAYOUT_PARAMETER_ABOVE, button_save_);
  button_load_->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                        jui_helper::LAYOUT_PARAMETER_TRUE);
  button_load_->SetCallback(
      [this](jui_helper::JUIView * view, const int32_t message) {
    LOGI("button_load_ click: %d", message);
    if (message == jui_helper::JUICALLBACK_BUTTON_UP) {
      EnableUI(false);
      LoadFromSnapshot();
    }
  });
  button_load_->SetAttribute("MinimumWidth", buttonWidth);

  jui_helper::JUIWindow::GetInstance()->AddView(button_sign_in_);
  jui_helper::JUIWindow::GetInstance()->AddView(button_select_);
  jui_helper::JUIWindow::GetInstance()->AddView(button_save_);
  jui_helper::JUIWindow::GetInstance()->AddView(button_load_);

  textViewFPS_ = new jui_helper::JUITextView("0.0FPS");
  textViewFPS_->SetAttribute("Gravity", jui_helper::ATTRIBUTE_GRAVITY_LEFT);
  textViewFPS_->SetAttribute("TextColor", 0xffffffff);
  textViewFPS_->SetAttribute("TextSize", jui_helper::ATTRIBUTE_UNIT_SP, 18.f);
  textViewFPS_->SetAttribute("Padding", 10, 10, 10, 10);
  jui_helper::JUIWindow::GetInstance()->AddView(textViewFPS_);

  // Progress bar
  progressBar_ = new jui_helper::JUIProgressBar();
  progressBar_->AddRule(jui_helper::LAYOUT_PARAMETER_CENTER_IN_PARENT,
                        jui_helper::LAYOUT_PARAMETER_TRUE);
  jui_helper::JUIWindow::GetInstance()->AddView(progressBar_);
  progressBar_->SetAttribute("Hidden", true);

  //Update game ui
  UpdateGameUI();

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
    button_select_->SetAttribute("Enabled", b);
    if (current_snapshot_.Valid())
      button_load_->SetAttribute("Enabled", b);
    else
      button_load_->SetAttribute("Enabled", false);
    button_save_->SetAttribute("Enabled", b);

    //Enable/Disable Game UI
    buttonLeft_->SetAttribute("Enabled", enable);
    buttonRight_->SetAttribute("Enabled", enable);
    for (int32_t i = 0; i < NUM_GAME_STAGES; ++i) {
      button_games_[i]->SetAttribute("Enabled", enable);
    }

    // Show progress bar?
    if (enable)
      progressBar_->SetAttribute(
          "Visibility", jui_helper::ViewVisibility::VIEW_VISIVILITY_GONE);
    else
      progressBar_->SetAttribute(
          "Visibility", jui_helper::ViewVisibility::VIEW_VISIVILITY_VISIBLE);
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
Java_com_google_example_games_cats_CollectAllTheStarsNativeActivity_nativeOnActivityResult(
    JNIEnv *env, jobject thiz, jobject activity, jint requestCode,
    jint resultCode, jobject data) {
  gpg::AndroidSupport::OnActivityResult(env, activity, requestCode, resultCode,
                                        data);
}

JNIEXPORT void
Java_com_google_example_games_cats_CollectAllTheStarsNativeActivity_nativeOnActivityCreated(
    JNIEnv *env, jobject thiz, jobject activity, jobject saved_instance_state) {
  gpg::AndroidSupport::OnActivityCreated(env, activity, saved_instance_state);
}

JNIEXPORT void
Java_com_google_example_games_cats_CollectAllTheStarsNativeActivity_nativeOnActivityDestroyed(
    JNIEnv *env, jobject thiz, jobject activity) {
  gpg::AndroidSupport::OnActivityDestroyed(env, activity);
}

JNIEXPORT void
Java_com_google_example_games_cats_CollectAllTheStarsNativeActivity_nativeOnActivityPaused(
    JNIEnv *env, jobject thiz, jobject activity) {
  gpg::AndroidSupport::OnActivityPaused(env, activity);
}

JNIEXPORT void
Java_com_google_example_games_cats_CollectAllTheStarsNativeActivity_nativeOnActivityResumed(
    JNIEnv *env, jobject thiz, jobject activity) {
  gpg::AndroidSupport::OnActivityResumed(env, activity);
}

JNIEXPORT void
Java_com_google_example_games_cats_CollectAllTheStarsNativeActivity_nativeOnActivitySaveInstanceState(
    JNIEnv *env, jobject thiz, jobject activity, jobject out_state) {
  gpg::AndroidSupport::OnActivitySaveInstanceState(env, activity, out_state);
}

JNIEXPORT void
Java_com_google_example_games_cats_CollectAllTheStarsNativeActivity_nativeOnActivityStarted(
    JNIEnv *env, jobject thiz, jobject activity) {
  gpg::AndroidSupport::OnActivityStarted(env, activity);
}

JNIEXPORT void
Java_com_google_example_games_cats_CollectAllTheStarsNativeActivity_nativeOnActivityStopped(
    JNIEnv *env, jobject thiz, jobject activity) {
  gpg::AndroidSupport::OnActivityStopped(env, activity);
}

}
