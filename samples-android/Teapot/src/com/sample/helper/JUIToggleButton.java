package com.sample.helper;

import android.app.NativeActivity;
import android.widget.CompoundButton;
import android.widget.ToggleButton;

//
//Java UI ToggleButton implementation
//
public class JUIToggleButton extends ToggleButton {
	private JUIForwardingPopupWindow dummyPopupWindow;

	public JUIForwardingPopupWindow getDummyWindow() {
		return dummyPopupWindow;
	}

	public JUIToggleButton(final NativeActivity activity) {
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