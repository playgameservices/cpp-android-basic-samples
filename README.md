Google Play Games C++ SDK Samples for Android
=============================================

Copyright (C) 2014 Google Inc.

<h2>Contents</h2>

These samples illustrate how to use Google Play Game Services with your Android game.

* **CollectAllTheStarsNative**: Demonstrates how to use the Saved Games feature to save game data. The sample signs the user in, synchronizes their data from a named game save, and then updates the UI to reflect the saved game state.

* **TrivialQuestNative**: Demonstrates how to use the Events and Quests features of Google Play Game Services. The sample displays a sign-in button and four buttons to simulate killing monsters. Clicking a button generates an event, and sends it to Google Play Game Services to track what the player is doing in the game.

* **minimalist**: Demonstrates a minimal use case for the Google Play Game Services API.

* **TbmpSkeletonNative**: A trivial, turn-based multiplayer game.  Many players can play together in this thrilling game, in which they send a shared gamestate string back and forth until someone finishes or cancels, or the second-to-last player leaves. Be the last one standing!

* **Button-Clicker**: Demonstrates real-time multiplayer using invites or quickmatch

* **Teapots**: Demonstrates use of the Leaderboard and Achievement APIs.

**Note:** In samples with corresponding counterparts for iOS and Web (particularly, CollectAllTheStars and TypeANumber), the player can play a game seamlessly across phones of different platforms. For example, you can play some levels of CollectAllTheStars on your Android device, and then pick up your iOS device and continue where you left off! TypeANumber shows your achievements and leaderboards on all platforms; when you make progress on one platform, that progress is reflected on the other devices, as well.

<h2>How to run a sample</h2>
To use these samples, you need the Google Play Game Services C++ SDK, which you
can [download from here](https://developers.google.com/games/services/downloads/).

After downloading the archive, unzip it to the  `./gpg-cpp-sdk` directory. Then, follow [these directions](https://developers.google.com/games/services/cpp/GettingStartedNativeClient).
<h2> Other resources </h2>
* [Google Android Vulkan Tutorials](https://github.com/ggfan/android-vulkan-tutorials)
* [Android Vulkan API Basic Samples](https://github.com/googlesamples/vulkan-basic-samples)
* [Google Android NDK Samples](https://github.com/googlesamples/android-ndk)

<h2>Acknowledgment</h2>
Some of these samples use the following open-source project:
JASONCPP: https://github.com/open-source-parsers/jsoncpp
