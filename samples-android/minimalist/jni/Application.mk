APP_STL := gnustl_static
APP_CPPFLAGS := -frtti -DCC_ENABLE_CHIPMUNK_INTEGRATION=1 -std=c++11 -Wno-deprecated-declarations -Wno-multichar -Wno-literal-suffix
APP_ABI := armeabi armeabi-v7a x86
NDK_TOOLCHAIN_VERSION := 4.8
