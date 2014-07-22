Google Play game services - TbmpSkeletonNative C++ Samples
===========================================
Copyright (C) 2013 Google Inc.

<h2>Contents</h2>

TbmpSkeletonNative: The sample demonstrates how to use turn-based multiplayer gaming feature with C++ code with Google Play game services native SDK.
A trivial turn-based-multiplayer game.  In this thrilling game, you can invite many friends, then send a shared gamestate string back and forth until someone finishes, cancels, or the second-to-last player leaves. The sample is compatible to Android, iOS SkeletonTbmp sample, you can enjoy cross platform play between them.

<h2>How to run a sample</h2>

1. Set up the project in Developer Console. For more info:

      https://developers.google.com/games/services/console/enabling
 
   Note your package name and the APP ID of the project.

1. Enable  Turn-based multiplayer setting in MULTIPLAYER SETTINGS
   (see the ones that the sample needs in its res/values/ids.xml)

<h3>Eclipse settings</h3>

1. Start Eclipse
1. Import the Google Play Services library project (available for download through the SDK manager):
    1. Click **File | Import | Android | Existing Android Code into Workspace**
    1. Select `SDK/extras/google/google_play_services/google_play_services_lib` (where `SDK` stands for the path where you installed your Android SDK)
    1. Click **Finish**
1. Import the sample
1. Go into the project properties window for that project (right-click, **Properties**) and check that this project has a reference to the `google_play_services_lib` project.

Your project should now compile. However, don't run it yet, since you still need to adjust your game's IDs
in order for the sample(s) to work.

Now jump to the *Modify IDs, compile and run* section and continue to follow the instructions there.

<h3>Modify IDs, compile and run</h3>

To set up a sample:

1. Change the package name from com.google.example.games.\* to your own package name
   (the same one you registered in Developer Console!). To do that, open **AndroidManifest.xml** and put
   your package name in the "package" attribute of the **manifest** tag. You will need to
   fix some of the references (particularly to the generated R class) because of the package name
   change. Ctrl+Shift+O in Eclipse (and Alt+Enter in Android Studio) should take care of most of the work.
1. Modify res/values/strings.xml and place your IDs there, as given by the
   Developer Console (create the leaderboards and achievements necessary for
   the sample, if any). Remember that the App ID is only the *numerical* portion
   of your client ID, so use `123456789012` and not `123456789012.apps.gooogleusercontent.com`.
1. Compile and run.

IMPORTANT: make sure to sign your apk with the same certificate
as the one whose fingerprint you configured on Developer Console, otherwise
you will see errors.

IMPORTANT: if you are testing an unpublished game, make sure that the account you intend
to sign in with (the account on the test device) is listed as a tester in the
project on your Developer Console setup (check the list in the "Testing"
section), otherwise the server will act as though your project did not exist and
return errors.

<h2>Support</h2>

First of all, take a look at our [troubleshooting guide](https://developers.google.com/games/services/android/troubleshooting). Most setup issues can be solved by following this guide.

If your question is not answered by the troubleshooting guide, we encourage you to post your question to [stackoverflow.com](stackoverflow.com). Our team answers questions there reguarly.
