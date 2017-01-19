package com.mediatek.contacts.plugin.vcard;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import android.accounts.Account;
import android.content.ContentProviderOperation;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.database.Cursor;
import android.database.sqlite.SQLiteException;
import android.net.Uri;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.Groups;
import android.provider.ContactsContract.CommonDataKinds.GroupMembership;
import android.text.TextUtils;
import android.util.Log;

import com.android.vcard.VCardEntry;
import com.android.vcard.VCardProperty;

public class OP09VCardEntry extends VCardEntry {
    public static final String LOG_TAG = "OP09VCardEntry";
    
    public static final String PROPERTY_GROUPS = "GROUPS";
    
    private final Account mAccount;
    
    private List<GroupData> mGroupList;
    private ContentResolver mResolver;
    private Map<String, String> groupIds = new HashMap<String, String>();
    
    public OP09VCardEntry(int vcardType, Account account) {
        super(vcardType, account);
//        mVCardType = vcardType;
        mAccount = account;
    }
    
    @Override
    public void addMoreIterateOneList(EntryElementIterator iterator) {
        // TODO Auto-generated method stub
        iterateOneList(mGroupList, iterator);
    }

    private void iterateOneList(List<? extends EntryElement> elemList,
            EntryElementIterator iterator) {
        if (elemList != null && elemList.size() > 0) {
            iterator.onElementGroupStarted(elemList.get(0).getEntryLabel());
            for (EntryElement elem : elemList) {
                iterator.onElement(elem);
            }
            iterator.onElementGroupEnded();
        }
    }

    @Override
    public void addProperty(VCardProperty property) {
        super.addProperty(property);
        final String propertyName = property.getName();
        final List<String> propertyValueList = property.getValueList();
        byte[] propertyBytes = property.getByteValue();

        if ((propertyValueList == null || propertyValueList.size() == 0)
                && propertyBytes == null) {
            return;
        }
        
        if (propertyName.equals(PROPERTY_GROUPS)) {
            for (String value : propertyValueList) {
                if (!TextUtils.isEmpty(value)) {
                    //Log.d(LOG_TAG, " propertyValueList.value = " + value);
                    addGroup(value);
                }
            }
        }
    }
        
    @Override
    public ArrayList<ContentProviderOperation> constructInsertOperations(ContentResolver resolver,
            ArrayList<ContentProviderOperation> operationList) {
        mResolver = resolver;
        return super.constructInsertOperations(resolver, operationList);
    }

    private void addGroup(String title) {
        if (mGroupList == null) {
            mGroupList = new ArrayList<GroupData>();
        }
        mGroupList.add(new GroupData(title));
    }

    class GroupData implements EntryElement {
        private final String mGrouptitle;
        private String mGroupId;

        public GroupData(String grouptitle) {
            mGrouptitle = grouptitle;

        }

        @Override
        public void constructInsertOperation(List<ContentProviderOperation> operationList,
                    int backReferenceIndex) {
            final ContentProviderOperation.Builder builder = ContentProviderOperation
                        .newInsert(Data.CONTENT_URI);
            builder.withValueBackReference(GroupMembership.RAW_CONTACT_ID, backReferenceIndex);
            builder.withValue(Data.MIMETYPE, GroupMembership.CONTENT_ITEM_TYPE);
            setGroupId();
            if (!TextUtils.isEmpty(mGroupId)) {
                builder.withValue(GroupMembership.GROUP_ROW_ID, mGroupId);
                operationList.add(builder.build());
            }
        }

        @Override
        public boolean isEmpty() {
            return TextUtils.isEmpty(mGrouptitle);
        }

        @Override
        public boolean equals(Object obj) {
            if (!(obj instanceof GroupData)) {
                return false;
            }
            GroupData groupData = (GroupData) obj;
            return TextUtils.equals(mGrouptitle, groupData.mGrouptitle);
        }

        @Override
        public int hashCode() {
            return mGrouptitle != null ? mGrouptitle.hashCode() : 0;
        }

        @Override
        public String toString() {
            return "mGrouptitle: " + mGrouptitle;
        }

        @Override
        public EntryLabel getEntryLabel() {
            return EntryLabel.EXT_GROUP;
        }

        public String getGrouptitle() {
            return mGrouptitle;
        }

        public void setGroupId() {
            if (!TextUtils.isEmpty(mGrouptitle)) {
                mGroupId = groupIds.get(mGrouptitle);
                if (!TextUtils.isEmpty(mGroupId)) {
                    Log.d(LOG_TAG, "groupIds has mGroupId = " + mGroupId);
                    return;
                }
                String accountType = mAccount.type;
                String accountName = mAccount.name;
                if (TextUtils.isEmpty(accountType)) {
                    accountType = "Local Phone Account";
                }
                if (TextUtils.isEmpty(accountName)) {
                    accountName = "Phone";
                }
                Cursor cursor = mResolver.query(
                            Groups.CONTENT_URI, new String[] {
                                Groups._ID
                            },
                            Groups.DELETED + "=0 AND " + Groups.ACCOUNT_TYPE + " = '" + accountType
                                    + "' AND "
                                    + Groups.TITLE + " = ?", new String[] {
                                mGrouptitle
                            }, null);
                if (cursor == null || cursor.getCount() == 0 || !cursor.moveToFirst()) {
                    ContentValues values = new ContentValues();
                    values.put(Groups.ACCOUNT_TYPE, accountType);
                    values.put(Groups.ACCOUNT_NAME, accountName);
                    values.put(Groups.TITLE, mGrouptitle);
                    final Uri groupUri = mResolver.insert(Groups.CONTENT_URI, values);
                    Long id = ContentUris.parseId(groupUri);
                    mGroupId = String.valueOf(id);

                    // Log.d(LOG_TAG, "insert mGroupId = " + mGroupId);
                } else {
                    mGroupId = cursor.getString(0);
                    // Log.d(LOG_TAG, "no insert mGroupId = " + mGroupId);
                }

                // Log.d(LOG_TAG, "out insert mGroupId = " + mGroupId);
                groupIds.put(mGrouptitle, mGroupId);
                if (cursor != null) {
                    try {
                        cursor.close();
                    } catch (SQLiteException e) {
                        Log.e(LOG_TAG, "SQLiteException on Cursor#close(): " + e.getMessage());
                    }
                    cursor = null;
                }
            }
        }
    }
    
    
}
