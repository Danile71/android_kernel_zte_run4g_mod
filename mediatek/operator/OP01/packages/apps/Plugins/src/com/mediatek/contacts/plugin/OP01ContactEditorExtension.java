package com.mediatek.contacts.plugin;

import android.content.ContentProviderOperation;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.pm.PackageManager.NameNotFoundException;
import android.net.Uri;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.telephony.PhoneNumberUtils;
import android.text.TextUtils;
import android.util.Log;
import android.widget.Toast;

import com.android.i18n.phonenumbers.AsYouTypeFormatter;
import com.android.i18n.phonenumbers.PhoneNumberUtil;

import com.mediatek.contacts.ext.Anr;
import com.mediatek.contacts.ext.ContactEditorExtension;
import com.mediatek.contacts.ext.ContactPluginDefault;
import com.mediatek.op01.plugin.R;

import java.util.ArrayList;

public class OP01ContactEditorExtension extends ContactEditorExtension {
    private static final String TAG = "OP01ContactEditorExtension";
    private static final String ACCOUNT_TYPE_USIM = "USIM Account"; 
    private ArrayList<Anr> mAnrsList = new ArrayList<Anr>();
    private ArrayList<Anr> mOldAnrsList = new ArrayList<Anr>();
    private int mAnrsCount = 1;
    private Context mContext;
    
    @Override
    public String getCommand() {
        return ContactPluginDefault.COMMD_FOR_OP01;
    }
    
    public boolean isSupportMultiAnr(String accountType, int anrCount, String commd) {
        Log.i(TAG, "OP01ContactEditorExtension: isSupportMultiAnr");
        if (accountType.equals(ACCOUNT_TYPE_USIM) && anrCount > 1) {
            return true;
        } else {
            return false;
        }
    }
    
    public void multiAnrUpdateContentValues(
                ContentValues values, String accountType, ArrayList<Anr> anrsList, String addNumber, 
                int anrCount, boolean isInsert, String commd) {
        Log.i(TAG, "OP01ContactEditorExtension: multiAnrUpdateContentValues");
        if (!accountType.equals(ACCOUNT_TYPE_USIM) || anrCount <= 1) {
            return ;
        }
        String anrTag = isInsert ? "anr" : "newAnr";
        int count = 1;
        for (Anr anr : anrsList) {
            if (count < 2) {
                count++;
                continue;
            }
            Log.d(TAG, "[buildAnrUpdateValues] " + anrTag + count + ":" + anr.mAdditionNumber);
            if (!TextUtils.isEmpty(anr.mAdditionNumber)) {
                values.put(anrTag + count, anr.mAdditionNumber);
            } else {
                values.put(anrTag + count, "");
            }
            count++;
        }
    }


    public void multiAnrWriteToDb(
                ContentResolver resolver, String accountType, ArrayList<Anr> anrsList, ArrayList<Anr> oldAnrsList, 
                String addNumber, int anrCount, long rawContactId, String commd) {
        Log.i(TAG, "OP01ContactEditorExtension: multiAnrWriteToDb anrCount: " + anrCount);
        if (!accountType.equals(ACCOUNT_TYPE_USIM) || anrCount <= 1) {
            return ;
        }
        ContentValues additionalvalues = new ContentValues();
        String whereadditional = null;
        Anr anr;

        for (int count = 1; count< anrCount; count++) {
            whereadditional = Data.RAW_CONTACT_ID + " = \'" + rawContactId + "\'" + " AND "
                        + Data.MIMETYPE + "='" + Phone.CONTENT_ITEM_TYPE + "'" + " AND "
                        + Data.IS_ADDITIONAL_NUMBER + " =" + (count + 1);
            Log.d(TAG, "[multiAnrWriteToDb]anrsList:" + anrsList.size());
            Log.d(TAG, "[multiAnrWriteToDb]oldAnrsList:" + oldAnrsList.size());
            if (anrsList.size() > count) {
                anr = anrsList.get(count);
                Log.d(TAG, "[multiAnrWriteToDb]anr" + (count + 1) + ":" + anr.mAdditionNumber);            
                if (!TextUtils.isEmpty(anr.mAdditionNumber)) {
                    if (oldAnrsList.size() > count && !TextUtils.isEmpty(oldAnrsList.get(count).mAdditionNumber)) {
                        additionalvalues.put(Phone.NUMBER, anr.mAdditionNumber);
                        int upRet = resolver.update(Data.CONTENT_URI, additionalvalues,
                                whereadditional, null);;
                        Log.d(TAG, "[multiAnrWriteToDb]update ret:" + upRet);
                    } else {
                        additionalvalues.put(Phone.RAW_CONTACT_ID, rawContactId);
                        additionalvalues.put(Data.MIMETYPE, Phone.CONTENT_ITEM_TYPE);
                        additionalvalues.put(Phone.NUMBER, anr.mAdditionNumber);
                        additionalvalues.put(Data.IS_ADDITIONAL_NUMBER, count + 1);
                        additionalvalues.put(Data.DATA2, Phone.TYPE_OTHER);
                        Uri insertRetUri = resolver.insert(Data.CONTENT_URI, additionalvalues);
                        Log.d(TAG, "[multiAnrWriteToDb]url insert ret:" + insertRetUri);
                    }
                } else {
                    int delRet = resolver.delete(Data.CONTENT_URI, whereadditional, null);
                    Log.d(TAG, "[multiAnrWriteToDb]delete ret: " + delRet);
                }
            } else if (oldAnrsList.size() > count) {
                int delRet = resolver.delete(Data.CONTENT_URI, whereadditional, null);
                Log.d(TAG, "[multiAnrWriteToDb]delete ret: " + delRet);
            } 
        }
    }

    public void multiAnrSetAnrList(String accountType, int anrCount, ArrayList<Anr> anrsList, 
            ArrayList<Anr> oldAnrsList, String commd) {
        Log.i(TAG, "OP01ContactEditorExtension: multiAnrSetAnrList");
        if (anrCount > 1 && accountType.equals(ACCOUNT_TYPE_USIM)) {
            mAnrsList.addAll(anrsList);
            mOldAnrsList.addAll(oldAnrsList);
            mAnrsCount = anrCount;
        }
    }

    
    public void multiAnrBuildInsertOperation(ArrayList<ContentProviderOperation> operationList, int backRef, String commd) {
        Log.i(TAG, "OP01ContactEditorExtension: multiAnrBuildInsertOperation");
        int count = 1;
        if (mAnrsCount > 1) {
            for (Anr anr : mAnrsList) {
                Log.d(TAG, "[buildAnrUpdateValues] additionalNumber" + count + ":" + anr.mAdditionNumber);
                if (count < 2 || count > mAnrsCount) {
                    count++;
                   continue;
                }
                ContentProviderOperation.Builder builder = ContentProviderOperation.newInsert(Data.CONTENT_URI);
                builder.withValueBackReference(Phone.RAW_CONTACT_ID, backRef);
                builder.withValue(Data.MIMETYPE, Phone.CONTENT_ITEM_TYPE);
                builder.withValue(Data.DATA2, Phone.TYPE_OTHER);
                builder.withValue(Phone.NUMBER, anr.mAdditionNumber);
                builder.withValue(Data.IS_ADDITIONAL_NUMBER, count);
                operationList.add(builder.build());
                count++;
            }
        }
        mAnrsCount = 0;
        mAnrsList.clear();
        mOldAnrsList.clear();
    }

    public int multiAnrImportAnrs(ArrayList<ContentProviderOperation> operationList, 
            String[] additionalNumberArray, int rawContactId, int looper, String countryCode, String commd) {
        Log.i(TAG, "OP01ContactEditorExtension: multiAnrImportAnrs");
        ContentProviderOperation.Builder builder;
        for (int i = 1; i < additionalNumberArray.length; i++) {
            if (!TextUtils.isEmpty(additionalNumberArray[i])) {
                Log.i(TAG, "[importUSimPart] additionalNumber before : " + additionalNumberArray[i]);
                AsYouTypeFormatter mFormatter = PhoneNumberUtil.getInstance()
                        .getAsYouTypeFormatter(countryCode);
                char[] cha = additionalNumberArray[i].toCharArray();
                int ii = cha.length;
                for (int num = 0; num < ii; num++) {
                    additionalNumberArray[i] = mFormatter.inputDigit(cha[num]);
                }
                Log.i(TAG, "[importUSimPart] additionalNumber after : " + additionalNumberArray[i]);
                builder = ContentProviderOperation.newInsert(Data.CONTENT_URI);
                builder.withValueBackReference(Phone.RAW_CONTACT_ID, rawContactId);
                builder.withValue(Data.MIMETYPE, Phone.CONTENT_ITEM_TYPE);
                builder.withValue(Data.DATA2, Phone.TYPE_OTHER);
                additionalNumberArray[i] = getReplaceString(additionalNumberArray[i]);
                builder.withValue(Phone.NUMBER, additionalNumberArray[i]);
                builder.withValue(Data.IS_ADDITIONAL_NUMBER, i + 1);
                operationList.add(builder.build());
                looper++;
            }
        }
        return looper;
    }
    
    public String getReplaceString(final String src) {
        Log.i(TAG, "getReplaceString: [getReplaceString()]");
        return src.replace(PhoneNumberUtils.PAUSE, 'p').replace(PhoneNumberUtils.WAIT, 'w');
    }

    public boolean isDuplicatePhoneType(String[] numberBuffer, ArrayList<Anr> anrsList, String commd) {
        boolean result = false;
        Log.i(TAG, "[isDuplicatePhoneType] numberBuffer: " + numberBuffer[1] + " size: " + anrsList.size());
        if (!TextUtils.isEmpty(numberBuffer[1]) || anrsList.size() > 3) {
            result = true;    
        }
        return result;
    }

    public boolean multiAnrIsAdditionalNumber(int isAnr, String commd) {
        Log.i(TAG, "multiAnrIsAdditionalNumber isAnr: " + isAnr);
        return false;
    }
    
    public String multiAnrCopyUpdateValue(ContentValues values, String simAnrNum, int count, String commd) {
        count++;
        Log.i(TAG, "multiAnrCopyUpdateValue count: " + count);
        if (count < 4 && count >= 1) {
            String tag = (count > 1) ? ("anr" + count) : "anr";
            Log.i(TAG, "multiAnrCopyUpdateValue simAnrNum: " + simAnrNum);
            values.put(tag, PhoneNumberUtils.stripSeparators(simAnrNum));
            Anr addNum = new Anr();
            addNum.mAdditionNumber = PhoneNumberUtils.stripSeparators(simAnrNum);
            if (count == 1) {
                mAnrsList.clear();
            } 
            mAnrsList.add(addNum);       
            mAnrsCount = count;
        }
        return mAnrsList.get(0).mAdditionNumber;
    }

    public String multiAnrCheckErrorMessage(String errorType, Context context, String commd) {
        Log.i(TAG, "multiAnrShowErrorMessage :");

        try {
            mContext = context.createPackageContext("com.mediatek.op01.plugin", 
                    Context.CONTEXT_INCLUDE_CODE | Context.CONTEXT_IGNORE_SECURITY);
        } catch(NameNotFoundException e){
            Log.d(TAG, "no com.mediatek.op01.plugin packages");
            return null;
        }
        Log.d(TAG, "multiAnrShowErrorMessage error is" + errorType);
        if (errorType.equals("-14")) {            
            mAnrsList.clear(); 
            mAnrsCount = 0;         
            return mContext.getString(R.string.multi_anrs_save_anr_fail);
        } else {
            return null;
        }
    }

}
