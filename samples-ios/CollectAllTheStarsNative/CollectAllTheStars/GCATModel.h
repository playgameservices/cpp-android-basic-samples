//
//  GCATModel.h
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

#import <Foundation/Foundation.h>
#include "GCATViewController.h"

typedef void(^DataUpdatedHandler)(void);

@interface GCATModel : NSObject
@property gpg::SnapshotMetadata currentSnapshot;

- (void)readCurrentSnapshot;
- (void)loadSnapshot:(DataUpdatedHandler)handler;
- (void)saveSnapshotWithImage: (UIImage *)snapshotImage completionHandler:(DataUpdatedHandler)handler;
- (void)setViewController: (GCATViewController*) screenViewController;
- (void)setStars:(int)stars forWorld:(int)world andLevel:(int)level;
- (int)getStarsForWorld:(int)world andLevel:(int)level;
@end
