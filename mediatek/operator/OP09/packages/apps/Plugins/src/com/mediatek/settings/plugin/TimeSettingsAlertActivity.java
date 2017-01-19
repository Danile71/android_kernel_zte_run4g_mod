package com.mediatek.settings.plugin;


import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.provider.Settings;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.RadioButton;

import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;

import com.mediatek.op09.plugin.R;
import com.mediatek.xlog.Xlog;

public class TimeSettingsAlertActivity extends AlertActivity implements OnClickListener,DialogInterface.OnClickListener {
    private static final String TAG = "TimeSettingsAlertActivity";
    private RadioButton mRadioBtn1 = null;
    private RadioButton mRadioBtn2 = null;
    //Test Current time settings:
    private int mCurrentTime = 0;
    private int mSelectTimeMode = 0;
    private static final int BEIJING_MODE = 0;
    private static final int LOCAL_MODE = 1;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        createDialog();
    }

    void createDialog() {
        final AlertController.AlertParams p = mAlertParams;
        p.mTitle = getString(R.string.reset_to_default_title);
        View view = getLayoutInflater().inflate(R.layout.time_display_mode, null);
        p.mView = view;
        mRadioBtn1 = (RadioButton)view.findViewById(R.id.beijing_radio);
        mRadioBtn2 = (RadioButton)view.findViewById(R.id.local_radio);
        
        mRadioBtn1.setOnClickListener(this);
        mRadioBtn2.setOnClickListener(this);
        // init radio button:
        mCurrentTime = Settings.System.getInt(getContentResolver(), Settings.System.CT_TIME_DISPLAY_MODE, BEIJING_MODE);
        Xlog.i(TAG, "get ct init time display mode:" + mCurrentTime);
        if (mCurrentTime == BEIJING_MODE) {
            mRadioBtn1.setChecked(true);
            mRadioBtn2.setChecked(false);
        } else if (mCurrentTime == LOCAL_MODE) {
            mRadioBtn1.setChecked(false);
            mRadioBtn2.setChecked(true);
        }
        p.mPositiveButtonText = getString(android.R.string.yes);
        p.mPositiveButtonListener = this;
        p.mNegativeButtonText = getString(android.R.string.no);
        p.mNegativeButtonListener = this;
        setupAlert();
    }
    
    @Override
    public void onClick(View view) {
        switch(view.getId()) {
            case R.id.beijing_radio:
                Xlog.d(TAG,"onClick -- beijing_radio");
                mRadioBtn1.setChecked(true);
                mRadioBtn2.setChecked(false);
                mCurrentTime = BEIJING_MODE;
                break;
            case R.id.local_radio:
                Xlog.d(TAG,"onClick -- local_radio");
                mRadioBtn1.setChecked(false);
                mRadioBtn2.setChecked(true);
                mCurrentTime = LOCAL_MODE;
                break;
            default:
                break;
        }
    }
    public void onClick(DialogInterface dialog, int which) {
        if (which == BUTTON_POSITIVE) {
            Xlog.d(TAG, "Pressed OK -- mCurrentTime = " + mCurrentTime);
            Settings.System.putInt(getContentResolver(), Settings.System.CT_TIME_DISPLAY_MODE, mCurrentTime);
            // Send broadcast that Call App can receive it.
            Intent intent = new Intent("com.mediatek.ct.TIME_DISPLAY_MODE");
            intent.putExtra("time_display_mode", mCurrentTime);
            sendBroadcast(intent);
        } else if (which == BUTTON_NEGATIVE) {
            Xlog.d(TAG, "Pressed Cancel");
        }
    }
}
