Google Play game services - CollectAllTheStarsNative C++ Samples
===========================================
Copyright (C) 2013 Google Inc.

<h2>Contents</h2>

CollectAllTheStarsNative: The sample demonstrates how to use Snapshot feature with C++ code with Google Play game services native SDK.

<h2>How to run a sample</h2>

1. Set up the project in Developer Console. For more info:

      https://developers.google.com/games/services/console/enabling
 
   Note your package name and the APP ID of the project.

1. Enable  Turn-based multiplayer setting in Saved Games
   (see the ones that the sample needs in its res/values/ids.xml)

<h3>Modify IDs, compile and run</h3>
1. In Android Studio toolbar, select CollectAllTheStars confituration to be the active project
1. In build.gradle(Module:collectAlltheStars), change applicationId to be your own package name
   (the same one you registered in Developer Console!).
1. Modify res/values/strings.xml and place your game ID there, as assigned by the
   Play Developer Console (create the leaderboards and achievements necessary for
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
