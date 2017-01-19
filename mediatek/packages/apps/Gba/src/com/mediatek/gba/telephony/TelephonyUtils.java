package com.mediatek.gba.telephony;

import android.content.Context;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.util.Log;

import com.android.internal.telephony.IPhoneSubInfo;
import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.uicc.UiccController;
import com.android.internal.util.HexDump;

import com.mediatek.common.telephony.ITelephonyEx;

import com.mediatek.gba.GbaConstant;
import com.mediatek.gba.GbaService;

import java.util.Arrays;
import java.io.UnsupportedEncodingException;

import org.apache.http.protocol.HTTP;

/**
 * implementation for TelephonyUtils.
 *
 * @hide
 */
public class TelephonyUtils {
    private static final String TAG = GbaService.TAG;

    private static final String SIM = "SIM";
    private static final String USIM = "USIM";
    private static final String ISIM = "ISIM";

    private static final int ISIM_GBA_SERVICE = 2;
    private static final int USIM_GBA_SERVICE = 68;

    private static final byte OK_RESPONSE = (byte) 0xDB;

    final protected static char[] hexArray = "0123456789abcdef".toCharArray();
    final protected static char[] hexUpperArray = "0123456789ABCDEF".toCharArray();

    /**
     * Get the card type of UICC/SIM.
     *
     * @return the card type.
     *
     */
    public static int getIccCardType() {
        ITelephonyEx iTelEx = getITelephonyEx();
        int gbaCardType = GbaConstant.GBA_CARD_UNKNOWN;

        gbaCardType = SystemProperties.getInt("gba.card.type", GbaConstant.GBA_CARD_UNKNOWN);
        if(gbaCardType > GbaConstant.GBA_CARD_UNKNOWN && gbaCardType <= GbaConstant.GBA_CARD_ISIM){
            Log.i(TAG, "Gba Card Type:" + gbaCardType);
            return gbaCardType;
        }

        if(getPhoneImpi() != null) {
            Log.d(TAG, "ISIM support");
            return GbaConstant.GBA_CARD_ISIM;
        }

        if (iTelEx == null) {
            Log.e(TAG, "ITelephony is null");
            return GbaConstant.GBA_CARD_UNKNOWN;
        }

        String iccType = "";
        try {
            iccType = iTelEx.getIccCardType(PhoneConstants.GEMINI_SIM_1);
        } catch (RemoteException e) {
            return GbaConstant.GBA_CARD_UNKNOWN;
        }

        Log.d(TAG, "getIccCardType:" + iccType);


        if (USIM.equalsIgnoreCase(iccType)) {
            return GbaConstant.GBA_CARD_USIM;
        } else if (SIM.equalsIgnoreCase(iccType)) {
            return GbaConstant.GBA_CARD_SIM;
        }

        return GbaConstant.GBA_CARD_UNKNOWN;
    }

    /**
     * Check UICC's GBA support is supported or not.
     *
     * @return indicate the UICC's GBA support or not.
     *
     */
    public static int getGbaSupported() {
        ITelephony iTel = getITelephony();

        if (iTel == null) {
            Log.e(TAG, "ITelephony is null");
            return GbaConstant.GBA_NONE;
        }

        IPhoneSubInfo iSubInfo = getSubscriberInfo();

        if (iSubInfo == null) {
            Log.e(TAG, "ITelephony is null");
            return GbaConstant.GBA_NONE;
        }

        try {
            if (!iTel.hasIccCard()) {
                Log.e(TAG, "No ICC Card");
                return GbaConstant.GBA_NONE;
            }

            int cardType = getIccCardType();
            Log.d(TAG, "cardType:" + cardType);
            if (cardType == GbaConstant.GBA_CARD_ISIM) {
                String ist = iSubInfo.getIsimIst();
                Log.d(TAG, "ist:" + ist);

                if (ist == null || ist.length() < 1) {
                    return GbaConstant.GBA_ME;
                }

                byte[] gba = hexToBytes(ist);
                Log.d(TAG, "gba:" + gba[0]);

                if ((gba[0] & ISIM_GBA_SERVICE) == ISIM_GBA_SERVICE) {
                    Log.d(TAG, "Support GBA in ISIM");
                    return GbaConstant.GBA_U;
                }
            } else if (cardType == GbaConstant.GBA_CARD_USIM) {
                if (iSubInfo.getUsimService(USIM_GBA_SERVICE)) {
                    Log.d(TAG, "Support GBA in USIM");
                    return GbaConstant.GBA_U;
                }
            }
        } catch (RemoteException e) {
            return GbaConstant.GBA_NONE;
        }

        Log.d(TAG, "GbaConstant.GBA_ME");
        return GbaConstant.GBA_ME;
    }

    /**
     * Returns the IMS private user identity (IMPI) that was loaded from the ISIM.
     *
     * @return the IMPI, or null if not present or not loaded
     * @hide
     */
    public static String getPhoneImpi() {
        IPhoneSubInfo iSubInfo = getSubscriberInfo();

        if (iSubInfo == null) {
            Log.e(TAG, "iSubInfo is null");
            return null;
        }

        try {
            Log.d(TAG, "getPhoneImpi");
            return iSubInfo.getIsimImpi();
        } catch (RemoteException ex) {
            ex.printStackTrace();
            return null;
        }
    }

    /**
     * Returns the MCC+MNC (mobile country code + mobile network code) of the
     * provider of the SIM. 5 or 6 decimal digits.
     *
     * @return the MCC+MNC
     */
    public static String getSimOperator() {
        ITelephony iTel = getITelephony();

        if (iTel == null) {
            Log.e(TAG, "ITelephony is null");
            return null;
        }

        try {
            return iTel.getSimOperator(PhoneConstants.GEMINI_SIM_1);
        } catch (RemoteException ex) {
            ex.printStackTrace();
            return null;
        }
    }

    /**
     * Utility function for 2G SIM Authentcation.
     * @param rand the random number.
     * @return the response of 2G SIM Authentcation.
     *
     */

    public static byte[] calculate2GSres(byte[] rand) {
        ITelephony iTel = getITelephony();
        byte[] res = null;

        if (iTel == null) {
            Log.e(TAG, "ITelephony is null");
            return null;
        }

        try {
            String strRand = bytesToHex(rand);
            Log.i(TAG, "[2G]rand:" + strRand);
            String strRes = iTel.simAuth(strRand);

            if (strRes != null) {
                res = hexToBytes(strRes);
            }
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }

        return res;
    }

    /**
     * Utility function for 3G AKA Authentcation.
     *
     * @param rand the random number.
     * @param autn the autn value.
     * @return the response of 3G AKA Authentcation.
     *
     */
    public static byte[] calculateAkaAuthAndRes(byte[] rand, byte[] autn) {
        ITelephony iTel = getITelephony();
        byte[] res = null;

        if (iTel == null) {
            Log.e(TAG, "ITelephony is null");
            return null;
        }

        try {
            Log.i(TAG, "[AKA]rand:" + bytesToHex(rand));
            Log.i(TAG, "autn:" + bytesToHex(autn));
            res = iTel.simAkaAuthentication(UiccController.APP_FAM_3GPP, rand, autn,
                        PhoneConstants.GEMINI_SIM_1);
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }

        return res;
    }

    /**
     * Utility function for GBA Authentcation.
     *
     * @param rand the random number.
     * @param autn the autn value.
     * @return the response of GBA Authentcation.
     *
     */
    public static byte[] calculateGbaAuthAndRes(byte[] rand, byte[] autn) {
        ITelephony iTel = getITelephony();
        byte[] res = null;

        if (iTel == null) {
            Log.e(TAG, "ITelephony is null");
            return null;
        }

        try {
            Log.i(TAG, "[GBA]rand:" + bytesToHex(rand));
            Log.i(TAG, "autn:" + bytesToHex(autn));
            res = iTel.simGbaAuthBootStrapMode(UiccController.APP_FAM_IMS, rand, autn,
                        PhoneConstants.GEMINI_SIM_1);
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }

        return res;
    }

    /**
     * Utility function to get EFgbabp information.
     *
     * @return the raw data of EFgbabp.
     *
     */
    public static String getGbaBootstrappingParameters(int cardType) {
        IPhoneSubInfo iSubInfo = getSubscriberInfo();

        if (iSubInfo == null) {
            Log.e(TAG, "iSubInfo is null");
            return null;
        }

        String gbabp = "";

        try {
            if (cardType == GbaConstant.GBA_CARD_ISIM) {
                gbabp = iSubInfo.getIsimGbabp();
            } else if (cardType == GbaConstant.GBA_CARD_USIM) {
                gbabp = iSubInfo.getUsimGbabp();
            }
        } catch (RemoteException e) {
            e.printStackTrace();
        }

        Log.i(TAG, "gbabp:" + gbabp);
        return gbabp;
    }

    /**
     * Utility function to set EFgbabp information.
     *
     * @param rand the random number.
     * @param btid the btid value.
     * @param keyLifetime the life time of NAF_Ks_ext.
     *
     */
    public static void setGbaBootstrappingParameters(int cardType, byte[] rand, String btid, String keyLifetime) {
        IPhoneSubInfo iSubInfo = getSubscriberInfo();

        if (iSubInfo == null) {
            Log.e(TAG, "iSubInfo is null");
            return;
        }

        /* The format of EF_GBABP (GBA Bootstrapping parameters) */
        try {
            String gbabp = String.format("%02x%s%02x%s%02x%s", rand.length,
                                         HexDump.toHexString(rand),
                                         btid.length(), HexDump.toHexString(btid.getBytes()),
                                         keyLifetime.length(),
                                         HexDump.toHexString(keyLifetime.getBytes()));
            Log.i(TAG, "Encoded:" + gbabp);

            if (cardType == GbaConstant.GBA_CARD_ISIM) {
                iSubInfo.setIsimGbabp(gbabp, null);
            } else if (cardType == GbaConstant.GBA_CARD_USIM) {
                iSubInfo.setUsimGbabp(gbabp, null);
            }
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    /**
     * Utility function to calculate Naf_KS_ext by UICC.
     *
     * @param nafId the nafId value.
     * @param impi the impi value.
     * @return the byte array value of Naf_KS_ext.
     *
     */
    public static byte[] calculateNafExternalKey(byte[] nafId, byte[] impi) {
        ITelephony iTel = getITelephony();
        byte[] res = null;

        if (iTel == null) {
            Log.e(TAG, "ITelephony is null");
            return null;
        }

        try {
            Log.i(TAG, "[GBA]Naf Id:" + bytesToHex(nafId));

            if (impi != null) {
                Log.i(TAG, "IMPI:" + bytesToHex(impi));
            }

            byte[] rawRes = iTel.simGbaAuthNafMode(UiccController.APP_FAM_IMS, nafId, impi,
                                PhoneConstants.GEMINI_SIM_1);

            if (rawRes != null && rawRes[0] == OK_RESPONSE) {
                int resLen = rawRes[1];
                res = new byte[resLen];
                res = Arrays.copyOfRange(rawRes, 2, 2 + resLen);
                Log.i(TAG, "key:" + bytesToHex(res));
            } else {
                Log.e(TAG, "Failed to get session key");
            }
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }

        return res;
    }

    /**
     * Utility function to get SIM/UICC's IMSI.
     *
     * @return the IMSI value.
     *
     */
    public static String getImsi() {
        IPhoneSubInfo iSubInfo = getSubscriberInfo();

        if (iSubInfo == null) {
            Log.e(TAG, "iSubInfo is null");
            return null;
        }

        try {
            return iSubInfo.getSubscriberId();
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }

        return null;
    }


    private static ITelephony getITelephony() {
        return ITelephony.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICE));
    }

    private static ITelephonyEx getITelephonyEx() {
        return ITelephonyEx.Stub.asInterface(
                ServiceManager.getService(Context.TELEPHONY_SERVICEEX));
    }

    private static IPhoneSubInfo getSubscriberInfo() {
        // get it each time because that process crashes a lot
        return IPhoneSubInfo.Stub.asInterface(ServiceManager.getService("iphonesubinfo"));
    }

    /**
     * Utility function to get IMEI.
     *
     * @return the IMEI value.
     *
     */
    public static String getImei() {
        IPhoneSubInfo iSubInfo = getSubscriberInfo();
        String imei = "";

        if (iSubInfo == null) {
            Log.e(TAG, "ITelephony is null");
            return null;
        }

        try {
            imei = iSubInfo.getDeviceId();
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }

        return imei;
    }

    /**
     * Utility function to convert byte array to hex string.
     *
     * @param bytes the byte array value.
     * @return the hex string value.
     *
     */
    public static String bytesToHex(byte[] bytes) {
        char[] hexChars = new char[bytes.length * 2];

        for ( int j = 0; j < bytes.length; j++ ) {
            int v = bytes[j] & 0xFF;
            hexChars[j * 2] = hexArray[v >>> 4];
            hexChars[j * 2 + 1] = hexArray[v & 0x0F];
        }

        return new String(hexChars);
    }

    /**
     * Utility function to convert hex string to byte array.
     *
     * @param inputString the hex string value.
     * @return the byte array.
     *
     */
    public static byte[] hexToBytes(String inputString) {
        if (inputString == null) {
            return null;
        }

        int len = inputString.length();
        Log.d(TAG, "hexToBytes: inputLen = " + len);
        byte[] result = new byte[len / 2];
        int[] temp = new int[2];

        for (int i = 0; i < len / 2; i++) {
            temp[0] = inputString.charAt(i * 2);
            temp[1] = inputString.charAt(i * 2 + 1);

            for (int j = 0; j < 2; j++) {
                if (temp[j] >= 'A' && temp[j] <= 'F') {
                    temp[j] = temp[j] - 'A' + 10;
                } else if (temp[j] >= 'a' && temp[j] <= 'f') {
                    temp[j] = temp[j] - 'a' + 10;
                } else if (temp[j] >= '0' && temp[j] <= '9') {
                    temp[j] = temp[j] - '0';
                } else {
                    return null;
                }
            }

            result[i] = (byte) (temp[0] << 4);
            result[i] |= temp[1];
        }

        return result;
    }
}