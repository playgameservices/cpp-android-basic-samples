Google Play Games C++ SDK Samples
=================================

Copyright (C) 2014 Google Inc.

<h2>Contents</h2>

These are the Android samples for Google Play game services.

* **samples-android** - a set of Android NDK samples

    * **CollectAllTheStarsNative**. Demonstrates how to use the Snapshots feature to save game data. The sample signs the user in, synchronizes their data from a named Snapshot, then updates the UI to reflect the game state saved in the Snapshot.

    * **TrivialQuestNative**. Demonstrates how to use the Events and Quests features of Google Play Services. The sample presents a sign in button and four buttons to simulate killing monsters in-game. When you click the buttons, an event is
created and sent to Google Play Games to track what the player is doing in game.

    * **minimalist**. Demostrate minimal usage of Google Play game services API.

    * **TbmpSkeletonNative** A trivial turn-based-multiplayer game.  In this thrilling game, you can invite many friends, then send a shared gamestate string back and forth until someone finishes, cancels, or the second-to-last player leaves.

    * **Teapots**. Demostrate usage of LeaderBoard and Achievement API.

* **samples-ios** - a set of iOS Native Client samples

    * **ButtonClicker**. Represents the new generation in modern button-clicking excitement. A simple multiplayer game sample that shows how to set up the Google Play real-time multiplayer API, invite friends, automatch, accept invitations, use the waiting room UI, send and receive messages and other multiplayer topics.

    * **CollectAllTheStars**. Demonstrates how to use the Snapshots feature to save game data. The sample signs the user in, synchronizes their data from a named Snapshot, then updates the UI to reflect the game state saved in the Snapshot.

    * **TrivialQuest**. Demonstrates how to use the Events and Quests features of Google Play Services. The sample presents a sign in button and four buttons to simulate killing monsters in-game. When you click the buttons, an event is
created and sent to Google Play Games to track what the player is doing in game.

    * **TypeANumber**. Shows leaderboards and achievements. In this exciting game, you type the score you think you deserve. But wait! There is a twist. If you are playing in easy mode, you get the score you requested. However, if you are playing in hard mode, you only get half! (tough game, we know).

   * **TbmpSkeleton** A trivial turn-based-multiplayer game.  In this thrilling game, you can invite many friends, then send a shared gamestate string back and forth until someone finishes, cancels, or the second-to-last player leaves.

**Note:** the samples that have corresponding counterparts for iOS and web (particularly, CollectAllTheStars and TypeANumber) are compatible across the platforms. This means that you can play some levels on CollectAllTheStars on your Android device, and then pick up your iOS device and continue where you left off! For TypeANumber, you will see your achievements and leaderboards on all platforms, and progress obtained on one will be reflected on the others.

<h2>How to run a sample</h2>
To use these samples, you will need the Play Games C++ SDK, which you
can download from:

[https://developers.google.com/games/services/downloads/](https://developers.google.com/games/services/downloads/)

Unzip the archive you download into ./gpg-cpp-sdk directory.

Then follow the instructions you can find here:

[https://developers.google.com/games/services/cpp/GettingStartedNativeClient](https://developers.google.com/games/services/cpp/GettingStartedNativeClient)

<h2>Acknowledgement</h2>
Some of samples are using OpenSource projects below,
JASONCPP: https://github.com/open-source-parsers/jsoncpp
