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