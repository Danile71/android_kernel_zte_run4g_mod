package com.mediatek.settings.plugin;

import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.preference.DialogPreference;
import android.preference.Preference;
import android.preference.PreferenceScreen;
import android.provider.Telephony;
import android.test.ActivityInstrumentationTestCase2;
import android.util.AttributeSet;
import android.util.Log;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.internal.R;
import com.mediatek.pluginmanager.PluginManager;
import com.mediatek.settings.ext.IApnSettingsExt;
import com.mediatek.settings.ext.IDeviceInfoSettingsExt;
import com.mediatek.settings.ext.ISettingsMiscExt;
import com.mediatek.settings.ext.ISimManagementExt;
import com.mediatek.settings.ext.IStatusGeminiExt;

public class OP02SettingsPluginTest extends ActivityInstrumentationTestCase2<MockActivity> {

    private static final String TAG = "OP02SettingsPluginTest";
    private static final String CU_NUMERIC_1 = "46001";
    private static final String CU_NUMERIC_2 = "46009";
    private static final String CU_APN_NET = "3gnet";
    private static final String CU_APN_WAP = "3gwap";

    private Context mContext;
    private MockActivity mActivity;

    private IApnSettingsExt mApnSettingsExt;
    private IDeviceInfoSettingsExt mDeviceInfoSettingsExt;
    private ISettingsMiscExt mSettingsMiscExt;
    private ISimManagementExt mSimManagementExt;
    private IStatusGeminiExt mIStatusGeminiExt;

    public OP02SettingsPluginTest() {
        super(MockActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContext = getInstrumentation().getContext();
        mActivity = getActivity();
        mApnSettingsExt = (IApnSettingsExt)PluginManager.createPluginObject(
                getInstrumentation().getContext(), IApnSettingsExt.class.getName());
        mDeviceInfoSettingsExt = (IDeviceInfoSettingsExt)PluginManager.createPluginObject(
                getInstrumentation().getContext(), IDeviceInfoSettingsExt.class.getName());
        mSettingsMiscExt = (ISettingsMiscExt)PluginManager.createPluginObject(
                getInstrumentation().getContext(), ISettingsMiscExt.class.getName());
        mSimManagementExt = (ISimManagementExt)PluginManager.createPluginObject(
                getInstrumentation().getContext(), ISimManagementExt.class.getName());
        mIStatusGeminiExt = (IStatusGeminiExt)PluginManager.createPluginObject(
                getInstrumentation().getContext(), IStatusGeminiExt.class.getName());
    }

    // Should disallow user edit default APN
    public void test01APNisAllowEditPresetApn() {
        String type = null;
        String apn = null;
        String numeric = CU_NUMERIC_1;
        int sourcetype = 0;
        assertTrue(!mApnSettingsExt.isAllowEditPresetApn(type, apn, numeric, sourcetype));
        sourcetype = 1;
        assertTrue(mApnSettingsExt.isAllowEditPresetApn(type, apn, numeric, sourcetype));

        numeric = CU_NUMERIC_2;
        sourcetype = 0;
        assertTrue(!mApnSettingsExt.isAllowEditPresetApn(type, apn, numeric, sourcetype));
        sourcetype = 1;
        assertTrue(mApnSettingsExt.isAllowEditPresetApn(type, apn, numeric, sourcetype));

        numeric = "46000";
        sourcetype = 0;
        assertTrue(mApnSettingsExt.isAllowEditPresetApn(type, apn, numeric, sourcetype));
    }

    // Should remove device info's stats pref summary
    public void test02DeviceinfoInitSummary() {
        String summaryString = "summary";
        Preference pref = new Preference(mContext);
        pref.setSummary(summaryString);
        assertTrue(pref.getSummary().equals(summaryString));
        mDeviceInfoSettingsExt.initSummary(pref);
        assertTrue(pref.getSummary().equals(""));
    }

    public void test03SettingMiscSetTimeoutPrefTitle() {
        final DialogPreference dialogPref = new CustomDialogPreference(mContext, null);
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                mActivity.getMockPreferenceGroup().addPreference(dialogPref);
            }
        });
        assertNull(dialogPref.getTitle());
        assertNull(dialogPref.getDialogTitle());
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                mSettingsMiscExt.setTimeoutPrefTitle(dialogPref);
            }
        });
        assertNotNull(dialogPref.getTitle());
        assertNotNull(dialogPref.getDialogTitle());
    }

    public void test04SimManagementUpdateSimManagementPref() {
        final PreferenceScreen prefScreen = new PreferenceScreen(mContext, null);
        prefScreen.setKey("3g_service_settings");
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                mActivity.getMockPreferenceGroup().addPreference(prefScreen);
            }
        });
        assertTrue(mActivity.getMockPreferenceGroup().getPreferenceCount() == 1);
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                mSimManagementExt.updateSimManagementPref(mActivity.getMockPreferenceGroup());
            }
        });
        if (!FeatureOption.MTK_GEMINI_3G_SWITCH) {
            assertTrue(mActivity.getMockPreferenceGroup().getPreferenceCount() == 0);
        } else {
            assertTrue(mActivity.getMockPreferenceGroup().getPreferenceCount() == 1);
        }
    }

    public void test05StatusGeminiInitUI() {
        final PreferenceScreen prefScreen = new PreferenceScreen(mContext, null);
        final Preference pref = new Preference(mContext);
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                mActivity.getMockPreferenceGroup().addPreference(prefScreen);
                prefScreen.addPreference(pref);
            }
        });
        assertTrue(prefScreen.getPreferenceCount() == 1);
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                mIStatusGeminiExt.initUI(prefScreen, pref);
            }
        });
        assertTrue(prefScreen.getPreferenceCount() == 0);
    }

    // 3G APN name should be revised according to 4G LTE spec
    public void test063GApnNames() {
        String g3NetName = mContext.getResources().getString(R.string.cu_3gnet_name);
        String g3WapName = mContext.getResources().getString(R.string.cu_3gwap_name);
        String g3NetNameInLte = mContext.getResources().getString(R.string.cu_3gnet_name_in_lte);
        String g3WapNameInLte = mContext.getResources().getString(R.string.cu_3gwap_name_in_lte);
        Uri queyrUri;
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            queyrUri = Telephony.Carriers.SIM1Carriers.CONTENT_URI;
        } else {
            queyrUri = Telephony.Carriers.CONTENT_URI;
        }
        String whereFor3gNet = "(numeric=\'" + CU_NUMERIC_1 + "\'" + " or numeric=\'" + CU_NUMERIC_2 + "\')" + " and apn=\'"
                + CU_APN_NET + "\'";
        String whereFor3gWap = "(numeric=\'" + CU_NUMERIC_1 + "\'" + " or numeric=\'" + CU_NUMERIC_2 + "\')" + " and apn=\'"
                + CU_APN_WAP + "\' and type is NULL";
        Cursor cursorFor3gNet = mContext.getContentResolver().query(queyrUri, new String[] {
                    "_id", "name", "apn", "type","sourcetype"}, whereFor3gNet, null, null);
        Cursor cursorFor3gWap = mContext.getContentResolver().query(queyrUri, new String[] {
                    "_id", "name", "apn", "type","sourcetype"}, whereFor3gWap, null, null);
        String g3NetNameQuery = null;
        String g3WapNameQuery = null;
        try {
            if (cursorFor3gNet != null && cursorFor3gNet.getCount() > 0) {
                cursorFor3gNet.moveToFirst();
                g3NetNameQuery = cursorFor3gNet.getString(1);
                Log.d(TAG, "test063GApnNameInLte cursorFor3gNet not null g3NetNameQuery=" + g3NetNameQuery);
            }
            if (cursorFor3gWap != null && cursorFor3gWap.getCount() > 0) {
                cursorFor3gWap.moveToFirst();
                g3WapNameQuery = cursorFor3gWap.getString(1);
                Log.d(TAG, "test063GApnNameInLte cursorFor3gWap not null g3WapNameQuery=" + g3WapNameQuery);
            }
            if (FeatureOption.MTK_LTE_SUPPORT) {
                assertEquals(g3NetNameQuery, g3NetNameInLte);
                assertEquals(g3WapNameQuery, g3WapNameInLte);
            } else {
                assertEquals(g3NetNameQuery, g3NetName);
                assertEquals(g3WapNameQuery, g3WapName);
            }
        } finally {
            if (cursorFor3gNet != null) {
                cursorFor3gNet.close();
            }
            if (cursorFor3gWap != null) {
                cursorFor3gWap.close();
            }
        }
    }

    public static class CustomDialogPreference extends DialogPreference {
        public CustomDialogPreference(Context context, AttributeSet attrs) {
            super(context, attrs);
        }
    }
}
