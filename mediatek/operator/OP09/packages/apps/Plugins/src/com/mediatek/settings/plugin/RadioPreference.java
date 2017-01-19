package com.mediatek.settings.plugin;

import android.content.Context;
import android.preference.Preference;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.view.View;
import android.widget.RadioButton;

import com.mediatek.op09.plugin.R;
import com.mediatek.xlog.Xlog;

/**
 * Used in ManualNetworkSelection to show GSM available network as a list
 * 
 */
public class RadioPreference extends Preference {

    private static final String TAG = "RadioButtonPreference";

    private String mTitle;
    private String mSummary;
    private RadioButton mRadioButton;
    private boolean mIsChecked;

    private String mCarrierNumeric;

    public RadioPreference(Context context) {
        this(context, "title", "summary");
    }

    public RadioPreference(Context context, AttributeSet attrs) {
        super(context, attrs);
        setLayoutResource(R.layout.carrier_radio_preference);
    }

    public RadioPreference(Context context, String title, String summary) {
        this(context, title, summary, false);
    }

    public RadioPreference(Context context, String title, String summary, boolean isChecked) {
        super(context);
        mTitle = title;
        mSummary = summary;
        mIsChecked = isChecked;
        setLayoutResource(R.layout.carrier_radio_preference);

        if (!TextUtils.isEmpty(mTitle)) {
            setTitle(mTitle);
        }
        if (mSummary != null) {
            setSummary(mSummary);
        }
    }

    @Override
    protected void onBindView(View view) {
        super.onBindView(view);
        mRadioButton = (RadioButton)view.findViewById(R.id.radiobutton);
        if (mRadioButton != null) {
            mRadioButton.setChecked(mIsChecked);
        } else {
            Xlog.d(TAG, "radio button can't be find");
        }
    }

    /**
     * set Checked
     * @param newCheckStatus boolean
     */
    public void setChecked(boolean newCheckStatus) {
        mIsChecked = newCheckStatus;
        notifyChanged();
    }
}
