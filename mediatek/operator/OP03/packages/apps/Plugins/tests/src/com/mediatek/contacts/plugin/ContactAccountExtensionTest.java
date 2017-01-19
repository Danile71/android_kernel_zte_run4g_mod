package com.mediatek.contacts.plugin;

import java.util.ArrayList;

import com.mediatek.contacts.ext.Anr;
import com.mediatek.contacts.ext.ContactAccountExtension;

import android.app.Instrumentation;
import android.content.ContentProviderOperation;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.res.Resources;
import android.database.Cursor;
import android.provider.ContactsContract.RawContacts;
import android.provider.ContactsContract.CommonDataKinds.Email;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.test.InstrumentationTestCase;

public class ContactAccountExtensionTest extends InstrumentationTestCase {
    private static final String TAG = "ContactAccountExtensionTest";

    private static final String ACCOUNT_TYPE_SIM = "SIM Account";
    private static final String ACCOUNT_TYPE_USIM = "USIM Account";

    private ContactAccountExtension mContactAccountExt = null;
    private Instrumentation mInst = null;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mInst = getInstrumentation();
        mContactAccountExt = new ContactsPlugin(mInst.getTargetContext()).createContactAccountExtension();
    }

    @Override
    protected void tearDown() throws Exception {
        if (mInst != null) {
            mInst = null;
        }
        if (mContactAccountExt != null) {
            mContactAccountExt = null;
        }
        super.tearDown();
    }

    /**
     * test:isFeatureEnabled(); setCurrentSlot(); getCurrentSlot(); isFeatureAccount(); isPhone();
     */
    public void testNormalMethods() {
        // test isFeatureEnabled
        assertFalse(mContactAccountExt.isFeatureEnabled("xxxx"));
        assertTrue(mContactAccountExt.isFeatureEnabled(ContactsPlugin.COMMD_FOR_AAS));

        // test setCurrentSlot & getCurrentSlot
        mContactAccountExt.setCurrentSlot(0, ContactsPlugin.COMMD_FOR_AAS);
        int curSlot = mContactAccountExt.getCurrentSlot(ContactsPlugin.COMMD_FOR_AAS);
        assertEquals(curSlot, 0);

        // test isFeatureAccount
        boolean result = mContactAccountExt.isFeatureAccount(ACCOUNT_TYPE_USIM, ContactsPlugin.COMMD_FOR_AAS);
        assertTrue(result);
        result = mContactAccountExt.isFeatureAccount(ACCOUNT_TYPE_SIM, ContactsPlugin.COMMD_FOR_AAS);
        assertFalse(result);

        // test isPhone
        result = mContactAccountExt.isPhone(Phone.CONTENT_ITEM_TYPE, ContactsPlugin.COMMD_FOR_AAS);
        assertTrue(result);
        result = mContactAccountExt.isPhone(Email.CONTENT_ITEM_TYPE, ContactsPlugin.COMMD_FOR_AAS);
        assertFalse(result);

        // test hidePhoneLabel
        result = mContactAccountExt.hidePhoneLabel(ACCOUNT_TYPE_SIM, Phone.CONTENT_ITEM_TYPE, null,
                ContactsPlugin.COMMD_FOR_AAS);
        assertTrue(result);
        result = mContactAccountExt.hidePhoneLabel(ACCOUNT_TYPE_SIM, Email.CONTENT_ITEM_TYPE, null,
                ContactsPlugin.COMMD_FOR_AAS);
        assertFalse(result);

        if (SimUtils.isUsim(SimUtils.getCurAccount())) {
            result = mContactAccountExt.hidePhoneLabel(ACCOUNT_TYPE_USIM, Phone.CONTENT_ITEM_TYPE, "0",
                    ContactsPlugin.COMMD_FOR_AAS);
            assertTrue(result);
            result = mContactAccountExt.hidePhoneLabel(ACCOUNT_TYPE_USIM, Email.CONTENT_ITEM_TYPE, "1",
                    ContactsPlugin.COMMD_FOR_AAS);
            assertFalse(result);
        } else {
            LogUtils.w(TAG, "Error, slot-0 not isert USIM Card");
        }
    }

    public void testGetTypeLabel() {
        final Resources res = getInstrumentation().getTargetContext().getResources();
        SimUtils.setCurrentSlot(0);
        CharSequence result = mContactAccountExt.getTypeLabel(res, Anr.TYPE_AAS, "1", 0, ContactsPlugin.COMMD_FOR_AAS);
        assertNotNull(result);
        result = mContactAccountExt.getTypeLabel(res, Anr.TYPE_AAS, "1", 0, ContactsPlugin.COMMD_FOR_AAS);
        assertNotNull(result);
        result = mContactAccountExt.getTypeLabel(res, Anr.TYPE_AAS, null, 0, ContactsPlugin.COMMD_FOR_AAS);
        assertNotNull(result);
        result = mContactAccountExt.getTypeLabel(res, Phone.TYPE_MOBILE, null, 0, ContactsPlugin.COMMD_FOR_AAS);
        assertNotNull(result);
        LogUtils.w(TAG, "testGetTypeLabel");
    }

    public void testGetCustomTypeLabel() {
        SimUtils.setCurrentSlot(-1);
        String result = mContactAccountExt.getCustomTypeLabel(Anr.TYPE_AAS, null, ContactsPlugin.COMMD_FOR_AAS);
        assertNull(result);
        SimUtils.setCurrentSlot(0);
        if (SimUtils.isUsim(SimUtils.getCurAccount())) {
            result = mContactAccountExt.getCustomTypeLabel(Anr.TYPE_AAS, null, ContactsPlugin.COMMD_FOR_AAS);
            assertNotNull(result);
            result = mContactAccountExt.getCustomTypeLabel(Anr.TYPE_AAS, "1", ContactsPlugin.COMMD_FOR_AAS);
            assertNotNull(result);
        } else {
            LogUtils.d(TAG, "Error, slot-0 not isert USIM Card");
        }
    }

    public void testUpdateContentValues() {
        mContactAccountExt.updateContentValues(null, null, null, null, -1, ContactsPlugin.COMMD_FOR_AAS);

        ArrayList<Anr> anrsList = new ArrayList<Anr>();
        Anr a1 = new Anr();
        a1.mAasIndex = "1";
        a1.mAdditionNumber = null;
        Anr a2 = new Anr();
        a2.mAasIndex = "1";
        a2.mAdditionNumber = "1233";

        anrsList.add(a1);
        anrsList.add(a2);

        ContentValues cv = new ContentValues();
        // CONTENTVALUE_ANR_INSERT
        mContactAccountExt.updateContentValues(ACCOUNT_TYPE_SIM, cv, anrsList, null,
                ContactAccountExtension.CONTENTVALUE_ANR_INSERT, ContactsPlugin.COMMD_FOR_AAS);
        mContactAccountExt.updateContentValues(ACCOUNT_TYPE_USIM, cv, anrsList, null,
                ContactAccountExtension.CONTENTVALUE_ANR_INSERT, ContactsPlugin.COMMD_FOR_AAS);

        // CONTENTVALUE_ANR_UPDATE
        mContactAccountExt.updateContentValues(ACCOUNT_TYPE_SIM, cv, anrsList, null,
                ContactAccountExtension.CONTENTVALUE_ANR_UPDATE, ContactsPlugin.COMMD_FOR_AAS);
        mContactAccountExt.updateContentValues(ACCOUNT_TYPE_USIM, cv, anrsList, null,
                ContactAccountExtension.CONTENTVALUE_ANR_UPDATE, ContactsPlugin.COMMD_FOR_AAS);

        // CONTENTVALUE_INSERT_SIM
        mContactAccountExt.updateContentValues(ACCOUNT_TYPE_USIM, cv, anrsList, null,
                ContactAccountExtension.CONTENTVALUE_INSERT_SIM, ContactsPlugin.COMMD_FOR_AAS);
    }

    public void testUpdateDataToDb() {
        mContactAccountExt.updateDataToDb(ACCOUNT_TYPE_SIM, null, null, null, 0, -1, ContactsPlugin.COMMD_FOR_AAS);

        // DB_UPDATE_ANR

        ContentResolver resolver = getInstrumentation().getTargetContext().getContentResolver();
        ArrayList<Anr> newAnrs = new ArrayList<Anr>();
        Anr new1 = new Anr();
        new1.mAasIndex = "1";
        new1.mAdditionNumber = null;
        Anr new2 = new Anr();
        new2.mAasIndex = "1";
        new2.mAdditionNumber = "1233";
        newAnrs.add(new1);
        newAnrs.add(new2);

        ArrayList<Anr> oldAnrs = new ArrayList<Anr>();
        Anr old1 = new Anr();
        old1.mAasIndex = "1";
        old1.mAdditionNumber = null;
        Anr old2 = new Anr();
        old2.mAasIndex = "1";
        old2.mAdditionNumber = "1233";
        Anr old3 = new Anr();
        old3.mAasIndex = "1";
        old3.mAdditionNumber = "1233";
        oldAnrs.add(old1);
        oldAnrs.add(old2);
        oldAnrs.add(old3);

        mContactAccountExt.updateDataToDb(ACCOUNT_TYPE_SIM, resolver, newAnrs, oldAnrs, 0,
                ContactAccountExtension.DB_UPDATE_ANR, ContactsPlugin.COMMD_FOR_AAS);

        mContactAccountExt.updateDataToDb(ACCOUNT_TYPE_USIM, resolver, newAnrs, oldAnrs, 0,
                ContactAccountExtension.DB_UPDATE_ANR, ContactsPlugin.COMMD_FOR_AAS);

        mContactAccountExt.updateDataToDb(ACCOUNT_TYPE_USIM, resolver, oldAnrs, newAnrs, 0,
                ContactAccountExtension.DB_UPDATE_ANR, ContactsPlugin.COMMD_FOR_AAS);
    }

    public void testBuildOperation() {
        mContactAccountExt.buildOperation(null, null, null, null, 0, -1, ContactsPlugin.COMMD_FOR_AAS);
        // TYPE_OPERATION_AAS
        ArrayList<ContentProviderOperation> operationList = new ArrayList<ContentProviderOperation>();
        ArrayList<Anr> anrsList = new ArrayList<Anr>();
        Anr a1 = new Anr();
        a1.mAasIndex = "1";
        a1.mAdditionNumber = null;
        Anr a2 = new Anr();
        a2.mAasIndex = "1";
        a2.mAdditionNumber = "1233";
        mContactAccountExt.buildOperation(ACCOUNT_TYPE_SIM, operationList, anrsList, null, 0,
                ContactAccountExtension.TYPE_OPERATION_AAS, ContactsPlugin.COMMD_FOR_AAS);

        mContactAccountExt.buildOperation(ACCOUNT_TYPE_USIM, operationList, anrsList, null, 0,
                ContactAccountExtension.TYPE_OPERATION_AAS, ContactsPlugin.COMMD_FOR_AAS);
    }

    public void testCheckOperationBuilder() {
        mContactAccountExt.checkOperationBuilder(null, null, null, -1, ContactsPlugin.COMMD_FOR_AAS);
        // TYPE_OPERATION_AAS
        ContentProviderOperation.Builder builder = ContentProviderOperation.newInsert(RawContacts.CONTENT_URI);
        mContactAccountExt.checkOperationBuilder(ACCOUNT_TYPE_SIM, builder, null,
                ContactAccountExtension.TYPE_OPERATION_AAS, ContactsPlugin.COMMD_FOR_AAS);
        // TYPE_OPERATION_INSERT
        mContactAccountExt.checkOperationBuilder(ACCOUNT_TYPE_SIM, builder, null,
                ContactAccountExtension.TYPE_OPERATION_INSERT, ContactsPlugin.COMMD_FOR_AAS);
        mContactAccountExt.checkOperationBuilder(ACCOUNT_TYPE_USIM, builder, null,
                ContactAccountExtension.TYPE_OPERATION_INSERT, ContactsPlugin.COMMD_FOR_AAS);
    }

    public void testBuildValuesForSim() {
        ContentValues values = new ContentValues();
        mContactAccountExt.buildValuesForSim(ACCOUNT_TYPE_SIM, getInstrumentation().getContext(), values, null, null,
                0, 0, null, ContactsPlugin.COMMD_FOR_AAS);

        ArrayList<String> additionalNumberArray = new ArrayList<String>();
        additionalNumberArray.add("123");
        ArrayList<Integer> phoneTypeArray = new ArrayList<Integer>();
        phoneTypeArray.add(new Integer(101));
        ArrayList<Anr> anrList = new ArrayList<Anr>();

        final int maxAnrCount = 2;
        final int dstSlotId = 0;

        mContactAccountExt.buildValuesForSim(ACCOUNT_TYPE_USIM, getInstrumentation().getContext(), values,
                additionalNumberArray, phoneTypeArray, maxAnrCount, dstSlotId, anrList, ContactsPlugin.COMMD_FOR_AAS);

        mContactAccountExt.buildValuesForSim(ACCOUNT_TYPE_USIM, getInstrumentation().getContext(), values,
                additionalNumberArray, phoneTypeArray, maxAnrCount, dstSlotId, anrList, ContactsPlugin.COMMD_FOR_AAS);
    }

    public void testGetProjection() {
        mContactAccountExt.getProjection(-1, null, ContactsPlugin.COMMD_FOR_AAS);
        mContactAccountExt.getProjection(ContactAccountExtension.PROJECTION_COPY_TO_SIM, null, ContactsPlugin.COMMD_FOR_AAS);
        mContactAccountExt.getProjection(ContactAccountExtension.PROJECTION_LOAD_DATA, null, ContactsPlugin.COMMD_FOR_AAS);
        mContactAccountExt.getProjection(ContactAccountExtension.PROJECTION_ADDRESS_BOOK, null, ContactsPlugin.COMMD_FOR_AAS);
    }
}
