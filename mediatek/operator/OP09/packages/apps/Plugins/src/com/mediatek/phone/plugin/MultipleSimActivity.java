package com.mediatek.phone.plugin;

import android.app.ActionBar;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.SystemProperties;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.telephony.PhoneStateListener;
import android.telephony.TelephonyManager;
import android.view.MenuItem;
import android.widget.ListView;

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.TelephonyProperties;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.op09.plugin.R;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;
import com.mediatek.telephony.TelephonyManagerEx;
import com.mediatek.xlog.Xlog;

import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;

public class MultipleSimActivity extends PreferenceActivity implements Preference.OnPreferenceChangeListener {

    private static final String TAG = "OP09MultipleSimActivity";
    private static final boolean DBG = true;

    public static final String SUB_TITLE_NAME = "sub_title_name";
    public static final String TARGET_CLASS = "TARGET_CLASS";
    public static final String TARGET_PACKAGE = "TARGET_PACKAGE";
    public static final String INIT_TITLE_NAME = "INIT_TITLE_NAME";
    public static final String INIT_TITLE_NAME_STR = "INIT_TITLE_NAME_STR";
    public static final String LIST_TITLE = "LIST_TITLE_NAME";

    private static final String SETTINGS_PACKAGE = "com.android.settings";
    private static final String EXTRA_SLOTID = "slotid";

    public static final int CHAR_COUNT = 4;
    private static final int SINGLE_SIM_CARD = 1;
    private static final int DOUBLE_SIM_CARD = 2;
    private static final String NATIVE_MCC_SIM1 = "460"; // 46003
    private static final String NATIVE_MCC_SIM2 = "455"; // 45502

    private int mSimNumbers = 0;
    private String mTargetClass = null;
    private String mTargetPackage = null;
    private int mTitleName;
    private String mTitleNameStr;
    private int mListTitle;
    private List<SimInfoRecord> mSimList;
    private HashMap<Object, Integer> mPref2CardSlot = new HashMap<Object, Integer>();

    private PreCheckForRunning mPreCheckForRunning;
    private TelephonyManager mTelephonyManager;
    private TelephonyManagerEx mTelephonyManagerEx;

    // CT hongmei
    private static final int SOLT_ID_0 = 0;
    private static final int SOLT_ID_1 = 1;

    private IntentFilter mIntentFilter;
    private final MultipleSimReceiver mReceiver = new MultipleSimReceiver();

    private class MultipleSimReceiver extends BroadcastReceiver {
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (Intent.ACTION_AIRPLANE_MODE_CHANGED.equals(action) || Intent.ACTION_DUAL_SIM_MODE_CHANGED.equals(action)) {
                Xlog.d(TAG, "MultipleSimReceiver Intent.ACTION_AIRPLANE_MODE_CHANGED.equals(action) start");
                updatePreferenceEnableState();
                Xlog.d(TAG, "MultipleSimReceiver Intent.ACTION_AIRPLANE_MODE_CHANGED.equals(action) end");
            } else if (TelephonyIntents.ACTION_SIM_INDICATOR_STATE_CHANGED.equals(action)) {
                updatePreferenceList();
            } else if (action.equals(TelephonyIntents.ACTION_SIM_INFO_UPDATE)) {
                // /M: add for hot swap {
                Xlog.d(TAG, "ACTION_SIM_INFO_UPDATE received");
                List<SimInfoRecord> temp = SimInfoManager.getInsertedSimInfoList(MultipleSimActivity.this);
                // If the SIM number is less than one, we don't need the list
                // activity any more, just finish it.
                if (temp.size() <= SINGLE_SIM_CARD) {
                    Xlog.d(TAG, "Activity finished");
                    MultipleSimActivity.this.finish();
                } else {
                    Xlog.d(TAG, "MultipleSimReceiver else start");
                    updatePreferenceEnableState();
                    Xlog.d(TAG, "MultipleSimReceiver else end");
                }
                // /@}
            }
        }
    }

    private PhoneStateListener mPhoneStateListener = new PhoneStateListener() {
        @Override
        public void onCallStateChanged(int state, String incomingNumber) {
            super.onCallStateChanged(state, incomingNumber);
            log("onCallStateChanged ans state is " + state);
            switch (state) {
            case TelephonyManager.CALL_STATE_IDLE:
                Xlog.d(TAG, "mPhoneStateListener else start");
                updatePreferenceEnableState();
                Xlog.d(TAG, "mPhoneStateListener else end");
                break;
            default:
                break;
            }
        }
    };

    class SIMInfoComparable implements Comparator<SimInfoRecord> {

        @Override
        public int compare(SimInfoRecord sim1, SimInfoRecord sim2) {
            return sim1.mSimSlotId - sim2.mSimSlotId;
        }
    }

    private void log(String msg) {
        if (DBG) {
            Xlog.d(TAG, msg);
        }
    }

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        addPreferencesFromResource(R.xml.multiple_sim);

        mPreCheckForRunning = new PreCheckForRunning(this);
        mTelephonyManagerEx = TelephonyManagerEx.getDefault();
        mTelephonyManager = (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
        mTelephonyManager.listen(mPhoneStateListener, PhoneStateListener.LISTEN_CALL_STATE);

        mTargetClass = getIntent().getStringExtra(TARGET_CLASS);
        mTargetPackage = getIntent().getStringExtra(TARGET_PACKAGE);

        mTitleNameStr = getIntent().getStringExtra(INIT_TITLE_NAME_STR);
        mTitleName = getIntent().getIntExtra(INIT_TITLE_NAME, -1);
        mListTitle = getIntent().getIntExtra(LIST_TITLE, -1);

        mSimList = SimInfoManager.getInsertedSimInfoList(this);
        mSimNumbers = mSimList.size();

        Collections.sort(mSimList, new SIMInfoComparable());
        mIntentFilter = new IntentFilter(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        mIntentFilter.addAction(TelephonyIntents.ACTION_SIM_INDICATOR_STATE_CHANGED);
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            mIntentFilter.addAction(Intent.ACTION_DUAL_SIM_MODE_CHANGED);
            // /M: add for hot swap {
            mIntentFilter.addAction(TelephonyIntents.ACTION_SIM_INFO_UPDATE);
            // /@}
        }

        skipUsIfNeeded();
        initPreferenceScreen();
        registerReceiver(mReceiver, mIntentFilter);
        ActionBar actionBar = getActionBar();
        if (actionBar != null) {
            // android.R.id.home will be triggered in onOptionsItemSelected()
            actionBar.setDisplayHomeAsUpEnabled(true);
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        updatePreferenceList();
        Xlog.d(TAG, "onResume start");
        updatePreferenceEnableState();
        Xlog.d(TAG, "onResume start");
        if (mTitleName > 0) {
            this.setTitle(mTitleName);
        }
        if (mTitleNameStr != null) {
            this.setTitle(mTitleNameStr);
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        final int itemId = item.getItemId();
        switch (itemId) {
        case android.R.id.home:
            finish();
            return true;
        default:
            break;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        return true;
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        log("-----------[onSaveInstanceState]-----------");
        super.onSaveInstanceState(outState);
        outState.putInt(INIT_TITLE_NAME, mTitleName);
        outState.putInt(LIST_TITLE, mListTitle);
    }

    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        log("-----------[onRestoreInstanceState]-----------");
        super.onRestoreInstanceState(savedInstanceState);
        mTitleName = savedInstanceState.getInt(INIT_TITLE_NAME, -1);
        mListTitle = savedInstanceState.getInt(LIST_TITLE, -1);
    }

    private void skipUsIfNeeded() {
        if (isCdmaPhone()) {
            Intent intent = new Intent();
            intent.setAction(Intent.ACTION_MAIN);
            int slotId = mSimList.get(0).mSimSlotId;
            long simId = mSimList.get(0).mSimInfoId;
            log("skipUsIfNeeded slotId" + mSimList.get(0).mSimSlotId);
            log("skipUsIfNeeded simId" + mSimList.get(0).mSimInfoId);
            // Settings app use "slotid" instead of "simId" in extra bundle
            if (mTargetPackage != null && mTargetPackage.equals(SETTINGS_PACKAGE)) {
                intent.putExtra(EXTRA_SLOTID, slotId);
            } else {
                intent.putExtra(PhoneConstants.GEMINI_SIM_ID_KEY, slotId);
            }
            intent.setClassName(mTargetPackage, mTargetClass);
            if (!FeatureOption.MTK_GEMINI_SUPPORT) {
                checkToStart(slotId, intent);
                finish();
            } else if (mSimNumbers == 1) {
                checkToStart(slotId, intent);
                finish();
            }
        } else if (mSimNumbers == 1 && mTargetClass != null) {
            Intent intent = new Intent();
            intent.setAction(Intent.ACTION_MAIN);
            int slotId = mSimList.get(0).mSimSlotId;
            if (mTargetPackage != null && mTargetPackage.equals(SETTINGS_PACKAGE)) {
                intent.putExtra(EXTRA_SLOTID, slotId);
            } else {
                intent.putExtra(PhoneConstants.GEMINI_SIM_ID_KEY, slotId);
            }
            intent.setClassName(mTargetPackage, mTargetClass);
            checkToStart(slotId, intent);
            finish();
        }
    }

    private void initPreferenceScreen() {
        PreferenceScreen prefSet = getPreferenceScreen();
        for (int i = 0; i < mSimNumbers; ++i) {
            SimPreference p = (SimPreference) prefSet.getPreference(i);
            p.setTitle(mSimList.get(i).mDisplayName);
            p.setSimColor(mSimList.get(i).mColor);
            p.setSimSlot(i);
            p.setSimName(mSimList.get(i).mDisplayName);
            p.setSimNumber(mSimList.get(i).mNumber);
            p.setSimIconNumber(getProperOperatorNumber(mSimList.get(i)));
            mPref2CardSlot.put(prefSet.getPreference(i), Integer.valueOf(mSimList.get(i).mSimSlotId));
        }
    }

    private boolean isCdmaRoaming() {
        boolean res = false;
        Xlog.i(TAG, "in isCdmaRoaming");
        String numeric = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC, "-1");
        Xlog.i(TAG, "isCdmaRoaming numeric :" + numeric);
        if (numeric.length() >= 3) {
            String mcc = numeric.substring(0, 3);
            Xlog.i(TAG, "mcc=" + mcc);
            if (NATIVE_MCC_SIM1.equals(mcc) || NATIVE_MCC_SIM2.equals(mcc)) {
                res = false;
            } else {
                res = true;
            }
        }
        return res;
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        PreferenceScreen prefSet = getPreferenceScreen();

        for (int i = 0; i < prefSet.getPreferenceCount(); i++) {
            if ((preference == prefSet.getPreference(i)) && (mTargetClass != null)) {
                int slotId = mPref2CardSlot.get(preference);
                long simId = mSimList.get(i).mSimInfoId;
                log("onPreferenceTreeClick slotId" + mPref2CardSlot.get(preference));
                log("onPreferenceTreeClick simId" + mSimList.get(i).mSimInfoId);
                if (isRadioOn(slotId)) {
                    Intent intent = new Intent();
                    intent.setAction(Intent.ACTION_MAIN);
                    if (mTargetPackage != null && mTargetPackage.equals(SETTINGS_PACKAGE)) {
                        intent.putExtra(EXTRA_SLOTID, slotId);
                    } else {
                        intent.putExtra(PhoneConstants.GEMINI_SIM_ID_KEY, slotId);
                    }
                    intent.putExtra(SUB_TITLE_NAME, SimInfoManager.getSimInfoBySlot(this, slotId).mDisplayName);
                    intent.setClassName(mTargetPackage, mTargetClass);
                    checkToStart(slotId, intent);
                }
            }
        }
        return false;
    }

    private String getProperOperatorNumber(SimInfoRecord info) {
        String res = null;
        if (info != null) {
            res = info.mNumber;
            switch (info.mDispalyNumberFormat) {
            case SimInfoManager.DISPALY_NUMBER_NONE:
                res = "";
                break;
            case SimInfoManager.DISPLAY_NUMBER_FIRST:
                if (res != null && res.length() > CHAR_COUNT) {
                    res = res.substring(0, CHAR_COUNT);
                }
                break;
            case SimInfoManager.DISPLAY_NUMBER_LAST:
                if (res != null && res.length() > CHAR_COUNT) {
                    res = res.substring(res.length() - CHAR_COUNT, res.length());
                }
                break;
            default:
                res = "";
                break;
            }
        }
        return res;
    }

    protected void onDestroy() {
        super.onDestroy();
        if (mPreCheckForRunning != null) {
            mPreCheckForRunning.deRegister();
        }
        unregisterReceiver(mReceiver);
    }

    private void updatePreferenceEnableState() {
        log("---------updatePreferenceEnableState---------");
        PreferenceScreen prefSet = getPreferenceScreen();

        // For single sim or only one sim inserted, we couldn't go here
        boolean isIdle = (mTelephonyManager.getCallState() == TelephonyManager.CALL_STATE_IDLE);
        for (int i = 0; i < mSimNumbers; ++i) {
            Preference p = prefSet.getPreference(i);
            p.setEnabled(isRadioOn(mPref2CardSlot.get(p)) && isIdle);
        }
        log("---------updatePreferenceEnableState  end---------");
    }

    private void checkToStart(int slotId, Intent intent) {
        mPreCheckForRunning.checkToRun(intent, slotId, 302);
    }

    private void updatePreferenceList() {
        log("---------[update mutiple list views]---------");
        ListView listView = (ListView) findViewById(android.R.id.list);
        listView.invalidateViews();
    }

    private boolean isRadioOn(int simId) {
        return true;
    }

    private boolean isCdmaPhone() {
        int sim0Type = mTelephonyManagerEx.getPhoneType(SOLT_ID_0);
        int sim1Type = mTelephonyManagerEx.getPhoneType(SOLT_ID_1);
        return sim0Type == PhoneConstants.PHONE_TYPE_CDMA || sim1Type == PhoneConstants.PHONE_TYPE_CDMA;
    }
}
