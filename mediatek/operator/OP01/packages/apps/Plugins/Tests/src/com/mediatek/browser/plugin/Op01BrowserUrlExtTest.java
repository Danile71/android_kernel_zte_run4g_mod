package com.mediatek.op01.tests;

import android.content.Context;
import android.test.InstrumentationTestCase;

import com.mediatek.browser.plugin.Op01BrowserUrlExt;
import com.mediatek.pluginmanager.PluginManager;
import com.mediatek.xlog.Xlog;

public class Op01BrowserUrlExtTest extends InstrumentationTestCase {

	private final String TAG = "Op01BrowserUrlExtTest";
    private static Op01BrowserUrlExt mPlugin = null;
    private Context mContext;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContext = this.getInstrumentation().getContext();
        Object plugin = PluginManager.createPluginObject(mContext, "com.mediatek.browser.ext.IBrowserUrlExt");
        if (plugin instanceof Op01BrowserUrlExt) {
            mPlugin = (Op01BrowserUrlExt)plugin;
        }
    }

    @Override    
    protected void tearDown() throws Exception {
        super.tearDown();
        mPlugin = null;
    }

    public void test01_getNavigationBarTitle() {
        if (mPlugin != null) {
            String ret = mPlugin.getNavigationBarTitle("baidu", "www.baidu.com");
            assertEquals("baidu", ret);
        }
    }

    public void test02_getOverrideFocusContent() {
        if (mPlugin != null) {
            String ret = mPlugin.getOverrideFocusContent(
            		true, "abc", "baidu", "www.baidu.com");
            assertEquals("www.baidu.com", ret);
        }
    }

    public void test03_getOverrideFocusTitle() {
        if (mPlugin != null) {
            String ret = mPlugin.getOverrideFocusTitle("baidu", "abc");
            assertEquals("baidu", ret);
        }
    }

}
