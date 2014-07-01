//
//  AppDelegate.m
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

#import "AppDelegate.h"
#import "ViewController.h"
#import <GooglePlus/GooglePlus.h>

@interface AppDelegate ()

@end

@implementation AppDelegate
/** Handles the URL for Sign-In. 
 *  @param application The app receiving the URL.
 *  @param url The URL passed to the app.
 *  @param sourceApplication The
 */
- (BOOL)application:(UIApplication *)application
            openURL:(NSURL *)url
  sourceApplication:(NSString *)sourceApplication
         annotation:(id)annotation {
    NSLog(@"URL received");
    return [GPPURLHandler handleURL:url sourceApplication:sourceApplication annotation:annotation];
}

- (BOOL)application:(UIApplication *)application
    didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
//  [GPGLauncherController sharedInstance].launcherDelegate = self;
    // Override point for customization after application launch.
    return YES;
}
							
- (void)applicationWillResignActive:(UIApplication *)application
{
  // Sent when the application is about to move from active to inactive state. This can occur
  // for certain types of temporary interruptions (such as an incoming phone call or SMS message)
  // or when the user quits the application and it begins the transition to the background state.
  // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame
  // rates. Games should use this method to pause the game.
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
  // Use this method to release shared resources, save user data, invalidate timers, and
  // store enough application state information to restore your application to its current state in
  // case it is terminated later.
  // If your application supports background execution, this method is called instead of
  // applicationWillTerminate: when the user quits.
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
  // Called as part of the transition from the background to the inactive state; here you can undo
  // many of the changes made on entering the background.
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
  // Restart any tasks that were paused (or not yet started) while the application was inactive.
  // If the application was previously in the background, optionally refresh the user interface.
}

- (void)applicationWillTerminate:(UIApplication *)application
{
  // Called when the application is about to terminate. Save data if appropriate.
  // See also applicationDidEnterBackground:.
}

/** Message handler for when a user accepts a quest from the quest list.
 *  @param quest The quest that the user accepted.
 */
//-(void)questListLauncherDidAcceptQuest:(GPGQuest *)quest
//{
//  NSLog(@"The \"%@\" quest with id %@ has been accepted.", quest.name,
//        quest.questId);
//}

/** Message handler for when the player accepts a reward for a quest.
 *  @param questMilestone An object representing an important progression point within the quest.
 */
//- (void)questListLauncherDidClaimRewardsForQuestMilestone:(GPGQuestMilestone *)questMilestone {
//  [questMilestone claimWithCompletionHandler:^(NSError *error) {
//    NSLog(@"Quest reward with id %@ has been claimed.", questMilestone.questMilestoneId);
//  }];
//}

/** Message handler for when the player reaches a quest milestone.
 *  @param questMilestone An object representing an important progression point within the quest.
 */
//-(void)questListLauncherController:(GPGLauncherController *)controller
//            didClaimQuestMilestone:(GPGQuestMilestone *)questMilestone
//{
//  NSLog(@"Quest milestone with id %@ has been reached.", questMilestone.questMilestoneId);
//}

/** Handler for when the Play Games quest picker is present.
 *  @param launcherController The controller that is managing the view.
 *  @return The ViewController with the launcher controller and this ViewController's View.
 */
- (UIViewController *)presentingViewControllerForLauncher
{
  return self.window.rootViewController;
}

@end