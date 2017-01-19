package com.mediatek.op.telephony;

import android.content.Context;
import android.database.Cursor;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.RawContacts;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.text.TextUtils;
import android.util.Log;
import android.provider.Telephony.SIMInfo;
import android.provider.Telephony.SimInfo;
import android.os.RemoteException;
import android.os.ServiceManager;
import com.android.internal.telephony.IIccPhoneBook;
import com.android.internal.telephony.ITelephony;
import com.mediatek.common.telephony.ITelephonyEx;
import com.mediatek.xlog.Xlog;


public class CallerInfoPlugin extends CallerInfoExt {
    private final static String TAG = "CallerInfoPlugin";

    private static final boolean MTK_GEMINI_SUPPORT = com.mediatek.common.featureoption.FeatureOption.MTK_GEMINI_SUPPORT;
    private static final int SLOT_ID1 = com.android.internal.telephony.PhoneConstants.GEMINI_SIM_1;
    private static final int SLOT_ID2 = com.android.internal.telephony.PhoneConstants.GEMINI_SIM_2;

    private static final int SIM_TYPE_SIM = 0;
    private static final int SIM_TYPE_USIM = 1;
    private static final int SIM_TYPE_UIM = 2;
    private static final String SIM_TYPE_USIM_TAG = "USIM";
    private static final String SIM_TYPE_UIM_TAG = "UIM";

    private static final String ACCOUNT_TYPE_SIM = "SIM Account";
    private static final String ACCOUNT_TYPE_USIM = "USIM Account";
    private static final String ACCOUNT_TYPE_UIM = "UIM Account";
    private static final int TYPE_AAS = 101;

    /**
     * For AAS redesign the Phone number's ypeLabel value. The method for instead of
     * Phone.getTypeLabel(context, numberType, numberLabel, cursor);
     * @param context
     * @param numberType
     * @param numberLabel
     * @param cursor
     * @return
     */
    public CharSequence getTypeLabel(Context context, int numberType, CharSequence numberLabel, Cursor cursor) {
        int slotId = SimInfo.SLOT_NONE;
        int indicateColumnIndex = cursor.getColumnIndex(Contacts.INDICATE_PHONE_SIM);
        if (indicateColumnIndex != -1) {
            final int indicate = cursor.getInt(indicateColumnIndex);     
          /* ALPS01105841  start */
           // slotId = SIMInfo.getSlotById(context, indicate);  
           SIMInfo simInfo = SIMInfo.getSIMInfoById(context, indicate);
           if(simInfo != null) {
                slotId = simInfo.mSlot; 
                }
        /* ALPS01105841  end */    
        }

        return getTypeLabel(context, numberType, numberLabel, slotId);
    }
    
    
    /**
     * For AAS redesign the Phone number's ypeLabel value. The method for instead of
     * Phone.getTypeLabel(context, numberType, numberLabel, slotId);
     * @param context
     * @param numberType
     * @param numberLabel
     * @param slotId
     * @return
     */
     public CharSequence getTypeLabel(Context context, int numberType, CharSequence numberLabel, int slotId){ 
		        String accountType = getAccountTypeBySlot(slotId);
		        Log.i(TAG, "[getTypeLabel] accountType=" + accountType + ", slotId=" + slotId + ", numberType=" + numberType);
		        
		        if (ACCOUNT_TYPE_SIM.equals(accountType)) {// Hide phone type if SIM card
		            return "";
		        } else if (ACCOUNT_TYPE_USIM.equals(accountType) && numberType == TYPE_AAS) {
		            if (TextUtils.isEmpty(numberLabel)) {
		                Log.w(TAG, "[getTypeLabel] Type aas but label index is empty.");
		                return "";
		            }
		            try {
		                Integer aasIdx = Integer.valueOf(numberLabel.toString());                         
		                return getAasTypeLabel(context, aasIdx.intValue(), slotId);
		            } catch (Exception e) {
		                Log.d(TAG, "[getTypeLabel] return numberLabel=" + numberLabel);
		                return numberLabel;
		            }
		        }
		        Log.d(TAG, "[getTypeLabel] get default label");
		        return super.getTypeLabel(context, numberType, numberLabel, slotId);         
     }
     

    /**
     * Return AAS label for the USim account.
     */
    private CharSequence getAasTypeLabel(Context context, int aasIdx, int slotId) {
        String aasLabel = "";
        if (slotId < SLOT_ID1 || slotId > SLOT_ID2 || aasIdx < 1) {
            Log.i(TAG, "[getAasTypeLabel] slotId=" + slotId + " aasIdx=" + aasIdx);
            return aasLabel;
        }

        try {
            final IIccPhoneBook iIccPhb = getIIccPhoneBook(slotId);
            if (iIccPhb != null) {
                aasLabel = iIccPhb.getUsimAasById(aasIdx);
            }
        } catch (Exception e) {
            Log.i(TAG, "[getAasTypeLabel] exception.");
        }

        if (aasLabel == null) {
            aasLabel = "";
        }
        Log.d(TAG, "[getAasTypeLabel] aasLabel=" + aasLabel);
        return aasLabel;
    }

    private IIccPhoneBook getIIccPhoneBook(int slotId) {
        String serviceName;
        if (MTK_GEMINI_SUPPORT) {
            serviceName = (slotId == SLOT_ID2) ? "simphonebook2"
                    : "simphonebook";
        } else {
            serviceName = "simphonebook";
        }
        final IIccPhoneBook iIccPhb = IIccPhoneBook.Stub.asInterface(ServiceManager.getService(serviceName));
        return iIccPhb;
    }

    private String getAccountTypeBySlot(int slotId) {
        Log.d(TAG, "[getAccountTypeBySlot] slotId:" + slotId);
        if (slotId < SLOT_ID1 || slotId > SLOT_ID2) {
            Log.e(TAG, "[getAccountTypeBySlot]Error slotid:" + slotId);
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
            Log.e(TAG, "[getAccountTypeBySlot]Error slotId:" + slotId + " no sim inserted!");
            simAccountType = null;
        }
        Log.d(TAG, "[getAccountTypeBySlot] accountType:" + simAccountType);
        return simAccountType;
    }

    private boolean isSimInserted(int slotId) {
        final ITelephonyEx iTel = ITelephonyEx.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICEEX));
        boolean isSimInsert = false;
        try {
            if (iTel != null) {
                if (MTK_GEMINI_SUPPORT) {
                    isSimInsert = iTel.hasIccCard(slotId);;
                } else {
                    isSimInsert = iTel.hasIccCard(SLOT_ID1);;
                }
            }
        } catch (RemoteException e) {
            e.printStackTrace();
            isSimInsert = false;
        }
        return isSimInsert;
    }

    private int getSimTypeBySlot(int slotId) {
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
            Log.e(TAG, "[getSimTypeBySlot] catched exception.");
            e.printStackTrace();
        }
        Log.d(TAG, "[getSimTypeBySlot] simType=" + simType);
        return simType;
    }
}
