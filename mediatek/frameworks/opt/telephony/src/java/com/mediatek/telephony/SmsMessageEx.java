package com.mediatek.telephony;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import android.telephony.Rlog;
import android.os.RemoteException;
import android.os.ServiceManager;

import android.telephony.SmsMessage;
import android.telephony.TelephonyManager;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.ISms;
import com.android.internal.telephony.SmsConstants;
import com.android.internal.telephony.GsmAlphabet;
import com.android.internal.telephony.GsmAlphabet.TextEncodingDetails;

import com.mediatek.common.telephony.ISmsMessageExt;
import com.mediatek.common.featureoption.FeatureOption;

/**
 * Manages SMS raw data parsing functions.
 * @hide
 */
public class SmsMessageEx implements ISmsMessageExt{

    private static final String TAG = "SMS";

    private static final SmsMessageEx sInstance = new SmsMessageEx();

    private SmsMessageEx() {

    }

    /**
     * Gets the default instance of the SmsMessageEx
     *
     * @return the default instance of the SmsMessageEx
     */
    public static SmsMessageEx getDefault() {
        return sInstance;
    }

    /**
     * Gets the SMS service name by specific SIM ID.
     *
     * @param slotId SIM ID
     * @return The SMS service name
     */
    private static String getSmsServiceName(int slotId) {
        if (slotId == PhoneConstants.GEMINI_SIM_1) {
            return "isms";
        } else if (slotId == PhoneConstants.GEMINI_SIM_2) {
            return "isms2";
        } else {
            return null;
        }
    }

    /**
     * @hide
     */
    protected static String getSmsFormat(int simId) {
        String isms = getSmsServiceName(simId);
        try {
            ISms iccISms = ISms.Stub.asInterface(ServiceManager.getService(isms));
            if (iccISms != null) {
                return iccISms.getFormat();
            } else {
                return android.telephony.SmsMessage.FORMAT_3GPP;
            }
        } catch (RemoteException ex) {
            return android.telephony.SmsMessage.FORMAT_3GPP;
        }
    }

    /**
     * Returns the tpdu from the pdu
     *
     * @return the tpdu for the message.
     * @hide
     */
    public byte[] getTpdu(SmsMessage msg, int slotId) {
        if(FeatureOption.EVDO_DT_VIA_SUPPORT == true) {
            String format = getSmsFormat(slotId);
            if (format.equals(SmsConstants.FORMAT_3GPP2)) {
                Rlog.d(TAG, "call getTpdu, format="+format+", slotId="+slotId);
                return msg.getPdu();
            }
        }

        Rlog.d(TAG, "call getTpdu, slotId="+slotId);
        byte[] pdu = msg.getPdu();
        if(pdu == null) {
            Rlog.d(TAG, "pdu is null");
            return null;
        }

        int smsc_len = (pdu[0] & 0xff) + 1;
        int tpdu_len = pdu.length - smsc_len;
        byte[] tpdu = new byte[tpdu_len];

        try {
            System.arraycopy(pdu, smsc_len, tpdu, 0, tpdu.length);
            return tpdu;
        } catch(ArrayIndexOutOfBoundsException e) {
            Rlog.e(TAG, "Out of boudns");
            return null;
        }
    }

    /**
     * Returns the smsc raw data from the pdu
     *
     * @return the raw smsc for the message.
     * @hide
     */
    public byte[] getSmsc(SmsMessage msg, int slotId) {
        if(FeatureOption.EVDO_DT_VIA_SUPPORT == true) {
            String format = getSmsFormat(slotId);
            if (format.equals(SmsConstants.FORMAT_3GPP2)) {
                Rlog.d(TAG, "call getSmsc, format="+format+", slotId="+slotId);
                return null;
            }
        }

        Rlog.d(TAG, "call getSmsc, slotId="+slotId);
        byte[] pdu = msg.getPdu();
        if(pdu == null) {
            Rlog.d(TAG, "pdu is null");
            return null;
        }

        int smsc_len = (pdu[0] & 0xff) + 1;
        byte[] smsc = new byte[smsc_len];

        try {
            System.arraycopy(pdu, 0, smsc, 0, smsc.length);
            return smsc;
        } catch(ArrayIndexOutOfBoundsException e) {
            Rlog.e(TAG, "Out of boudns");
            return null;
        }
    }

    public ArrayList<String> fragmentTextUsingTed(String text, TextEncodingDetails ted, int slotId) {
        boolean useCdmaFormat = false;

        if (!SmsManagerEx.getDefault().isImsSmsSupported(slotId)) {
            // use Voice technology to determine SMS format.
            useCdmaFormat = (TelephonyManager.getDefault().getCurrentPhoneType()
                             == TelephonyManager.PHONE_TYPE_CDMA);
        } else {
        // IMS is registered with SMS support, check the SMS format supported
            useCdmaFormat = (SmsConstants.FORMAT_3GPP2.
                equals(SmsManagerEx.getDefault().getImsSmsFormat(slotId)));
        }

        int limit;
        if (ted.codeUnitSize == SmsConstants.ENCODING_7BIT) {
            int udhLength;
            if (ted.languageTable != 0 && ted.languageShiftTable != 0) {
                udhLength = GsmAlphabet.UDH_SEPTET_COST_TWO_SHIFT_TABLES;
            } else if (ted.languageTable != 0 || ted.languageShiftTable != 0) {
                udhLength = GsmAlphabet.UDH_SEPTET_COST_ONE_SHIFT_TABLE;
            } else {
                udhLength = 0;
            }

            if (ted.msgCount > 1) {
                udhLength += GsmAlphabet.UDH_SEPTET_COST_CONCATENATED_MESSAGE;
            }

            if (udhLength != 0) {
                udhLength += GsmAlphabet.UDH_SEPTET_COST_LENGTH;
            }

            limit = SmsConstants.MAX_USER_DATA_SEPTETS - udhLength;
        } else {
            if (ted.msgCount > 1) {
                limit = SmsConstants.MAX_USER_DATA_BYTES_WITH_HEADER;
            } else {
                limit = SmsConstants.MAX_USER_DATA_BYTES;
            }
        }

        int pos = 0;  // Index in code units.
        int textLen = text.length();
        ArrayList<String> result = new ArrayList<String>(ted.msgCount);
        while (pos < textLen) {
            int nextPos = 0;  // Counts code units.
            if (ted.codeUnitSize == SmsConstants.ENCODING_7BIT) {
                if (useCdmaFormat && ted.msgCount == 1) {
                    // For a singleton CDMA message, the encoding must be ASCII...
                    nextPos = pos + Math.min(limit, textLen - pos);
                } else {
                    // For multi-segment messages, CDMA 7bit equals GSM 7bit encoding (EMS mode).
                    nextPos = GsmAlphabet.findGsmSeptetLimitIndex(text, pos, limit,
                            ted.languageTable, ted.languageShiftTable);
                }
            } else {  // Assume unicode.
                nextPos = pos + Math.min(limit / 2, textLen - pos);
            }
            if ((nextPos <= pos) || (nextPos > textLen)) {
                Rlog.e(TAG, "fragmentText failed (" + pos + " >= " + nextPos + " or " +
                          nextPos + " >= " + textLen + ")");
                break;
            }
            result.add(text.substring(pos, nextPos));
            pos = nextPos;
        }
        return result;
    }

}
