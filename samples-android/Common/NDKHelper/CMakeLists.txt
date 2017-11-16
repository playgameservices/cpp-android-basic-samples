# Copyright (C) 2017 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##

# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_VERBOSE_MAKEFILE on)

project(ndkhelper)

IF (NOT TARGET ndkhelper)
  add_library(ndkhelper STATIC
        src/main/cpp/gestureDetector.cpp
        src/main/cpp/gl3stub.cpp
        src/main/cpp/GLContext.cpp
        src/main/cpp/interpolator.cpp
        src/main/cpp/JNIHelper.cpp
        src/main/cpp/perfMonitor.cpp
        src/main/cpp/sensorManager.cpp
        src/main/cpp/shader.cpp
        src/main/cpp/tapCamera.cpp
        src/main/cpp/vecmath.cpp
  )

  target_include_directories(ndkhelper PRIVATE
        ${NDK_HELPER_PATH}
        ${ANDROID_NDK}/sources/android/native_app_glue
  )

  set_target_properties(ndkhelper PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY
        "${CMAKE_CURRENT_SOURCE_DIR}/libs/${ANDROID_ABI}"
  )

ENDIF(NOT TARGET ndkhelper)
