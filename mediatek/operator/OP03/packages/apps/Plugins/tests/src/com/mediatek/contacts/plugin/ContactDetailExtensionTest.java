package com.mediatek.contacts.plugin;

import com.mediatek.contacts.ext.ContactDetailExtension;

import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.test.InstrumentationTestCase;
import android.view.View;
import android.widget.TextView;

public class ContactDetailExtensionTest extends InstrumentationTestCase {
    private static final String TAG = "ContactDetailExtensionTest";

    private ContactDetailExtension mContactDetailExt = null;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContactDetailExt = new ContactsPlugin(getInstrumentation().getTargetContext()).createContactDetailExtension();
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void testRepChar() {
        String result = mContactDetailExt.repChar(null, (char) 0, (char) 0, (char) 0,
                ContactDetailExtension.STRING_PRIMART, ContactsPlugin.COMMD_FOR_AAS);
        assertNotNull(result);
        result = mContactDetailExt.repChar(null, (char) 0, (char) 0, (char) 0, ContactDetailExtension.STRING_ADDITINAL,
                ContactsPlugin.COMMD_FOR_AAS);
        assertNotNull(result);
    }

    public void testUpdateView() {
        TextView testTextView = new TextView(getInstrumentation().getContext());
        SimUtils.setCurrentSlot(0);

        mContactDetailExt.updateView(testTextView, 0, ContactDetailExtension.VIEW_UPDATE_NONE, ContactsPlugin.COMMD_FOR_AAS);
        mContactDetailExt.updateView(testTextView, 0, ContactDetailExtension.VIEW_UPDATE_HINT, ContactsPlugin.COMMD_FOR_AAS);
        mContactDetailExt.updateView(testTextView, 1, ContactDetailExtension.VIEW_UPDATE_HINT, ContactsPlugin.COMMD_FOR_AAS);
        mContactDetailExt.updateView(testTextView, 0, ContactDetailExtension.VIEW_UPDATE_VISIBILITY, ContactsPlugin.COMMD_FOR_AAS);
        assertTrue(testTextView.getVisibility() == View.GONE);
    }

    /**
     * test: getMaxEmptyEditors(), getAdditionNumberCount
     */
    public void testNormalMethods() {
        SimUtils.setCurrentSlot(-1);
        mContactDetailExt.getMaxEmptyEditors(Phone.CONTENT_ITEM_TYPE, ContactsPlugin.COMMD_FOR_AAS);
        SimUtils.setCurrentSlot(0);
        if (SimUtils.isUsim(SimUtils.getCurAccount())) {
            mContactDetailExt.getMaxEmptyEditors(Phone.CONTENT_ITEM_TYPE, ContactsPlugin.COMMD_FOR_AAS);
        } else {
            LogUtils.w(TAG, "Error, slot-0 not isert USIM Card");
        }

        mContactDetailExt.getAdditionNumberCount(0, ContactsPlugin.COMMD_FOR_AAS);
    }

    public void testIsDoublePhoneNumber() {
        String[] buffer = new String[2];
        String[] bufferName = new String[2];
        SimUtils.setCurrentSlot(-1);
        mContactDetailExt.isDoublePhoneNumber(buffer, bufferName, ContactsPlugin.COMMD_FOR_AAS);

        SimUtils.setCurrentSlot(0);
        if (SimUtils.isUsim(SimUtils.getCurAccount())) {
            bufferName[0] = "test";
            bufferName[1] = "test";
            boolean result = mContactDetailExt.isDoublePhoneNumber(buffer, bufferName, ContactsPlugin.COMMD_FOR_AAS);
            assertTrue(result);
        } else {
            LogUtils.w(TAG, "Error, not isert USIM Card");
        }
    }
}
