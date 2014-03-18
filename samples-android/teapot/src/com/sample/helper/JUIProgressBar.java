package com.sample.helper;

import android.app.NativeActivity;
import android.widget.ProgressBar;

/*
 * Java UI ProgressBar implementation
 */
public class JUIProgressBar extends ProgressBar {
    private JUIForwardingPopupWindow dummyPopupWindow;

    public JUIForwardingPopupWindow getDummyWindow() {
        return dummyPopupWindow;
    }

    public JUIProgressBar(final NativeActivity activity) {
        super(activity);
        dummyPopupWindow = new JUIForwardingPopupWindow(activity, this);
    }

    public JUIProgressBar(final NativeActivity activity, int param ) {
        super(activity, null, param );
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