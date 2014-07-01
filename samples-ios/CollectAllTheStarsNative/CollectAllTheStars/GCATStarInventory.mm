//
//  GCATStarInventory.m
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

#import "GCATStarInventory.h"
@interface GCATStarInventory ()
@property (nonatomic, strong) NSMutableDictionary *starDict;

@end


@implementation GCATStarInventory

NSString *const kCloudSaveVersionNumber = @"1.1";
NSString *const kJSONVersionKey = @"version";
NSString *const kJSONLevelsKey = @"levels";


- (id) init {
  self = [super init];
  if (self) {
    _starDict = [[NSMutableDictionary alloc] init];
  }
  return self;
    
}


+ (GCATStarInventory *)emptyInventory {
  return [[GCATStarInventory alloc] init];
}


// Convert our NSData JSON object into data that our object can use. Luckily,
// the conversion is pretty straightforward.
+ (GCATStarInventory *)starInventoryFromCloudData:(NSData *)saveData {
  GCATStarInventory *inv = [[GCATStarInventory alloc] init];
  NSLog(@"I am trying to load %@", [[NSString alloc] initWithData:saveData encoding:NSUTF8StringEncoding]);

  NSError *error = nil;
  NSDictionary *savedState = (NSDictionary *)[NSJSONSerialization JSONObjectWithData:saveData options:0 error:&error];
  if (error) {
    NSLog(@"Got error trying to read data from JSON %@", [error localizedDescription]);
  }
  if ([(NSString *)[savedState objectForKey:kJSONVersionKey] isEqualToString:kCloudSaveVersionNumber]) {
    inv.starDict = [[savedState objectForKey:kJSONLevelsKey] mutableCopy];
  } else {
    // Otherwise, we're just going to stick with our empty inventory
  }
  return inv;
}

// Takes our internal data, grabs the JSON representation and returns that as
// NSData.
- (NSData *)getCloudSaveData {
  NSDictionary *saveMe = [NSDictionary dictionaryWithObjectsAndKeys:kCloudSaveVersionNumber, kJSONVersionKey,
                          self.starDict, kJSONLevelsKey, nil];
  NSError *error = nil;
  NSData *saveData = [NSJSONSerialization dataWithJSONObject:saveMe options:0 error:&error];
  if (error) {
    NSLog(@"Got an error trying to write data to JSON %@", [error localizedDescription]);
  }
  // Just for debugging
  NSString *jsonString = [[NSString alloc] initWithData:saveData encoding:NSUTF8StringEncoding];
  NSLog(@"I got yer cloud save data right here... %@",jsonString);
  
  return saveData;
}

- (int)getStarsForWorld:(int)world andLevel:(int)level {
  NSString *worldLevel = [NSString stringWithFormat:@"%d-%d",world,level];
  if ([self.starDict objectForKey:worldLevel]) {
    return  [(NSNumber *)[self.starDict objectForKey:worldLevel]intValue];
  }
  return 0;
}


- (void)setStars:(int)starNum forWorld:(int)world andLevel:(int)level {
  [self.starDict setObject:[NSNumber numberWithInt:starNum] forKey:[NSString stringWithFormat:@"%d-%d", world, level]];

}




@end
