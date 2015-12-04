/*
 * Copyright 2015 The Android Open Source Project
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
package com.sample.helper;

import android.app.NativeActivity;
import android.widget.SeekBar;

/*
 * Java UI SeelBar implementation
 */
public class JUISeekBar extends SeekBar {
    private JUIForwardingPopupWindow dummyPopupWindow;

    public JUIForwardingPopupWindow getDummyWindow() {
        return dummyPopupWindow;
    }

    public JUISeekBar(final NativeActivity activity) {
        super(activity);
        setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
            public void onStopTrackingTouch(SeekBar arg0) {
                JUIHelper
                        .JUICallbackHandler(
                                getId(),
                                JUIHelper.JUICALLBACK_SEEKBAR_STOP_TRACKING_TOUCH,
                                0, 0);
            }

            public void onStartTrackingTouch(SeekBar arg0) {
                JUIHelper.JUICallbackHandler(getId(),
                        JUIHelper.JUICALLBACK_SEEKBAR_START_TRACKING_TOUCH, 0,
                        0);
            }

            public void onProgressChanged(SeekBar arg0, int arg1, boolean arg2) {
                JUIHelper.JUICallbackHandler(getId(),
                        JUIHelper.JUICALLBACK_SEEKBAR_PROGRESSCHANGED, arg1,
                        arg2 ? 1 : 0);
            }
        });

        dummyPopupWindow = new JUIForwardingPopupWindow(activity, this);
    }

    @Override
    protected void onLayout(boolean changed, int left, int top, int right,
            int bottom) {
        super.onLayout(changed, left, top, right, bottom);

        // Put dummy popupWindow over the control
        // so that relativeLayout can pass through touch events to native
        // activity for a background area
        if (changed) {
            dummyPopupWindow.update(this);
        }
    }
}