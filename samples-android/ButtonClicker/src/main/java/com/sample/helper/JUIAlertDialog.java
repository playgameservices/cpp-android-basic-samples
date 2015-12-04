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

import android.view.View;
import android.widget.RelativeLayout;
import android.app.AlertDialog;
import android.app.NativeActivity;
import android.content.DialogInterface;

/*
 * Java UI AlertDialog implementation
 * Helper functions for jui_helper
 */
public class JUIAlertDialog extends AlertDialog {

    NativeActivity activity_;
    RelativeLayout dialogRelativeLayout_;
    int id_ = 0;

    public JUIAlertDialog(final NativeActivity act) {
        super(act);
        activity_ = act;

        // Setup relative layout
        dialogRelativeLayout_ = new RelativeLayout(activity_);
        setView(dialogRelativeLayout_);

        setOnDismissListener(new OnDismissListener() {
            @Override
            public void onDismiss(DialogInterface dialog) {
                if (id_ != 0)
                    JUIHelper.JUICallbackHandler(id_,
                            JUIHelper.JUICALLBACK_DIALOG_DISMISSED, 0, 0);
            }
        });

        setOnCancelListener(new OnCancelListener() {
            @Override
            public void onCancel(DialogInterface dialog) {
                if (id_ != 0)
                    JUIHelper.JUICallbackHandler(id_,
                            JUIHelper.JUICALLBACK_DIALOG_CANCELLED, 0, 0);
            }
        });
    }

    public void setID(int id) {
        id_ = id;
    }

    public void addView(final View view) {
        activity_.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (dialogRelativeLayout_ != null)
                    dialogRelativeLayout_.addView(view);
            }
        });
        return;
    }

    private OnClickListener listener = new OnClickListener() {
        @Override
        public void onClick(DialogInterface dialog, int which) {
            if (id_ != 0)
                JUIHelper.JUICallbackHandler(id_,
                        JUIHelper.JUICALLBACK_BUTTON_UP, which, 0);
        }
    };

    public void setButtonPositive(CharSequence text) {
        setButton(BUTTON_POSITIVE, text, listener);
    }

    public void setButtonNegative(CharSequence text) {
        setButton(BUTTON_NEGATIVE, text, listener);
    }

    public void setButtonNeutral(CharSequence text) {
        setButton(BUTTON_NEUTRAL, text, listener);
    }

}