package com.mediatek.settings.plugin;

import android.content.Context;
import android.preference.Preference;
import android.preference.PreferenceGroup;
import android.preference.PreferenceScreen;

import com.mediatek.xlog.Xlog;

/**
 * Phase out. Beijing Tianyi power saving mode feature has conflict with mtk common power saving feture.
 * We phase out this OP09 feature, and if customer want to enable it, just let BatteryExt extend 
 * DefaultBatteryExt.
 */
public class BatteryExt {

    private static final String TAG = "PowerSavings";
    private Context mContext;
    private static final String KEY_CPU_SAVING = "cpu_dtm";

    public BatteryExt(Context context) {
        Xlog.e(TAG, "BatteryExt - new BatteryExt(context)" + context.getPackageName());
        mContext = context;
    }

    public void loadPreference(Context context, PreferenceGroup listGroup) {
        // TODO Auto-generated method stub
        Xlog.e(TAG, "BatteryExt - loadPreference()");
        PowerSavingPreference preference = new PowerSavingPreference(mContext);
        /** Remove common feature CPU Saving mode */
        Preference cpuSaving = listGroup.findPreference(KEY_CPU_SAVING);
        if (cpuSaving != null) {
            listGroup.removePreference(cpuSaving);
        }
        preference.setOrder(-4);
        listGroup.addPreference(preference);
    }

    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        return preference instanceof PowerSavingPreference;
    }
}
