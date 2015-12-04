package com.sample.helper;

import android.app.NativeActivity;
import android.widget.CheckedTextView;

/*
 * Java UI CheckedTextView implementation
 */
public class JUICheckedTextView extends CheckedTextView {
    private JUIForwardingPopupWindow dummyPopupWindow;

    public JUIForwardingPopupWindow getDummyWindow() {
        return dummyPopupWindow;
    }

    public JUICheckedTextView(final NativeActivity activity) {
        super(activity);

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