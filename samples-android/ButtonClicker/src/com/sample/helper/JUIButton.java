package com.sample.helper;

import android.app.NativeActivity;
import android.graphics.drawable.Drawable;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;

//
//Java UI SeekBar implementation
//
public class JUIButton extends Button {
    private JUIForwardingPopupWindow dummyPopupWindow;

    public JUIForwardingPopupWindow getDummyWindow() {
        return dummyPopupWindow;
    }

    public JUIButton(final NativeActivity activity) {
        super(activity, null, android.R.attr.buttonStyle);

        final Drawable d = getBackground();

        setOnTouchListener(new View.OnTouchListener() {

            @Override
            public boolean onTouch(View v, MotionEvent event) {
                Log.i("test", "clicked, action" + event.getAction());
                switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    // setPressed(true);
                    d.setState(PRESSED_ENABLED_STATE_SET);

                    JUIHelper.JUICallbackHandler(getId(),
                            JUIHelper.JUICALLBACK_BUTTON_DOWN, 0, 0);
                    return true;
                    // break;
                case MotionEvent.ACTION_CANCEL:
                    JUIHelper.JUICallbackHandler(getId(),
                            JUIHelper.JUICALLBACK_BUTTON_CANCEL, 0, 0);
                    return true;
                case MotionEvent.ACTION_UP:
                    JUIHelper.JUICallbackHandler(getId(),
                            JUIHelper.JUICALLBACK_BUTTON_UP, 0, 0);
                    d.setState(ENABLED_STATE_SET);

                    // setPressed(false);
                    // break;
                    return true;
                case MotionEvent.ACTION_MOVE:
                    d.setState(PRESSED_ENABLED_STATE_SET);
                    return true;

                }

                return false;
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