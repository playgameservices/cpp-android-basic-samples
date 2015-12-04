package com.sample.helper;

import android.app.NativeActivity;
import android.widget.RadioGroup;

//
//Java UI LinearLayout implementation
//
public class JUIRadioGroup extends RadioGroup {
	public JUIForwardingPopupWindow getDummyWindow() {
		return null;
	}

	public JUIRadioGroup(final NativeActivity activity) {
		super(activity);
	}
}