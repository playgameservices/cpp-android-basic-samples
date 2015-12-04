package com.sample.helper;

import android.app.NativeActivity;
import android.widget.RelativeLayout;

/*
 * Java UI RelativeLayout implementation
 */
public class JUIRelativeLayout extends RelativeLayout {
    public JUIForwardingPopupWindow getDummyWindow() {
        return null;
    }

    public JUIRelativeLayout(final NativeActivity activity) {
        super(activity);
    }
}