package com.mediatek.contacts.plugin;

import java.util.ArrayList;

import com.mediatek.contacts.plugin.ContactsPlugin;
import com.mediatek.contacts.plugin.SimUtils;
import com.mediatek.contacts.ext.ContactAccountExtension;

import android.app.Instrumentation;
import android.content.ContentProviderOperation;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.ContactsContract.Contacts;
import android.test.InstrumentationTestCase;
import android.text.TextUtils;

public class OpContactAccountExtensionTest extends InstrumentationTestCase {

    public ContactAccountExtension mOpContactAccountExtension = null;
    private Instrumentation mInst = null;
    private Context mTargetContext = null;

    @Override
    protected void setUp() throws Exception {
        super.setUp();

        mOpContactAccountExtension = new ContactsPlugin(null).createContactAccountExtension();
        mInst = getInstrumentation();
        mTargetContext = mInst.getTargetContext();
    }

    @Override
    protected void tearDown() throws Exception {
        if (mOpContactAccountExtension != null) {
            mOpContactAccountExtension = null;
        }
        super.tearDown();
    }

    public void testIsFeatureEnabled() {
        String command = Utils.COMMD_FOR_SNE;
        assertTrue(mOpContactAccountExtension.isFeatureEnabled(command));

        command = ContactAccountExtension.FEATURE_AAS;
        assertFalse(mOpContactAccountExtension.isFeatureEnabled(command));
    }

    public void testIsFeatureAccount() {
        String command = Utils.COMMD_FOR_SNE;
        String accountType = SimUtils.ACCOUNT_TYPE_USIM;
        assertTrue(mOpContactAccountExtension.isFeatureAccount(accountType, command));

        accountType = SimUtils.ACCOUNT_TYPE_SIM;
        assertFalse(mOpContactAccountExtension.isFeatureAccount(accountType, command));

        accountType = SimUtils.ACCOUNT_TYPE_USIM;
        command = ContactAccountExtension.FEATURE_AAS;
        assertFalse(mOpContactAccountExtension.isFeatureAccount(accountType, command));
    }

    // setCurrentSlot()
    public void testSetCurrentSlot() {
        int slotId = 0;
        mOpContactAccountExtension.setCurrentSlot(slotId, Utils.COMMD_FOR_SNE);
        assertEquals(slotId, SimUtils.getCurSlotId());
        assertEquals(SimUtils.ACCOUNT_TYPE_USIM, SimUtils.getCurAccount());
    }

    public void testHidePhoneLabel() {
        String accountType = SimUtils.ACCOUNT_TYPE_SIM;
        assertTrue(mOpContactAccountExtension.hidePhoneLabel(accountType, null, null, Utils.COMMD_FOR_SNE));

        accountType = SimUtils.ACCOUNT_TYPE_USIM;
        assertFalse(mOpContactAccountExtension.hidePhoneLabel(accountType, null, null, Utils.COMMD_FOR_SNE));

    }

    // set the nickname value
    public void testUpdateContentValues() {
        String accountType = SimUtils.ACCOUNT_TYPE_USIM;
        String text = null;
        ContentValues values = new ContentValues();
        int type = 0;// 1,2,3anr,---

        // 01--right type,not null text,put sne success
        text = "nicknameofme";
        if (mOpContactAccountExtension.updateContentValues(accountType, values, null, text, type, Utils.COMMD_FOR_SNE)) {
            assertEquals(text, values.get("sne"));
        }
        // 02-sim
        accountType = SimUtils.ACCOUNT_TYPE_SIM;
        assertFalse(mOpContactAccountExtension.updateContentValues(accountType, values, null, text, type, Utils.COMMD_FOR_SNE));

        // 03-the type is not right,super
        accountType = SimUtils.ACCOUNT_TYPE_USIM;
        type = 1;// do not CONTENTVALUE_ANR_UPDATE
        assertFalse(mOpContactAccountExtension.updateContentValues(accountType, values, null, text, type, Utils.COMMD_FOR_SNE));
    }

    // test the length of nickname,the par--feature is not useful
    public void testIsTextValid() {
        String text = null;
        int slotId = 0;
        int feature = 0;

        // 01--empty
        assertTrue(mOpContactAccountExtension.isTextValid(text, slotId, feature, Utils.COMMD_FOR_SNE));
        // 02--valid
        text = "nickmmmm";
        assertTrue(mOpContactAccountExtension.isTextValid(text, slotId, feature, Utils.COMMD_FOR_SNE));
        // 03--too long
        text = "abcdefghijklmnopqrstu";
        assertFalse(mOpContactAccountExtension.isTextValid(text, slotId, feature, Utils.COMMD_FOR_SNE));
        // 04
        text = "端午春asdfghjklop";
        assertFalse(mOpContactAccountExtension.isTextValid(text, slotId, feature, Utils.COMMD_FOR_SNE));
        // 05
        text = "端午春as";
        assertTrue(mOpContactAccountExtension.isTextValid(text, slotId, feature, Utils.COMMD_FOR_SNE));
    }

    // update nickname value
    // TODO
    public void testUpdateDataToDb() {
        String accountType = SimUtils.ACCOUNT_TYPE_USIM;
        ContentResolver resolver = mTargetContext.getContentResolver();

        ArrayList<String> arrNickname = new ArrayList<String>();
        String updateNickname = "ghjkk";
        String oldNickname = "abc";
        arrNickname.add(updateNickname);// 0
        arrNickname.add(oldNickname);// 1
        long rawId = 0; // TODO how to get the rawId,// Contacts.CONTENT_URI
        int type = 0;// DB_UPDATE_NICKNAME = 0;DB_UPDATE_ANR=1
        // 1.sim
        accountType = SimUtils.ACCOUNT_TYPE_SIM;
        boolean updateSuccess = mOpContactAccountExtension.updateDataToDb(accountType, resolver, arrNickname, null,
                rawId, type, Utils.COMMD_FOR_SNE);
        assertFalse(updateSuccess);

        // 2.type not 0,
        type = 1;
        updateSuccess = mOpContactAccountExtension
                .updateDataToDb(accountType, resolver, arrNickname, null, rawId, type, Utils.COMMD_FOR_SNE);
        assertFalse(updateSuccess);

        // 3.usim--update
        accountType = SimUtils.ACCOUNT_TYPE_USIM;
        type = 0;
        updateSuccess = mOpContactAccountExtension
                .updateDataToDb(accountType, resolver, arrNickname, null, rawId, type, Utils.COMMD_FOR_SNE);
        assertTrue(updateSuccess);

        // 4.--arrNickname null ,or do not have 2 values--false
        arrNickname.clear();
        updateSuccess = mOpContactAccountExtension
                .updateDataToDb(accountType, resolver, arrNickname, null, rawId, type, Utils.COMMD_FOR_SNE);
        assertFalse(updateSuccess);

        // 04--insert
        updateNickname = "newnicknamein";
        oldNickname = "";
        arrNickname.add(updateNickname);
        arrNickname.add(oldNickname);
        updateSuccess = mOpContactAccountExtension
                .updateDataToDb(accountType, resolver, arrNickname, null, rawId, type, Utils.COMMD_FOR_SNE);
        assertTrue(updateSuccess);

        // 05--delete--
        arrNickname.clear();
        updateNickname = "";
        oldNickname = "haveaname";
        arrNickname.add(updateNickname);
        arrNickname.add(oldNickname);
        updateSuccess = mOpContactAccountExtension
                .updateDataToDb(accountType, resolver, arrNickname, null, rawId, type, Utils.COMMD_FOR_SNE);
        assertTrue(updateSuccess);
    }

    // buildsne value
    public void testBuildOperation() {
        String accountType = SimUtils.ACCOUNT_TYPE_USIM;
        int type = 1;// TYPE_OPERATION_SNE
        ArrayList<ContentProviderOperation> operationList = new ArrayList<ContentProviderOperation>();
        String text = "nicknameoo";
        int backRef = 0;
        // 1.usim,right type
        assertTrue(mOpContactAccountExtension.buildOperation(accountType, operationList, null, text, backRef, type,
                Utils.COMMD_FOR_SNE));
        // 2.sim,
        accountType = SimUtils.ACCOUNT_TYPE_SIM;
        assertTrue(mOpContactAccountExtension.buildOperation(accountType, operationList, null, text, backRef, type,
                Utils.COMMD_FOR_SNE));
        // 3.usim,false type,call super--
        accountType = SimUtils.ACCOUNT_TYPE_USIM;
        type = 2;
        assertFalse(mOpContactAccountExtension.buildOperation(accountType, operationList, null, text, backRef, type,
                Utils.COMMD_FOR_SNE));
    }

    // build sne from curcor
    // public void testBuildOperationFromCursor() {
    // // buildOperationFromCursor(String accountType,
    // // ArrayList<ContentProviderOperation> operationList,
    // // final Cursor cursor, int index)
    // String accountType = SimUtils.ACCOUNT_TYPE_USIM;
    // ArrayList<ContentProviderOperation> operationList = new
    // ArrayList<ContentProviderOperation>();
    // int index = 7;// do not know ?? log info
    //
    // final int slotId = 0;
    // //final Uri iccUri = SimCardUtils.SimUri.getSimUri(slotId);
    // final Uri iccUri = Uri.parse("content://icc/pbr");//maybe other value
    // final String[] COLUMN_NAMES = new String[] { "index", "name", "number",
    // "emails", "additionalNumber", "groupIds" };
    // Cursor cursor = mTargetContext.getContentResolver().query(iccUri,
    // COLUMN_NAMES, null, null, null);
    // // 1.usim,
    // boolean buildSuccess = mOpContactAccountExtension
    // .buildOperationFromCursor(accountType, operationList, cursor,
    // index);
    //
    // int sneColumnIdx = cursor.getColumnIndex("sne");
    // if (sneColumnIdx != -1) {
    // String nickname = cursor.getString(sneColumnIdx);
    // if (!TextUtils.isEmpty(nickname)) {
    // assertTrue(buildSuccess);
    // }
    // } else {
    // assertFalse(buildSuccess);
    // }
    // // 2.sim,
    // }
    // the other functions

}
