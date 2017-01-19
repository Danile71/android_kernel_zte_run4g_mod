
package com.mediatek.settings.plugin;

import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.net.ConnectivityManager;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceGroup;
import android.preference.PreferenceScreen;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.view.Window;
import android.widget.Toast;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.OperatorInfo;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.PhoneProxy;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.TelephonyProperties;
import com.android.internal.telephony.cdma.CDMAPhone;
import com.android.internal.telephony.gemini.GeminiPhone;
import com.android.phone.INetworkQueryService;
import com.android.phone.INetworkQueryServiceCallback;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.telephony.ITelephonyEx;
import com.mediatek.common.telephony.internationalroaming.InternationalRoamingConstants;
import com.mediatek.op09.plugin.R;
import com.mediatek.settings.plugin.CdmaNetworkSwitcherPreference.OnSwitchNetworkListener;
import com.mediatek.telephony.TelephonyManagerEx;
import com.mediatek.xlog.Xlog;

import java.util.HashMap;
import java.util.List;

/**
 * Handles manual network selection for both CDMA and GSM.
 */
public class ManualNetworkSelection extends PreferenceActivity implements OnSwitchNetworkListener,
        DialogInterface.OnCancelListener {

    private static final String TAG = "ManualNetworkSelection";
    private static final boolean DBG = true;

    // TODO: Slot 1 SIM card type is saved in this System Settings value by IR
    // framework. We use this to query SIM 1's card type here. It is just
    // workaround to make sure we get correct card type.
    private static final String PROP_INTERNATIONAL_CARD_TYPE = "gsm.internationalcard.type";
    private static final String KEY_CDMA_NETWORK_SELECTION = "cdma_network_selection";
    private static final String KEY_CDMA_NETWORK_SELECTION_DETAIL = "cdma_network_selectioin_detail";
    private static final String KEY_GSM1_NETWORK_SELECTION = "gsm_sim1_network_selection";
    private static final String KEY_GSM2_NETWORK_SELECTION = "gsm_sim2_network_selection";
    private static final String KEY_MANUAL_NOTES = "key_notes";

    private static final String ACTION_PS_DETACH_DONE = "com.mediatek.action.PS_DETACH_DONE";
    private static final String EXTRA_SIM_ID = "com.mediatek.intent.extra.SIM_ID";

    // Extra key and values of SIM detect broadcast.
    private static final String INTENT_KEY_DETECT_STATUS = "simDetectStatus";
    private static final String INTENT_KEY_NEW_SIM_STATUS = "newSIMStatus";
    private static final String EXTRA_VALUE_REMOVE_SIM = "REMOVE";

    // CDMA network switching dialog
    private static final int DIALOG_SWITCH_NETWORK_REGISTERED = 1001;
    private static final int DIALOG_SWITCH_NETWORK_FAILED = 1002;
    private static final int DIALOG_SWITCH_CDMA_NETWORK_FAILED_DATA_CONNECT = 1006;

    // GSM network switching dialog
    private static final int DIALOG_NETWORK_SELECTION = 1003;
    private static final int DIALOG_NETWORK_LIST_LOAD = 1004;
    private static final int DIALOG_PHONE_SWITCHING = 1005;

    private static final int EVENT_NETWORK_SELECTION_DONE = 101;
    private static final int EVENT_NETWORK_SCAN_COMPLETED = 102;
    private static final int EVENT_NETWORK_SCAN_COMPLETED_2 = 103;
    private static final int MSG_SWITCH_NETWORK = 201;

    // Status that will be retured in the callback.
    private static final int QUERY_OK = 0;

    // GSM NETWORK_RAT, GPRS and EDGE is 2G, or else is 3G.
    private static final int NETWORK_RAT_2G = 0;
    private static final int NETWORK_RAT_3G = 1;

    private static final int RIL_RADIO_TECHNOLOGY_UMTS = 3;

    // Map of GSM network information.
    private HashMap<Preference, OperatorInfo> mNetworkMap = new HashMap<Preference, OperatorInfo>();
    private Phone mPhone;
    private GeminiPhone mGeminiPhone;

    private boolean mIsForeground;

    private Preference mCdmaNetworkSelectionPref;
    private CdmaNetworkSwitcherPreference mCdmaNetworkSelectionDetailPref;
    private PreferenceGroup mGsm1NetworkSelectionPref;
    private PreferenceGroup mGsm2NetworkSelectionPref;
    private Preference mNotesPref;

    // Network selection mode.
    private static final int CDMA_SELECTION_MODE = 0;
    private static final int GSM_SELECTION_MODE = 1;
    private int mCurrentSelectionMode = CDMA_SELECTION_MODE;

    // Indicates the phone switching process, update UI for different states.
    private static final int SWITCH_PROCESS_DONE = 0;
    private static final int SWITCH_PROCESS_BEGIN = 1;
    private static final int SWITCH_PROCESS_SWITCH_PHONE_DONE = 2;
    private static final int SWITCH_PROCESS_SEARCHING_NETWORK = 3;
    private int mPhoneSwitchingMode = SWITCH_PROCESS_DONE;

    private int mTargetSlot;
    private int mExModemSlot;
    private boolean mIsExternalSlot;
    private boolean mIsDualModeCard;
    private String mSelectedNetwork = "";
    
    private String mSelectedNetworkText;

    private ITelephony mTelephony;
    private ITelephonyEx mTelephonyEx;
    private TelephonyManager mTelephonyManager;
    private TelephonyManagerEx mTelephonyManagerEx;

    private final Handler mCdmaAvoidListHandler = new AvoidSysHandler();

    private boolean mForceDisableData;

    private boolean mDelayRegAlertAfterGsmListLoaded;

    // GSM network selection service.
    private INetworkQueryService mNetworkQueryService = null;

    private ConnectivityManager mConnectivityManager;

    private boolean mWaitDataDisconnected;

    private static final int MSG_DATA_DETACH_TIME_OUT = 2001;
    private static final int DATA_DETACH_TIME_OUT_DELAY = 10 * 1000;

    private static final int MSG_DATA_DETACHED = 2002;
    private static final int DATA_DETACHED_DELAY = 4 * 1000;

    //CDMA manual network selection status
    private static final int CDMA_MANUAL_SWITCH_PROCESS_BEGIN = 200;
    private static final int CDMA_MANUAL_SWITCH_PROCESS_SEARCHING = 201;
    private static final int CDMA_MANUAL_SWITCH_PROCESS_DONE = 202;

    private int mCdmaManuallyNetworkSelectState;

    // OP09 2014-04-14 new requirement, don't show CDMA network seleciton.
    private boolean mShowCdmaNetworkSelection;

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();
            log("onReceive: action = " + action + ", mPhoneSwitchingMode = " + mPhoneSwitchingMode
                    + ", mTargetSlot = " + mTargetSlot + ", this = " + this);
            if (action.equals(Intent.ACTION_AIRPLANE_MODE_CHANGED)
                    || action.equals(TelephonyIntents.ACTION_SIM_INDICATOR_STATE_CHANGED)) {
                updateScreen();
            } else if (action.equals(TelephonyIntents.ACTION_RADIO_TECHNOLOGY_CHANGED)) {
                log("RADIO_TECHNOLOGY_CHANGED: mPhoneSwitchingMode = " + mPhoneSwitchingMode);
                if (mIsExternalSlot) {
                    if (isExternalSlotInCdmaMode()) {
                        mCurrentSelectionMode = CDMA_SELECTION_MODE;
                    } else {
                        mCurrentSelectionMode = GSM_SELECTION_MODE;
                    }
                }

                if (mPhoneSwitchingMode == SWITCH_PROCESS_BEGIN) {
                    mPhoneSwitchingMode = SWITCH_PROCESS_SWITCH_PHONE_DONE;
                    removeDialog(DIALOG_PHONE_SWITCHING);
                }
                updateScreen();
            } else if (TelephonyIntents.ACTION_SIM_DETECTED.equals(action)) {
                // Finish SIM2's manual network settings fragment if SIM2 is
                // plugged out.
                String simDetectStatus = intent.getStringExtra(INTENT_KEY_DETECT_STATUS);
                log("ACTION_SIM_DETECTED: mTargetSlot = " + mTargetSlot + ",simDetectStatus = "
                        + simDetectStatus);
                if ((mTargetSlot != mExModemSlot)
                        && (EXTRA_VALUE_REMOVE_SIM.equals(simDetectStatus))) {
                    final boolean simInserted = isSimInserted(mTargetSlot);
                    log("ACTION_SIM_DETECTED: simInserted = " + simInserted);
                    if (!simInserted) {
                        finish();
                    }
                }
            } else if (action.equals(ACTION_PS_DETACH_DONE)) {
                int slotId = intent.getIntExtra(EXTRA_SIM_ID, -1);
                log("ACTION_PS_DETACH_DONE: slotId = " + slotId + " mWaitDataDisconnected = " + mWaitDataDisconnected);
                if (mWaitDataDisconnected) {
                    mDataDetachHandler.removeMessages(MSG_DATA_DETACH_TIME_OUT);
                    mDataDetachHandler.removeMessages(MSG_DATA_DETACHED);
                    mDataDetachHandler.sendEmptyMessageDelayed(MSG_DATA_DETACHED, DATA_DETACHED_DELAY);
                    mWaitDataDisconnected = false;
                }
            }
        }
    };

    private PhoneStateListener mPhoneStateListener = new PhoneStateListener() {
        @Override
        public void onCallStateChanged(int state, String incomingNumber) {
            log("onCallStateChanged , mTargetSlot=" + mTargetSlot);
            updateScreen();
        }

        @Override
        public void onServiceStateChanged(ServiceState serviceState) {
            processCdmaSelectNextNetworkManually(serviceState);

            final int simStat = mTelephonyManagerEx.getSimState(mTargetSlot);
            // TODO: ask pervious owner about why we need to judge SIM state.
            if (simStat == TelephonyManager.SIM_STATE_ABSENT
                    && mPhoneSwitchingMode == SWITCH_PROCESS_DONE) {
                log("onServiceStateChanged return directly, simStat = " + simStat
                        + ", mPhoneSwitchingMode = " + mPhoneSwitchingMode + ", mTargetSlot="
                        + mTargetSlot);
                return;
            }

            final int regState = serviceState.getRegState();
            log("onServiceStateChanged: regState = " + regState + ", mPhoneSwitchingMode = "
                    + mPhoneSwitchingMode + ", mTargetSlot=" + mTargetSlot + ", this = " + this);

            if ((mPhoneSwitchingMode == SWITCH_PROCESS_SWITCH_PHONE_DONE)
                    && regState == ServiceState.REGISTRATION_STATE_NOT_REGISTERED_AND_SEARCHING) {
                mPhoneSwitchingMode = SWITCH_PROCESS_SEARCHING_NETWORK;
            } else if (mPhoneSwitchingMode == SWITCH_PROCESS_SEARCHING_NETWORK) {
                final boolean isG2C = isExternalSlotInCdmaMode();
                log("SEARCHING_SERVICE: regState = " + regState + ", isG2C = " + isG2C);
                if (regState == ServiceState.REGISTRATION_STATE_HOME_NETWORK
                        || regState == ServiceState.REGISTRATION_STATE_ROAMING) {
                    mPhoneSwitchingMode = SWITCH_PROCESS_DONE;
                    if (isG2C) {
                        // In CDMA mode, show alert dialog directly when registered network duration switch phone
                        showDialog(DIALOG_SWITCH_NETWORK_REGISTERED);
                        mCdmaNetworkSelectionDetailPref.updateNetworkSearchingState(false);
                    } else {
                        // Do not show reg alert dialog here, or GSM network loading dialog will cover it. Delay
                        // to show this alert after GSM network load finish
                        mDelayRegAlertAfterGsmListLoaded = true;
                        if (mNetworkQueryService == null) {
                            startAndBindNetworkQueryService();
                        } else {
                            loadGsmNetworksList();
                        }
                    }
                } else if (regState == ServiceState.REGISTRATION_STATE_NOT_REGISTERED_AND_NOT_SEARCHING) {
                    mPhoneSwitchingMode = SWITCH_PROCESS_DONE;
                    showDialog(DIALOG_SWITCH_NETWORK_FAILED);
                    if (isG2C) {
                        mCdmaNetworkSelectionDetailPref.updateNetworkSearchingState(false);
                    } else {
                        displayEmptyNetworkList(true);
                    }
                }
            }

            // If we are still in manual network selection mode, we need to
            // update the checked state of the carrier list when the device
            // register on network because the OPERATOR_NUMERIC system property
            // has just updated.
            if (regState == ServiceState.REGISTRATION_STATE_HOME_NETWORK
                    || regState == ServiceState.REGISTRATION_STATE_ROAMING) {
                // We need to reset the block flag when we register on network.
                if (mTargetSlot == PhoneConstants.GEMINI_SIM_1) {
                    log("[IRC] onServiceStateChanged blockSwitchPhone flag=false");
                    mGeminiPhone.blockSwitchPhoneDuringManualSelection(false);
                }

                if ((mTargetSlot == PhoneConstants.GEMINI_SIM_1 && !isExternalSlotInCdmaMode())
                        || (mTargetSlot == PhoneConstants.GEMINI_SIM_2 && getPreferenceScreen()
                                .findPreference(KEY_GSM2_NETWORK_SELECTION) != null)) {
                    updateCarrierPreferenceCheckedState();
                }
            }

            // Only works for dual-mode SIM card
            if (mIsDualModeCard) {
                updateScreen();
            }
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mTargetSlot = getIntent().getIntExtra(PhoneConstants.GEMINI_SIM_ID_KEY, -1);
        mExModemSlot = PhoneFactory.getExternalModemSlot();
        mIsExternalSlot = (mTargetSlot == mExModemSlot);
        mShowCdmaNetworkSelection = getResources().getBoolean(R.bool.show_cdma_network_selection);

        mConnectivityManager = (ConnectivityManager)getSystemService(Context.CONNECTIVITY_SERVICE);

        setTitleAndInitPreferences();

        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            mGeminiPhone = (GeminiPhone) PhoneFactory.getDefaultPhone();
        } else {
            mPhone = PhoneFactory.getDefaultPhone();
        }

        if (mIsExternalSlot && isExternalSlotInCdmaMode()) {
            mCurrentSelectionMode = CDMA_SELECTION_MODE;
        } else {
            mCurrentSelectionMode = GSM_SELECTION_MODE;
        }

        mTelephony = ITelephony.Stub.asInterface(ServiceManager.getService("phone"));
        mTelephonyEx = ITelephonyEx.Stub.asInterface(ServiceManager.getService("phoneEx"));
        mTelephonyManager = (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
        mTelephonyManagerEx = TelephonyManagerEx.getDefault();
        mTelephonyManagerEx.listen(mPhoneStateListener, PhoneStateListener.LISTEN_SERVICE_STATE
                | PhoneStateListener.LISTEN_CALL_STATE, mTargetSlot);

        IntentFilter intentFilter = new IntentFilter(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        intentFilter.addAction(TelephonyIntents.ACTION_SIM_INDICATOR_STATE_CHANGED);
        intentFilter.addAction(TelephonyIntents.ACTION_RADIO_TECHNOLOGY_CHANGED);
        intentFilter.addAction(TelephonyIntents.ACTION_SIM_DETECTED);
        intentFilter.addAction(ACTION_PS_DETACH_DONE);

        registerReceiver(mReceiver, intentFilter);

        // If this is slot 2, start and bind service early, so that we can start
        // search gsm network quickly
        if (mTargetSlot != mExModemSlot && mTargetSlot == PhoneConstants.GEMINI_SIM_2) {
            startAndBindNetworkQueryService();
        }

        log("onCreate: mTargetSlot=" + mTargetSlot + ", mExModemSlot=" + mExModemSlot
                + ", mCurrentSelectionMode" + mCurrentSelectionMode
                + ", mShowCdmaNetworkSelection = " + mShowCdmaNetworkSelection);
    }

    @Override
    public void onResume() {
        super.onResume();

        mIsForeground = true;
        mIsDualModeCard = isDualModeCard();
        updateScreen();
    }

    @Override
    protected void onPause() {
        super.onPause();

        mIsForeground = false;
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        unregisterReceiver(mReceiver);
        mCdmaNetworkSelectionDetailPref.setOnPerformSwitchNetowrkListener(null);
        mTelephonyManagerEx
                .listen(mPhoneStateListener, PhoneStateListener.LISTEN_NONE, mTargetSlot);
        if (mNetworkQueryService != null) {
            unbindService(mNetworkQueryServiceConnection);
        }
    }

    private void setTitleAndInitPreferences() {
        // Update Activity's title according to SIM slot.
        int slotResId = (mTargetSlot == PhoneConstants.GEMINI_SIM_1) ? R.string.sim_slot_1
                : R.string.sim_slot_2;
        String simSlot = getResources().getString(slotResId);
        setTitle(getResources()
                .getString(R.string.manual_network_selection_title_with_sim, simSlot));

        addPreferencesFromResource(R.xml.ct_manual_network_selection);

        mCdmaNetworkSelectionPref = findPreference(KEY_CDMA_NETWORK_SELECTION);
        mCdmaNetworkSelectionDetailPref = (CdmaNetworkSwitcherPreference) findPreference(KEY_CDMA_NETWORK_SELECTION_DETAIL);
        mGsm1NetworkSelectionPref = (PreferenceGroup) findPreference(KEY_GSM1_NETWORK_SELECTION);
        mGsm2NetworkSelectionPref = (PreferenceGroup) findPreference(KEY_GSM2_NETWORK_SELECTION);
        mNotesPref = findPreference(KEY_MANUAL_NOTES);

        if (mTargetSlot == PhoneConstants.GEMINI_SIM_1) {
            if (!mShowCdmaNetworkSelection) {
                getPreferenceScreen().removePreference(mCdmaNetworkSelectionPref);
            } else {
                mCdmaNetworkSelectionDetailPref.setOnPerformSwitchNetowrkListener(this);
            }
            getPreferenceScreen().removePreference(mCdmaNetworkSelectionDetailPref);
            getPreferenceScreen().removePreference(mGsm2NetworkSelectionPref);
        } else if (mTargetSlot == PhoneConstants.GEMINI_SIM_2) {
            getPreferenceScreen().removePreference(mCdmaNetworkSelectionPref);
            getPreferenceScreen().removePreference(mCdmaNetworkSelectionDetailPref);
            getPreferenceScreen().removePreference(mGsm1NetworkSelectionPref);
            getPreferenceScreen().removePreference(mNotesPref);
        }
    }

    private void updateScreen() {
        boolean isAirplanMode = Settings.System.getInt(getContentResolver(),
                Settings.System.AIRPLANE_MODE_ON, -1) > 0;
        boolean isRadioOn = false;
        try {
            isRadioOn = mTelephonyEx.isRadioOn(mTargetSlot);
        } catch (RemoteException e) {
            Xlog.w(TAG, "Telephony exception when querying radio state.", e);
        }
        // log("updateScreen: isAirplanMode = " + isAirplanMode +
        // ", isRadioOn = " + isRadioOn);

        if (isRadioOn && !isAirplanMode) {
            if (mTargetSlot == PhoneConstants.GEMINI_SIM_1) {
                final boolean externalSlotInRoaming = SlotSettingsFragment
                        .externalSlotInRoamingService();
                setCdmaNetworkSelectionEnabled(externalSlotInRoaming);
                mGsm1NetworkSelectionPref.setEnabled(mIsDualModeCard && externalSlotInRoaming);
                Phone phone = mGeminiPhone.getPhonebyId(mTargetSlot);
                log("updateScreen: externalSlotInRoaming = " + externalSlotInRoaming + ", phone = "
                        + phone);
                if (phone != null) {
                    if (phone.getPhoneType() == PhoneConstants.PHONE_TYPE_CDMA) {
                        mCdmaNetworkSelectionDetailPref.updateSwitcherState(externalSlotInRoaming);
                    } else if (phone.getPhoneType() == PhoneConstants.PHONE_TYPE_GSM) {
                        boolean isRegistered = (phone.getServiceState().getRegState() == 
                                ServiceState.REGISTRATION_STATE_HOME_NETWORK 
                                || phone.getServiceState().getRegState() == ServiceState.REGISTRATION_STATE_ROAMING);
                        log("updateScreen: GSM phone registered state = " + isRegistered);
                        // TODO: we won't let to switch to CDMA when GSM phone is
                        // searching network, can not use isRegistered as condition.
                        mCdmaNetworkSelectionDetailPref.updateSwitcherState(false);
                    }
                }
            } else if (mTargetSlot == PhoneConstants.GEMINI_SIM_2) {
                mGsm2NetworkSelectionPref.setEnabled(true);
            }

            if (mTelephonyManager.getCallState() != TelephonyManager.CALL_STATE_IDLE) {
                setCdmaNetworkSelectionEnabled(false);
                mGsm1NetworkSelectionPref.setEnabled(false);
            }
        } else {
            if (mTargetSlot == PhoneConstants.GEMINI_SIM_1) {
                setCdmaNetworkSelectionEnabled(false);
                mGsm1NetworkSelectionPref.setEnabled(false);
            } else if (mTargetSlot == PhoneConstants.GEMINI_SIM_2) {
                mGsm2NetworkSelectionPref.setEnabled(false);
            }
        }
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        if (preference == mCdmaNetworkSelectionPref) {
            log("onPreferenceTreeClick cdma network selection, mTargetSlot = " + mTargetSlot);
            if (getPreferenceScreen().findPreference(KEY_CDMA_NETWORK_SELECTION_DETAIL) == null) {
                getPreferenceScreen().addPreference(mCdmaNetworkSelectionDetailPref);
            }
            updateNetworkSelectionMode(CDMA_SELECTION_MODE);
        } else if (preference == mGsm1NetworkSelectionPref) {
            log("onPreferenceTreeClick gsm network selection, mTargetSlot = " + mTargetSlot);
            updateNetworkSelectionMode(GSM_SELECTION_MODE);
        } else if (preference instanceof CarrierRadioPreference) {
            Preference selectedCarrier = preference;
            String networkStr = selectedCarrier.getTitle().toString();
            log("onPreferenceTreeClick: selected network = " + networkStr);
            Message msg = mNetworkSelectionHandler.obtainMessage(EVENT_NETWORK_SELECTION_DONE);
            if (FeatureOption.MTK_GEMINI_SUPPORT) {
                final OperatorInfo ni = mNetworkMap.get(selectedCarrier);
                mGeminiPhone.getPhonebyId(mTargetSlot).selectNetworkManually(ni, msg);
                mSelectedNetwork = ni.getOperatorAlphaLong();
                // Block switch phone when user try to register on a specific
                // GSM network, if it failes, we need auto switch to another GSM
                // network.
                if (mTargetSlot == PhoneConstants.GEMINI_SIM_1) {
                    log("[IRC] onPreferenceTreeClick blockSwitchPhone for network selecton ni.state=" + ni.getState() +
                            ", flag=" + (ni.getState() == OperatorInfo.State.FORBIDDEN));
                    mGeminiPhone.blockSwitchPhoneDuringManualSelection(true);
                }
            } else {
                mPhone.selectNetworkManually(mNetworkMap.get(selectedCarrier), msg);
            }
            SlotSettingsFragment.setGsmAutoNetowrkSelection(this, false);
            displayNetworkSeletionInProgress(networkStr);
        }
        return super.onPreferenceTreeClick(preferenceScreen, preference);
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        Builder builder = new AlertDialog.Builder(this);
        Dialog dialog = null;
        log("onCreateDialog: id = " + id);
        switch (id) {
            case DIALOG_SWITCH_NETWORK_REGISTERED:
                String message = this.getString(R.string.manual_network_register_hint_ok,
                        SystemProperties.get(TelephonyProperties.PROPERTY_OPERATOR_ALPHA));
                dialog = builder.setMessage(message)
                        .setPositiveButton(android.R.string.ok, new OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                log("DIALOG_SWITCH_NETWORK_REGISTERED onClick OK.");
                            }
                        }).create();
                dialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
                break;

            case DIALOG_SWITCH_NETWORK_FAILED:
                dialog = builder.setMessage(getSwitchNetworkFailErrorMessage())
                        .setPositiveButton(android.R.string.ok, new OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                log("DIALOG_SWITCH_NETWORK_FAILED onClick OK");
                            }
                        }).create();
                dialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
                break;
                
            case DIALOG_SWITCH_CDMA_NETWORK_FAILED_DATA_CONNECT:
                dialog = builder.setMessage(getResources().getString(R.string.manual_network_switch_cdma_fail_data_connect))
                .setPositiveButton(android.R.string.ok, new OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        log("DIALOG_SWITCH_CDMA_NETWORK_FAILED_DATA_CONNECT onClick OK");
                    }
                }).create();
                dialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
                break;

            case DIALOG_NETWORK_LIST_LOAD:
                // reinstate the cancelablity of the dialog.
                dialog = new ProgressDialog(this);
                ((ProgressDialog) dialog).setMessage(getResources().getString(
                        R.string.load_networks_progress));
                ((ProgressDialog) dialog).setCancelable(false);
                ((ProgressDialog) dialog).setOnCancelListener(this);
                break;

            case DIALOG_NETWORK_SELECTION:
                dialog = new ProgressDialog(this);
                ((ProgressDialog) dialog).setMessage(mSelectedNetworkText);
                ((ProgressDialog) dialog).setCancelable(false);
                ((ProgressDialog) dialog).setIndeterminate(true);
                break;

            case DIALOG_PHONE_SWITCHING:
                dialog = new ProgressDialog(this);
                ((ProgressDialog) dialog).setMessage(getResources().getString(
                        R.string.manual_network_switching_phone));
                ((ProgressDialog) dialog).setCancelable(false);
                ((ProgressDialog) dialog).setIndeterminate(true);
                break;

            default:
                break;
        }
        return dialog;
    }

    @Override
    public void onPrepareDialog(int id, Dialog dialog) {
        super.onPrepareDialog(id, dialog);
        AlertDialog alertDialog = null;
        switch (id) {
        case DIALOG_SWITCH_NETWORK_REGISTERED:
            String message = this.getString(R.string.manual_network_register_hint_ok,
                    SystemProperties.get(TelephonyProperties.PROPERTY_OPERATOR_ALPHA));
            alertDialog = (AlertDialog) dialog;
            alertDialog.setMessage(message);
            break;

        case DIALOG_SWITCH_NETWORK_FAILED:
            alertDialog = (AlertDialog) dialog;
            alertDialog.setMessage(getSwitchNetworkFailErrorMessage());
            break;
        default:
            break;
        }
    }

    @Override
    public void onCancel(DialogInterface dialog) {
        log("onCancel, force quit gsm network network query. dialog = " + dialog);
        forceQuitGSMNetworkQuery();
        if (mTargetSlot != mExModemSlot) {
            finish();
        }
    }

    // TODO: slot 2 not implemented
    private void updateNetworkSelectionMode(int newMode) {
        log("updateNetworkSelectionMode: mCurrentSelectionMode = " + mCurrentSelectionMode
                + ",newMode = " + newMode + ", mTargetSlot = " + mTargetSlot);
        if (mIsExternalSlot) {
            if (newMode == CDMA_SELECTION_MODE) {
                mCurrentSelectionMode = CDMA_SELECTION_MODE;
                clearGsmNetworkList();
                if (!isExternalSlotInCdmaMode()) {
                    enterCdmaNetworkMode();
                }
            } else if (newMode == GSM_SELECTION_MODE) {
                mCurrentSelectionMode = GSM_SELECTION_MODE;
                getPreferenceScreen().removePreference(mCdmaNetworkSelectionDetailPref);
                if (isExternalSlotInCdmaMode()) {
                    enterGsmNetworkMode();
                } else {
                    if (mNetworkQueryService == null) {
                        startAndBindNetworkQueryService();
                    } else {
                        loadGsmNetworksList();
                    }
                }
            }
            updateScreen();
        } else {
            Xlog.e(TAG, "updateNetworkSelectionMode should not come here");
        }
    }

    /**
     * Switch dual mode phone to CDMA mode.
     */
    private void enterCdmaNetworkMode() {
        int result = forceSwitchPhone();
        log("enterCdmaNetworkMode, mPhoneSwitchingMode=" + mPhoneSwitchingMode + ", result="
                + result + ", mTargetSlot=" + mTargetSlot);
        if (result == InternationalRoamingConstants.SIM_SWITCH_RESULT_SUCCESS) {
            mPhoneSwitchingMode = SWITCH_PROCESS_BEGIN;
            log("Switch phone begin, update network searching state to true.");
            mCdmaNetworkSelectionDetailPref.updateNetworkSearchingState(true);
            showDialog(DIALOG_PHONE_SWITCHING);
        } else {
            handleSwitchPhoneError(result);
        }
    }

    /**
     * Switch dual mode phone to GSM mode.
     */
    private void enterGsmNetworkMode() {
        int result = forceSwitchPhone();
        log("enterGsmNetworkMode, mPhoneSwitchingMode=" + mPhoneSwitchingMode + ", result="
                + result + ", mTargetSlot=" + mTargetSlot);
        if (result == InternationalRoamingConstants.SIM_SWITCH_RESULT_SUCCESS) {
            mPhoneSwitchingMode = SWITCH_PROCESS_BEGIN;
            showDialog(DIALOG_PHONE_SWITCHING);
        } else {
            handleSwitchPhoneError(result);
        }
    }

    private boolean isExternalSlotInCdmaMode() {
        Phone phone = (mGeminiPhone).getPhonebyId(mExModemSlot);
        return (phone.getPhoneType() == PhoneConstants.PHONE_TYPE_CDMA);
    }

    private Phone getCDMAPhone() {
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            PhoneProxy externalPhone = (PhoneProxy) mGeminiPhone.getPhonebyId(PhoneFactory
                    .getExternalModemSlot());
            if (externalPhone != null
                    && externalPhone.getPhoneType() == PhoneConstants.PHONE_TYPE_CDMA) {
                return externalPhone.getActivePhone();
            }
        }
        log("getCDMAPhone return null.");
        return null;
    }

    private PreferenceGroup getGsmNetworkPref(int slotId) {
        if (mTargetSlot == PhoneConstants.GEMINI_SIM_1) {
            return mGsm1NetworkSelectionPref;
        } else if (mTargetSlot == PhoneConstants.GEMINI_SIM_2) {
            return mGsm2NetworkSelectionPref;
        }

        // Should never run into this case.
        Xlog.w(TAG, "Should never run into this case: mTargetSlot = " + mTargetSlot);
        return null;
    }

    private boolean isDualModeCard() {
        int cardType = InternationalRoamingConstants.CARD_TYPE_UNKNOWN;
        try {
            if (mTelephonyEx != null) {
                cardType = mTelephonyEx.getInternationalCardType(mTargetSlot);
            }
        } catch (RemoteException e) {
            Xlog.w(TAG, "Error to getInternationalCardType.", e);
            return false;
        }
        log("isDualModeCard 1 cardtype=" + cardType);

        // TODO: [Workaround]If cardType got from ITelephonyEx is unknown,
        // double query it from IR framework. ITelephonyEx has a bug that when
        // network switched, it cannot get the correct card type
        if (cardType == InternationalRoamingConstants.CARD_TYPE_UNKNOWN) {
            try {
                cardType = Settings.System.getInt(getContentResolver(),
                        PROP_INTERNATIONAL_CARD_TYPE);
            } catch (SettingNotFoundException e) {
                log("isDualModeCard error happened while query Setting system");
            }
            log("isDualModeCard 2 cardtype=" + cardType);
        }
        return cardType == InternationalRoamingConstants.CARD_TYPE_DUAL_MODE;
    }

    private boolean isSimInserted(int slotId) {
        if (mTelephonyManagerEx != null) {
            return mTelephonyManagerEx.hasIccCard(slotId);
        }
        return false;
    }

    private int forceSwitchPhone() {
        return (mGeminiPhone.internationalModeForceSimSwitch(mTargetSlot));
    }

    private void handleSwitchPhoneError(int result) {
        Toast.makeText(this, R.string.manual_network_switch_phone_fail, Toast.LENGTH_LONG).show();
    }

    /** GSM network selection Service connection */
    private final ServiceConnection mNetworkQueryServiceConnection = new ServiceConnection() {

        /** Handle the task of binding the local object to the service */
        public void onServiceConnected(ComponentName className, IBinder service) {
            if (DBG) {
                log("connection created, binding local service.");
            }
            mNetworkQueryService = INetworkQueryService.Stub.asInterface(service);
            // as soon as it is bound, run a query.
            loadGsmNetworksList();
        }

        /** Handle the task of cleaning up the local binding */
        public void onServiceDisconnected(ComponentName className) {
            if (DBG) {
                log("connection disconnected, cleaning local binding.");
            }
            mNetworkQueryService = null;
        }
    };

    /**
     * This implementation of INetworkQueryServiceCallback is used to receive
     * callback notifications from the network query service.
     */
    private final INetworkQueryServiceCallback mCallback = new INetworkQueryServiceCallback.Stub() {

        /** place the message on the looper queue upon query completion. */
        public void onQueryComplete(List<OperatorInfo> networkInfoArray, int status) {
            if (DBG) {
                log("notifying message loop of query completion.");
            }
            Message msg;
            if (mTargetSlot == PhoneConstants.GEMINI_SIM_2) {
                msg = mNetworkSelectionHandler.obtainMessage(EVENT_NETWORK_SCAN_COMPLETED_2,
                        status, 0, networkInfoArray);
            } else {
                msg = mNetworkSelectionHandler.obtainMessage(EVENT_NETWORK_SCAN_COMPLETED, status,
                        0, networkInfoArray);
            }
            msg.sendToTarget();
        }
    };

    private final Handler mNetworkSelectionHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar;
            switch (msg.what) {
                case EVENT_NETWORK_SCAN_COMPLETED:
                    log("EVENT_NETWORK_SCAN_COMPLETED: mTargetSlot = " + mTargetSlot);
                    if (FeatureOption.MTK_GEMINI_SUPPORT
                            && mTargetSlot == PhoneConstants.GEMINI_SIM_2) {
                        return;
                    }
                    gsmNetworksListLoaded((List<OperatorInfo>) msg.obj, msg.arg1);
                    break;

                case EVENT_NETWORK_SCAN_COMPLETED_2:
                    log("EVENT_NETWORK_SCAN_COMPLETED_2: mTargetSlot = " + mTargetSlot);
                    if (FeatureOption.MTK_GEMINI_SUPPORT
                            && mTargetSlot == PhoneConstants.GEMINI_SIM_1) {
                        return;
                    }
                    gsmNetworksListLoaded((List<OperatorInfo>) msg.obj, msg.arg1);
                    break;

                case EVENT_NETWORK_SELECTION_DONE:
                    // Dismiss all dialog when manual select done.
                    if (DBG) {
                        log("hideProgressPanel");
                    }
                    removeDialog(DIALOG_NETWORK_SELECTION);

                    ar = (AsyncResult) msg.obj;
                    if (ar.exception != null) {
                        log("Manual network selection failed!");
                        displayNetworkSelectionFailed(ar.exception);
                    } else {
                        log("Manual network selection succeeded!");
                        displayNetworkSelectionSucceeded();
                    }
                    break;

                default:
                    break;
            }
        }
    };

    private void startAndBindNetworkQueryService() {
        Intent i = new Intent(INetworkQueryService.class.getName());
        i.putExtra(PhoneConstants.GEMINI_SIM_ID_KEY, mTargetSlot);
        startService(i);
        bindService(i, mNetworkQueryServiceConnection, Context.BIND_AUTO_CREATE);
    }

    private void loadGsmNetworksList() {
        if (DBG) {
            log("loadGsmNetworksList...");
        }

        if (FeatureOption.MTK_GEMINI_SUPPORT && !PhoneFactory.isDualTalkMode()) {
            if (mConnectivityManager.getMobileDataEnabled()) {
                int dataConnectionId = (int) Settings.System.getLong(getContentResolver(), 
                        Settings.System.GPRS_CONNECTION_SETTING,
                        Settings.System.GPRS_CONNECTION_SETTING_DEFAULT) - 1;
                log("loadGsmNetworksList, dataConnectionId = " + dataConnectionId + " mTargetSlot = " + mTargetSlot);
                if (dataConnectionId != mTargetSlot) {
                    mForceDisableData = true;
                    log("loadGsmNetworksList, mForceDisableData = " + mForceDisableData);
                    setMobileDataEnabled(false);
                    mWaitDataDisconnected = true;
                }
            }
        }

        loadGsmNetworListImpl();
    }

    private void loadGsmNetworListImpl() {
        if (DBG) {
            log("loadGsmNetworListImpl, mWaitDataDisconnected = " + mWaitDataDisconnected + " mTargetSlot = " + mTargetSlot);
        }
        if (mIsForeground) {
            showDialog(DIALOG_NETWORK_LIST_LOAD);
        }
        if (!mWaitDataDisconnected) {
            networkQuery();
        } else {
            mDataDetachHandler.sendEmptyMessageDelayed(MSG_DATA_DETACH_TIME_OUT, DATA_DETACH_TIME_OUT_DELAY);
        }
    }

    private void networkQuery() {
        log("networkQuery, mWaitDataDisconnected = " + mWaitDataDisconnected + " mTargetSlot = " + mTargetSlot);
        // Delegate query request to the service.
        try {
            mNetworkQueryService.startNetworkQuery(mCallback);
        } catch (RemoteException e) {
            Xlog.w(TAG, "RemoteException when startNetworkQuery.", e);
        }
        displayEmptyNetworkList(true);
    }

    /**
     * networksListLoaded has been rewritten to take an array of OperatorInfo
     * objects and a status field, instead of an AsyncResult. Otherwise, the
     * functionality which takes the OperatorInfo array and creates a list of
     * preferences from it, remains unchanged.
     */
    private void gsmNetworksListLoaded(List<OperatorInfo> result, int status) {
        if (DBG) {
            log("gsmNetworksListLoaded networks list loaded.");
        }

        // Add for dismiss the dialog is showing.
        removeDialog(DIALOG_NETWORK_LIST_LOAD);
        clearGsmNetworkList();

        if (status != QUERY_OK) {
            log("Error happend while querying available networks.");
            displayNetworkQueryFailed(status);
            displayEmptyNetworkList(true);
        } else {
            if (result != null) {
                if (mDelayRegAlertAfterGsmListLoaded) {
                    mDelayRegAlertAfterGsmListLoaded = false;
                    showDialog(DIALOG_SWITCH_NETWORK_REGISTERED);
                }

                displayEmptyNetworkList(false);

                String currentNumeric = "";
                if (mTargetSlot == PhoneConstants.GEMINI_SIM_1) {
                    currentNumeric = SystemProperties.get(
                            TelephonyProperties.PROPERTY_OPERATOR_NUMERIC, "");
                } else if (mTargetSlot == PhoneConstants.GEMINI_SIM_2) {
                    currentNumeric = SystemProperties.get(
                            TelephonyProperties.PROPERTY_OPERATOR_NUMERIC_2, "");
                }

                // Create a preference for each item in the list.
                // Use the operator name instead of the mildly
                // confusing mcc/mnc, add forbidden at the end if needed.
                for (OperatorInfo ni : result) {
                    CarrierRadioPreference carrier = new CarrierRadioPreference(this, null);
                    carrier.setTitle(getNetworkTitle(ni));
                    carrier.setPersistent(false);
                    carrier.setCarrierNumeric(ni.getOperatorNumeric());
                    carrier.setCarrierRate(getNetworkRate(ni));
                    getGsmNetworkPref(mTargetSlot).addPreference(carrier);

                    mNetworkMap.put(carrier, ni);

                    // Set current selected carrier to checked state for both CDMA and GSM slot.
                    carrier.setChecked(ni.getState() == OperatorInfo.State.CURRENT);

                    if (DBG) {
                        log("  " + ni);
                    }
                }
            } else {
                displayEmptyNetworkList(true);
            }
        }
        forceQuitGSMNetworkQuery();
    }

    private void clearGsmNetworkList() {
        for (Preference p : mNetworkMap.keySet()) {
            getGsmNetworkPref(mTargetSlot).removePreference(p);
        }
        mNetworkMap.clear();
    }

    private void displayEmptyNetworkList(boolean isEmpty) {
        if (mTargetSlot == PhoneConstants.GEMINI_SIM_1) {
            if (isEmpty) {
                clearGsmNetworkList();
            }
        } else if (mTargetSlot == PhoneConstants.GEMINI_SIM_2) {
            if (isEmpty) {
                getPreferenceScreen().removePreference(mGsm2NetworkSelectionPref);
            } else {
                if (getPreferenceScreen().findPreference(KEY_GSM2_NETWORK_SELECTION) == null) {
                    getPreferenceScreen().addPreference(mGsm2NetworkSelectionPref);
                }
            }
        }
    }

    /**
     * Returns the title of the network obtained in the manual search.
     * 
     * @param OperatorInfo contains the information of the network.
     * @return Long Name if not null/empty, otherwise Short Name if not
     *         null/empty, else MCCMNC string.
     */
    private String getNetworkTitle(OperatorInfo ni) {
        log("getNetworkTitle: ni = " + ni);
        if (!TextUtils.isEmpty(ni.getOperatorAlphaLong())) {
            return ni.getOperatorAlphaLong() + "(" + ni.getOperatorNumeric() + ")";
        } else if (!TextUtils.isEmpty(ni.getOperatorAlphaShort())) {
            return ni.getOperatorAlphaShort() + "(" + ni.getOperatorNumeric() + ")";
        } else {
            return ni.getOperatorNumeric();
        }
    }

    /**
     * Get the RATE from operator name, 3G means it has 3G capability.
     * 
     * @param ni
     * @return
     */
    private int getNetworkRate(OperatorInfo ni) {
        String operatorLong = ni.getOperatorAlphaLong();
        log("getNetworkType: ni = " + ni + ",operatorLong = " + operatorLong);
        if (!TextUtils.isEmpty(operatorLong) && operatorLong.length() >= 2) {
            String ratStr = operatorLong.substring(operatorLong.length() - 2);
            log("getNetworkType: ratStr = " + ratStr);
            if (ratStr.equals("3G")) {
                return NETWORK_RAT_3G;
            }
        }
        return NETWORK_RAT_2G;
    }

    /**
     * Get the RATE for the network type, only GSM or EDGE is 2G GSM network,
     * type value bigger than UMTS(3) represents 3G.
     * 
     * @param networkType
     * @return
     */
    private int getRateForNetworkType(int networkType) {
        if (networkType >= RIL_RADIO_TECHNOLOGY_UMTS) {
            return NETWORK_RAT_3G;
        }
        return NETWORK_RAT_2G;
    }

    /**
     * Update the checked status of the carrier preference, check both the
     * numerice and the network rate, check the radio of the carrier which
     * matches these two items.
     */
    private void updateCarrierPreferenceCheckedState() {
        String currentNumeric = "";
        String currentNetworkType = "";
        if (mTargetSlot == PhoneConstants.GEMINI_SIM_1) {
            currentNumeric = SystemProperties.get(TelephonyProperties.PROPERTY_OPERATOR_NUMERIC,
                    "0");
            currentNetworkType = SystemProperties.get(TelephonyProperties.PROPERTY_CS_NETWORK_TYPE,
                    "-1");
        } else {
            currentNumeric = SystemProperties.get(TelephonyProperties.PROPERTY_OPERATOR_NUMERIC_2,
                    "-1");
            currentNetworkType = SystemProperties.get(
                    TelephonyProperties.PROPERTY_CS_NETWORK_TYPE_2, "0");
        }
        
        final int currentNetworkRate = getRateForNetworkType(Integer.parseInt(currentNetworkType));
        final PreferenceGroup gsmNetworkPref = getGsmNetworkPref(mTargetSlot);
        final int preferenceCount = gsmNetworkPref.getPreferenceCount();
        log("updateCarrierPreferenceCheckedState: currentNumeric = " + currentNumeric
                + ",currentNetworkType = " + currentNetworkType + ",currentNetworkRate = "
                + currentNetworkRate + ", preferenceCount = " + preferenceCount);
        for (int index = 0; index < preferenceCount; index++) {
            Preference pref = gsmNetworkPref.getPreference(index);
            if (pref instanceof CarrierRadioPreference) {
                CarrierRadioPreference carrierPref = (CarrierRadioPreference) pref;
                carrierPref.setChecked(carrierPref.getCarrierNumeric().equals(currentNumeric)
                        && currentNetworkRate == carrierPref.getCarrierRate());
            }
        }
    }

    private void displayNetworkSeletionInProgress(String networkStr) {
        mSelectedNetworkText = getResources().getString(R.string.register_on_network, networkStr);
        if (mIsForeground) {
            showDialog(DIALOG_NETWORK_SELECTION);
        }
    }

    private void displayNetworkQueryFailed(int error) {
        updateCarrierPreferenceCheckedState();
        String status = getResources().getString(R.string.network_query_error);
        showToastAlert(status);
    }

    private void displayNetworkSelectionSucceeded() {
        updateCarrierPreferenceCheckedState();
        String status = getString(R.string.manual_network_register_hint_ok, mSelectedNetwork);
        log("displayNetworkSelectionSucceeded mTargetSlot=" + mTargetSlot + ", status=" + status);
        showDialogAlert(status);
    }

    private void displayNetworkSelectionFailed(Throwable ex) {
        String status = getString(R.string.manual_network_register_hint_fail, mSelectedNetwork);
        showDialogAlert(status);
    }

    private void showDialogAlert(String message) {
        Builder builder = new AlertDialog.Builder(this);
        Dialog dialog = builder.setMessage(message)
                .setPositiveButton(com.mediatek.internal.R.string.ok, new OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        log("onClick OK.");
                    }
                }).create();
        dialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
        dialog.show();
    }

    private void showToastAlert(String status) {
        Toast.makeText(this, status, Toast.LENGTH_LONG).show();
    }

    @Override
    public void onSwitchNetwork() {
        if (mConnectivityManager.getMobileDataEnabled() 
                && mConnectivityManager.getMobileDataEnabledGemini(PhoneConstants.GEMINI_SIM_1)) {
            log("onSwitchNetwork: because the CDMA data connected, will not switch the network." + 
                "show dialog DIALOG_SWITCH_CDMA_NETWORK_FAILED_DATA_CONNECT.");
            showDialog(DIALOG_SWITCH_CDMA_NETWORK_FAILED_DATA_CONNECT);
            return;
        }

        CDMAPhone cdmaPhone = (CDMAPhone) getCDMAPhone();
        if (cdmaPhone != null) {
            cdmaPhone.setAvoidSYS(true, mCdmaAvoidListHandler.obtainMessage(MSG_SWITCH_NETWORK));
            log("onSwitchNetwork: update network searching state to true.");
            mCdmaNetworkSelectionDetailPref.updateNetworkSearchingState(true);
        } else {
            log("onSwitchNetwork can't get cdma phone, error is happened");
            showToastAlert(getResources().getString(R.string.manual_network_get_cdmaphone_fail));
        }
    }

    private class AvoidSysHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            log("AvoidSysHandler receive " + msg + ", mTargetSlot=" + mTargetSlot);
            AsyncResult ar = (AsyncResult) msg.obj;
            switch (msg.what) {
                case MSG_SWITCH_NETWORK:
                    log("AvoidSysHandler update network searching state to false.");
                    mCdmaNetworkSelectionDetailPref.updateNetworkSearchingState(false);
                    if (null == ar.exception) {
                        mCdmaManuallyNetworkSelectState = CDMA_MANUAL_SWITCH_PROCESS_BEGIN;
                        log("AvoidSysHandler handleMessage msg.what = " + MSG_SWITCH_NETWORK +
                            " mCdmaManuallyNetworkSelectState = " + mCdmaManuallyNetworkSelectState);
                    } else {
                        showDialog(DIALOG_SWITCH_NETWORK_FAILED);
                    }

                    break;

                default:
                    break;
            }
        }
    }

    private void log(String msg) {
        Xlog.d(TAG, "[MNWS" + mTargetSlot + "]" + msg);
    }

    /**
     * Set mobile data enable or disable for GSM manual network selection. 
     * When user need to do GSM manual network selection, the other card data connection 
     * should disable because of modem limitation.
     * @param enable enable is true enable the data connect, or else disable the data connection. 
     */
    private void setMobileDataEnabled(boolean enable) {
        log("setMobileDataEnabled, enable = " + enable);
        if (mConnectivityManager != null) {
            log("setMobileDataEnabled, cm.setMobileDataEnabled = " + enable);
            mConnectivityManager.setMobileDataEnabled(enable);
        }
    }

    @Override
    protected void onStop() {
        super.onStop();
        log("onStop, force quit gsm network network query.");
        forceQuitGSMNetworkQuery();
    }

    private void forceQuitGSMNetworkQuery() {
        log("forceQuitGSMNetworkQuery");
        // Request that the service stop the query with this callback object.
        try {
            if (mNetworkQueryService != null) {
                mNetworkQueryService.stopNetworkQuery(mCallback);
            }
        } catch (RemoteException e) {
            throw new RuntimeException(e);
        }

        removeDialog(DIALOG_NETWORK_LIST_LOAD);

        // Reset to enable the data connection which disabled before GSM manual network selection.
        log("forceQuitGSMNetworkQuery, mForceDisableData =" + mForceDisableData);
        if (mForceDisableData) {
            setMobileDataEnabled(true);
            mForceDisableData = false;
            mWaitDataDisconnected = false;
            mDataDetachHandler.removeMessages(MSG_DATA_DETACH_TIME_OUT);
            mDataDetachHandler.removeMessages(MSG_DATA_DETACHED);
        }
    }

    private String getSwitchNetworkFailErrorMessage() {
        String networkString = null;
        if (mTargetSlot == PhoneConstants.GEMINI_SIM_1) {
            networkString = SystemProperties
                    .get(TelephonyProperties.PROPERTY_OPERATOR_ALPHA);
        } else if (mTargetSlot == PhoneConstants.GEMINI_SIM_2) {
            networkString = SystemProperties
                    .get(TelephonyProperties.PROPERTY_OPERATOR_ALPHA_2);
        }
        String errorMessage;
        if (networkString != null) {
            errorMessage = getString(R.string.manual_network_register_hint_fail,
                    networkString);
        } else {
            errorMessage = getString(R.string.manual_network_register_hint_fail_null);
        }
        return errorMessage;
    }

    private Handler mDataDetachHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            log("mDataDetachHandler receive " + msg + ", mTargetSlot=" + mTargetSlot);
            switch (msg.what) {
                case MSG_DATA_DETACH_TIME_OUT:
                case MSG_DATA_DETACHED:
                    networkQuery();
                    mWaitDataDisconnected = false;
                    break;
                default:
                    break;
            }
        }
    };

    // process cdma select next network manually.
    private void processCdmaSelectNextNetworkManually(ServiceState serviceState) {
        final int regState = serviceState.getRegState();
        log("processCdmaSelectNextNetworkManually, mCdmaManuallyNetworkSelectState = " + mCdmaManuallyNetworkSelectState
                + ", mTargetSlot=" + mTargetSlot + " regState = " + regState);
        if (mCdmaManuallyNetworkSelectState == CDMA_MANUAL_SWITCH_PROCESS_BEGIN) {
            if (regState == ServiceState.REGISTRATION_STATE_NOT_REGISTERED_AND_SEARCHING) {
                mCdmaManuallyNetworkSelectState = CDMA_MANUAL_SWITCH_PROCESS_SEARCHING;
            }
            log("processCdmaSelectNextNetworkManually, mCdmaManuallyNetworkSelectState = " + mCdmaManuallyNetworkSelectState
                    + ", mTargetSlot=" + mTargetSlot + " regState = " + regState);
        } else if ((mCdmaManuallyNetworkSelectState == CDMA_MANUAL_SWITCH_PROCESS_SEARCHING)
                && (regState == ServiceState.REGISTRATION_STATE_HOME_NETWORK 
                    || regState == ServiceState.REGISTRATION_STATE_ROAMING)) {
            mCdmaManuallyNetworkSelectState = CDMA_MANUAL_SWITCH_PROCESS_DONE;
            log("processCdmaSelectNextNetworkManually, mCdmaManuallyNetworkSelectState = " + mCdmaManuallyNetworkSelectState
                    + ", mTargetSlot=" + mTargetSlot + " regState = " + regState
                    + " showDialog(DIALOG_SWITCH_NETWORK_REGISTERED)");
            String message = this.getString(R.string.manual_network_register_hint_ok,
                    SystemProperties.get(TelephonyProperties.PROPERTY_OPERATOR_ALPHA));
            showDialogAlert(message);
        }
    }

    private void setCdmaNetworkSelectionEnabled(boolean enabled) {
        if (mShowCdmaNetworkSelection) {
            mCdmaNetworkSelectionPref.setEnabled(enabled);
        }
    }
}
