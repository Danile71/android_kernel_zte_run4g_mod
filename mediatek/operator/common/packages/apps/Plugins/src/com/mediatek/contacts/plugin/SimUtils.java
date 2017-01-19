package com.mediatek.contacts.plugin;

import com.mediatek.contacts.ext.Anr;
import com.mediatek.contacts.ext.ContactAccountExtension;
import com.android.internal.telephony.IIccPhoneBook;
import com.android.internal.telephony.EncodeException;
import com.android.internal.telephony.GsmAlphabet;
import com.mediatek.common.telephony.AlphaTag;
import com.mediatek.common.telephony.ITelephonyEx;

import android.content.ContentValues;
import android.content.Context;
import android.content.res.Resources;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.text.TextUtils;
import android.util.Log;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;

public class SimUtils {
    private static final String TAG = "SimUtils";

    public static final String KEY_SLOT = "slot";

    private static final boolean MTK_GEMINI_SUPPORT = com.mediatek.common.featureoption.FeatureOption.MTK_GEMINI_SUPPORT;
    private static final int SLOT_ID1 = com.android.internal.telephony.PhoneConstants.GEMINI_SIM_1;
    private static final int SLOT_ID2 = com.android.internal.telephony.PhoneConstants.GEMINI_SIM_2;
    private static final String SIMPHONEBOOK_SERVICE = "simphonebook";
    private static final String SIMPHONEBOOK2_SERVICE = "simphonebook2";

    private static final int SIM_TYPE_SIM = 0;
    private static final int SIM_TYPE_USIM = 1;
    private static final int SIM_TYPE_UIM = 2;
    private static final String SIM_TYPE_USIM_TAG = "USIM";
    private static final String SIM_TYPE_UIM_TAG = "UIM";

    public static final String ACCOUNT_TYPE_SIM = "SIM Account";
    public static final String ACCOUNT_TYPE_USIM = "USIM Account";
    public static final String ACCOUNT_TYPE_UIM = "UIM Account";

    public static final String IS_ADDITIONAL_NUMBER = "1";

    private static int[] MAX_USIM_AAS_NAME_LENGTH = { -1, -1 };
    private static int[] MAX_USIM_AAS_COUNT = { -1, -1 };
    private static int[] MAX_USIM_ANR_COUNT = { -1, -1 };
    private static HashMap<Integer, List<AlphaTag>> sAasMap = new HashMap<Integer, List<AlphaTag>>(2);

    private static final int ERROR = -1;

    private static int sCurSlotId = -1;
    private static String sCurrentAccount = null;

    public static void setCurrentSlot(int slotId) {
        sCurSlotId = slotId;
        sCurrentAccount = getAccountTypeBySlot(slotId);
        LogUtils.d(TAG, "setCurrentSlot() sCurSlotId=" + sCurSlotId + " sCurrentAccount=" + sCurrentAccount);
    }

    public static String getCurAccount() {
        LogUtils.d(TAG, "getCurAccount() sCurrentAccount=" + sCurrentAccount);
        return sCurrentAccount;
    }

    public static int getCurSlotId() {
        LogUtils.d(TAG, "getCurSlotId() sCurSlotId=" + sCurSlotId);
        return sCurSlotId;
    }

    public static String getAccountTypeBySlot(int slotId) {
        LogUtils.d(TAG, "[getAccountTypeBySlot] slotId:" + slotId);
        if (slotId < SLOT_ID1 || slotId > SLOT_ID2) {
            LogUtils.e(TAG, "[getAccountTypeBySlot]Error slotid:" + slotId);
            return null;
        }
        int simtype = SIM_TYPE_SIM;
        String simAccountType = ACCOUNT_TYPE_SIM;

        if (isSimInserted(slotId)) {
            simtype = getSimTypeBySlot(slotId);
            if (SIM_TYPE_USIM == simtype) {
                simAccountType = ACCOUNT_TYPE_USIM;
            } else if (SIM_TYPE_UIM == simtype) { // UIM
                simAccountType = ACCOUNT_TYPE_UIM;
            }

        } else {
            LogUtils.e(TAG, "[getAccountTypeBySlot]Error slotId:" + slotId + " no sim inserted!");
            simAccountType = null;
        }
        LogUtils.d(TAG, "[getAccountTypeBySlot] accountType:" + simAccountType);
        return simAccountType;
    }

    private static boolean isSimInserted(int slotId) {
        final ITelephonyEx iTel = ITelephonyEx.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICEEX));
        boolean isSimInsert = false;
        try {
            if (iTel != null) {
                if (MTK_GEMINI_SUPPORT) {
                    isSimInsert = iTel.hasIccCard(slotId);
                } else {
                    isSimInsert = iTel.hasIccCard(SLOT_ID1);
                }
            }
        } catch (RemoteException e) {
            e.printStackTrace();
            isSimInsert = false;
        }
        return isSimInsert;
    }

    private static int getSimTypeBySlot(int slotId) {
        final ITelephonyEx iTel = ITelephonyEx.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICEEX));
        int simType = SIM_TYPE_SIM;
        try {
            if (MTK_GEMINI_SUPPORT) {
                if (SIM_TYPE_USIM_TAG.equals(iTel.getIccCardType(slotId))) {
                    simType = SIM_TYPE_USIM;
                } else if (SIM_TYPE_UIM_TAG.equals(iTel.getIccCardType(slotId))) {
                    simType = SIM_TYPE_UIM;
                }
            } else {
                if (SIM_TYPE_USIM_TAG.equals(iTel.getIccCardType(SLOT_ID1))) {
                    simType = SIM_TYPE_USIM;
                } else if (SIM_TYPE_UIM_TAG.equals(iTel.getIccCardType(SLOT_ID1))) {
                    simType = SIM_TYPE_UIM;
                }
            }
        } catch (Exception e) {
            LogUtils.e(TAG, "[getSimTypeBySlot] catched exception.");
            e.printStackTrace();
        }
        return simType;
    }

    /**
     * refresh local aas list. after you change the USim card aas info, please refresh local info.
     * @param slot
     * @return
     */
    public static boolean refreshAASList(int slot) {
        if (slot < SLOT_ID1 || slot > SLOT_ID2) {
            LogUtils.d(TAG, "refreshAASList() slot=" + slot);
            return false;
        }

        try {
            final IIccPhoneBook iIccPhb = getIIccPhoneBook(slot);
            if (iIccPhb != null) {
                List<AlphaTag> atList = iIccPhb.getUsimAasList();
                Iterator<AlphaTag> iter = atList.iterator();
                while (iter.hasNext()) {
                    AlphaTag entry = iter.next();
                    String tag = entry.getAlphaTag();
                    if (TextUtils.isEmpty(tag)) {
                        iter.remove();
                    }
                    LogUtils.d(TAG, "refreshAASList. tag=" + tag);
                }
                sAasMap.put(slot, atList);
            }
        } catch (Exception e) {
            LogUtils.d(TAG, "catched exception.");
            sAasMap.put(slot, null);
        }

        return true;
    }

    /**
     * get USim card aas info without null tag. It will return all aas info that can be used in
     * application.
     * @param slot
     * @return
     */
    public static List<AlphaTag> getAAS(int slot) {
        List<AlphaTag> atList = new ArrayList<AlphaTag>();
        if (slot < SLOT_ID1 || slot > SLOT_ID2) {
            LogUtils.e(TAG, "getAAS() slot=" + slot);
            return atList;
        }
        // Here, force to refresh the list.
        refreshAASList(slot);

        List<AlphaTag> list = sAasMap.get(slot);

        return list != null ? list : atList;
    }

    /**
     * Get the max length of AAS.
     * @param slot The USIM Card slotId
     * @return
     */
    public static int getAASTextMaxLength(int slot) {
        if (slot < SLOT_ID1 || slot > SLOT_ID2) {
            LogUtils.e(TAG, "getAASMaxLength() slot=" + slot);
            return ERROR;
        }
        LogUtils.d(TAG, "getAASMaxLength() slot:" + slot + "|maxNameLen:" + MAX_USIM_AAS_NAME_LENGTH[slot]);
        if (MAX_USIM_AAS_NAME_LENGTH[slot] < 0) {
            try {
                final IIccPhoneBook iIccPhb = getIIccPhoneBook(slot);
                if (iIccPhb != null) {
                    MAX_USIM_AAS_NAME_LENGTH[slot] = iIccPhb.getUsimAasMaxNameLen();
                }
            } catch (android.os.RemoteException e) {
                LogUtils.d(TAG, "catched exception.");
                MAX_USIM_AAS_NAME_LENGTH[slot] = -1;
            }
        }
        LogUtils.d(TAG, "getAASMaxLength() end slot:" + slot + "|maxNameLen:" + MAX_USIM_AAS_NAME_LENGTH[slot]);
        return MAX_USIM_AAS_NAME_LENGTH[slot];
    }

    public static String getAASById(int slotId, int index) {
        if (slotId < SLOT_ID1 || slotId > SLOT_ID2 || index < 1) {
            return "";
        }
        String aas = "";
        try {
            final IIccPhoneBook iIccPhb = getIIccPhoneBook(slotId);
            if (iIccPhb != null) {
                aas = iIccPhb.getUsimAasById(index);
            }
        } catch (Exception e) {
            LogUtils.e(TAG, "getUSIMAASById() catched exception.");
        }
        if (aas == null) {
            aas = "";
        }
        LogUtils.d(TAG, "getUSIMAASById() aas=" + aas);
        return aas;
    }

    public static int getAasIndexByName(String aas, int slotId) {
        if (slotId < SLOT_ID1 || slotId > SLOT_ID2 || TextUtils.isEmpty(aas)) {
            LogUtils.e(TAG, "getAasIndexByName() error slotId=" + slotId + "aas=" + aas);
            return ERROR;
        }
        // here, it only can compare type name
        LogUtils.d(TAG, "getAasIndexByName, tag=" + aas);
        List<AlphaTag> atList = getAAS(slotId);
        Iterator<AlphaTag> iter = atList.iterator();
        while (iter.hasNext()) {
            AlphaTag entry = iter.next();
            String tag = entry.getAlphaTag();
            if (aas.equalsIgnoreCase(tag)) {
                LogUtils.d(TAG, "getAasIndexByName, tag=" + tag);
                return entry.getRecordIndex();
            }
        }
        return ERROR;
    }

    public static int insertUSIMAAS(int slotId, String aasName) {
        if (slotId < SLOT_ID1 || slotId > SLOT_ID2 || TextUtils.isEmpty(aasName)) {
            return ERROR;
        }
        int result = ERROR;
        try {
            final IIccPhoneBook iIccPhb = getIIccPhoneBook(slotId);
            if (iIccPhb != null) {
                result = iIccPhb.insertUsimAas(aasName);
            }
        } catch (Exception e) {
            LogUtils.e(TAG, "insertUSIMAAS() catched exception.");
        }

        return result;
    }

    public static boolean updateUSIMAAS(int slotId, int index, int pbrIndex, String aasName) {
        if (slotId < SLOT_ID1 || slotId > SLOT_ID2) {
            return false;
        }
        boolean result = false;
        try {
            final IIccPhoneBook iIccPhb = getIIccPhoneBook(slotId);
            if (iIccPhb != null) {
                result = iIccPhb.updateUsimAas(index, pbrIndex, aasName);
            }
        } catch (Exception e) {
            LogUtils.e(TAG, "updateUSIMAAS() catched exception.");
        }
        refreshAASList(slotId);

        return result;
    }

    public static boolean removeUSIMAASById(int slotId, int index, int pbrIndex) {
        if (slotId < SLOT_ID1 || slotId > SLOT_ID2) {
            return false;
        }
        boolean result = false;
        try {
            final IIccPhoneBook iIccPhb = getIIccPhoneBook(slotId);
            if (iIccPhb != null) {
                result = iIccPhb.removeUsimAasById(index, pbrIndex);
            }
        } catch (Exception e) {
            LogUtils.e(TAG, "removeUSIMAASById() catched exception.");
        }
        refreshAASList(slotId);

        return result;
    }

    public static boolean isAasTextValid(String text, int slot) {
        if (TextUtils.isEmpty(text)) {
            return false;
        }
        final int MAX = getAASTextMaxLength(slot);
        try {
            GsmAlphabet.stringToGsm7BitPacked(text);
            if (text.length() > MAX) {
                return false;
            }
        } catch (EncodeException e) {
            if (text.length() > ((MAX - 1) >> 1)) {
                return false;
            }
        }
        return true;
    }

    /**
     * Get the max size of AAS.
     * @param slot The USIM Card slotId
     * @return
     */
    public static int getAASMaxCount(int slot) {
        if (slot < SLOT_ID1 || slot > SLOT_ID2) {
            return ERROR;
        }

        LogUtils.d(TAG, "[getUSIMAASMaxCount]slot:" + slot + "|maxGroupCount:" + MAX_USIM_AAS_COUNT[slot]);
        if (MAX_USIM_AAS_COUNT[slot] < 0) {
            try {
                final IIccPhoneBook iIccPhb = getIIccPhoneBook(slot);
                if (iIccPhb != null) {
                    MAX_USIM_AAS_COUNT[slot] = iIccPhb.getUsimAasMaxCount();
                }
            } catch (android.os.RemoteException e) {
                LogUtils.d(TAG, "catched exception.");
                MAX_USIM_AAS_COUNT[slot] = -1;
            }
        }
        LogUtils.d(TAG, "[getUSIMAASMaxCount]end slot:" + slot + "|maxGroupCount:" + MAX_USIM_AAS_COUNT[slot]);
        return MAX_USIM_AAS_COUNT[slot];
    }

    public static int getAnrCount(int slot) {
        if (slot < SLOT_ID1 || slot > SLOT_ID2) {
            return ERROR;
        }

        LogUtils.d(TAG, "[getUSIMAASMaxCount]slot:" + slot + "|maxGroupCount:" + MAX_USIM_ANR_COUNT[slot]);
        if (MAX_USIM_ANR_COUNT[slot] < 0) {
            try {
                final IIccPhoneBook iIccPhb = getIIccPhoneBook(slot);
                if (iIccPhb != null) {
                    MAX_USIM_ANR_COUNT[slot] = iIccPhb.getAnrCount();
                }
            } catch (android.os.RemoteException e) {
                LogUtils.d(TAG, "catched exception.");
                MAX_USIM_ANR_COUNT[slot] = -1;
            }
        }
        LogUtils.d(TAG, "[getUSIMAASMaxCount]end slot:" + slot + "|maxGroupCount:" + MAX_USIM_ANR_COUNT[slot]);
        return MAX_USIM_ANR_COUNT[slot];
    }

    private static IIccPhoneBook getIIccPhoneBook(int slotId) {
        LogUtils.d(TAG, "[getIIccPhoneBook]slotId:" + slotId);
        String serviceName;
        if (MTK_GEMINI_SUPPORT) {
            serviceName = (slotId == SLOT_ID2) ? SIMPHONEBOOK2_SERVICE : SIMPHONEBOOK_SERVICE;
        } else {
            serviceName = SIMPHONEBOOK_SERVICE;
        }
        final IIccPhoneBook iIccPhb = IIccPhoneBook.Stub.asInterface(ServiceManager.getService(serviceName));
        return iIccPhb;
    }
    // --------------------------------------------
    private static int[] MAX_USIM_SNE_MAX_LENGTH = { -1, -1 };

    /**
     * check whether the Usim support SNE field.
     * @return
     */
    public static boolean hasSne(int slot) {
        LogUtils.d(TAG, "[hasSne]slot:" + slot);
        boolean hasSne = false;
        if (slot < SLOT_ID1 || slot > SLOT_ID2) {
            return hasSne;
        }
        try {
            final IIccPhoneBook iIccPhb = getIIccPhoneBook(slot);
            if (iIccPhb != null) {
                hasSne = iIccPhb.hasSne();
                LogUtils.d(TAG, "hasSne, hasSne=" + hasSne);
            }
        } catch (android.os.RemoteException e) {
            LogUtils.e(TAG, "[hasSne] exception.");
        }
        LogUtils.d(TAG, "[hasSne] hasSne:" + hasSne);
        return hasSne;
    }

    /**
     * get the max length of SNE field.
     * @return
     */
    public static int getSneRecordMaxLen(int slot) {
        LogUtils.d(TAG, "[getSneRecordMaxLen]end slot:" + slot);
        if (slot < SLOT_ID1 || slot > SLOT_ID2) {
            return ERROR;
        }
        if (MAX_USIM_SNE_MAX_LENGTH[slot] < 0) {
            try {
                final IIccPhoneBook iIccPhb = getIIccPhoneBook(slot);
                if (iIccPhb != null) {
                    MAX_USIM_SNE_MAX_LENGTH[slot] = iIccPhb.getSneRecordLen();
                    LogUtils.d(TAG, "getSneRecordMaxLen, len=" + MAX_USIM_SNE_MAX_LENGTH[slot]);
                }
            } catch (android.os.RemoteException e) {
                LogUtils.e(TAG, "catched exception.");
                MAX_USIM_SNE_MAX_LENGTH[slot] = -1;
            }
        }
        LogUtils.d(TAG, "[getSneRecordMaxLen]maxNameLen:" + MAX_USIM_SNE_MAX_LENGTH[slot]);
        return MAX_USIM_SNE_MAX_LENGTH[slot];
    }
    public static boolean isUsim(String accountType) {
        return ACCOUNT_TYPE_USIM.equals(accountType);
    }

    public static boolean isSim(String accountType) {
        return ACCOUNT_TYPE_SIM.equals(accountType);
    }

    public static boolean isPhone(String mimeType) {
        return Phone.CONTENT_ITEM_TYPE.equals(mimeType);
    }

    public static boolean isAasPhoneType(int type) {
        return Anr.TYPE_AAS == type;
    }

    public static String getSuffix(int count) {
        if (count <= 0) {
            return "";
        } else {
            return String.valueOf(count);
        }
    }
}
