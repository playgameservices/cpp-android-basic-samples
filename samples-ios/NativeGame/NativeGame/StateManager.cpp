/* Copyright (c) 2014 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "StateManager.h"

#ifdef __APPLE__
//
//Logging for CoreFoundation
//
#include <CoreFoundation/CoreFoundation.h>
extern "C" void NSLog(CFStringRef format, ...);
const int32_t BUFFER_SIZE = 256;
#define LOGI(...) {char c[BUFFER_SIZE];\
    snprintf(c,BUFFER_SIZE,__VA_ARGS__);\
    CFStringRef str = CFStringCreateWithCString(kCFAllocatorDefault, c, kCFStringEncodingMacRoman);\
    NSLog(str);\
    CFRelease(str);\
    }
#else
#include "android/log.h"
#define DEBUG_TAG "TeapotNativeActivity"
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, DEBUG_TAG, __VA_ARGS__))

#endif

bool StateManager::isSignedIn = false;
std::unique_ptr<gpg::GameServices> StateManager::gameServices;

void OnAuthActionFinished(gpg::AuthOperation op, gpg::AuthStatus status) {
  LOGI("OnAuthActionFinished");
}

void OnAuthActionStarted(gpg::AuthOperation op) {
  LOGI("OnAuthActionStarted");
  switch ( op ) {
  case gpg::AuthOperation::SIGN_IN:
    LOGI("Signing In");
    break;
  case gpg::AuthOperation::SIGN_OUT:
    LOGI("Signing Out");
    break;
  }
}

gpg::GameServices *StateManager::GetGameServices() {
  return gameServices.get();
}

void StateManager::BeginUserInitiatedSignIn() {
  if (!gameServices->IsAuthorized()) {
    LOGI("StartAuthorizationUI");
    gameServices->StartAuthorizationUI();
  }
}

void StateManager::SignOut() {
  if (gameServices->IsAuthorized()) {
    LOGI("SignOut");
    gameServices->SignOut();
  }
}

void StateManager::UnlockAchievement(const char *achievementId) {
  if (gameServices->IsAuthorized()) {
    LOGI("Achievement unlocked");
    gameServices->Achievements().Unlock(achievementId);
  }
}

void StateManager::SubmitHighScore(const char *leaderboardId, uint64_t score) {
  if (gameServices->IsAuthorized()) {
    LOGI("High score submitted");
    gameServices->Leaderboards().SubmitScore(leaderboardId, score);
  }
}

void StateManager::ShowAchievements()
{
    if (gameServices->IsAuthorized()) {
        LOGI("Show achievement");
        gameServices->Achievements().ShowAllUI();
    }
}

void StateManager::ShowLeaderboard(const char *leaderboardId)
{
    if (gameServices->IsAuthorized()) {
        LOGI("Show achievement");
        gameServices->Leaderboards().ShowUI(leaderboardId);
    }    
}


void StateManager::InitServices(gpg::PlatformConfiguration &pc,gpg::GameServices::Builder::OnAuthActionFinishedCallback callback) {
  LOGI("Initializing Services");
  if (!gameServices) {
    LOGI("Uninitialized services, so creating");
    gameServices = gpg::GameServices::Builder()
      .SetLogging(gpg::DEFAULT_ON_LOG, gpg::LogLevel::VERBOSE)
      //   .SetOnAuthActionFinished(OnAuthActionFinished)
      //.SetOnAuthActionStarted(OnAuthActionStarted)
      // Add a test scope (we don't actually use this).
      //    .AddOauthScope("https://www.googleapis.com/auth/appstate")
      //    .InternalSetRootURL("https://www-googleapis-staging.sandbox.google.com/")
      .SetOnAuthActionFinished([callback](gpg::AuthOperation op, gpg::AuthStatus status){
          LOGI("Sign in finished with a result of %d", status);
          if( status == gpg::AuthStatus::VALID )
              isSignedIn = true;
          else
              isSignedIn = false;
          callback( op, status);
      } )
      .Create(pc);
      
  }
  LOGI("Created");
}


