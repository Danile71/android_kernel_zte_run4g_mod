package com.mediatek.mms.op09;

import com.mediatek.mms.ext.IStringReplacement;
import com.mediatek.mms.op09.Op09MmsPluginTestRunner.BasicCase;
import com.mediatek.pluginmanager.PluginManager;

public class StringReplacementTest extends BasicCase {
    private IStringReplacement mStringReplacement;
    private static final String[] mExpectedArray = new String[] {
            "Phone", "UIM Card"};
    private static final String[] mExpectedStrings = new String[] {
            "Save message to UIM/SIM card",
            "Select UIM/SIM",
            "Manage UIM/SIM card messages",
            "Manage messages stored on your UIM/SIM card",
            "Text messages on UIM card",
            "No messages on the UIM card.",
            "Get UIM capacity failed.",
            "This message on the UIM will be deleted.",
            "UIM card full",
            "It only allows to be read due to limitation of current mobile network type.",
            "SMS camps on current network will be deleted.",
            "Current capacity changes with the network which UIM camps on."
            };

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mStringReplacement = (IStringReplacement)PluginManager.createPluginObject(mContext,
                "com.mediatek.mms.ext.IStringReplacement");
    }

    /// M: This case need to set the language in English.
    public void test001GetStringAPI() {
        String[] actualArray = mStringReplacement.getSaveLocationString();
        assertEquals(mExpectedArray[0], actualArray[0]);
        assertEquals(mExpectedArray[1], actualArray[1]);

        assertEquals(mExpectedStrings[IStringReplacement.SAVE_MSG_TO_CARD - 1],
                mStringReplacement.getCTStrings(IStringReplacement.SAVE_MSG_TO_CARD));
        assertEquals(mExpectedStrings[IStringReplacement.SELECT_CARD - 1],
                mStringReplacement.getCTStrings(IStringReplacement.SELECT_CARD));
        assertEquals(mExpectedStrings[IStringReplacement.MANAGE_CARD_MSG_TITLE - 1],
                mStringReplacement.getCTStrings(IStringReplacement.MANAGE_CARD_MSG_TITLE));
        assertEquals(mExpectedStrings[IStringReplacement.MANAGE_CARD_MSG_SUMMARY - 1],
                mStringReplacement.getCTStrings(IStringReplacement.MANAGE_CARD_MSG_SUMMARY));
        assertEquals(mExpectedStrings[IStringReplacement.MANAGE_UIM_MESSAGE - 1],
                mStringReplacement.getCTStrings(IStringReplacement.MANAGE_UIM_MESSAGE));
        assertEquals(mExpectedStrings[IStringReplacement.UIM_EMPTY - 1],
                mStringReplacement.getCTStrings(IStringReplacement.UIM_EMPTY));
        assertEquals(mExpectedStrings[IStringReplacement.GET_CAPACITY_FAILED - 1],
                mStringReplacement.getCTStrings(IStringReplacement.GET_CAPACITY_FAILED));
        assertEquals(mExpectedStrings[IStringReplacement.CONFIRM_DELETE_MSG - 1],
                mStringReplacement.getCTStrings(IStringReplacement.CONFIRM_DELETE_MSG));
        assertEquals(mExpectedStrings[IStringReplacement.UIM_FULL_TITLE - 1],
                mStringReplacement.getCTStrings(IStringReplacement.UIM_FULL_TITLE));
        assertEquals(mExpectedStrings[IStringReplacement.MESSAGE_CANNOT_BE_OPERATED - 1],
                mStringReplacement.getCTStrings(IStringReplacement.MESSAGE_CANNOT_BE_OPERATED));
        assertEquals(mExpectedStrings[IStringReplacement.CONFIRM_DELETE_SELECTED_MESSAGES - 1],
                mStringReplacement.getCTStrings(IStringReplacement.CONFIRM_DELETE_SELECTED_MESSAGES));
    }

}
