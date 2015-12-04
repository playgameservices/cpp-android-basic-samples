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