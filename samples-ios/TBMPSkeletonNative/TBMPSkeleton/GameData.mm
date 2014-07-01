//
//  GameData.m
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

#import "GameData.h"

static const NSString * stringToPassKey = @"data";
static const NSString * turnCounterKey = @"turnCounter";


@implementation GameData

- (NSData *)jsonifyAndConvertToData {
  NSDictionary *myRepresentation = @{stringToPassKey: self.stringToPassAround,
                                     turnCounterKey: @(self.turnCounter)};

  NSError *errorToCatch;

  // The Android version uses UTF16 encoding. This makes sure we're comptible with them.
  NSData *jsonifiedData = [NSJSONSerialization dataWithJSONObject:myRepresentation options:0 error:&errorToCatch];
  NSString *tempConversion = [[NSString alloc] initWithData:jsonifiedData encoding:NSUTF8StringEncoding];
  NSData *convertedTo16Data = [tempConversion dataUsingEncoding:NSUTF16StringEncoding allowLossyConversion:YES];
    

  if (errorToCatch) {
    NSLog(@"Error trying to convert data %@",[errorToCatch localizedDescription]);
  }

  NSLog(@"My data is this %@", [[NSString alloc] initWithData:convertedTo16Data encoding:NSUTF16StringEncoding]);
  return convertedTo16Data;
}

- (id)initWitDataFromGPG:(NSData *)data {
  self = [super init];

  if (self) {
    NSError *errorToCatch;
    NSDictionary *convertedData = (NSDictionary *)[NSJSONSerialization JSONObjectWithData:data options:0 error:&errorToCatch];
    if (errorToCatch) {
      NSLog(@"Error trying to read in data %@", [errorToCatch localizedDescription]);
    }
    NSLog(@"This is my converted data dictionary %@", convertedData);
    self.stringToPassAround = (NSString *)[convertedData objectForKey:stringToPassKey];
    self.turnCounter = (int)[(NSNumber *) [convertedData objectForKey:turnCounterKey] intValue];
  }
  return self;
}

@end
