package com.mediatek.contacts.plugin;

import com.mediatek.contacts.ext.LabeledEditorExtension;

import android.test.InstrumentationTestCase;

public class DialtactsExtensionTest extends InstrumentationTestCase {
    private static final String TAG = "DialtactsExtensionTest";

    public void testStartActivity() {
        LabeledEditorExtension dialtactsExt = new ContactsPlugin(getInstrumentation().getTargetContext())
                .createLabeledEditorExtension();
        SimUtils.setCurrentSlot(-1);
        dialtactsExt.onTypeSelectionChange(0, ContactsPlugin.COMMD_FOR_AAS);
        assertFalse(false);

        SimUtils.setCurrentSlot(0);
        if (SimUtils.isUsim(SimUtils.getCurAccount())) {
            dialtactsExt.onTypeSelectionChange(0, ContactsPlugin.COMMD_FOR_AAS);
            assertTrue(true);
        } else {
            LogUtils.e(TAG, "testStartActivity, the slot 0 is not USIM");
        }
    }
}
