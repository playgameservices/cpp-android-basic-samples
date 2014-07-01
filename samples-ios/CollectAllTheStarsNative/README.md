# Collect All the Stars!

A sample application that demonstrates loading and saving to the cloud, as well
as handling conflicts. Should work cross-platform with the Android version of
the game.
This sample is using C++ NativeClient SDK with conjunction of ObjectiveC++.

## Code

Collect All the Stars consists of a number of files that might be of interest to
you:

* `GCATAppDelegate` contains a little code required to handle sign-in (but not
  much -- just the URL handler).

* `GCATConstants` contains our game's client ID

* `GCATGameModel` is the game's model. This is where all the interesting logic
  is to load and save game state from the cloud.

* `GCATInventory` is a helper class that helps keep track of the player's star
  level, and prints it out in a JSON format that can be easily stored / read
  from the cloud.

* `GCATViewController_iPhone.storyboard` is the main storyboard used by the
  application

* `ViewController` is the ViewController for the game itself. It contains much
  of the logic to display the stars, sign the user in, and hide and show various
  buttons.

## A note on the cloud save conflict resolution logic

This test application was designed to demonstrate a typical use of cloud save:
keeping track of the number of stars earned in a level, and making sure a user
doesn't lose progress from one device to the next. In most mobile games, a user
can only gain stars in a level; they'll never lose any. The conflict resolution
logic is built around this assumption. In the case where the same level from two
data sets has different numbers of stars, the larger number always wins.

That being said, this is a test application, so we do allow you to save a
smaller number of stars than what the user had before. The conflict resolution
logic doesn't really take this into account.

Also, note that in a real game, you should attempt to load and save cloud save
data without requiring an explicit action from your user.

## Running the sample application

To run Collect All the Stars on your own device, you will need to create your
own version of the game in the Play Console. Once you have done that, you will
copy over your client ID to your own `Constants.h` file. To follow this process,
perform the following steps:

1. Open up your CollectAllTheStars project settings. Select the "CollectAllTheStars" target and,
  on the "Summary" tab, change the Bundle Identifier to
  something appropriate for your Provisioning Profile. (It will probably look like
  com.<your_company>.CollectAllTheStars)
    * If you plan on only running this on an emulator, you can leave it as-is.
2. Click the "Info" tab and go down to the bottom where you see "URL Types". Expand
  this and change the "Identifier" and "URL Schemes" from `com.example.CollectAllTheStars` to
  whatever you used in Step 1.
3. If you have already created this application in the Play Console (because you
  have created the Android or web version of the game, for example), you can
  skip steps 4 and 5 below. All you will need to do is...
    * Link the iOS version of your game, as described in the "Link Your Platform-
      Specific Apps" section of the console documentation
    * Create a separate client ID for the iOS version of the game, as described in
      the "Create a client ID" section of the [Console Documentation](https://developers.google.com/games/services/console/enabling).
        * Use the Bundle ID that you created in Step 1.
4. Create your own application in the Play Console, as described in our [Developer
  Documentation](https://developers.google.com/games/services/console/enabling). Make
  sure you follow the "iOS" instructions for creating your client ID and linking
  your application.
    * Again, you will be using the Bundle ID that you created in Step 1.
    * You can leave your App Store ID blank for testing purposes.
5. Make a note of your client ID and application ID as described in the
  documentation
6. Once that's done, open up your `Constants.h` file, and replace the `CLIENT_ID` value
  with your own OAuth2.0 client ID.

That's it! Your application should be ready to run! 

## Troubleshooting / Known Issues

* This ain't the prettiest game you've ever played.
