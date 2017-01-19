package com.mediatek.mms.op09;

import android.content.Intent;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;

import com.mediatek.mms.ext.IMmsPreference;
import com.mediatek.mms.op09.Op09MmsPluginTestRunner.BasicCase;
import com.mediatek.pluginmanager.PluginManager;

public class MmsPreferenceTest extends BasicCase {
    private IMmsPreference mMmsPreference;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mMmsPreference = (IMmsPreference)PluginManager.createPluginObject(mContext,
                "com.mediatek.mms.ext.IMmsPreference");
    }

    public void test001ModifyDataRoamingPreference() {
        String RETRIEVAL_DURING_ROAMING = "pref_key_mms_retrieval_during_roaming";
        Intent intent = new Intent();
        intent.putExtra("SIMID", (long)mSimIdCdma);

        mMmsPreference.modifyDataRoamingPreference(mContext, intent);
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(mContext);
        assertTrue(sp.getBoolean(mSimIdCdma + "_" + RETRIEVAL_DURING_ROAMING, false));

        intent.putExtra("ROAMING_STATUS", 0);
        mMmsPreference.modifyDataRoamingPreference(mContext, intent);
        assertFalse(sp.getBoolean(mSimIdCdma + "_" + RETRIEVAL_DURING_ROAMING, true));
    }

}
