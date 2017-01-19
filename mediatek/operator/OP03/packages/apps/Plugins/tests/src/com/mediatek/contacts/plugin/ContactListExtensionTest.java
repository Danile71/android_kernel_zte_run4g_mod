package com.mediatek.contacts.plugin;

import java.util.ArrayList;

import com.mediatek.contacts.ext.ContactListExtension;

import android.content.ContentProviderOperation;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.test.InstrumentationTestCase;

public class ContactListExtensionTest extends InstrumentationTestCase {

    private static final String ACCOUNT_TYPE_SIM = "SIM Account";
    private static final String ACCOUNT_TYPE_USIM = "USIM Account";

    private ContactListExtension mContactListExt = null;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContactListExt = new ContactsPlugin(getInstrumentation().getTargetContext()).createContactListExtension();
    }

    @Override
    protected void tearDown() throws Exception {
        if (mContactListExt != null) {
            mContactListExt = null;
        }
        super.tearDown();
    }

    public void testCheckPhoneTypeArray() {
        ArrayList<Integer> phoneTypeArray = new ArrayList<Integer>();
        mContactListExt.checkPhoneTypeArray(ACCOUNT_TYPE_SIM, phoneTypeArray, ContactsPlugin.COMMD_FOR_AAS);
        mContactListExt.checkPhoneTypeArray(ACCOUNT_TYPE_USIM, phoneTypeArray, ContactsPlugin.COMMD_FOR_AAS);
        assertTrue(phoneTypeArray.isEmpty());

        phoneTypeArray.add(new Integer(0));
        phoneTypeArray.add(new Integer(1));
        final int size = phoneTypeArray.size();
        mContactListExt.checkPhoneTypeArray(ACCOUNT_TYPE_USIM, phoneTypeArray, ContactsPlugin.COMMD_FOR_AAS);
        assertTrue(size != phoneTypeArray.size());
    }

    public void testGenerateDataBuilder() {
        ContentProviderOperation.Builder builder = ContentProviderOperation.newInsert(Data.CONTENT_URI);
        String[] columnNames = new String[] { Data.DATA2, Data.DATA3 };
        boolean result = mContactListExt.generateDataBuilder(null, null, builder, columnNames, ACCOUNT_TYPE_SIM,
                Phone.CONTENT_ITEM_TYPE, -1, 0, ContactsPlugin.COMMD_FOR_AAS);
        assertFalse(result);

        result = mContactListExt.generateDataBuilder(null, null, builder, columnNames, ACCOUNT_TYPE_USIM,
                Phone.CONTENT_ITEM_TYPE, -1, 0, ContactsPlugin.COMMD_FOR_AAS);
        assertTrue(result);

        result = mContactListExt.generateDataBuilder(null, null, builder, columnNames, ACCOUNT_TYPE_USIM,
                Phone.CONTENT_ITEM_TYPE, -1, 1, ContactsPlugin.COMMD_FOR_AAS);
        assertTrue(result);
    }
}
