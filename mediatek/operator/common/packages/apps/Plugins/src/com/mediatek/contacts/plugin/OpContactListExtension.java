package com.mediatek.contacts.plugin;

import android.content.Context;
import android.content.ContentProviderOperation.Builder;
import android.database.Cursor;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import com.android.internal.telephony.GsmAlphabet;
import com.android.internal.telephony.EncodeException;
import android.content.ContentValues;
import android.text.TextUtils;
import com.mediatek.contacts.ext.ContactListExtension;

import java.util.ArrayList;

public class OpContactListExtension extends ContactListExtension {
    private final static String TAG = "OpMtkListImpl";

    public void checkPhoneTypeArray(String accountType, ArrayList<Integer> phoneTypeArray, String commd) {
        if (ContactsPlugin.COMMD_FOR_AAS.equals(commd)) {
            if (SimUtils.isUsim(accountType) && !phoneTypeArray.isEmpty()) {
                phoneTypeArray.remove(0);
            }
        }
    }

    public boolean generateDataBuilder(Context context, Cursor dataCursor, Builder builder, String[] columnNames,
            String accountType, String mimeType, int slotId, int index, String commd) {
        if (ContactsPlugin.COMMD_FOR_AAS.equals(commd)) {
            if (SimUtils.isUsim(accountType) && SimUtils.isPhone(mimeType)) {
                //String isAnr = dataCursor.getString(dataCursor.getColumnIndex(Data.IS_ADDITIONAL_NUMBER));
                if (Data.DATA2.equals(columnNames[index])) {
                    LogUtils.d(TAG, "generateDataBuilder, DATA2 to be TYPE_MOBILE");
                    builder.withValue(Data.DATA2, Phone.TYPE_MOBILE);
                    return true;
                }
                if (Data.DATA3.equals(columnNames[index])) {
                    LogUtils.d(TAG, "generateDataBuilder, DATA3 to be null");
                    builder.withValue(Data.DATA3, null);
                    return true;
                }
            }
            LogUtils.d(TAG, "[generateDataBuilder] false.");
            return false;
        }
        return false;
    }

public String buildSimNickname(String accountType, ContentValues values,
            ArrayList<String> nicknameArray, int slotId, String defValue, String cmd) {
        if (Utils.isSNECalled(cmd) && SimUtils.isUsim(accountType)) {
            String simNickname = null;
            if (!nicknameArray.isEmpty() && SimUtils.hasSne(slotId)) {
                simNickname = nicknameArray.remove(0);
                simNickname = TextUtils.isEmpty(simNickname) ? "" : simNickname;
                int len = SimUtils.getSneRecordMaxLen(slotId);

                LogUtils.d(TAG, "[buildSimNickname]before Endode simNickname=" + simNickname);
                try {// the code copy from CustomAasActivity.
                    GsmAlphabet.stringToGsm7BitPacked(simNickname);
                    if (simNickname.length() > len) {
                        simNickname = "";// simNickname.substring(0, len);
                    }
                } catch (EncodeException e) {
                    LogUtils.e(TAG, "Error at GsmAlphabet.stringToGsm7BitPacked()!");
                    if (simNickname.length() > ((len - 1) >> 1)) {
                        simNickname = "";// simNickname.substring(0, len);
                    }
                }
                LogUtils.d(TAG, "[buildSimNickname]after Endode simNickname=" + simNickname);
                values.put("sne", simNickname);
            }
            return simNickname;
        }
        return defValue;
    }
}
