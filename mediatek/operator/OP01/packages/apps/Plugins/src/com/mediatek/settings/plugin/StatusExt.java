package com.mediatek.settings.plugin;

import android.content.Context;
import android.content.IntentFilter;
import android.preference.Preference;
import android.preference.PreferenceScreen;

import com.android.internal.telephony.PhoneConstants;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.op01.plugin.R;
import com.mediatek.settings.ext.DefaultStatusExt;

public class StatusExt extends DefaultStatusExt {
    private Context mContext;
    
    public StatusExt(Context context) {
        super();
        mContext = context;
    }
    
    public void updateOpNameFromRec(Preference p, String name) {
        p.setSummary(name);
    }
    
    public void updateServiceState(Preference p, String name) {
        
    }
    public void addAction(IntentFilter intent, String action) {
        intent.addAction(action);
    }

    public void customizeGeminiImei(String imeiKey, String imeiSvKey, PreferenceScreen parent, int slotId) {
        if (FeatureOption.MTK_SINGLE_IMEI) {
            Preference imei = parent.findPreference(imeiKey); 
            Preference imeiSv = parent.findPreference(imeiSvKey);
            if (slotId == PhoneConstants.GEMINI_SIM_1) {
                imei.setTitle(mContext.getString(R.string.status_imei));
                imeiSv.setTitle(mContext.getString(R.string.status_imei_sv));
            } else {
                parent.removePreference(imei);
                parent.removePreference(imeiSv);
            }
        }
    }
}
