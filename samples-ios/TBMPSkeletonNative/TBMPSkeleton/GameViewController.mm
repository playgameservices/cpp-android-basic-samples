//
//  GameViewController.m
//  TBMPTest
//
//  Copyright (c) 2014 Google. All rights reserved.
//
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

#import "GameViewController.h"
#import "GameData.h"

/*
 * Preprocessors
 */

@interface GameViewController () <UIAlertViewDelegate>
@property (weak, nonatomic) IBOutlet UISwitch *leaveSwitch;
@property (weak, nonatomic) IBOutlet UISwitch *playerLostSwitch;
@property (weak, nonatomic) IBOutlet UIButton *takeTurnButton;
@property (weak, nonatomic) IBOutlet UILabel *turnNumberLabel;
@property (weak, nonatomic) IBOutlet UIButton *leaveButton;
@property (weak, nonatomic) IBOutlet UIButton *dismissButton;
@property (weak, nonatomic) IBOutlet UIButton *rematchButton;
@property (weak, nonatomic) IBOutlet UIButton *cancelButton;
@property (weak, nonatomic) IBOutlet UITextField *turnTextField;
@property (weak, nonatomic) IBOutlet UISwitch *winGameSwitch;

@property (nonatomic) GameData *gameData;

@end

@implementation GameViewController

- (void)setMatch:(gpg::TurnBasedMatch&)match {
  _match = match;
  
  NSLog(@"Setting a match -- the pending participant is %s", match.PendingParticipant().Id().c_str());
  
  if (_match.Data().size()) {
    NSData* data = [NSData dataWithBytes:&_match.Data()[0] length:_match.Data().size()];
    self.gameData = [[GameData alloc] initWitDataFromGPG:data];
  } else {
    self.gameData = [[GameData alloc] init];
    self.gameData.turnCounter = 1;
  }
}

/*
 * Find result for given participant
 */
- (gpg::MatchResult)findResultForParticipant:(const std::string*)participantId {
  
  if (self.match.ParticipantResults().HasResultsForParticipant(*participantId))
  {
    return self.match.ParticipantResults().MatchResultForParticipant(*participantId);
  }
  
  return gpg::MatchResult::NONE;
}

/*
 * Check if the player is still in game
 */
- (BOOL)isPlayerStillInGame:(gpg::MultiplayerParticipant*)participantToCheck {
  // First, is the player in the game? (i.e. Have they quit?)
  if (participantToCheck->Status() == gpg::ParticipantStatus::INVITED ||
      participantToCheck->Status() == gpg::ParticipantStatus::JOINED ||
      participantToCheck->Status() == gpg::ParticipantStatus::NOT_INVITED_YET) {
    // Next, are they in the results array? (i.e. Have they lost?)
    if ([self findResultForParticipant:&participantToCheck->Id()] != gpg::MatchResult::NONE) {
      return YES;
    }
  }
  return NO;
}

/*
 * This is called when a player really wants to stand up from the table and leave the game, and
 * not for a typical "I have been eliminated" scenario. Call takeLosingTurn for that.
 *
 * Note that any changes the player has made to the gameData will NOT be saved. It you want
 * to do that, take a turn with the localParticipantId as the next player. Then, in the
 * completion handler, you can call leaveInTurn.
 *
 * One other side effect here, the game will be silently cancelled if all but one player has left.
 * If you want to avoid this, you should call finishWithData and declare the remaining player the
 * winner.
 */
- (void)playerQuits {
  if (self.match.Status() == gpg::MatchStatus::MY_TURN) {
    //Leave a game
    self.manager->LeaveMatchDuringMyTurn(self.match,
                                         self.match.SuggestedNextParticipant(),
                                         [self](gpg::MultiplayerStatus status) {
                                           NSLog(@"Left the game");
                                           dispatch_async(dispatch_get_main_queue(), ^{
                                             [self.navigationController popViewControllerAnimated:YES];
                                           });
                                         });
    return;
  }
}

/*
 * Cancel current match
 */
- (IBAction)cancelMatch:(id)sender {
  self.manager->CancelMatch(self.match,
                            [self](gpg::MultiplayerStatus status) {
                              NSLog(@"Canceled the game");
                              dispatch_async(dispatch_get_main_queue(), ^{
                                [self.navigationController popViewControllerAnimated:YES];
                              });
                            });
}

/*
 * Dismiss current match
 */
- (IBAction)dismissMatch:(id)sender {
  self.manager->DismissMatch(self.match);
  [self.navigationController popViewControllerAnimated:YES];
}

/*
 * Rematch selected match
 */
-(IBAction)rematch:(id)sender {
  self.manager->Rematch(
                        self.match,
                        [self](gpg::TurnBasedMultiplayerManager::TurnBasedMatchResponse
                               matchResponse) {
                          NSLog(@"Remathing the game");
                          if (gpg::IsSuccess(matchResponse.status)) {
                            // Start the match
                            [self refreshInterfaceFromMatchData];
                          }
                        });
}

/*
 * Leave match
 */
- (IBAction)leaveMatch:(id)sender {
  if (self.match.Status() == gpg::MatchStatus::MY_TURN) {
    //Leave a game
    self.manager->LeaveMatchDuringMyTurn(self.match,
                                   self.match.SuggestedNextParticipant(),
                                   [self](gpg::MultiplayerStatus status) {
                                     NSLog(@"Left the game");
                                     dispatch_async(dispatch_get_main_queue(), ^{
                                       [self.navigationController popViewControllerAnimated:YES];
                                     });
                                   });

  } else {
    self.manager->LeaveMatchDuringTheirTurn(self.match,
                                            [self](gpg::MultiplayerStatus status) {
                                              NSLog(@"Left the game");
                                              dispatch_async(dispatch_get_main_queue(), ^{
                                                [self.navigationController popViewControllerAnimated:YES];
                                              });
                                            });
  }
  return;
}

/*
 * Take turn based on playGame UI parameters
 */
- (void)takeTurn:(bool) winning losing:(bool) losing {
  
  //Find the participant for the local player.
  gpg::MultiplayerParticipant localParticipant;
  for (auto& participant : self.match.Participants()) {
    if (participant.Player().Valid() &&
        participant.Player().Id() == self.localPlayerId) {
      localParticipant = participant;
    }
  }
  
  NSLog(@"Taking my turn. local participant id:%s", localParticipant.Id().c_str());
  
  // Convert NSData to std::vector
  std::vector<uint8_t> match_data;
  NSData* data = self.gameData.jsonifyAndConvertToData;
  const uint8_t* p = (const uint8_t*)[data bytes];
  match_data.assign(p, p + [data length]);
  
  //By default, passing through existing participatntResults
  gpg::ParticipantResults results = self.match.ParticipantResults();
  if (winning) {
    //Create winning participants result
    results = self.match.ParticipantResults()
    .WithResult(localParticipant.Id(),  // localplayer's ParticipantID
                0,                      // placing
                gpg::MatchResult::WIN   // status
                );
    // Note that we need to pass participantID rather than playerID
    
  } else if (losing) {
    //Create losing participants result
    results = self.match.ParticipantResults()
    .WithResult(localParticipant.Id(),  // localplayer's ParticipantID
                0,                      // placing
                gpg::MatchResult::LOSS  // status
                );
    // Note that we need to pass participantID rather than playerID
  }
  
  //Take normal turn
  self.manager->TakeMyTurn(self.match,
                           match_data,
                           results,
                           self.match.SuggestedNextParticipant(),
                               [self](
                                      gpg::TurnBasedMultiplayerManager::TurnBasedMatchResponse const &
                                      response) {
                                 NSLog(@"Took turn");
                                 if (gpg::IsSuccess(response.status)) {
                                   NSLog(@"Game has ended.");
                                   dispatch_async(dispatch_get_main_queue(), ^{
                                     [self.navigationController popViewControllerAnimated:YES];
                                   });
                                 } else {
                                   NSLog(@"I got an error! %d", response.status);
                                 }
                               });
}

- (IBAction)takeTurnWasPressed:(id)sender {
  self.gameData.stringToPassAround = self.turnTextField.text;
  self.gameData.turnCounter++;
  
  // Did the player click "win game?", or have all other players been eliminated?
  if (self.winGameSwitch.on )
  {
    [self takeTurn:YES losing:NO];
  }
  else if (self.playerLostSwitch.on) {
    [self takeTurn:NO losing:YES];
  }
  else if (self.leaveSwitch.on) {
    [self playerQuits];
  }
  else {
    NSLog(@"Taking normal turn");
    [self takeTurn:NO losing:NO];
  }
}

- (void)alertView:(UIAlertView *)alertView didDismissWithButtonIndex:(NSInteger)buttonIndex {
  // Let's just confirm, but I believe I'm getting this because we've determine that I'm the
  // only player left in the game
  [self takeTurn:YES losing:NO];
}

- (void)enableInterfaceIfMyTurn {
  NSArray *controlsToEnable = @[self.leaveSwitch, self.playerLostSwitch, self.takeTurnButton,
                                self.turnTextField, self.winGameSwitch];
  bool enable = (self.localPlayerId == self.match.PendingParticipant().Player().Id());
  
  for (UIControl *shouldEnable in controlsToEnable) {
    shouldEnable.enabled = enable;
  }
}

- (IBAction)playerMadeSomeTextFieldEdit:(id)sender {
  self.takeTurnButton.enabled = YES;
}

- (IBAction)switchWasChanged:(id)sender {
  // Can't more than one of 'em on at the same time.
  [self.playerLostSwitch setOn:(sender == self.playerLostSwitch) animated:YES];
  [self.leaveSwitch setOn:(sender == self.leaveSwitch) animated:YES];
  [self.winGameSwitch setOn:(sender == self.winGameSwitch) animated:YES];
}

- (void)refreshInterfaceFromMatchData {
  // Populate our data with the turn data
  self.turnTextField.text = self.gameData.stringToPassAround;
  self.turnNumberLabel.text = [NSString stringWithFormat:@"Turn %d", self.gameData.turnCounter];
  
  // Show match management buttons if any of them is needed
  if( self.dismissCurrentMatch || self.leaveCurrentMatch || self.cancelCurrentMatch || self.rematchCurrent)
  {
    // Show match management buttons
    self.dismissButton.enabled = self.dismissCurrentMatch;
    self.leaveButton.enabled = self.leaveCurrentMatch;
    self.cancelButton.enabled = self.cancelCurrentMatch;
    self.rematchButton.enabled = self.rematchCurrent;
  }
  else
  {
    // Hide match management buttons
    self.dismissButton.hidden = YES;
    self.leaveButton.hidden = YES;
    self.cancelButton.hidden = YES;
    self.rematchButton.hidden = YES;
  }
  
  [self enableInterfaceIfMyTurn];
}

-(void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
  [self.turnTextField resignFirstResponder];
}

- (void)viewWillAppear:(BOOL)animated {
  [super viewWillAppear:animated];
  
  // Let's diable the take turn button. Require you to edit something first.
  self.takeTurnButton.enabled = NO;
  [self.winGameSwitch setOn:NO];
  
  [self refreshInterfaceFromMatchData];
  
  if (self.match.SuggestedNextParticipant().Valid() == false)
  {
    [[[UIAlertView alloc] initWithTitle:@"You won!"
                                message:@"All other players have been eliminated! You win!"
                               delegate:self
                      cancelButtonTitle:@"Hooray!"
                      otherButtonTitles:nil] show];
  }
}

- (void)viewDidLoad {
  [super viewDidLoad];
	// Do any additional setup after loading the view.
}

- (void)didReceiveMemoryWarning {
  [super didReceiveMemoryWarning];
  // Dispose of any resources that can be recreated.
}

@end
