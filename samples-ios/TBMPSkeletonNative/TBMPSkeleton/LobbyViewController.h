//
//  ViewController.h
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
#include <gpg/gpg.h>

@interface LobbyViewController : UIViewController
{
  std::unique_ptr<gpg::GameServices> service_;
}
- (void)OnAuthActionStarted:(gpg::AuthOperation) op;
- (void)OnAuthActionFinished:(gpg::AuthOperation) op status:(gpg::AuthStatus)status;
- (void)refreshButtons:(BOOL)b;
- (void)showMatchInbox;


@end
