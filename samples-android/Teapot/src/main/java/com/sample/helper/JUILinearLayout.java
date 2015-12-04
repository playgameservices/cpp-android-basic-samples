package com.sample.helper;

import android.app.NativeActivity;
import android.widget.LinearLayout;

/*
 * Java UI LinearLayout implementation
 */
public class JUILinearLayout extends LinearLayout {
    public JUIForwardingPopupWindow getDummyWindow() {
        return null;
    }

    public JUILinearLayout(final NativeActivity activity) {
        super(activity);
    }
}