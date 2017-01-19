package com.mediatek.contacts.plugin;

import java.util.ArrayList;

import com.mediatek.contacts.ext.ContactListExtension;
import com.mediatek.contacts.plugin.ContactsPlugin;
import com.mediatek.contacts.plugin.SimUtils;

import android.content.ContentValues;
import android.test.InstrumentationTestCase;

public class OpContactsListExtensionTest extends InstrumentationTestCase {

    public ContactListExtension mOpContactListExtension = null;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mOpContactListExtension = new ContactsPlugin(null).createContactListExtension();
    }

    @Override
    protected void tearDown() throws Exception {
        if (mOpContactListExtension != null) {
            mOpContactListExtension = null;
        }
        super.tearDown();
    }

    // USIM card in slot(0)
    public void testBuildSimNickname() {
        String simNickname = null;
        String accountType = SimUtils.ACCOUNT_TYPE_USIM;
        int slotId = 0;
        // to construct an ContentValue---
        ContentValues values = new ContentValues();
        // values.put("", value);
        ArrayList<String> nicknameArray = new ArrayList<String>();
        String defValue = "d";
        int maxLength = SimUtils.getSneRecordMaxLen(slotId);

        // 01--sim,can not put it into values.try change the 01--02
        accountType = SimUtils.ACCOUNT_TYPE_SIM;
        String nickname = "abcdef";
        nicknameArray.add(nickname);
        simNickname = mOpContactListExtension.buildSimNickname(accountType, values, nicknameArray, slotId, defValue,
                Utils.COMMD_FOR_SNE);
        assertFalse(nickname.equals(simNickname));

        // 02--usim ,slotId--has sne;snelenth==17
        accountType = SimUtils.ACCOUNT_TYPE_USIM;
        simNickname = mOpContactListExtension.buildSimNickname(accountType, values, nicknameArray, slotId, defValue,
                Utils.COMMD_FOR_SNE);
        assertTrue(nickname.equals(simNickname));

        // test if the nickname is valid.
        // 03--usim,but the length of nickname is too long ,can't put it into
        // values.
        nicknameArray.clear();
        nickname = "wwwwweeeeetttttuuuuyyy";
        nicknameArray.add(nickname);
        simNickname = mOpContactListExtension.buildSimNickname(accountType, values, nicknameArray, slotId, defValue,
                Utils.COMMD_FOR_SNE);
        if (nickname.length() > maxLength) {
            assertFalse(nickname.equals(simNickname));
        }
        // 04--usim,the nickname array is null,so the returned value is null ,do
        // not the defValue.
        nicknameArray.clear();
        simNickname = mOpContactListExtension.buildSimNickname(accountType, values, nicknameArray, slotId, defValue,
                Utils.COMMD_FOR_SNE);
        assertFalse(defValue.equals(simNickname));

        // 05--usim,the nickname has chinese string ,
        nicknameArray.clear();
        nickname = "中秋lll";
        nicknameArray.add(nickname);
        simNickname = mOpContactListExtension.buildSimNickname(accountType, values, nicknameArray, slotId, defValue,
                Utils.COMMD_FOR_SNE);
        assertTrue(nickname.equals(values.get("sne")));

        // 06--usim,the nickname is all the chinese string,check the length.
        nicknameArray.clear();
        nickname = "中秋端午春中秋端午中秋";
        nicknameArray.add(nickname);
        simNickname = mOpContactListExtension.buildSimNickname(accountType, values, nicknameArray, slotId, defValue,
                Utils.COMMD_FOR_SNE);
        if (nickname.length() > ((maxLength - 1) >> 1)) {
            assertFalse(nickname.equals(simNickname));
        }
    }
}
