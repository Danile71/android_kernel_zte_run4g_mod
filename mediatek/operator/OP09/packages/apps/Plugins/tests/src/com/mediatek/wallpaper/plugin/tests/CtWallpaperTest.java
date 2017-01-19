package com.mediatek.wallpaper.plugin.tests;

import android.test.InstrumentationTestRunner;

import junit.framework.TestSuite;

public class CtWallpaperTest extends InstrumentationTestRunner {
    @Override
    public TestSuite getAllTests() {
        TestSuite suite = new TestSuite();
        suite.addTestSuite(OP09WallpaperTest.class);
        return suite;
    }
}
