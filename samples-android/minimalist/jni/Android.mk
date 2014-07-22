# Copyright (C) 2010 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/.. $(LOCAL_PATH)/../../../gpg-cpp-sdk/android/include

LOCAL_MODULE    := native-activity
LOCAL_SRC_FILES := main.cpp StateManager.cpp
LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv1_CM -lz
LOCAL_LDFLAGS += -L$(LOCAL_PATH)/../../../gpg-cpp-sdk/android/lib/$(TARGET_ARCH_ABI)
LOCAL_LDFLAGS += -L$(NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.8/libs/$(TARGET_ARCH_ABI) -lgnustl_static
LOCAL_STATIC_LIBRARIES := android_native_app_glue gpg-1

include $(BUILD_SHARED_LIBRARY)

$(call import-add-path,$(LOCAL_PATH)/../../..)
$(call import-module,gpg-cpp-sdk/android)
$(call import-module,android/native_app_glue)
