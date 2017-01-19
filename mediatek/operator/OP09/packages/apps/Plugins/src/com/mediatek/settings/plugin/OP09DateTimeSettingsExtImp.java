package com.mediatek.settings.plugin;

import android.content.Context;
import android.content.Intent;
import android.os.SystemProperties;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.preference.PreferenceCategory;
import android.preference.PreferenceScreen;
import android.provider.Telephony.SIMInfo;

import com.android.internal.telephony.PhoneConstants;

import com.mediatek.op09.plugin.R;
import com.mediatek.settings.ext.DefaultDateTimeSettingsExt;
import com.mediatek.telephony.TelephonyManagerEx;
import com.mediatek.xlog.Xlog;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class OP09DateTimeSettingsExtImp extends DefaultDateTimeSettingsExt {

    private static final String TAG = "DateTimeSettingsExtImp";
    private static final int SINGLE_SIM_CARD = 1;
    private static final int DOUBLE_SIM_CARD = 2;
    private static final int AUTO_TIME_NETWORK_INDEX = 0;
    private static final String ACTION_TIME_MODE = "com.mediatek.settings.intent.action.CT_TIME_ROAMING_SETTING";
    private static final String PACKAGE_NAME = "com.mediatek.op09.plugin";
    private static final String TIMEZONECLASS_NAME = "com.mediatek.settings.plugin.TimeZoneActivity";

    // Tianyi date and time feature may cause CTS fail, so disable this feature here
    private static final boolean ENABLE_CT_TIANYI_DATE_TIME_FEATURE = false;

    private Context mContext;

    public OP09DateTimeSettingsExtImp(Context context) {
        mContext = context;
    }

    /**
     * Customize the Date/time preference status based on plugin's specific
     * logic. It will be called onRume life cycle callback, to ensure the status
     * is always right
     */

    @Override
    public void customizeDateTimePreferenceStatus(Context context,
            ListPreference listPreference, CheckBoxPreference checkBoxPreference) {
        if (ENABLE_CT_TIANYI_DATE_TIME_FEATURE) {
            boolean isEnabled = true;
            int phoneType = PhoneConstants.PHONE_TYPE_NONE;
            String sim1Nitz = "";
            String sim2Nitz = "";
            String defaultValue = "";
            Xlog.d(TAG, "op09 getDateTimeStatus");
            ArrayList<SIMInfo> simInfoList = (ArrayList<SIMInfo>) getSimInfo(context);
            phoneType = TelephonyManagerEx.getDefault().getPhoneType(PhoneConstants.GEMINI_SIM_1);
            Xlog.d(TAG, "sim slot 1 phoneType = " + phoneType);
            if (simInfoList != null && simInfoList.size() == DOUBLE_SIM_CARD) {
                isEnabled = isEnableDateTime(isEnabled, phoneType, defaultValue);
            } else if (simInfoList != null && simInfoList.size() == SINGLE_SIM_CARD) {
                isEnabled = isEnableSingleDateTime(isEnabled, phoneType,
                        defaultValue, simInfoList);
            } else {
                Xlog.i(TAG, "getDateTimeStatus Error: no SIM inserted");
            }
            if (!isEnabled) {
                listPreference.setValueIndex(AUTO_TIME_NETWORK_INDEX);
                listPreference.setEnabled(false);
                checkBoxPreference.setChecked(true);
                checkBoxPreference.setEnabled(false);
            }
        }
    }

    private boolean isEnableSingleDateTime(boolean isEnabled, int phoneType,
            String defaultValue, ArrayList<SIMInfo> simInfoList) {
        String sim1Nitz;
        String sim2Nitz;
        Xlog.i(TAG, "getDateTimeStatus One sim!");
        SIMInfo simInfo = simInfoList.get(0);
        if (simInfo.mSlot == PhoneConstants.GEMINI_SIM_1) {
            Xlog.i(TAG, "Only SIM1 inserted");
            if (phoneType == PhoneConstants.PHONE_TYPE_GSM) {
                sim1Nitz = SystemProperties.get("gsm.nitz.time", defaultValue);
                Xlog.d(TAG, "sim1Nitz is :" + sim1Nitz);
                if (!("".equals(sim1Nitz))) {
                    isEnabled = false;
                    Xlog.d(TAG, "Support NITZ disabled for single G mode");
                } else {
                    Xlog.d(TAG, "Not support NITZ");
                }
            } else if (phoneType == PhoneConstants.PHONE_TYPE_CDMA) {
                isEnabled = false;
                Xlog.d(TAG, "SIM1 registered CDMA with");
            } else {
                Xlog.d(TAG, "None type phoneType = " + phoneType);
            }
        } else if (simInfo.mSlot == PhoneConstants.GEMINI_SIM_2) {
            Xlog.i(TAG, "Only SIM2 inserted");
            sim2Nitz = SystemProperties.get("gsm.nitz.time.2", defaultValue);
            Xlog.d(TAG, "sim2Nitz is :" + sim2Nitz);
            if (!("".equals(sim2Nitz))) {
                isEnabled = false;
                Xlog.d(TAG, "SIM2 network supports NITZ");
            }
        }
        return isEnabled;
    }

    private boolean isEnableDateTime(boolean isEnabled, int phoneType,
            String defaultValue) {
        String sim1Nitz;
        String sim2Nitz;
        if (phoneType == PhoneConstants.PHONE_TYPE_GSM) {
            sim1Nitz = SystemProperties.get("gsm.nitz.time", defaultValue);
            sim2Nitz = SystemProperties.get("gsm.nitz.time.2", defaultValue);
            Xlog.d(TAG, "GG dual stand by, sim1Nitz is :" + sim1Nitz + "; sim2Nitz is :" + sim2Nitz);
            if (!("".equals(sim1Nitz)) && !("".equals(sim2Nitz))) {
                isEnabled = false;
                Xlog.d(TAG, "Both network support NITZ");
            } else {
                Xlog.d(TAG, "One of not support NITZ so enabled");
            }
        } else if (phoneType == PhoneConstants.PHONE_TYPE_CDMA) {
            isEnabled = false;
            Xlog.d(TAG, "SIM1 registered CDMA network");
        } else {
            Xlog.d(TAG, "None type");
        }
        return isEnabled;
    }

    /**
     * M: customize Preference Screen, add or remove preference.
     */
    @Override
    public void customizePreferenceScreen(Context context, PreferenceScreen pref) {
        PreferenceCategory prefCategory = new PreferenceCategory(context);
        prefCategory.setTitle(mContext
                .getString(R.string.roaming_settings_time_category_title));
        PreferenceScreen timeZonePref = new PreferenceScreen(context, null);
        Intent intentTimeZone = new Intent();
        intentTimeZone.setClassName(PACKAGE_NAME, TIMEZONECLASS_NAME);
        timeZonePref.setIntent(intentTimeZone);
        timeZonePref.setTitle(mContext
                .getString(R.string.time_zone_recommend_title));
        PreferenceScreen timeModePref = new PreferenceScreen(context, null);
        timeModePref.setTitle(mContext.getString(R.string.time_settings_title));
        Intent intentTimeMode = new Intent();
        intentTimeMode.setAction(ACTION_TIME_MODE);
        timeModePref.setIntent(intentTimeMode);
        setPrefEnableState(context, timeZonePref, timeModePref);
        pref.addPreference(prefCategory);
        prefCategory.addPreference(timeZonePref);
        prefCategory.addPreference(timeModePref);
    }

    private void setPrefEnableState(Context context, PreferenceScreen timeZone,
            PreferenceScreen timeMode) {
        timeZone.setEnabled(true);
        timeMode.setEnabled(true);
        SimInformation simInformation = new SimInformation(context);
        boolean isRoaming = simInformation.isInternationalRoamingStatus();
        Xlog.d(TAG, "isRoaming = " + isRoaming);
        if (!isRoaming) {
            timeMode.setEnabled(false);
            timeZone.setEnabled(false);
        } else {
            int phoneType = simInformation.getSlot1PhoneTypeGemini();
            Xlog.d(TAG, "phoneType = " + phoneType);
            String sim1Nitz = "";
            String sim2Nitz = "";
            String defaultValue = "";
            if (simInformation.insertedSimSlot() == SimInformation.TWO_SIM_INSERTED) {
                Xlog.d(TAG, "timezone status: two sim insearted");
                if (phoneType == PhoneConstants.PHONE_TYPE_GSM) {
                    sim1Nitz = SystemProperties.get("gsm.nitz.time",
                            defaultValue);
                    sim2Nitz = SystemProperties.get("gsm.nitz.time.2",
                            defaultValue);
                    Xlog.d(TAG, "GG dual stand by, sim1Nitz is :" + sim1Nitz + "; sim2Nitz is :" + sim2Nitz);
                    if (!("".equals(sim1Nitz)) && !("".equals(sim1Nitz))) {
                        timeZone.setEnabled(false);
                        Xlog.d(TAG, "sim1 and sim2 setEnabled false");
                    }
                } else if (phoneType == PhoneConstants.PHONE_TYPE_CDMA) {
                    timeZone.setEnabled(false);
                    Xlog.d(TAG, "SIM1 registered CDMA network.");
                }
            } else if (simInformation.insertedSimSlot() == SimInformation.ONLY_SLOT1_INSERTED) {
                Xlog.d(TAG, "timezone status: only Slot1 one sim insearted");
                if (phoneType == PhoneConstants.PHONE_TYPE_GSM) {
                    sim1Nitz = SystemProperties.get("gsm.nitz.time",
                            defaultValue);
                    Xlog.d(TAG, "Only inserted SIM1, sim1Nitz is :" + sim1Nitz);
                    if (!("".equals(sim1Nitz))) {
                        timeZone.setEnabled(false);
                        Xlog.d(TAG, "Slot1 registered GSM, and supports NITZ");
                    }
                } else if (phoneType == PhoneConstants.PHONE_TYPE_CDMA) {
                    timeZone.setEnabled(false);
                    Xlog.d(TAG, "tstatus: Slot1 registered CDMA supports NITZ");
                } else {
                    Xlog.d(TAG, "phoneType :" + phoneType);
                }
            } else if (simInformation.insertedSimSlot() == SimInformation.ONLY_SLOT2_INSERTED) {
                sim2Nitz = SystemProperties
                        .get("gsm.nitz.time.2", defaultValue);
                Xlog.d(TAG, "Only Slot2 insearted, sim2Nitz is: " + sim2Nitz);
                if (!("".equals(sim2Nitz))) {
                    timeZone.setEnabled(false);
                    Xlog.d(TAG, "timezone status: Slot2 support NITZ");
                }
            }
        }
    }

    private List<SIMInfo> getSimInfo(Context context) {
        Xlog.d(TAG, "getSimInfo()");
        List<SIMInfo> siminfoList = new ArrayList<SIMInfo>();
        List<SIMInfo> simList = SIMInfo.getInsertedSIMList(context);
        int simSlot = 0;
        siminfoList.clear();
        if (simList.size() == DOUBLE_SIM_CARD) {
            if (simList.get(0).mSlot > simList.get(1).mSlot) {
                Collections.swap(simList, 0, 1);
            }
            for (int i = 0; i < simList.size(); i++) {
                siminfoList.add(simList.get(i));
            }
        } else if (simList.size() == SINGLE_SIM_CARD) {
            siminfoList.add(simList.get(0));
        }
        return siminfoList;
    }
}
