package com.mediatek.phone.plugin.callsetting;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.preference.Preference;
import android.preference.PreferenceScreen;

import com.mediatek.op09.plugin.R;
import com.mediatek.phone.ext.DefaultCallSettingsConnection;
import com.mediatek.xlog.Xlog;

public class OP09CallSettingsConnection extends DefaultCallSettingsConnection {

    private static final String TAG = "CallSettingsConnection";
    private static final String CDMA_CALL_OPTION_CLASS_NAME = "com.mediatek.phone.plugin.CdmaAdditionalCallOptions";

    private static final String BUTTON_CFU_KEY = "button_cfu_key";
    private static final String BUTTON_CFB_KEY = "button_cfb_key";
    private static final String BUTTON_CFNRY_KEY = "button_cfnry_key";
    private static final String BUTTON_CFNRC_KEY = "button_cfnrc_key";
    private static final String BUTTON_CFC_KEY = "button_cfc_key";

    private Context mContext;

    /**
     *
     * @param context get current context.
     */
    public OP09CallSettingsConnection(Context context) {
        Xlog.i(TAG, "CallSettingsConnectionExt: " + context.getPackageName());
        mContext = context;
    }

    @Override
    public void startCallSettingsActivity(Activity activity) {

        if (activity != null) {
            Intent intent = new Intent();
            intent.setClassName("com.mediatek.op09.plugin", "com.mediatek.phone.plugin.callsetting.CallSettings");
            Xlog.v(TAG, "CallSettingsOP09Extension start activity");
            activity.startActivity(intent);
            Xlog.v(TAG, "CallSettingsOP09Extension end activity");
            activity.finish();
            Xlog.v(TAG, "CallSettingsOP09Extension  activity finish");
        }
    }

    @Override
    public void setCallForwardPrefsTitle(PreferenceScreen prefSet) {
        Xlog.d(TAG, "OP09 setCallForwardPrefsTitle");
        Preference buttonCFU = prefSet.findPreference(BUTTON_CFU_KEY);
        Preference buttonCFB = prefSet.findPreference(BUTTON_CFB_KEY);
        Preference buttonCFNRy = prefSet.findPreference(BUTTON_CFNRY_KEY);
        Preference buttonCFNRc = prefSet.findPreference(BUTTON_CFNRC_KEY);
        Preference buttonCFC = prefSet.findPreference(BUTTON_CFC_KEY);

        if (buttonCFU != null) {
            buttonCFU.setTitle(mContext.getString(R.string.ct_labelCFU));
        }
        if (buttonCFB != null) {
            buttonCFB.setTitle(mContext.getString(R.string.ct_labelCFB));
        }
        if (buttonCFNRy != null) {
            buttonCFNRy.setTitle(mContext.getString(R.string.ct_labelCFNRy));
       }
        if (buttonCFNRc != null) {
            buttonCFNRc.setTitle(mContext.getString(R.string.ct_labelCFNRc));
       }
        if (buttonCFC != null) {
            buttonCFC.setTitle(mContext.getString(R.string.ct_labelCFC));
        }
     }

    @Override
    public String getCdmaCallOptionClassName() {
        return CDMA_CALL_OPTION_CLASS_NAME;
    }
}
