package com.mediatek.contacts.plugin;

//import com.mediatek.op03.plugin.R;
import com.mediatek.contacts.plugin.AASSNEResources;
import com.mediatek.contacts.ext.Anr;
import com.mediatek.contacts.ext.ContactAccountExtension;
import com.android.internal.telephony.GsmAlphabet;
import com.android.internal.telephony.EncodeException;

import android.content.ContentProviderOperation;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.res.Resources;
import android.database.Cursor;
import android.net.Uri;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.RawContacts;
import android.provider.ContactsContract.CommonDataKinds.Nickname;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.telephony.PhoneNumberUtils;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import com.mediatek.contacts.plugin.AASSNEOptions;
import java.util.ArrayList;

public class OpContactAccountExtension extends ContactAccountExtension {
    private final static String TAG = "OpContactAccountExtension";

    private Context mContext = null;

    public OpContactAccountExtension(Context context) {
        mContext = context;
    }

   public OpContactAccountExtension() {
        mContext = null;
    }

    public boolean isFeatureEnabled(String commd) {
	LogUtils.e(TAG, "isFeatureEnabled ASS" + ContactsPlugin.COMMD_FOR_AAS);
        if (ContactsPlugin.COMMD_FOR_AAS.equals(commd)) {
            return true;
        }
       else if(Utils.isSNECalled(commd)) {
           return true;
       }
        return false;
    }

    public void setCurrentSlot(int slotId, String commd) {
        if ((ContactsPlugin.COMMD_FOR_AAS.equals(commd))|| (Utils.isSNECalled(commd))) {
            SimUtils.setCurrentSlot(slotId);
        }
    }

    public int getCurrentSlot(String commd) {
        if ((ContactsPlugin.COMMD_FOR_AAS.equals(commd)) || (Utils.isSNECalled(commd))) {
            return SimUtils.getCurSlotId();
        }
        return -1;
    }

    public boolean isFeatureAccount(String accountType, String commd) {
        if ((isFeatureEnabled(commd)) || (Utils.isSNECalled(commd))) {
            return SimUtils.isUsim(accountType);
        }
        return false;
    }

    public boolean isPhone(String mimeType, String commd) {
        if (ContactsPlugin.COMMD_FOR_AAS.equals(commd)) {
            return SimUtils.isPhone(mimeType);
        }
        return false;
    }

    public boolean hidePhoneLabel(String accountType, String mimeType, String value, String commd) {
        if (ContactsPlugin.COMMD_FOR_AAS.equals(commd)) {
            if (SimUtils.isSim(accountType) && SimUtils.isPhone(mimeType)) {
                LogUtils.d(TAG, "hidePhoneLabel, hide label for sim card.");
                return true;
            }
    
            if (SimUtils.isUsim(SimUtils.getCurAccount()) && SimUtils.isPhone(mimeType)
                    && !SimUtils.IS_ADDITIONAL_NUMBER.equals(value)) {
                // primary number, hide phone label
                LogUtils.d(TAG, "hidePhoneLabel, hide label for primary numbe.");
                return true;
            }
            LogUtils.d(TAG, "hidePhoneLabel: false");
            return false;
        }
       else if (Utils.isSNECalled(commd) && accountType != null && SimUtils.isSim(accountType)) {
            LogUtils.d(TAG, "hidePhoneLabel, hide label for sim card.");
            return true;
        }
        return false;
    }

private boolean buildNicknameValues(String accountType, ContentValues values, String nickName) {
        if (SimUtils.isUsim(accountType)) {
            values.put("sne", TextUtils.isEmpty(nickName) ? "" : nickName);
            return true;
        }
        return false;
    }

    public CharSequence getTypeLabel(Resources res, int type, CharSequence label, int slotId, String commd) {
        if (ContactsPlugin.COMMD_FOR_AAS.equals(commd)) {
            String accountType = SimUtils.getAccountTypeBySlot(slotId);
            LogUtils.d(TAG, "getTypeLabel() slotId=" + slotId + " accountType=" + accountType);
            if (SimUtils.isSim(accountType)) {
                LogUtils.d(TAG, "getTypeLabel() SIM Account no Label.");
                return "";
            }
            if (SimUtils.isUsim(accountType) && SimUtils.isAasPhoneType(type)) {
                LogUtils.d(TAG, "getTypeLabel() USIM Account label=" + label);
                if (TextUtils.isEmpty(label)) {
                    return "";
                }
                try {
                    final Integer aasIdx = Integer.valueOf(label.toString());
                    final String tag = SimUtils.getAASById(slotId, aasIdx.intValue());
                    LogUtils.d(TAG, "getTypeLabel() index" + aasIdx + " tag=" + tag);
                    return tag;
                } catch (Exception e) {
                    LogUtils.e(TAG, "getTypeLabel() return label=" + label);
                }
            }
            return Phone.getTypeLabel(res, type, label);
        }
        return Phone.getTypeLabel(res, type, label);
    }

    public String getCustomTypeLabel(int type, String customColumn, String commd) {
        if (ContactsPlugin.COMMD_FOR_AAS.equals(commd)) {
            LogUtils.d(TAG, "getCustomTypeLabel() type=" + type + " customColumn=" + customColumn);
            if (SimUtils.isUsim(SimUtils.getCurAccount()) && SimUtils.isAasPhoneType(type)) {
                if (!TextUtils.isEmpty(customColumn)) {
                    int aasIdx = Integer.valueOf(customColumn).intValue();
                    if (aasIdx > 0) {
                        final String tag = SimUtils.getAASById(SimUtils.getCurSlotId(), aasIdx);
                        LogUtils.d(TAG, "getCustomTypeLabel() index" + aasIdx + " tag=" + tag);
                        return tag;
                    }
                }
                return mContext.getResources().getString(AASSNEResources.aas_phone_type_none);
            }
            return null;
        }
        return null;
    }

    private boolean buildAnrInsertValues(String accountType, ContentValues values, ArrayList anrsList) {
        if (SimUtils.isUsim(accountType)) {
            int count = 0;
            for (Object obj : anrsList) {
                Anr anr = (Anr) obj;
                String additionalNumber = TextUtils.isEmpty(anr.mAdditionNumber) ? "" : anr.mAdditionNumber;
                values.put("anr" + SimUtils.getSuffix(count), additionalNumber);
                values.put("aas" + SimUtils.getSuffix(count), anr.mAasIndex);
                count++;
                LogUtils.d(TAG, "[buildAnrInsertValues] aasIndex=" + anr.mAasIndex + ", additionalNumber="
                        + additionalNumber);
            }
            return true;
        }
        return false;
    }

    private boolean buildAnrUpdateValues(String accountType, ContentValues updatevalues, ArrayList<Anr> anrsList,
            String updateAdditionalNumber) {
        if (SimUtils.isUsim(accountType)) {
            int count = 0;
            for (Anr anr : anrsList) {
                LogUtils.d(TAG, "[buildAnrUpdateValues] additionalNumber : " + anr.mAdditionNumber);
                if (!TextUtils.isEmpty(anr.mAdditionNumber)) {
                    updatevalues.put("newAnr" + SimUtils.getSuffix(count), anr.mAdditionNumber);
                    updatevalues.put("aas" + SimUtils.getSuffix(count), anr.mAasIndex);
                }
                count++;
            }
            return true;
        }
        return false;
    }
    /** need rahul help as else part are different*/
    public boolean updateContentValues(String accountType, ContentValues values, ArrayList anrsList, String text,
            int type ,String commd) {
        if (ContactsPlugin.COMMD_FOR_AAS.equals(commd)) {
            switch (type) {
            case CONTENTVALUE_ANR_INSERT:
                return buildAnrInsertValues(accountType, values, anrsList);
            case CONTENTVALUE_ANR_UPDATE:
                return buildAnrUpdateValues(accountType, values, anrsList, text);
            case CONTENTVALUE_INSERT_SIM:
                values.remove(Phone.TYPE);
                return true;
            default:
                return super.updateContentValues(accountType, values, anrsList, text, type, commd);
            }
        }
        else if (Utils.isSNECalled(commd) && type == CONTENTVALUE_NICKNAME) {
            return buildNicknameValues(accountType, values, text);
        }
        else {
            return false;
}
        //return super.updateContentValues(accountType, values, anrsList, text, type, commd);
    }

    public boolean updateDataToDb(String accountType, ContentResolver resolver, ArrayList newArr, ArrayList oldArr,
            long rawId, int type, String commd) {
        if (ContactsPlugin.COMMD_FOR_AAS.equals(commd)) {
            switch (type) {
            case DB_UPDATE_ANR:
                return updateAnrToDb(accountType, resolver, newArr, oldArr, rawId);
            default:
                return false;
            }
        }
       else if (Utils.isSNECalled(commd) && type == DB_UPDATE_NICKNAME) {
            if (newArr == null || newArr.size() < 2) {
                LogUtils.e(TAG, "[updateDataToDb]DB_UPDATE_NICKNAME-error newArr");
                return false;
            }
            String updateNickname = String.valueOf(newArr.get(0));
            String oldNickname = String.valueOf(newArr.get(1));
            return updateNicknameToDB(accountType, resolver, updateNickname, oldNickname, rawId);
        }
        return false;
    }

    public boolean isTextValid(String text, int slotId, int feature, String cmd) {
	if (com.mediatek.contacts.plugin.AASSNEOptions.SNE) {
		return Utils.isSNECalled(cmd) ? isSneNicknameValid(text, slotId) : false;
	}
	return true;
    }

    private boolean isSneNicknameValid(String nickName, int slotId) {
        if (TextUtils.isEmpty(nickName)) {
            return true;
        }
        final int maxLength = SimUtils.getSneRecordMaxLen(slotId);
        try {
            GsmAlphabet.stringToGsm7BitPacked(nickName);
            if (nickName.length() > maxLength) {
                return false;
            }
        } catch (EncodeException e) {
            if (nickName.length() > ((maxLength - 1) >> 1)) {
                return false;
            }
        }
        return true;
    }

    private boolean updateAnrToDb(String accountType, ContentResolver resolver, ArrayList anrsList,
            ArrayList oldAnrsList, long rawId) {
        if (SimUtils.isUsim(accountType)) {
            String whereadditional = Data.RAW_CONTACT_ID + " = \'" + rawId + "\'" + " AND " + Data.MIMETYPE + "='"
                    + Phone.CONTENT_ITEM_TYPE + "'" + " AND " + Data.IS_ADDITIONAL_NUMBER + " =1" + " AND " + Data._ID
                    + " =";
            LogUtils.d(TAG, "[updateAnrInfoToDb] whereadditional:" + whereadditional);

            // Here, mAnrsList.size() should be the same as mOldAnrsList.size()
            int newSize = anrsList.size();
            int oldSize = oldAnrsList.size();
            int count = Math.min(newSize, oldSize);
            String additionNumber;
            String aas;
            String oldAdditionNumber;
            String oldAas;
            long dataId;
            String where;
            ContentValues additionalvalues = new ContentValues();

            int i = 0;
            for (; i < count; i++) {
                Anr newAnr = (Anr) anrsList.get(i);
                Anr oldAnr = (Anr) oldAnrsList.get(i);
                where = whereadditional + oldAnr.mId;

                additionalvalues.clear();
                if (!TextUtils.isEmpty(newAnr.mAdditionNumber) && !TextUtils.isEmpty(oldAnr.mAdditionNumber)) {// update
                    additionalvalues.put(Phone.NUMBER, newAnr.mAdditionNumber);
                    additionalvalues.put(Data.DATA2, Anr.TYPE_AAS);
                    additionalvalues.put(Data.DATA3, newAnr.mAasIndex);

                    int upadditional = resolver.update(Data.CONTENT_URI, additionalvalues, where, null);
                    LogUtils.d(TAG, "upadditional is " + upadditional);
                } else if (!TextUtils.isEmpty(newAnr.mAdditionNumber) && TextUtils.isEmpty(oldAnr.mAdditionNumber)) {// insert
                    additionalvalues.put(Phone.RAW_CONTACT_ID, rawId);
                    additionalvalues.put(Data.MIMETYPE, Phone.CONTENT_ITEM_TYPE);
                    additionalvalues.put(Phone.NUMBER, newAnr.mAdditionNumber);
                    additionalvalues.put(Data.DATA2, Anr.TYPE_AAS);
                    additionalvalues.put(Data.DATA3, newAnr.mAasIndex);
                    additionalvalues.put(Data.IS_ADDITIONAL_NUMBER, 1);

                    Uri upAdditionalUri = resolver.insert(Data.CONTENT_URI, additionalvalues);
                    LogUtils.d(TAG, "upAdditionalUri is " + upAdditionalUri);
                } else if (TextUtils.isEmpty(newAnr.mAdditionNumber)) {// delete
                    int deleteAdditional = resolver.delete(Data.CONTENT_URI, where, null);
                    LogUtils.d(TAG, "deleteAdditional is " + deleteAdditional);
                }
            }

            // in order to avoid error, do the following operations.
            while (i < oldSize) {// delete one
                Anr oldAnr = (Anr) oldAnrsList.get(i);
                dataId = oldAnr.mId;
                where = whereadditional + dataId;
                int deleteAdditional = resolver.delete(Data.CONTENT_URI, where, null);
                LogUtils.d(TAG, "deleteAdditional is " + deleteAdditional);
                i++;
            }

            while (i < newSize) {// insert one
                Anr newAnr = (Anr) anrsList.get(i);
                additionalvalues.clear();
                if(!TextUtils.isEmpty(newAnr.mAdditionNumber)) {
                    additionalvalues.put(Phone.RAW_CONTACT_ID, rawId);
                    additionalvalues.put(Data.MIMETYPE, Phone.CONTENT_ITEM_TYPE);
                    additionalvalues.put(Phone.NUMBER, newAnr.mAdditionNumber);
                    additionalvalues.put(Data.DATA2, Anr.TYPE_AAS);
                    additionalvalues.put(Data.DATA3, newAnr.mAasIndex);
                    additionalvalues.put(Data.IS_ADDITIONAL_NUMBER, 1);

                    Uri upAdditionalUri = resolver.insert(Data.CONTENT_URI, additionalvalues);
                    LogUtils.d(TAG, "upAdditionalUri is " + upAdditionalUri);
                }
                i++;
            }
            return true;
        }
        return false;
    }

 private boolean updateNicknameToDB(String accountType, ContentResolver resolver, String updateNickname,
            String oldNickname, long rawId) {
        if (SimUtils.isUsim(accountType)) {
            ContentValues nicknamevalues = new ContentValues();
            String whereNickname = Data.RAW_CONTACT_ID + " = \'" + rawId + "\'" + " AND " + Data.MIMETYPE + "='"
                    + Nickname.CONTENT_ITEM_TYPE + "'";

            updateNickname = TextUtils.isEmpty(updateNickname)?"":updateNickname;
            LogUtils.d(TAG, "[updateNickname]whereNickname is=" + whereNickname + " updateNickname:="
                    + updateNickname);

            if (!TextUtils.isEmpty(updateNickname) && !TextUtils.isEmpty(oldNickname)) {
                nicknamevalues.put(Nickname.NAME, updateNickname);
                int upNickname = resolver.update(Data.CONTENT_URI, nicknamevalues, whereNickname, null);
                LogUtils.d(TAG, "[updateNickname] upNickname is " + upNickname);
            } else if (!TextUtils.isEmpty(updateNickname) && TextUtils.isEmpty(oldNickname)) {
                nicknamevalues.put(Nickname.RAW_CONTACT_ID, rawId);
                nicknamevalues.put(Data.MIMETYPE, Nickname.CONTENT_ITEM_TYPE);
                nicknamevalues.put(Nickname.NAME, updateNickname);
                Uri upNicknameUri = resolver.insert(Data.CONTENT_URI, nicknamevalues);
                LogUtils.d(TAG, "[updateNickname] upNicknameUri is " + upNicknameUri);
            } else if (TextUtils.isEmpty(updateNickname)) {
                // update nickname is null,delete name row
                int deleteNickname = resolver.delete(Data.CONTENT_URI, whereNickname, null);
                LogUtils.d(TAG, "[updateNickname] deleteNickname is " + deleteNickname);
            }
            return true;
        }
        return false;
    }
    public boolean buildOperation(String accountType, ArrayList<ContentProviderOperation> operationList,
            ArrayList anrList, String text, int backRef, int type, String commd) {
        if (ContactsPlugin.COMMD_FOR_AAS.equals(commd)) {
            switch (type) {
            case TYPE_OPERATION_AAS:
                return buildAnrOperation(accountType, operationList, anrList, backRef);
            default:
                return super.buildOperation(accountType, operationList, anrList, text, backRef, type, commd);
            }
        }
       else if (Utils.isSNECalled(commd) && type == TYPE_OPERATION_SNE) {
            return buildSneOperation(accountType, operationList, text, backRef);
        }
        return false;
    }


private boolean buildSneOperation(String accountType, ArrayList<ContentProviderOperation> operationList,
            String nickname, int backRef) {
        if (SimUtils.isUsim(accountType)) {
            // build SNE ContentProviderOperation
            if (!TextUtils.isEmpty(nickname)) {
                ContentProviderOperation.Builder builder = ContentProviderOperation.newInsert(Data.CONTENT_URI);
                LogUtils.d(TAG, "[buildSneInsertOperation] nickname:" + nickname);
                builder.withValueBackReference(Nickname.RAW_CONTACT_ID, backRef);
                builder.withValue(Data.MIMETYPE, Nickname.CONTENT_ITEM_TYPE);
                builder.withValue(Nickname.DATA, nickname);
                operationList.add(builder.build());
                return true;
            }
        }
        return true;
    }

public boolean buildOperationFromCursor(String accountType, ArrayList<ContentProviderOperation> operationList,
            final Cursor cursor, int index, String cmd) {
        if (Utils.isSNECalled(cmd) && SimUtils.isUsim(accountType)) {
            // build SNE ContentProviderOperation from cursor
            int sneColumnIdx = cursor.getColumnIndex("sne");
            LogUtils.d(TAG, "[buildOperationFromCurson] sneColumnIdx:" + sneColumnIdx);
            if (sneColumnIdx != -1) {
                String nickname = cursor.getString(sneColumnIdx);
                LogUtils.d(TAG, "[buildOperationFromCurson] nickname:" + nickname);
                if (!TextUtils.isEmpty(nickname)) {
                    ContentProviderOperation.Builder builder = ContentProviderOperation.newInsert(Data.CONTENT_URI);
                    builder.withValueBackReference(Nickname.RAW_CONTACT_ID, index);
                    builder.withValue(Data.MIMETYPE, Nickname.CONTENT_ITEM_TYPE);
                    builder.withValue(Nickname.DATA, nickname);
                    operationList.add(builder.build());
                    return true;
                }
            }
        }
        return false;
    }

    private boolean buildAnrOperation(String accountType, ArrayList<ContentProviderOperation> operationList,
            ArrayList anrList, int backRef) {
        if (SimUtils.isUsim(accountType)) {
            // build Anr ContentProviderOperation
            for (Object obj : anrList) {
                Anr anr = (Anr) obj;
                if (!TextUtils.isEmpty(anr.mAdditionNumber)) {
                    LogUtils.d(TAG, "additionalNumber=" + anr.mAdditionNumber + " aas=" + anr.mAasIndex);

                    ContentProviderOperation.Builder builder = ContentProviderOperation.newInsert(Data.CONTENT_URI);
                    builder.withValueBackReference(Phone.RAW_CONTACT_ID, backRef);
                    builder.withValue(Data.MIMETYPE, Phone.CONTENT_ITEM_TYPE);
                    builder.withValue(Data.DATA2, Anr.TYPE_AAS);
                    builder.withValue(Phone.NUMBER, anr.mAdditionNumber);
                    builder.withValue(Data.DATA3, anr.mAasIndex);

                    builder.withValue(Data.IS_ADDITIONAL_NUMBER, 1);
                    operationList.add(builder.build());
                }
            }
            return true;
        }
        return false;
    }

    public boolean checkOperationBuilder(String accountType, ContentProviderOperation.Builder builder, Cursor cursor,
            int type, String commd) {
        if (ContactsPlugin.COMMD_FOR_AAS.equals(commd)) {
            switch (type) {
            case TYPE_OPERATION_AAS:
                return checkAasOperationBuilder(accountType, builder, cursor);
            case TYPE_OPERATION_INSERT:
                return checkInsertOperation(accountType, builder);
            default:
                return false;
            }
        }
        return super.checkOperationBuilder(accountType, builder, cursor, type, commd);
    }

    private boolean checkInsertOperation(String accountType, ContentProviderOperation.Builder builder) {
        if (!SimUtils.isUsim(accountType)) {
            builder.withValue(Data.DATA2, 2);
        } else {
            builder.withValue(Data.DATA2, Anr.TYPE_AAS);
        }
        return true;
    }

    private boolean checkAasOperationBuilder(String accountType, ContentProviderOperation.Builder builder, Cursor cursor) {
        if (SimUtils.isUsim(accountType)) {
            int aasColumn = cursor.getColumnIndex("aas");
            if (aasColumn >= 0) {
                String aas = cursor.getString(aasColumn);
                builder.withValue(Data.DATA2, Anr.TYPE_AAS);
                builder.withValue(Data.DATA3, aas);
            }
            return true;
        }
        return false;
    }

    public boolean buildValuesForSim(String accountType, Context context, ContentValues values,
            ArrayList<String> additionalNumberArray, ArrayList<Integer> phoneTypeArray, int maxAnrCount, int dstSlotId,
            ArrayList anrsList, String commd) {
        if (ContactsPlugin.COMMD_FOR_AAS.equals(commd) && SimUtils.isUsim(accountType)) {
            int anrNum = additionalNumberArray.size();
            int loop = anrNum < maxAnrCount ? anrNum : maxAnrCount;

            for (int j = 0; j < loop; j++) {
                String simAnrNum = additionalNumberArray.remove(0);
                int type = phoneTypeArray.remove(0);
                String phoneTypeName = context.getString(Phone.getTypeLabelResource(type));
                int aasIndex = SimUtils.getAasIndexByName(phoneTypeName, dstSlotId);

                simAnrNum = TextUtils.isEmpty(simAnrNum) ? "" : simAnrNum.replace("-", "");
                values.put("anr" + SimUtils.getSuffix(j), PhoneNumberUtils.stripSeparators(simAnrNum));
                values.put("aas" + SimUtils.getSuffix(j), aasIndex);

                Anr anr = new Anr();
                anr.mAdditionNumber = simAnrNum;
                anr.mAasIndex = String.valueOf(aasIndex);
                anrsList.add(anr);
                LogUtils.d(TAG, "[buildAnrValuesForSim] anr=" + anr + ", index=" + aasIndex + ", phoneTypeName="
                        + phoneTypeName);
            }
            return true;
        }
        return false;
    }

    public String[] getProjection(int type, String[] defaultProjection, String commd) {
        if (ContactsPlugin.COMMD_FOR_AAS.equals(commd)) {
            switch(type){
            case PROJECTION_COPY_TO_SIM:
                final String[] newProjection = new String[] { 
                        Contacts._ID, 
                        Contacts.Data.MIMETYPE, 
                        Contacts.Data.DATA1,
                        Contacts.Data.IS_ADDITIONAL_NUMBER, 
                        Contacts.Data.DATA2 }; // DATA2 is phone type
                return newProjection;
    
            case PROJECTION_LOAD_DATA:
                final String[] phoneColumns = {
                        Phone.DISPLAY_NAME,
                        Phone.PHOTO_ID,
                        Phone.NUMBER,
                        Phone.TYPE,
                        Phone.LABEL,
                        RawContacts.ACCOUNT_TYPE,
                        RawContacts.INDICATE_PHONE_SIM };
                    return phoneColumns;
    
            case PROJECTION_ADDRESS_BOOK:
                final String[] addressBookColumnName = new String[] { "name", "number", "emails", "additionalNumber",
                        "groupIds", "aas" };
                return addressBookColumnName;
    
            default:
                return defaultProjection;
            }
        }
        return defaultProjection;
    }
}
