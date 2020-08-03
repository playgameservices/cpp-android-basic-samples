Android Google Play Games Native Samples
=======================================
These samples illustrate how to use Google Play Game Services with your Android game.

* **CollectAllTheStarsNative**: Demonstrates how to use the Saved Games feature to save game data. The sample signs the user in, synchronizes their data from a named game save, and then updates the UI to reflect the saved game state.

* **minimalist**: Demonstrates a minimal use case for the Google Play Game Services API.

* **Teapots**: Demonstrates use of the Leaderboard and Achievement APIs.

Pre-requisites
--------------
1. Change the applicationId inside each individual samples build.gradle to your own package name
2. Modify res/values/ids.xml and place your IDs there, as given by the
   Developer Console (create the leaderboards and achievements necessary for
   the sample, if any). In the Developer console, select a resource type
   (Achievements, Events, Leaderboards) and click "Get Resources".  Copy the
    contents from the console and replace the contents of res/values/ids.xml.

Build with Android Studio
-------------------------
This project requires Android Studio 1.5+.
1. Android SDK version r10e or eariler.
2. Launch Android Studio.
3. Import the project by selecting File > New > Import Project and select
        samples-android/build.gradle.
4. Edit the local.properties file (which is created by Android Studio) and
        set the ndk.dir property to the location of your r10e or eariler NDK.
5. Change applicationId in the sample app build.gradle to your own package name
        that matches the configuration in the Play Game developer console.
6. Replace res/values/ids.xml contents with your values from the developer console.
7. Run the select project. Android Studio will compile, load and start your project on your device

All projects could be built at one shot from Android Studio menu "Build" -> "Clean Project"/"Rebuild Project"
you could also build with Gradle on Terminal/Command line

Build using Gradle on OS X or Linux
-----------------------------------
1. Install Android Studio
1. Set the path to the Android SDK

    export ANDROID_HOME=~/Library/Android/sdk

1. Add the SDK to your path

    export PATH=$PATH;$ANDROID_HOME/tools;$ANDROID_HOME/platform-tools

1. Set the path to the Android NDK

    export ANDROID_NDK_HOME=<PATH_TO_NDK>/android-ndk-r10e

1. Execute the build script

    ./gradlew assemble

Build on Windows using Gradle
-----------------------------
1. Install Android Studio
1. Set the path to the Android SDK

    set ANDROID_HOME=C:\Users\<yourusername>\AppData\Local\Android\sdk

1. Add the SDK to your path

    set PATH=%PATH%;%ANDROID_HOME%\tools;%ANDROID_HOME%\platform-tools

1. Set the path to the Android NDK

    set ANDROID_NDK_HOME=C:\Users\<yourusername>\Desktop\android-ndk-r10e

1. (Optional) On some versions of Windows it is helpful to map the samples folder to a shortened path.

    subst G: C:\<full-path-to-sample-folder>

1. Execute the build script

    G:
    gradlew.bat assemble

Support
-------
If you've found an error in these samples, please [file an issue](https://github.com/playgameservices/cpp-android-basic-samples/issues/new).

Patches are encouraged, and may be submitted by [forking this project](https://github.com/playgameservices/cpp-android-basic-samples/fork) and
submitting a pull request through GitHub. Please see [CONTRIBUTING.md](CONTRIBUTING.md) for more details.

- [Stack Overflow](http://stackoverflow.com/questions/tagged/google-play-games)
- [Android Tools Feedbacks](http://tools.android.com/feedback)


License
-------
Copyright 2015 Google, Inc.

Licensed to the Apache Software Foundation (ASF) under one or more contributor
license agreements.  See the NOTICE file distributed with this work for
additional information regarding copyright ownership.  The ASF licenses this
file to you under the Apache License, Version 2.0 (the "License"); you may not
use this file except in compliance with the License.  You may obtain a copy of
the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
License for the specific language governing permissions and limitations under
the License.
