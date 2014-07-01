//
//  GCATModel.m
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

#import "GCATModel.h"
#import "GCATStarInventory.h"
#import "GCATViewController.h"

#define DEFAULT_SAVE_NAME "snapshotTemp"

@interface GCATModel ()

@property (nonatomic, strong) NSNumber *starSaveSlot;
@property (nonatomic, copy) DataUpdatedHandler updatedHandler;
@property (nonatomic, strong) GCATStarInventory *inventory;
@property (nonatomic, weak) GCATViewController* screenViewController;
@end

@implementation GCATModel

- (id)init {
  self = [super init];
  if (self) {
    _starSaveSlot = [NSNumber numberWithInt:0];
    _inventory = [GCATStarInventory emptyInventory];
  }
  return self;
}

// Used to update starts after model changes, model-view.
- (void) setViewController: (GCATViewController*) screenViewController{
  _screenViewController = screenViewController;
}

/*
 * Read snapshot from cloud
 */
- (void) readCurrentSnapshot {
  NSLog(@"Reading snapshot");
  gpg::SnapshotManager::ReadResponse const &response = self.screenViewController->service_->Snapshots().ReadBlocking(self.currentSnapshot);
  if (gpg::IsSuccess(response.status)) {
    NSLog(@"Successfully read %zu blocks", response.data.size());
    NSData* data = [NSData dataWithBytes:reinterpret_cast<const void *>(response.data.data())
                                  length:response.data.size()];
    self.inventory = [GCATStarInventory starInventoryFromCloudData:data];
    [self.screenViewController refreshStarDisplay];
  }
  else {
    NSLog(@"Error while loading snapshot data: %d", response.status);
  }
}

/*
 * save snapshop
 */
- (void)saveSnapshotWithImage:(UIImage *)snapshotImage completionHandler:(DataUpdatedHandler)handler {
  NSLog(@"Saving snapshot");
  
  std::string fileName;
  if (self.currentSnapshot.Valid() == false) {
    fileName = DEFAULT_SAVE_NAME;
    NSLog(@"Creating new snapshot %s", fileName.c_str());
  } else {
    fileName = self.currentSnapshot.FileName();
  }
  
  self.screenViewController->service_->Snapshots().Open(fileName,
                                                        gpg::SnapshotConflictPolicy::MANUAL,
                                                        [self, snapshotImage, handler](gpg::SnapshotManager::OpenResponse const & response) {
                                                          if (IsSuccess(response.status)) {
                                                            gpg::SnapshotMetadata metadata = response.data;
                                                            if (response.conflict_id != "") {
                                                              //Conflict detected
                                                              NSLog(@"Snapshot conflict detected going to resolve that");
                                                              [self resolveSnapshotWithBaseMetadata:response.conflict_base
                                                                                     remoteMetadata:response.conflict_remote
                                                                                         conflictId:response.conflict_id];
                                                            }
                                                            
                                                            // Save the snapshot.
                                                            self.currentSnapshot = response.data;
                                                            [self commitCurrentSnapshotWithImage:snapshotImage completionHandler:handler];
                                                          }
                                                          else
                                                          {
                                                            //Failed, just call handler
                                                            handler();
                                                          }
                                                        });
}

/*
 * load snapshot
 */
- (void)loadSnapshot:(DataUpdatedHandler)handler
{
  if (self.currentSnapshot.Valid() == false)
  {
    handler();
    return;
  }
  self.screenViewController->service_->Snapshots().Open(_currentSnapshot.FileName(),
                                                        gpg::SnapshotConflictPolicy::MANUAL,
                                                        [self, handler](gpg::SnapshotManager::OpenResponse const & response) {
                                                          if (IsSuccess(response.status)) {
                                                            gpg::SnapshotMetadata metadata = response.data;
                                                            if (response.conflict_id != "") {
                                                              //Conflict detected
                                                              NSLog(@"Snapshot conflict detected going to resolve that");
                                                              [self resolveSnapshotWithBaseMetadata:response.conflict_base
                                                                                     remoteMetadata:response.conflict_remote
                                                                                         conflictId:response.conflict_id];
                                                            }
                                                            
                                                            // Save the snapshot.
                                                            _currentSnapshot = response.data;
                                                            [self readCurrentSnapshot];
                                                            handler();
                                                          }
                                                        });
}


/**
 * Insert any resolution code here.
 *
 * In this sample, we just take the newer of the conflicting snapshots, an alternative would
 * be to merge the data.
 */
- (void)resolveSnapshotWithBaseMetadata :(const gpg::SnapshotMetadata&)conflictingSnapshotBase
                          remoteMetadata:(const gpg::SnapshotMetadata&)conflictingSnapshotRemote
                              conflictId:(std::string)conflictId {
  
  NSLog(@"Resolving snapshot conflicts: %s >> %s",
        conflictingSnapshotBase.Description().c_str(),
        conflictingSnapshotRemote.Description().c_str());
  
  gpg::SnapshotMetadata final = conflictingSnapshotBase; // The resolved snapshot.
  
  // For this sample, we use the snapshot with the latest timestamp. Alternatively, you could
  // take the union of the two snapshots as is demonstrated in the Android version.
  if (conflictingSnapshotRemote.LastModifiedTime() >
      conflictingSnapshotBase.LastModifiedTime()) {
    final = conflictingSnapshotRemote;
  }
  
  self.currentSnapshot = final;
  
  //Resolve conflict
  gpg::SnapshotMetadataChange::Builder builder;
  gpg::SnapshotMetadataChange metadata_change =
  builder.SetDescription("CollectAllTheStar savedata ").Create();
  
  //For now, we would just choose the newest version of snapshot
  gpg::SnapshotManager::CommitResponse commitResponse =
  self.screenViewController->service_->Snapshots().ResolveConflictBlocking(final,
                                                                           metadata_change,
                                                                           conflictId);
  if (IsSuccess(commitResponse.status)) {
    NSLog(@"Conflict resolution succeeded");
    [self readCurrentSnapshot];
  } else {
    NSLog(@"Conflict resolution failed error: %d", commitResponse.status);
  }
}

/**
 * Saves the current Snapshot object stored in the
 */
- (void)commitCurrentSnapshotWithImage:(UIImage *)snapshotImage completionHandler:(DataUpdatedHandler)handler {
  if (self.currentSnapshot.Valid() == false) {
    NSLog(@"Error while committing snapshot, no current snapshot");
    handler();
    return;
  }
  
  //Convert UIImage to png stl::vector
  NSData *imageData = UIImagePNGRepresentation(snapshotImage);
  std::vector<uint8_t> vecImage;
  vecImage.assign(reinterpret_cast<const uint8_t*>([imageData bytes]),
                  reinterpret_cast<const uint8_t*>([imageData bytes]) + [imageData length]);

  //Played time
  std::chrono::minutes min(1);
  
  // Create a snapshot change to be committed with a description, cover image, and play time.
  gpg::SnapshotMetadataChange::Builder builder;
  gpg::SnapshotMetadataChange metadata_change =
  builder.SetDescription("Saved via iOS NativClient")
  .SetPlayedTime(self.currentSnapshot.PlayedTime() + min)
  .SetCoverImageFromPngData(vecImage)
  .Create();
  
  //Convert NSData to stl::vector
  NSData* data = [self.inventory getCloudSaveData];
  std::vector<uint8_t> v;
  v.assign(reinterpret_cast<const uint8_t*>([data bytes]),
           reinterpret_cast<const uint8_t*>([data bytes]) + [data length]);
  
  // Save the snapshot.
  gpg::SnapshotManager::CommitResponse commitResponse =
  self.screenViewController->service_->Snapshots().CommitBlocking(self.currentSnapshot,
                                                                  metadata_change,
                                                                  v);
  
  if (IsSuccess(commitResponse.status)) {
    NSLog(@"Successfully saved %s", self.currentSnapshot.Description().c_str());
  } else {
    NSLog(@"Error while saving: %d", commitResponse.status);
  }
  handler();
}

// Setters / Getters
- (void)setStars:(int)stars forWorld:(int)world andLevel:(int)level {
  [self.inventory setStars:stars forWorld:world andLevel:level];
}

- (int)getStarsForWorld:(int)world andLevel:(int)level {
  return [self.inventory getStarsForWorld:world andLevel:level];
}

@end
