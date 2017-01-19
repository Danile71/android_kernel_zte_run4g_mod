package com.mediatek.contacts.plugin.simcontacts;

import java.util.ArrayList;

import android.content.ContentProviderOperation;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.OperationApplicationException;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.RemoteException;
import android.provider.ContactsContract;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.PhoneLookup;
import android.provider.ContactsContract.RawContacts;
import android.provider.ContactsContract.CommonDataKinds.Note;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.CommonDataKinds.StructuredName;
import android.util.Log;

import com.mediatek.contacts.ext.ContactPluginDefault;
import com.mediatek.contacts.ext.SimServiceExtension;
import com.android.vcard.VCardConfig;
import com.android.vcard.VCardUtils;

public class OP09SimServiceExtension extends SimServiceExtension {
    private static final String TAG = "OP09SimServiceExtension";
    
    private static final String INTERNATIONAL_FREE_NUMBER = "+8618918910000";
    
    public static final String EXT_ACCOUNTTYPE_ACCOUNT_NAME_LOCAL_PHONE = "Phone";
    public static final String EXT_ACCOUNTTYPE_ACCOUNT_TYPE_LOCAL_PHONE = "Local Phone Account";
    
    private static boolean sIsRunningNumberCheck = false;
    
    private Context mPluginContext;
    
    public OP09SimServiceExtension(Context context) {
        mPluginContext = context;
    }
    
    @Override
    public String getCommond() {
        return ContactPluginDefault.COMMD_FOR_OP09;
    }

    @Override
    public boolean importViaReadonlyContact(Bundle bundle, final ContentResolver cr, String commond) {
        if (!ContactPluginDefault.COMMD_FOR_OP09.equals(commond)) {
            return false;
        }
        
        Log.i(TAG, Contacts.CONTENT_ITEM_TYPE);
        new Thread(new Runnable() {

            @Override
            public void run() {
                Log.i(TAG, "isRunningNumberCheck before: " + sIsRunningNumberCheck);
                if (sIsRunningNumberCheck) {
                    return;
                }
                sIsRunningNumberCheck = true;
                Log.i(TAG, "isRunningNumberCheck after: " + sIsRunningNumberCheck);
                Uri uri = Uri.withAppendedPath(PhoneLookup.CONTENT_FILTER_URI, Uri
                        .encode(INTERNATIONAL_FREE_NUMBER));
                Log.i(TAG, "getContactInfoByPhoneNumbers(), uri = " + uri);

                Cursor contactCursor = cr.query(uri, new String[] {
                        PhoneLookup.DISPLAY_NAME, PhoneLookup.PHOTO_ID
                }, null, null, null);
                try {
                    if (contactCursor != null && contactCursor.getCount() > 0) {
                        return;
                    } else {
                        String name = VCardUtils.parseQuotedPrintable(
                                "=E5=9B=BD=E9=99=85=E6=BC=AB=E6=B8=B8=E5=85=8D=E8" +
                                        "=B4=B9=E6=9C=8D=E5=8A=" + "\n" + "=A1=E7=83=AD=E7=BA=BF",
                                false,
                                VCardConfig.DEFAULT_INTERMEDIATE_CHARSET,
                                "UTF-8");
                        String note = VCardUtils
                                .parseQuotedPrintable(
                                        "=E4=B8=AD=E5=9B=BD=E7=94=B5=E4=BF=A1=E5=9B=BD=E9=99=85=E6=BC"
                                                +
                                                "=AB=E6=B8="
                                                + "\r\n"
                                                + "=B8=E5=85=8D=E8=B4=B9=E6=9C=8D=E5=8A=A1=E7=83=AD=E7=BA=BF",
                                        false,
                                        VCardConfig.DEFAULT_INTERMEDIATE_CHARSET,
                                        "UTF-8");

                        final ArrayList<ContentProviderOperation> operationList = new ArrayList<ContentProviderOperation>();
                        ContentProviderOperation.Builder builder = ContentProviderOperation
                                .newInsert(RawContacts.CONTENT_URI);
                        ContentValues contactvalues = new ContentValues();
                        contactvalues.put(RawContacts.ACCOUNT_NAME,
                                EXT_ACCOUNTTYPE_ACCOUNT_NAME_LOCAL_PHONE);
                        contactvalues.put(RawContacts.ACCOUNT_TYPE,
                                EXT_ACCOUNTTYPE_ACCOUNT_TYPE_LOCAL_PHONE);
                        contactvalues.put(RawContacts.INDICATE_PHONE_SIM,
                                ContactsContract.RawContacts.INDICATE_PHONE);
                        contactvalues.put(RawContacts.IS_SDN_CONTACT, -1);
                        builder.withValues(contactvalues);
                        builder.withValue(RawContacts.AGGREGATION_MODE,
                                RawContacts.AGGREGATION_MODE_DISABLED);
                        operationList.add(builder.build());

                        builder = ContentProviderOperation.newInsert(Data.CONTENT_URI);
                        builder.withValueBackReference(Phone.RAW_CONTACT_ID, 0);
                        builder.withValue(Data.MIMETYPE, Phone.CONTENT_ITEM_TYPE);
                        builder.withValue(Phone.TYPE, Phone.TYPE_MOBILE);
                        builder.withValue(Phone.NUMBER, INTERNATIONAL_FREE_NUMBER);
                        builder.withValue(Data.IS_PRIMARY, 1);
                        operationList.add(builder.build());

                        builder = ContentProviderOperation.newInsert(Data.CONTENT_URI);
                        builder.withValueBackReference(StructuredName.RAW_CONTACT_ID, 0);
                        builder.withValue(Data.MIMETYPE, StructuredName.CONTENT_ITEM_TYPE);
                        builder.withValue(StructuredName.DISPLAY_NAME, name);
                        operationList.add(builder.build());

                        builder = ContentProviderOperation.newInsert(Data.CONTENT_URI);
                        builder.withValueBackReference(StructuredName.RAW_CONTACT_ID, 0);
                        builder.withValue(Data.MIMETYPE, Note.CONTENT_ITEM_TYPE);
                        builder.withValue(Note.NOTE, note);
                        operationList.add(builder.build());
                        try {
                            cr.applyBatch(
                                    ContactsContract.AUTHORITY, operationList);
                        } catch (RemoteException e) {
                            Log.e(TAG, String.format("%s: %s", e.toString(), e.getMessage()));
                        } catch (OperationApplicationException e) {
                            Log.e(TAG, String.format("%s: %s", e.toString(), e.getMessage()));
                        }

                    }
                } finally {
                    // when this service start,but the contactsprovider has not
                    // been started yet.
                    // the contactCursor perhaps null, but not always.(first
                    // load will weekup the provider)
                    // so add null block to avoid nullpointerexception
                    if (contactCursor != null) {
                        contactCursor.close();
                    }
                }
                Log.i(TAG, "isRunningNumberCheck insert: " + sIsRunningNumberCheck);
                sIsRunningNumberCheck = false;
            }
        }).start();
        return true;
    }

}
