//
//  AppDelegate.m
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

#import <GooglePlus/GooglePlus.h>

#import "AppDelegate.h"

#import "LobbyViewController.h"

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{

  // Register for push notifications
  NSLog(@"Registering for push notifications");
  [[UIApplication sharedApplication] registerForRemoteNotificationTypes:
   (UIRemoteNotificationTypeBadge | UIRemoteNotificationTypeAlert |
    UIRemoteNotificationTypeSound)];
  
  // Look to see if our application was launched from a notification
  NSDictionary *remoteNotification =
  [launchOptions objectForKey:UIApplicationLaunchOptionsRemoteNotificationKey];
  
  // If one exists, see if it's a game invitation
  if (remoteNotification) {
    if (gpg::TryHandleNotificationFromLaunchOptions(remoteNotification)) {
      // Yes!  A didReceiveRealTimeInviteForRoom method is being called in our app delegate!
    } else {
      // You probably want to do other notification checking here.
    }
  }
  return YES;
}
- (void)application:(UIApplication *)application didReceiveRemoteNotification:(NSDictionary *)userInfo {
  NSLog(@"Received remote notification %@", userInfo);
  if (gpg::TryHandleRemoteNotification(userInfo)) {
    NSLog(@"Okay, in theory, my delegate method should be receving a callback now,");
  }
}

- (BOOL)application:(UIApplication *)application openURL:(NSURL *)url sourceApplication:(NSString *)sourceApplication annotation:(id)annotation {
  return [GPPURLHandler handleURL:url sourceApplication:sourceApplication annotation:annotation];
}

- (void)application:(UIApplication *)application
didRegisterForRemoteNotificationsWithDeviceToken:(NSData *)deviceToken {
  NSLog(@"Got deviceToken from APNS! %@", deviceToken);
  gpg::RegisterDeviceToken(deviceToken, true);  //For Sandbox
}

- (void)applicationWillResignActive:(UIApplication *)application
{
  // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
  // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
  // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
  // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
  // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
  // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}

- (void)applicationWillTerminate:(UIApplication *)application
{
  // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}

@end
