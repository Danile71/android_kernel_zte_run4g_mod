package com.mediatek.calloption.plugin;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.provider.Contacts.People;
import android.provider.Contacts.Intents.Insert;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyProperties;

import com.android.phone.Constants;
import com.mediatek.common.telephony.ITelephonyEx;
import com.mediatek.telephony.TelephonyManagerEx;

public class OP09CallOptionUtils {

    private static final String TAG = "OP09CallOptionUtils";
    private static String[] ITEL_PROPERTY_ICC_OPERATOR_NUMERIC = {
        TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC,
        TelephonyProperties.PROPERTY_OPERATOR_NUMERIC_2,
    };
    public static final String MTK_GEMINI_SUPPORT = "ro.mediatek.gemini_support";

    public static Intent getCallIntent(String number) {
        final Intent intent = new Intent(Intent.ACTION_CALL_PRIVILEGED, getCallUri(number));
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        return intent;
    }

    public static Intent getCallIntentWithSlot(String number, int slot) {
        final Intent intent = new Intent(Intent.ACTION_CALL_PRIVILEGED, getCallUri(number));
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.putExtra(Constants.EXTRA_SLOT_ID, slot);
        return intent;
    }

    public static Intent getCallIntentWithOrigin(String number, String callOrigin) {
        final Intent intent = getCallIntent(number);
        if (callOrigin != null) {
            intent.putExtra(Constants.EXTRA_ORIGINAL_SIM_ID, callOrigin);
        }
        return intent;
    }

    public static Intent getIPCallIntent(String number) {
        final Intent intent = getCallIntent(number);
        intent.putExtra(Constants.EXTRA_IS_IP_DIAL, true);
        return intent;
    }

    public static Intent getSMSIntent(String number) {
        Uri uri = Uri.fromParts("sms", number, null);
        return new Intent(Intent.ACTION_VIEW, uri);
    }

    public static Intent getAddToContactIntent(String number) {
        final Intent intent = new Intent(Intent.ACTION_INSERT_OR_EDIT);
        intent.putExtra(Insert.PHONE, number);
        /// !!!! need to check below code
        //intent.putExtra("fromWhere", "CALL_LOG");
        intent.setType(People.CONTENT_ITEM_TYPE);
        return intent;
    }

    public static Intent getCopyToDialerIntent(String number) {
        return new Intent(Intent.ACTION_DIAL, getCallUri(number));
    }

    private static Uri getCallUri(String number) {
        // !!!!! need to check below code again
        /*if (FeatureOption.MTK_GEMINI_SUPPORT) {
            return Uri.fromParts(Constants.SCHEME_TEL, number, null);
        } else {
            if (PhoneNumberUtils.isUriNumber(number)) {
                return Uri.fromParts(Constants.SCHEME_SIP, number, null);
            }*/
            return Uri.fromParts(Constants.SCHEME_TEL, number, null);
        //}
        /** @ } */
    }

    public static boolean isSimInsert(final int slot) {
        final ITelephonyEx iTel = ITelephonyEx.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICEEX));
        boolean isSimInsert = false;
        try {
            if (iTel != null) {
                if (SystemProperties.getBoolean(MTK_GEMINI_SUPPORT, false)) {
                    isSimInsert = iTel.hasIccCard(slot);
                } else {
                    isSimInsert = iTel.hasIccCard(0);
                }
            }
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        return isSimInsert;
    }

    public static boolean isCDMAPhoneTypeBySlot(final int slot) {
        TelephonyManagerEx telephony = TelephonyManagerEx.getDefault();
        
        return telephony.getPhoneType(slot) == PhoneConstants.PHONE_TYPE_CDMA;
    }

    public static boolean isNetworkRoaming(int slot) {
        if (SystemProperties.getBoolean(MTK_GEMINI_SUPPORT, false)) {
            return true;
        } else {
            return TelephonyManager.getDefault().isNetworkRoaming();
        }
    }


    /**
     * @param slot id
     * @return mcc
     */
    public static String getCountryBySlotId(int slotId) {
        log("getCountryBySlotId() slotId : " + slotId);
        final ITelephonyEx iTel = ITelephonyEx.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICEEX));
        final TelephonyManager telephonyManager = TelephonyManager.getDefault();
        String mccMnc = null;
        String mcc = null;
        try {
            if (null != iTel && null != telephonyManager) {
                if (SystemProperties.getBoolean(MTK_GEMINI_SUPPORT, false)) {
                    if (iTel.hasIccCard(slotId)) {
                        mccMnc = SystemProperties.get(ITEL_PROPERTY_ICC_OPERATOR_NUMERIC[slotId]);
                    } else {
                        log("there is no valid SIM card in the gemini phone!");
                    }
                } else {
                    if (iTel.hasIccCard(0)) {
                        mccMnc = telephonyManager.getSimOperator();
                    } else {
                        log("there is no valid SIM card in the phone!");
                    }
                }
            }
        } catch (RemoteException e) {
            e.printStackTrace();
        }

        log("getCountryBySlotId() mccMnc : " + mccMnc);
        if (null != mccMnc && mccMnc.length() > 3) {
            mcc = mccMnc.substring(0, 3);
        }
        return mcc;
    }

    private static void log(String msg) {
        Log.d(TAG, msg);
    }

}
