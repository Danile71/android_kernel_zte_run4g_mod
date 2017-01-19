package com.mediatek.mms.op09;

import com.mediatek.mms.ext.IUnreadMessageNumber;
import com.mediatek.mms.op09.Op09MmsPluginTestRunner.BasicCase;
import com.mediatek.pluginmanager.PluginManager;

public class UnreadMessageNumberTest extends BasicCase {
    private IUnreadMessageNumber mUnreadMessageNumber;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mUnreadMessageNumber = (IUnreadMessageNumber)PluginManager.createPluginObject(mContext,
                "com.mediatek.mms.ext.IUnreadMessageNumber");
    }

    public void test001UpdateUnreadMsgNumber() {
        mUnreadMessageNumber.updateUnreadMessageNumber(mContext);
    }

}
