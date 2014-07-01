//
//  GCATViewController.m
//  CollectAllTheStars
//
//  Created by Todd Kerpelman on 5/7/13.
//  Copyright (c) 2013 Google. All rights reserved.
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
//

#import "GCATConstants.h"
#import "GCATViewController.h"
#import "GCATModel.h"

#include "gpg/gpg.h"

@interface GCATViewController () <UIPickerViewDelegate, UIPickerViewDataSource, UIActionSheetDelegate>
@property (nonatomic) int currentWorld;
@property (nonatomic) int pickerSelectedRow;
@property (nonatomic, strong) GCATModel *gameModel;


@property (weak, nonatomic) IBOutlet UIButton *signInButton;
@property (weak, nonatomic) IBOutlet UIButton *listSnapshotsButton;
@property (weak, nonatomic) IBOutlet UIActivityIndicatorView *indicator;
@property (weak, nonatomic) IBOutlet UIButton *loadButton;
@property (weak, nonatomic) IBOutlet UIButton *saveButton;
@property (weak, nonatomic) IBOutlet UIButton *changeWorldButton;
@property (weak, nonatomic) IBOutlet UILabel *worldLabel;

// All of our buttons! HOoray
@property (weak, nonatomic) IBOutlet UIButton *level1Button;
@property (weak, nonatomic) IBOutlet UIButton *level2Button;
@property (weak, nonatomic) IBOutlet UIButton *level3Button;
@property (weak, nonatomic) IBOutlet UIButton *level4Button;
@property (weak, nonatomic) IBOutlet UIButton *level5Button;
@property (weak, nonatomic) IBOutlet UIButton *level6Button;
@property (weak, nonatomic) IBOutlet UIButton *level7Button;
@property (weak, nonatomic) IBOutlet UIButton *level8Button;
@property (weak, nonatomic) IBOutlet UIButton *level9Button;
@property (weak, nonatomic) IBOutlet UIButton *level10Button;
@property (weak, nonatomic) IBOutlet UIButton *level11Button;
@property (weak, nonatomic) IBOutlet UIButton *level12Button;
@property (nonatomic, strong) NSArray *levelButtons;
@end


static NSString * const kDeclinedGooglePreviously = @"UserDidDeclineGoogleSignIn";

@implementation GCATViewController

# pragma mark - Sign-in functions
-(void)startGoogleGamesSignIn
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
    .SetDefaultOnLog(gpg::LogLevel::VERBOSE)  //For debugging log
    .EnableSnapshots()                        //Enable Snapshot
    .Create(platform_configuration);
  }
}

/*
 * Callback: Authentication started
 */
- (void)OnAuthActionStarted:(gpg::AuthOperation) op
{
  dispatch_async(dispatch_get_main_queue(), ^{
    [self refreshButtons:NO];
  });
}

/*
 * Callback: Authentication finishes
 */
- (void)OnAuthActionFinished:(gpg::AuthOperation) op status:(gpg::AuthStatus)status {
  if(gpg::IsSuccess(status))
  {
    dispatch_async(dispatch_get_main_queue(), ^{
      [self refreshButtons:YES];
      [self.signInButton setTitle:@"Sign out" forState:UIControlStateNormal];
    });
  }
  else
  {
    dispatch_async(dispatch_get_main_queue(), ^{
      [self refreshButtons:NO];
      [self.signInButton setTitle:@"Sign in" forState:UIControlStateNormal];
    });
  }
}

// Refresh our buttons depending on whether or not the user has signed in to
// Play Games
-(void)refreshButtons:(BOOL)b
{
  dispatch_async(dispatch_get_main_queue(), ^{
    for (UIButton *button in self.levelButtons) {
      button.enabled = b;
    }
    self.worldLabel.enabled = b;
    self.changeWorldButton.enabled = b;
    
    if (self.gameModel.currentSnapshot.Valid() == false)
    {
      self.loadButton.enabled = NO;
    }
    else
      self.loadButton.enabled = b;

    self.saveButton.enabled = b;
    self.listSnapshotsButton.enabled = b;
    
    self.signInButton.enabled = b;
    if (b) {
      [self.indicator stopAnimating];
    } else {
      [self.indicator startAnimating];
    }
  });
}

- (IBAction)signInPressed:(id)sender {
  // Start authorization
  if (service_->IsAuthorized()) {
    service_->SignOut();
  } else {
    service_->StartAuthorizationUI();
  }
}

- (IBAction)showListPressed:(id)sender {
  const int32_t MAX_SNAPSHOTS = 3;
  const bool ALLOW_CREATE_SNAPSHOT_INUI = true;
  const bool ALLOW_DELETE_SNAPSHOT_INUI = true;
  const char* const SNAPSHOT_UI_TITLE = "Collect All The Stars";
  
  [self refreshButtons:NO];
  service_->Snapshots().ShowSelectUIOperation(
                                              ALLOW_CREATE_SNAPSHOT_INUI,
                                              ALLOW_DELETE_SNAPSHOT_INUI,
                                              MAX_SNAPSHOTS,
                                              SNAPSHOT_UI_TITLE,
                                              [self](gpg::SnapshotManager::SnapshotSelectUIResponse const & response) {
                                                NSLog(@"Snapshot selected");
                                                if (IsSuccess(response.status)) {
                                                  if (response.data.Valid()) {
                                                    NSLog(@"Description: %s", response.data.Description().c_str());
                                                    NSLog(@"FileName %s", response.data.FileName().c_str());
                                                    
                                                    //Opening the snapshot data
                                                    self.gameModel.currentSnapshot = response.data;
                                                    [self.gameModel loadSnapshot:^()
                                                     {
                                                       [self refreshButtons:YES];
                                                     }];
                                                  } else {
                                                    NSLog(@"Creating new snapshot");
                                                    dispatch_async(dispatch_get_main_queue(), ^{
                                                      [self saveToTheCloud];
                                                    });
                                                  }
                                                } else {
                                                  NSLog(@"ShowSelectUIOperation returns an error %d", response.status);
                                                  [self refreshButtons:YES];
                                                }
                                              });
}

// In a real game, we'd probably want to save and load behind the scenes.
// Here we're calling these explicitly through buttons so you can try out
// different scenarios.
-(void)saveToTheCloud {
  [self refreshButtons:NO];
  
  [self.gameModel saveSnapshotWithImage:[self takeScreenshot] completionHandler:^{
    [self refreshStarDisplay];
    [self.saveButton setTitle:@"Save" forState:UIControlStateNormal];
    [self refreshButtons:YES];
  }];
}

- (void)loadFromTheCloud {
  
  [self refreshButtons:NO];

  [self.gameModel loadSnapshot:^{
    [self refreshStarDisplay];
    [self.saveButton setTitle:@"Save" forState:UIControlStateNormal];
    [self refreshButtons:YES];
  }];
}


# pragma mark - Actual game stuff

// We'll let every click increment the button a bit
- (IBAction)levelButtonClicked:(id)sender {
  int levelNum = [self.levelButtons indexOfObject:sender] + 1;
  int starNum = [self.gameModel getStarsForWorld:self.currentWorld andLevel:levelNum] + 1;
  if (starNum > 5) starNum = 0;
  [self.gameModel setStars:starNum forWorld:self.currentWorld andLevel:levelNum];
  [self.saveButton setTitle:@"Save*" forState:UIControlStateNormal];
  [self refreshStarDisplay];
}

// Update our level buttons
- (void)refreshStarDisplay {
  unichar blackStar = 0x2605;
  NSString *fullStar = [NSString stringWithCharacters:&blackStar length:1];
  unichar whitestar = 0x2606;
  NSString *emptyStar = [NSString stringWithCharacters:&whitestar length:1];
  
  for (int i=0; i<[self.levelButtons count]; i++) {
    int level = i+1;
    int starCount = [self.gameModel getStarsForWorld:self.currentWorld andLevel:level];
    NSString *blackStarText = [@"" stringByPaddingToLength:starCount withString:fullStar startingAtIndex:0];
    NSString *starText = [blackStarText stringByPaddingToLength:5 withString:emptyStar startingAtIndex:0];
    NSString *buttonText = [NSString stringWithFormat:@"%d-%d\n%@", self.currentWorld, level, starText];
    UIButton *buttonToUpdate = [self.levelButtons objectAtIndex:i];
    buttonToUpdate.titleLabel.lineBreakMode = NSLineBreakByWordWrapping;
    buttonToUpdate.titleLabel.textAlignment = NSTextAlignmentCenter;
    [buttonToUpdate setTitle:buttonText forState:UIControlStateNormal];
  }
  self.worldLabel.text = [NSString stringWithFormat:@"World %d",self.currentWorld];
}

// Manual load to current snapshot.
- (IBAction)loadWasPressed:(id)sender {
  [self loadFromTheCloud];
}

// Manual save to a snapshot.
- (IBAction)saveWasPressed:(id)sender {
  [self saveToTheCloud];
}


# pragma mark - PickerView methods
- (NSInteger)numberOfComponentsInPickerView:(UIPickerView *)pickerView
{
  return 1;
}

- (NSInteger)pickerView:(UIPickerView *)pickerView numberOfRowsInComponent:(NSInteger)component
{
  return 20;
}

- (NSString *)pickerView:(UIPickerView *)pickerView titleForRow:(NSInteger)row forComponent:(NSInteger)component
{
  return [NSString stringWithFormat:@"World %d",row+1];
}

- (void)pickerView:(UIPickerView *)pickerView didSelectRow:(NSInteger)row inComponent:(NSInteger)component
{
  self.pickerSelectedRow = row;
}

- (IBAction)changeWorld:(id)sender {
  UIActionSheet *menu = [[UIActionSheet alloc] initWithTitle:@"Choose World"
                                                    delegate:self
                                           cancelButtonTitle:@"Done"
                                      destructiveButtonTitle:@"Cancel"
                                           otherButtonTitles:nil];
  // Add the picker
  UIPickerView *pickerView = [[UIPickerView alloc] initWithFrame:CGRectMake(0,185,0,0)];
  pickerView.delegate = self;
  pickerView.showsSelectionIndicator = YES;    // note this is default to NO
  [pickerView selectRow:self.currentWorld - 1 inComponent:0 animated:YES];
  
  [menu addSubview:pickerView];
  [menu showInView:self.view];
  [menu setBounds:CGRectMake(0,0,320, 700)];
  
}

-(void)actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex
{
  if (buttonIndex == 1) { //Done
    self.currentWorld = self.pickerSelectedRow + 1;
    [self refreshStarDisplay];
  }
}

- (UIImage *) takeScreenshot {
  // Parts taken from:
  //    http://stackoverflow.com/questions/12687909/ios-screenshot-part-of-the-screen
  UIGraphicsBeginImageContext(self.view.bounds.size);
  [self.view.layer renderInContext:UIGraphicsGetCurrentContext()];
  UIImage *sourceImage = UIGraphicsGetImageFromCurrentImageContext();
  UIGraphicsEndImageContext();
  UIGraphicsBeginImageContext(self.view.frame.size);
  [sourceImage drawAtPoint:CGPointMake(0, -160)];
  UIImage *croppedImage = UIGraphicsGetImageFromCurrentImageContext();
  UIGraphicsEndImageContext();
  
  return croppedImage;
}

# pragma mark - Standard lifecycle functions

- (void)viewDidLoad
{
  [super viewDidLoad];
  self.levelButtons = [NSArray arrayWithObjects:self.level1Button,
                       self.level2Button, self.level3Button,
                       self.level4Button, self.level5Button,
                       self.level6Button, self.level7Button,
                       self.level8Button, self.level9Button,
                       self.level10Button, self.level11Button,
                       self.level12Button, nil];
  self.currentWorld = 1;
  self.gameModel = [[GCATModel alloc] init];
  [self.gameModel setViewController: self];
  
  NSLog(@"Init GameServices");
  [self startGoogleGamesSignIn];
  
  [self refreshButtons:NO];
}


-(void)viewWillAppear:(BOOL)animated
{
  [super viewWillAppear:animated];
  [self refreshStarDisplay];
}

- (void)didReceiveMemoryWarning
{
  [super didReceiveMemoryWarning];
  // Dispose of any resources that can be recreated.
}

@end
