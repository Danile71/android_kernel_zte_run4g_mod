package com.mediatek.settings.plugin;

import android.content.Context;
import android.preference.CheckBoxPreference;
import android.provider.Settings;
import android.util.AttributeSet;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ImageView;
import android.widget.TextView;

import com.mediatek.op09.plugin.R;
import com.mediatek.xlog.Xlog;

/**
 * 
 * 
 * Description:
 * <p>
 * Customization for CT,Power Saving Mode
 * <p>
 * <p>
 * This preference have a image view as a button to set related params.
 * <p>
 * 
 */
public class PowerSavingSettingPref extends CheckBoxPreference implements OnClickListener {

    private static final String TAG = "Power Savings";
    private static final float DISABLED_ALPHA = 0.4f;
    private static final String KEY_SWITCH = "power_saving_switch";
    /** ImageView as a button for setting */
    private ImageView mSettingsButton;
    private TextView mSummaryText;
    private OnClickListener mImageViewClickLister;
    private Context mContext;

    public PowerSavingSettingPref(Context context, AttributeSet attrs) {
        super(context, attrs);
        mContext = context;
        setLayoutResource(R.layout.power_saving_item);
        // setWidgetLayoutResource(R.layout.preference_inputmethod_widget);
        Xlog.d(TAG, "PowerSavingSettingPref  new attrs");
    }

    /**
     * Binds the created View to the data for this Preference
     */
    @Override
    protected void onBindView(View view) {
        super.onBindView(view);
        View powerSavingItem = view.findViewById(R.id.power_saving_pref);
        powerSavingItem.setOnClickListener(this);
        mSettingsButton = (ImageView) view.findViewById(R.id.power_saving_settings);
        mSettingsButton.setOnClickListener(mImageViewClickLister);
        mSummaryText = (TextView) view.findViewById(android.R.id.summary);
        enableSettingsButton();
    }

    /**
     * @param imageViewClickLister
     *            The listener of setting button
     */
    public void setImageViewClickLister(OnClickListener imageViewClickLister) {
        this.mImageViewClickLister = imageViewClickLister;
    }

    /**
     * The check box is checked
     */
    private void onCheckBoxClicked() {
        boolean isItemChecked = isChecked();
        /** Disable/enable setting view */
        mSettingsButton.setClickable(!isItemChecked);
        setChecked(!isItemChecked);
        if (!isItemChecked) {
            /** Turn off/on related functions though current key */
            PowerSavingManager.getInstance(getContext()).turnOnPowerSavingMode(getKey());
        }
    }

    /**
     * Disable/enable settings button
     */
    private void enableSettingsButton() {
        if (mSettingsButton != null) {
            // When power savings mode is disable, disable the settings button
            final boolean powerSavingSwitched = 
                Settings.System.getInt(mContext.getContentResolver(), PowerSavingManager.CT_POWER_SAVING_MODE, 0) == 1;
            final boolean checked = isChecked() && powerSavingSwitched;
            mSettingsButton.setEnabled(checked);
            mSettingsButton.setClickable(checked);
            mSettingsButton.setFocusable(checked);
            if (!checked) {
                mSettingsButton.setAlpha(DISABLED_ALPHA);
            }
        }
    }

    public void onClick(View v) {
        // TODO Auto-generated method stub
        if (!isEnabled()) {
            return;
        }
        onCheckBoxClicked();
    }
}
