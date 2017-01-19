package com.mediatek.phone.plugin.callsetting;

import android.app.ActionBar;
import android.app.ActionBar.Tab;
import android.app.Activity;
import android.app.Fragment;
import android.app.FragmentTransaction;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.preference.PreferenceFragment;
import android.provider.Settings;

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.mediatek.common.telephony.ITelephonyEx;
import com.mediatek.op09.plugin.R;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;
import com.mediatek.telephony.TelephonyManagerEx;
import com.mediatek.xlog.Xlog;

import java.util.Collections;
import java.util.Comparator;
import java.util.List;

/**
 * Description:
 * <p>
 * Customization for CT,Call Setting
 * <p>
 */
public class CallSettings extends Activity {

    private static final int SIM_SLOT_ID_GENERAL_TAB = -1;
    private static final int PHONE_TYPE_FOR_GENERAL_TAB = -1;
    private static final String TAG = "CallSettings";
    public static final String FRAGMENT_TAG_SIM_1 = "SIM1";
    public static final String FRAGMENT_TAG_SIM_2 = "SIM2";
    public static final String FRAGMENT_TAG_GENERAL = "GENERAL";

    private boolean mNotStopped;
    private ITelephonyEx mTelephonyEx;
    private TelephonyManagerEx mTelephonyManagerEx;
    private List<SimInfoRecord> mSimList;

    // Used to sort SimInfo list by slot id, so that SIM1 tab always shown first
    private SIMInfoComparable mSimInfoComparable = new SIMInfoComparable();

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Xlog.d(TAG, "mReceiver [action = " + action + "]");
            if (TelephonyIntents.ACTION_SIM_INFO_UPDATE.equals(action)
                    || TelephonyIntents.ACTION_RADIO_TECHNOLOGY_CHANGED.equals(action)) {
                updateTabs();
                setScreenEnabled();
            } else if (Intent.ACTION_AIRPLANE_MODE_CHANGED.equals(action)
                    || Intent.ACTION_DUAL_SIM_MODE_CHANGED.equals(action)) {
                setScreenEnabled();
            }
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mTelephonyEx = ITelephonyEx.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICEEX));
        mTelephonyManagerEx = TelephonyManagerEx.getDefault();
    }

    @Override
    protected void onStart() {
        super.onStart();
        mNotStopped = true;
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(TelephonyIntents.ACTION_SIM_INFO_UPDATE);
        intentFilter.addAction(TelephonyIntents.ACTION_RADIO_TECHNOLOGY_CHANGED);
        intentFilter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        intentFilter.addAction(Intent.ACTION_DUAL_SIM_MODE_CHANGED);
        registerReceiver(mReceiver, intentFilter);
    }

    @Override
    public void onResume() {
        super.onResume();
        updateSimList();
        updateTabs();
        setScreenEnabled();
    }

    @Override
    protected void onStop() {
        super.onStop();
        mNotStopped = false;
        unregisterReceiver(mReceiver);
    }

    private void updateTabs() {
        if (mNotStopped) {
            clearAllFragment();
            int simSize = mSimList.size();
            if (simSize > 0) {
                getActionBar().setNavigationMode(ActionBar.NAVIGATION_MODE_TABS);
                for (int i = 0; i < simSize; i++) {
                    int slotId = mSimList.get(i).mSimSlotId;
                    addCallSettingTab(slotId, getSimType(slotId), mSimList.get(i).mDisplayName);
                }
                addGeneralCallSettingTab();
            } else {
                getActionBar().setNavigationMode(ActionBar.NAVIGATION_MODE_STANDARD);
                Fragment generalFrag = new GeneralCallSettings();
                getFragmentManager().beginTransaction().replace(android.R.id.content, generalFrag, FRAGMENT_TAG_GENERAL)
                        .commit();
            }
        }
    }

    private void clearAllFragment() {
        // When tab is unselected, the fragment will be detached automatically
        getActionBar().removeAllTabs();
    }

    private void setScreenEnabled() {
        boolean isAirplaneOn = Settings.System.getInt(getContentResolver(), Settings.Global.AIRPLANE_MODE_ON, -1) > 0;
        int simSize = mSimList.size();
        String indexTag;
        Fragment indexFrag;
        PreferenceFragment indexPreferenceFrag;
        boolean indexSlotRadioOn = false;
        int indexSimSlot;
        for (int i = 0; i < simSize; i++) {
            indexSimSlot = mSimList.get(i).mSimSlotId;
            indexTag = getSimTagBySlotId(indexSimSlot);
            indexFrag = getFragmentManager().findFragmentByTag(indexTag);
            if (!isAirplaneOn) {
                indexSlotRadioOn = slotRadioOn(indexSimSlot);
                Xlog.d(TAG, "setScreenEnabled indexSimSlot=" + indexSimSlot + ", indexTag=" + indexTag
                        + ", indexSlotRadioOn=" + indexSlotRadioOn + ", indexFrag=" + indexFrag);
            }
            if (indexFrag instanceof PreferenceFragment) {
                indexPreferenceFrag = (PreferenceFragment) indexFrag;
                indexPreferenceFrag.getPreferenceScreen().setEnabled(!isAirplaneOn && indexSlotRadioOn);
            }
        }
    }

    private boolean slotRadioOn(int slotId) {
        boolean isRadioOn = false;
        try {
            if (mTelephonyEx != null) {
                isRadioOn = mTelephonyEx.isRadioOn(slotId);
                Xlog.d(TAG, "Slot " + slotId + " is in radion state " + isRadioOn);
            }
        } catch (RemoteException e) {
            Xlog.w(TAG, "mTelephonyEx exception");
        }
        return isRadioOn;
    }

    private void updateSimList() {
        mSimList = SimInfoManager.getInsertedSimInfoList(this);
        Collections.sort(mSimList, mSimInfoComparable);
    }

    @SuppressWarnings("unchecked")
    private void addCallSettingTab(int slotId, int phoneType, String simName) {
        TabListener<Fragment> tabListener;
        String title;
        String tag;
        if (slotId >= 0) {
            tabListener = createTabListener(slotId, phoneType);
            title = getResources().getString(R.string.call_setting_tab_SIM, slotId + 1, simName);
            tag = getSimTagBySlotId(slotId);
        } else {
            tabListener = createTabListener(SIM_SLOT_ID_GENERAL_TAB, PHONE_TYPE_FOR_GENERAL_TAB);
            title = getResources().getString(R.string.call_setting_tab_general);
            tag = FRAGMENT_TAG_GENERAL;
        }
        getActionBar().addTab(getActionBar()
                .newTab()
                .setText(title)
                .setTag(tag)
                .setTabListener(tabListener));
    }

    private void addGeneralCallSettingTab() {
        addCallSettingTab(SIM_SLOT_ID_GENERAL_TAB, -1, null);
    }

    public static String getSimTagBySlotId(int slotId) {
        switch (slotId) {
        case PhoneConstants.GEMINI_SIM_1:
            return FRAGMENT_TAG_SIM_1;
        case PhoneConstants.GEMINI_SIM_2:
            return FRAGMENT_TAG_SIM_2;
        default:
            throw new IllegalArgumentException();
        }
    }

    public static int getSimSlotIdByTag(String tag) {
        if (tag.equals(FRAGMENT_TAG_SIM_1)) {
            return PhoneConstants.GEMINI_SIM_1;
        } else if (tag.equals(FRAGMENT_TAG_SIM_2)) {
            return PhoneConstants.GEMINI_SIM_2;
        } else {
            throw new IllegalArgumentException();
        }
    }

    @SuppressWarnings("rawtypes")
    private TabListener createTabListener(int slotId, int phoneType) {
        switch (phoneType) {
        case PhoneConstants.PHONE_TYPE_CDMA:
            return new TabListener<CdmaCallSettings>(this, getSimTagBySlotId(slotId), CdmaCallSettings.class);
        case PhoneConstants.PHONE_TYPE_GSM:
            return new TabListener<GsmCallSettings>(this, getSimTagBySlotId(slotId), GsmCallSettings.class);
        case PHONE_TYPE_FOR_GENERAL_TAB:
            return new TabListener<GeneralCallSettings>(this, FRAGMENT_TAG_GENERAL, GeneralCallSettings.class);
        default:
            throw new IllegalArgumentException();
        }
    }

    private int getSimType(int slotId) {
        return mTelephonyManagerEx.getPhoneType(slotId);
    }

    public static class TabListener<T extends Fragment> implements ActionBar.TabListener {
        private final Activity mActivity;
        private final String mTag;
        private final Class<T> mClass;
        private final Bundle mArgs;
        private Fragment mFragment;

        public TabListener(Activity activity, String tag, Class<T> clz) {
            this(activity, tag, clz, null);
        }

        public TabListener(Activity activity, String tag, Class<T> clz, Bundle args) {
            mActivity = activity;
            mTag = tag;
            mClass = clz;
            mArgs = args;

            // Check to see if we already have a fragment for this tab, probably
            // from a previously saved state.  If so, deactivate it, because our
            // initial state is that a tab isn't shown.
            mFragment = mActivity.getFragmentManager().findFragmentByTag(mTag);
            if (mFragment != null && !mFragment.isDetached()) {
                FragmentTransaction ft = mActivity.getFragmentManager().beginTransaction();
                ft.detach(mFragment);
                ft.commit();
            }
        }

        public void onTabSelected(Tab tab, FragmentTransaction ft) {
            if (mFragment == null) {
                mFragment = Fragment.instantiate(mActivity, mClass.getName(), mArgs);
                ft.add(android.R.id.content, mFragment, mTag);
            } else {
                ft.attach(mFragment);
            }
        }

        public void onTabUnselected(Tab tab, FragmentTransaction ft) {
            if (mFragment != null) {
                ft.detach(mFragment);
            }
        }

        @Override
        public void onTabReselected(Tab arg0, FragmentTransaction arg1) {
        }
    }

    public static class SIMInfoComparable implements Comparator<SimInfoRecord> {
        @Override
        public int compare(SimInfoRecord sim1, SimInfoRecord sim2) {
            return sim1.mSimSlotId - sim2.mSimSlotId;
        }
    }
}
