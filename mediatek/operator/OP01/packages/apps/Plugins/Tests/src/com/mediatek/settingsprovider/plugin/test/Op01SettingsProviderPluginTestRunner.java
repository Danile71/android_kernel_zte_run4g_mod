package com.mediatek.settingsprovider.plugin.test;

import android.test.InstrumentationTestRunner;

import junit.framework.TestSuite;

public class Op01SettingsProviderPluginTestRunner extends InstrumentationTestRunner {

    @Override
    public TestSuite getTestSuite() {
        TestSuite suite = new TestSuite();
        suite.addTestSuite(Op01SettingsProviderPluginTest.class);
        return suite;
    }
    
}
