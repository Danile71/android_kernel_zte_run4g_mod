package com.mediatek.engineermode.networkselect;

import android.app.Activity;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Spinner;
import android.widget.Toast;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.RILConstants;
import com.android.internal.telephony.gemini.GeminiPhone;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.telephony.ITelephonyEx;
import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

/**
 * 
 * For setting network mode
 * @author mtk54043
 *
 */
public class NetworkSelectActivity extends Activity {
    private static final String TAG = "EM/NetworkMode";
    private static final int EVENT_QUERY_NETWORKMODE_DONE = 101;
    private static final int EVENT_SET_NETWORKMODE_DONE = 102;

    private static final int MODEM_FDD = 1;
    private static final int MODEM_TD = 2;
    private static final int MODEM_NO3G = 3;

    private static final int MODEM_MASK_WCDMA = 0x04;
    private static final int MODEM_MASK_TDSCDMA = 0x08;

    private static final int WCDMA_PREFERRED_INDEX = 0;
    private static final int GSM_ONLY_INDEX = 1;
    private static final int WCDMA_ONLY_INDEX = 2;
    private static final int GSM_WCDMA_AUTO_INDEX = 3;
    private static final int LTE_ONLY_INDEX = 4;
    private static final int LTE_GSM_WCDMA_INDEX = 5;
    private static final int LTE_WCDMA_INDEX = 6;
    private static final int LTE_GSM_INDEX = 7;

    private static final int GSM_ONLY_INDEX_TD = 0;
    private static final int WCDMA_ONLY_INDEX_TD = 1;
    private static final int GSM_WCDMA_AUTO_INDEX_TD = 2;
    private static final int LTE_ONLY_INDEX_TD = 3;
    private static final int LTE_GSM_WCDMA_INDEX_TD = 4;
    private static final int LTE_WCDMA_INDEX_TD = 5;
    private static final int LTE_GSM_INDEX_TD = 6;

    private static final int WCDMA_PREFERRED = Phone.NT_MODE_WCDMA_PREF;
    private static final int GSM_ONLY = Phone.NT_MODE_GSM_ONLY;
    private static final int WCDMA_ONLY = Phone.NT_MODE_WCDMA_ONLY;
    private static final int GSM_WCDMA_AUTO = Phone.NT_MODE_GSM_UMTS;
    private static final int LTE_ONLY = Phone.NT_MODE_LTE_ONLY;
    private static final int LTE_GSM_WCDMA = Phone.NT_MODE_LTE_GSM_WCDMA;
    private static final int LTE_GSM_WCDMA_PREFERRED = RILConstants.NETWORK_MODE_LTE_GSM_WCDMA_PREF;
    private static final int LTE_WCDMA = Phone.NT_MODE_LTE_WCDMA;
    private static final int LTE_GSM = Phone.NT_MODE_LTE_GSM;

    private Phone mPhone = null;
    private GeminiPhone mGeminiPhone = null;

    private int mModemType;
    private int mSimType = PhoneConstants.GEMINI_SIM_1;
    private int[] mNetworkTypeValues;
    private int mCurrentSelected = 0;
    private int mUserSelectMode = -1;
    private ITelephonyEx mITelephonyEx = null;

    private Spinner mPreferredNetworkSpinner = null;

    AdapterView.OnItemSelectedListener mPreferredNetworkHandler = new AdapterView.OnItemSelectedListener() {
        public void onItemSelected(AdapterView parent, View v, int pos, long id) {
            Xlog.d(TAG, "onItemSelected " + pos);
            if (mCurrentSelected == pos) {
                return; // avoid listener being invoked by setSelection()
            }
            mCurrentSelected = pos;

            Message msg = mHandler.obtainMessage(EVENT_SET_NETWORKMODE_DONE);

            int settingsNetworkMode;
            if (mSimType == PhoneConstants.GEMINI_SIM_1 || !FeatureOption.MTK_DT_SUPPORT) {
                settingsNetworkMode = android.provider.Settings.Global.getInt(getContentResolver(),
                                              android.provider.Settings.Global.PREFERRED_NETWORK_MODE, Phone.PREFERRED_NT_MODE);
            } else {
                settingsNetworkMode = android.provider.Settings.Global.getInt(getContentResolver(),
                    android.provider.Settings.Global.PREFERRED_NETWORK_MODE_2, Phone.PREFERRED_NT_MODE);
            }
            int selectNetworkMode = mNetworkTypeValues[pos];

            if (settingsNetworkMode != selectNetworkMode) {
                Xlog.d(TAG, "selectNetworkMode " + selectNetworkMode);
                mUserSelectMode = selectNetworkMode;
                Xlog.d(TAG, "mUserSelectMode: " + mUserSelectMode);
                android.provider.Settings.Global.putInt(getContentResolver(),
                        android.provider.Settings.Global.USER_PREFERRED_NETWORK_MODE,
                        mUserSelectMode);
                if (FeatureOption.MTK_GEMINI_SUPPORT) {
                    mGeminiPhone.setPreferredNetworkTypeGemini(selectNetworkMode, msg, mSimType);
                } else {
                    mPhone.setPreferredNetworkType(selectNetworkMode, msg);
                }
            }
        }

        public void onNothingSelected(AdapterView parent) {

        }
    };

    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            AsyncResult ar;
            switch (msg.what) {
            case EVENT_QUERY_NETWORKMODE_DONE:
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    int type = ((int[]) ar.result)[0];
                    Xlog.d(TAG, "Get Preferred Type " + type);
                    int index = -1;
                    if (mModemType == MODEM_FDD) {
                        switch (type) {
                        case WCDMA_PREFERRED:
                            index = WCDMA_PREFERRED_INDEX;
                            break;
                        case GSM_ONLY:
                            index = GSM_ONLY_INDEX;
                            break;
                        case WCDMA_ONLY:
                            index = WCDMA_ONLY_INDEX;
                            break;
                        case GSM_WCDMA_AUTO:
                            index = GSM_WCDMA_AUTO_INDEX;
                            break;
                        case LTE_ONLY:
                            index = LTE_ONLY_INDEX;
                            break;
                        case LTE_GSM_WCDMA:
                        case LTE_GSM_WCDMA_PREFERRED:
                            index = LTE_GSM_WCDMA_INDEX;
                            break;
                        case LTE_WCDMA:
                            index = LTE_WCDMA_INDEX;
                            break;
                        case LTE_GSM:
                            index = LTE_GSM_INDEX;
                            break;
                        default:
                            break;
                        }
                    } else {
                        switch (type) {
                        case WCDMA_PREFERRED:
                            // "CDMA preferred" is the same with "Auto" on TD
                            index = GSM_WCDMA_AUTO_INDEX_TD;
                            break;
                        case GSM_ONLY:
                            index = GSM_ONLY_INDEX_TD;
                            break;
                        case WCDMA_ONLY:
                            index = WCDMA_ONLY_INDEX_TD;
                            break;
                        case GSM_WCDMA_AUTO:
                            index = GSM_WCDMA_AUTO_INDEX_TD;
                            break;
                        case LTE_ONLY:
                            index = LTE_ONLY_INDEX_TD;
                            break;
                        case LTE_GSM_WCDMA:
                        case LTE_GSM_WCDMA_PREFERRED:
                            index = LTE_GSM_WCDMA_INDEX_TD;
                            break;
                        case LTE_WCDMA:
                            index = LTE_WCDMA_INDEX_TD;
                            break;
                        case LTE_GSM:
                            index = LTE_GSM_INDEX_TD;
                            break;
                        default:
                            break;
                        }
                    }
                    if (index >= 0) {
                        mCurrentSelected = index;
                        mPreferredNetworkSpinner.setSelection(index, true);
                    }
                } else {
                    Toast.makeText(NetworkSelectActivity.this, R.string.query_preferred_fail, Toast.LENGTH_SHORT).show();
                }
                break;
            case EVENT_SET_NETWORKMODE_DONE:
                ar = (AsyncResult) msg.obj;
                if (ar.exception != null) {
                    if (FeatureOption.MTK_GEMINI_SUPPORT) {
                        mGeminiPhone.getPhonebyId(mSimType).getPreferredNetworkType(
                                obtainMessage(EVENT_QUERY_NETWORKMODE_DONE));
                    } else {
                        mPhone.getPreferredNetworkType(obtainMessage(EVENT_QUERY_NETWORKMODE_DONE));
                    }
                } else {
                    Xlog.d(TAG, "mUserSelectMode: " + mUserSelectMode);
                    android.provider.Settings.Global.putInt(getContentResolver(),
                            android.provider.Settings.Global.USER_PREFERRED_NETWORK_MODE, mUserSelectMode);
                }
                break;
            default:
                break;
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.networkmode_switching);
        mITelephonyEx = ITelephonyEx.Stub.asInterface(ServiceManager.getService("phoneEx"));

        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            mGeminiPhone = (GeminiPhone) PhoneFactory.getDefaultPhone();
        } else {
            mPhone = PhoneFactory.getDefaultPhone();
        }
        mPreferredNetworkSpinner = (Spinner) findViewById(R.id.networkModeSwitching);
    }

    @Override
    protected void onResume() {
        mSimType = GeminiPhone.get3GSimId();
        Xlog.d(TAG, "get3GSimId(): " + mSimType);
        mSimType = getIntent().getIntExtra("mSimType", mSimType);

        if (mSimType < 0 || mSimType > PhoneConstants.GEMINI_SIM_NUM) {
            Xlog.d(TAG, "3G/4G is Off.");
            mSimType = PhoneConstants.GEMINI_SIM_1;
        }

        mModemType = getModemType();
        if (mModemType == MODEM_TD) {
            // No "CDMA preferred" for TD
            mNetworkTypeValues = new int[] {GSM_ONLY, WCDMA_ONLY, GSM_WCDMA_AUTO, LTE_ONLY, LTE_GSM_WCDMA, LTE_WCDMA, LTE_GSM};
            ArrayAdapter<String> adapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item,
                    getResources().getStringArray(R.array.mTddNetworkLabels));
            adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            mPreferredNetworkSpinner.setAdapter(adapter);
        } else if (mModemType == MODEM_FDD) {
            mNetworkTypeValues = new int[] {WCDMA_PREFERRED, GSM_ONLY, WCDMA_ONLY, GSM_WCDMA_AUTO, LTE_ONLY, LTE_GSM_WCDMA, LTE_WCDMA, LTE_GSM};
            ArrayAdapter<String> adapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item,
                    getResources().getStringArray(R.array.mWcdmaNetworkLabels));
            adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            mPreferredNetworkSpinner.setAdapter(adapter);
        } else {
//            mPreferredNetworkSpinner.setEnabled(false);
            Xlog.w(TAG, "Isn't TD/WCDMA modem: " + mModemType);
        }
        mPreferredNetworkSpinner.setOnItemSelectedListener(mPreferredNetworkHandler);

        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            mGeminiPhone.getPhonebyId(mSimType).getPreferredNetworkType(
                    mHandler.obtainMessage(EVENT_QUERY_NETWORKMODE_DONE));
        } else {
            mPhone.getPreferredNetworkType(mHandler.obtainMessage(EVENT_QUERY_NETWORKMODE_DONE));
        }
        super.onResume();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    private int getModemType() {
        String mt;
        if (mSimType == PhoneConstants.GEMINI_SIM_1) {
            mt = SystemProperties.get("gsm.baseband.capability");
            Xlog.i(TAG, "gsm.baseband.capability " + mt);
        } else {
            mt = SystemProperties.get("gsm.baseband.capability2");
            Xlog.i(TAG, "gsm.baseband.capability2 " + mt);
        }
        int mode = MODEM_NO3G;

        if (mt == null) {
            mode = MODEM_NO3G;
        } else {
            try {
                int mask = Integer.valueOf(mt);
                if ((mask & MODEM_MASK_TDSCDMA) == MODEM_MASK_TDSCDMA) {
                    mode = MODEM_TD;
                } else if ((mask & MODEM_MASK_WCDMA) == MODEM_MASK_WCDMA) {
                    mode = MODEM_FDD;
                } else {
                    mode = MODEM_NO3G;
                }
            } catch (NumberFormatException e) {
                mode = MODEM_NO3G;
            }
        }
        return mode;
    }
}
