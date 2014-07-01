//
//  ViewController.m
//  TrivialQuest2
//
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
#import "ViewController.h"
#import "Constants.h"

#include <sstream>
#include "gpg/gpg.h"

@interface ViewController()<UIAlertViewDelegate>

@end

@implementation ViewController
# pragma mark - Sign in stuff
/*
 * Initialize GooglePlayGameServices
 */
- (void)initializeGooglePlayGameServices
{
  gpg::IosPlatformConfiguration platform_configuration;
  platform_configuration.SetClientID(CLIENT_ID)
  .SetOptionalViewControllerForPopups(self);
  
  if (service_ == nullptr) {
    // Game Services have not been initialized, create a new Game Services.
    gpg::GameServices::Builder builder;
    service_ = builder.SetOnAuthActionStarted([self](gpg::AuthOperation op) {
      [self OnAuthActionStarted:op];
    })
    .SetOnAuthActionFinished(
                             [self](gpg::AuthOperation op, gpg::AuthStatus status) {
                               [self OnAuthActionFinished:op status:status];
                             })
    .SetOnQuestCompleted([self](gpg::Quest quest){
      NSLog(@"Completed a quest!!");
      [self QuestCompleted];
    })
    .SetDefaultOnLog(gpg::LogLevel::VERBOSE)  //For debugging log
    .Create(platform_configuration);
  }
}

/*
 * Callback: Authentication started
 */
- (void)OnAuthActionStarted:(gpg::AuthOperation) op
{
  [self refreshButtons:NO];
}

/*
 * Callback: Authentication finishes
 */
- (void)OnAuthActionFinished:(gpg::AuthOperation) op status:(gpg::AuthStatus)status {
  if (gpg::IsSuccess(status))
  {
    [self refreshButtons:YES];
    dispatch_async(dispatch_get_main_queue(), ^{
      [self.signInButton setTitle:@"Sign out" forState:UIControlStateNormal];
    });
  }
  else
  {
    [self refreshButtons:NO];
    dispatch_async(dispatch_get_main_queue(), ^{
      [self.signInButton setTitle:@"Sign in" forState:UIControlStateNormal];
    });
  }
}

/** Performs any events before the user is signed in for Google+.
 *  @param sender A reference for the object sending the message.
 */
- (IBAction)signInClicked:(id)sender {
  // Start authorization
  if (service_->IsAuthorized()) {
    service_->SignOut();
  } else {
    service_->StartAuthorizationUI();
  }
}

/*
 * Claim quest milestone
 */
- (void)ClaimMilestone: (gpg::QuestMilestone) milestone
{
  [self refreshButtons:NO];
  service_->Quests().ClaimMilestone(milestone, [self,milestone](gpg::QuestManager::ClaimMilestoneResponse milestoneResponse) {
    if( gpg::IsSuccess(milestoneResponse.status) )
    {
      NSLog(@"Claiming reward");
      auto begin = milestone.CompletionRewardData().begin();
      auto end = milestone.CompletionRewardData().end();
      NSString* str = [NSString stringWithFormat:@"You got %s", std::string(begin, end).c_str()];
      UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Reward claimed"
                                                      message:str
                                                     delegate:self
                                            cancelButtonTitle:nil
                                            otherButtonTitles:@"OK", nil];
      alert.delegate = self;
      dispatch_async(dispatch_get_main_queue(), ^{
        [alert show];
      });
    }
    else
      [self refreshButtons:YES];
  });
}

/*
 * Show quest compoletion message
 */
- (void)QuestCompleted
{
  [self refreshButtons:NO];
  UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Quest"
                                                  message:@"Completed quest!!"
                                                 delegate:self
                                        cancelButtonTitle:nil
                                        otherButtonTitles:@"OK", nil];
  alert.delegate = self;
  dispatch_async(dispatch_get_main_queue(), ^{
    [alert show];
  });
}

/** Show the Quest Chooser */
- (IBAction)ShowQuests:(id)sender {
  [self refreshButtons:NO];
  service_->Quests().ShowAllUI([self](
                                      gpg::QuestManager::QuestUIResponse const & response) {
    if (IsSuccess(response.status)) {
      if (response.accepted_quest.Valid())
      {
        NSLog(@"Accepted a quest");
        [self refreshButtons:YES];
      }
      if (response.milestone_to_claim.Valid())
      {
        NSLog(@"Claimed a milestone");
        [self ClaimMilestone:response.milestone_to_claim];
      }
    } else {
      NSLog(@"Invalid response status");
      [self refreshButtons:YES];
    }
  });
}

/** Show event detail alert. */
- (IBAction)ShowEvents:(id)sender {
  NSLog(@"---- Showing Event Counts -----");
  [self refreshButtons:NO];
  
  service_->Events().FetchAll([self](gpg::EventManager::FetchAllResponse const &response)
                              {
                                if (IsSuccess(response.status))
                                {
                                  NSLog(@"Showing event status");
                                  std::ostringstream str;
                                  auto begin = response.data.begin();
                                  auto end = response.data.end();
                                  while( begin != end)
                                  {
                                    str <<begin->second.Name() << ": " << begin->second.Count() << "\n";
                                    begin++;
                                  }
                                  
                                  UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Event status"
                                                                                  message:[NSString stringWithUTF8String:str.str().c_str()]
                                                                                 delegate:self
                                                                        cancelButtonTitle:nil
                                                                        otherButtonTitles:@"OK", nil];
                                  alert.delegate = self;
                                  dispatch_async(dispatch_get_main_queue(), ^{
                                    [alert show];
                                  });
                                }
                                else{
                                  [self refreshButtons:YES];
                                }
                              });
}

/** Simulates attacking a "blue" monster in-game.
 *  @param sender A reference for the object sending the message.
 */
- (IBAction)attackBlue:(id)sender {
  NSLog(@"Attacked a blue monster.");
  service_->Events().Increment(BLUE_MONSTER_EVENT_ID);
}

/** Simulates attacking a "green" monster in-game.
 *  @param sender A reference for the object sending the message.
 */
- (IBAction)attackGreen:(id)sender {
  NSLog(@"Attacked a green monster.");
  service_->Events().Increment(GREEN_MONSTER_EVENT_ID);
}

/** Simulates attacking a "red" monster in-game.
 *  @param sender A reference for the object sending the message.
 */
- (IBAction)attackRed:(id)sender {
  NSLog(@"Attacked a red monster.");
  service_->Events().Increment(RED_MONSTER_EVENT_ID);
}

/** Simulates attacking a "yellow" monster in-game.
 *  @param sender A reference for the object sending the message.
 */
- (IBAction)attackYellow:(id)sender {
  NSLog(@"Attacked a yellow monster.");
  service_->Events().Increment(YELLOW_MONSTER_EVENT_ID);
}

- (void)didReceiveMemoryWarning
{
  [super didReceiveMemoryWarning];
  // Dispose of any resources that can be recreated.
}

/** Updates UI components after the user sign-in status changes. */

-(void)refreshButtons:(BOOL)b
{
  dispatch_async(dispatch_get_main_queue(), ^{
    // Start authorization
    self.signInButton.enabled = YES;  //Sign in/out button always enabled
    self.attackBlueButton.enabled = b;
    self.attackGreenButton.enabled = b;
    self.attackRedButton.enabled = b;
    self.attackYellowButton.enabled = b;
    self.showQuestsButton.enabled = b;
    self.showEventsButton.enabled = b;
  });
}

- (void)alertView:(UIAlertView *)alertView
didDismissWithButtonIndex:(NSInteger)buttonIndex
{
  [self refreshButtons:YES];
}

/** Initializes all of the Google services when the view loads and signs the user in. */
- (void)viewDidLoad
{
  [super viewDidLoad];
  NSLog(@"Init GameServices");
  [self initializeGooglePlayGameServices];
}

@end
