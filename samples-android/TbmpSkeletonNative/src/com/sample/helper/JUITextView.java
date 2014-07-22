package com.sample.helper;

import android.app.NativeActivity;
import android.widget.TextView;

/*
 * Java UI TextView implementation
 */
public class JUITextView extends TextView {
    public JUITextView(final NativeActivity activity) {
        super(activity);
    }

    public JUIForwardingPopupWindow getDummyWindow() {
        return null;
    }

}