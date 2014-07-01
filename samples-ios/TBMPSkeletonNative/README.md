# TBMP Skeleton

A sample application that demonstrates some simple turn-based multiplayer using both
invites and matchmaking with strangers with NativeClient SDK.
It's also compatible with the Android version
for some cross-platform word-typing action! 

## Code

TBMP Skeleton consists of a number of files that might be of interest to you:

* `AppDelegate` contains some of the code required to handle incoming notifications
and sign-in.

* `Constants.h` contains the constants that you will need to run this game on your 
own.

* `GameData` is a small class that represents the data passed by the game. It includes
some convenience methods to convert this data back and forth into JSON. (The Android version
of this game used UTF16 strings, so we had to as well.)

* `GameViewController` is the ViewController for the game itself. It also contains 
most of the logic for determining who the next player is, setting game results,
submitting a turn, finishing a match, or leaving a match entirely.

* `LobbyViewController` contains methods that handle sign-in and create real-time
mutliplayer games, either through invites (using the PeoplePickerVC) or through 
automatching.

* `Main.storyboard` is the main storyboard used by the application. We currently
use the same storyboard for both iPhone and iPad games 

## Running the sample application

To run TBMP Skeleton on your own device, you will need to create
your own version of the game in the Play Console and copy over some information to
your Xcode project. To follow this process, perform the following steps:

1. Open up your TBMP Skeleton project settings. Select the "TBMP Skeleton" target and,
  on the "General" tab, change the Bundle Identifier from `com.google.example.games.tbmpskel` to
  something appropriate for your Provisioning Profile. (It will probably look like
  `com.<your_company>.tbmpskel`)
2. Click the "Info" tab and go down to the bottom where you see "URL Types". Expand
  this and change the "Identifier" and "URL Schemes" from `com.example.TBMPSkeleton` to
  whatever you used in Step 1.
3. Create your own application in the Play Console, as described in our [Developer
  Documentation](https://developers.google.com/games/services/console/enabling). Make
  sure you follow the "iOS" instructions for creating your client ID and linking
  your application.
    * If you have already created an application (because you tested the Android version,
  for instance), you can use that application, and just add a new linked iOS client to the same
  application.
    * Again, you will be using the Bundle ID that you created in Step 1.
    * You can leave your App Store ID blank for testing purposes.
 	* Don't forget to turn on the "Turn-based multiplayer" switch!
4. If you want to try out receiving invites and "it's your turn" notifications, you will 
  need to get an APNS certificate from iTunes Connect and upload it to the developer console 
  as well. Please review our documentation for instructions on how to do this.
5. Make a note of your client ID and application ID as described in the
  documentation
6. Once that's done, open up your `Constants.h` file, and replace the `CLIENT_ID` value
  with your own OAuth2.0 client ID.
7. Go to your TBMPSkeleton-info.plist file and replace the `GPGApplicationID` value with
  the actual Applicaton ID of your game.

That's it! Your application should be ready to run!  Give it a try, and add some word typing
excitement to your evening!

## Known issues

* We should probably add some icons and other supporting graphics.
* Right now, I reload the entire set of match data when in the refreshPendingGames call.
In reality, we should only do that when we make that call from a push notification.
