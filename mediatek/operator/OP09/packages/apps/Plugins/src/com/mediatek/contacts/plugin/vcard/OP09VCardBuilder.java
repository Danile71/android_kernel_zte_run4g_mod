package com.mediatek.contacts.plugin.vcard;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.database.Cursor;
import android.database.sqlite.SQLiteException;
import android.provider.ContactsContract.Groups;
import android.provider.ContactsContract.CommonDataKinds.GroupMembership;
import android.text.TextUtils;
import android.util.Log;

import com.android.vcard.VCardBuilder;

public class OP09VCardBuilder extends VCardBuilder{
    public static final String LOG_TAG = "OP09VCardBuilder";

    public OP09VCardBuilder(int vcardType, String charset) {
        super(vcardType, charset);
    }

    private static Map<String, String> groups = new HashMap<String, String>();

    public OP09VCardBuilder appendGroups(final List<ContentValues> contentValuesList,
            ContentResolver resolver) {

        if (contentValuesList != null) {
            StringBuffer groupIds = new StringBuffer();
            for (ContentValues contentValues : contentValuesList) {
                final String group = contentValues.getAsString(GroupMembership.GROUP_ROW_ID);
                if (TextUtils.isEmpty(group)) {
                    continue;
                }

                // if this group id in Map groups, just get the title from Map,
                // not need to query databases.
                String groupTitle = groups.get(group);
                if (!TextUtils.isEmpty(groupTitle)) {
                    appendLineWithCharsetAndQPDetection(OP09VCardEntry.PROPERTY_GROUPS, groupTitle);
                    continue;
                }

                if (!TextUtils.isEmpty(groupIds)) {
                    groupIds.append(",");
                }
                groupIds.append(group);
            }
            // if there have group ids not in Map groups, select the titles from
            // databases.
            if (!TextUtils.isEmpty(groupIds)) {
                Cursor cursor = resolver.query(
                        Groups.CONTENT_URI, new String[] {
                        Groups.TITLE
                    },
                        Groups.DELETED + "=0 " + "AND " + Groups._ID + " in ("
                                + groupIds.toString()
                                + ")", null, null);
                if (cursor == null || cursor.getCount() == 0 || !cursor.moveToFirst()) {

                } else {
                    String title;
                    cursor.moveToPosition(-1);
                    while (cursor.moveToNext()) {
                        title = cursor.getString(0);
                        if (!TextUtils.isEmpty(title)) {
                            appendLineWithCharsetAndQPDetection(OP09VCardEntry.PROPERTY_GROUPS,
                                    title);
                        }
                    }
                }
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
        return this;
    }

    /** @} */

}
