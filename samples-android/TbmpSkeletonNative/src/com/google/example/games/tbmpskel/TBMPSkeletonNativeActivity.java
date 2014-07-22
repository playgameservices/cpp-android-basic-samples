/*
 * Copyright 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.google.example.games.tbmpskel;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.NativeActivity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;

public class TBMPSkeletonNativeActivity extends NativeActivity {
    // Load SO
    static {
        System.load("libTBMPSkeletonNativeActivity.so");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // Hide toolbar
        int SDK_INT = android.os.Build.VERSION.SDK_INT;
        if (SDK_INT >= 19) {
            setImmersiveSticky();

            View decorView = getWindow().getDecorView();
            decorView
                    .setOnSystemUiVisibilityChangeListener(new View.OnSystemUiVisibilityChangeListener() {
                        @Override
                        public void onSystemUiVisibilityChange(int visibility) {
                            setImmersiveSticky();
                        }
                    });
        }

        nativeOnActivityCreated(this, savedInstanceState);
    }

    @SuppressLint("InlinedApi")
    @SuppressWarnings("deprecation")
    protected void onResume() {
        super.onResume();

        // Hide toolbar
        int SDK_INT = android.os.Build.VERSION.SDK_INT;
        if (SDK_INT >= 11 && SDK_INT < 14) {
            getWindow().getDecorView().setSystemUiVisibility(
                    View.STATUS_BAR_HIDDEN);
        } else if (SDK_INT >= 14 && SDK_INT < 19) {
            getWindow().getDecorView().setSystemUiVisibility(
                    View.SYSTEM_UI_FLAG_FULLSCREEN
                            | View.SYSTEM_UI_FLAG_LOW_PROFILE);
        } else if (SDK_INT >= 19) {
            setImmersiveSticky();
        }

        nativeOnActivityResumed(this);
    }

    @SuppressLint("InlinedApi")
    void setImmersiveSticky() {
        View decorView = getWindow().getDecorView();
        decorView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
    }

    protected void onPause() {
        super.onPause();
        // This call is to suppress 'E/WindowManager():
        // android.view.WindowLeaked...' errors.
        // Since orientation change events in NativeActivity comes later than
        // expected, we can not dismiss
        // popupWindow gracefully from NativeActivity.
        // So we are releasing popupWindows explicitly triggered from Java
        // callback through JNI call.
        OnPauseHandler();

        nativeOnActivityPaused(this);
    }

    protected void onDestroy() {
        super.onDestroy();
        nativeOnActivityDestroyed(this);
    }

    protected void onStart() {
        super.onStart();
        nativeOnActivityStarted(this);
    }

    protected void onStop() {
        super.onStop();
        nativeOnActivityStopped(this);
    }

    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        nativeOnActivitySaveInstanceState(this, outState);
    }

    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        nativeOnActivityResult(this, requestCode,resultCode, data);
    }

    // Implemented in C++.
    public static native void nativeOnActivityResult(Activity activity,
            int requestCode, int resultCode, Intent data);

    public static native void nativeOnActivityCreated(Activity activity,
            Bundle savedInstanceState);

    private static native void nativeOnActivityDestroyed(Activity activity);

    private static native void nativeOnActivityPaused(Activity activity);

    private static native void nativeOnActivityResumed(Activity activity);

    private static native void nativeOnActivitySaveInstanceState(
            Activity activity, Bundle outState);

    private static native void nativeOnActivityStarted(Activity activity);

    private static native void nativeOnActivityStopped(Activity activity);

    native public void OnPauseHandler();
}
