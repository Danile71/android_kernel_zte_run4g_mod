package com.mediatek.systemui.plugin;

import android.test.InstrumentationTestRunner;

import junit.framework.TestSuite;

public class Op01StatusBarPluginTestRunner extends InstrumentationTestRunner {

    @Override
    public TestSuite getTestSuite() {
        TestSuite suite = new TestSuite();
        suite.addTestSuite(Op01StatusBarPluginTest.class);
        return suite;
    }

}
