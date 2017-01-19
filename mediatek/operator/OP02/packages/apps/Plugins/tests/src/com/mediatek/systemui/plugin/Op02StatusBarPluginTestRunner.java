package com.mediatek.systemui.plugin;

import android.test.InstrumentationTestRunner;

import junit.framework.TestSuite;

public class Op02StatusBarPluginTestRunner extends InstrumentationTestRunner {

    @Override
    public TestSuite getTestSuite() {
        TestSuite suite = new TestSuite();
        suite.addTestSuite(Op02StatusBarPluginTest.class);
        return suite;
    }

}
