LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := NativeGameActivity
LOCAL_SRC_FILES := NativeGameActivity.cpp \
TeapotRenderer.cpp \
StateManager.cpp \
NDKHelper/GLContext.cpp \
NDKHelper/gestureDetector.cpp \
NDKHelper/perfMonitor.cpp	\
NDKHelper/shader.cpp \
NDKHelper/vecmath.cpp \
NDKHelper/JNIHelper.cpp \
NDKHelper/interpolator.cpp \
NDKHelper/sensorManager.cpp \
NDKHelper/tapCamera.cpp \
NDKHelper/gl3stub.c \
JavaUIHelper/JavaUI.cpp \
JavaUIHelper/JavaUI_Layouts.cpp \
JavaUIHelper/JavaUI_Window.cpp	\
JavaUIHelper/JavaUI_Dialog.cpp	\
JavaUIHelper/JavaUI_Toast.cpp


LOCAL_C_INCLUDES := $(LOCAL_PATH)/.. $(LOCAL_PATH)/NDKHelper/ $(LOCAL_PATH)/JavaUIHelper/ $(LOCAL_PATH)/../../../gpg-cpp-sdk/android/include/

LOCAL_CFLAGS :=

LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv2 -lz
LOCAL_STATIC_LIBRARIES := cpufeatures android_native_app_glue ndk_helper

ifneq ($(filter %armeabi-v7a,$(TARGET_ARCH_ABI)),)
LOCAL_CFLAGS += -mhard-float -D_NDK_MATH_NO_SOFTFP=1
LOCAL_LDLIBS += -lm_hard
ifeq (,$(filter -fuse-ld=mcld,$(APP_LDFLAGS) $(LOCAL_LDFLAGS)))
LOCAL_LDFLAGS += -Wl,--no-warn-mismatch
endif
endif

LOCAL_LDFLAGS += -L$(LOCAL_PATH)/../../../gpg-cpp-sdk/android/lib/$(TARGET_ARCH_ABI) -lgpg
LOCAL_LDFLAGS += -L$(NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.8/libs/$(TARGET_ARCH_ABI) -lgnustl_static


include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
$(call import-module,android/cpufeatures)

