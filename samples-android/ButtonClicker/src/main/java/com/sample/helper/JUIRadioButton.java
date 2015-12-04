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

import android.annotation.TargetApi;
import android.app.NativeActivity;
import android.os.Build;
import android.widget.CompoundButton;
import android.widget.RadioButton;

/*
 * Java UI RadioButton implementation
 */
public class JUIRadioButton extends RadioButton {
    private JUIForwardingPopupWindow dummyPopupWindow;

    @TargetApi(Build.VERSION_CODES.ICE_CREAM_SANDWICH)
    public JUIForwardingPopupWindow getDummyWindow() {
        return dummyPopupWindow;
    }

    public JUIRadioButton(final NativeActivity activity) {
        super(activity);

        setOnCheckedChangeListener(new OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView,
                    boolean isChecked) {
                JUIHelper.JUICallbackHandler(getId(),
                        JUIHelper.JUICALLBACK_COMPOUNDBUTTON_CHECKEDCHANGED,
                        isChecked ? 1 : 0, 0);
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