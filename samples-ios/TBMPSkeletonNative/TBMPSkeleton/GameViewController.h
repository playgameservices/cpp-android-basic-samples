//
//  GameViewController.h
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

#import <UIKit/UIKit.h>
#import <gpg/gpg.h>

@interface GameViewController : UIViewController
{
@public
  
}
@property (nonatomic) gpg::TurnBasedMultiplayerManager* manager;
@property (nonatomic) gpg::TurnBasedMatch& match;
@property (nonatomic) std::string localPlayerId;
@property (nonatomic) bool dismissCurrentMatch;
@property (nonatomic) bool leaveCurrentMatch;
@property (nonatomic) bool cancelCurrentMatch;
@property (nonatomic) bool rematchCurrent;
@end
