
package com.mediatek.mediatekdm.test;

import android.test.AndroidTestCase;
import android.util.Log;

public class BareTestCase extends AndroidTestCase {
    private static final String TAG = "MDMTest/BareTestCase";

    @Override
    protected void setUp() throws Exception {
        Log.e(TAG, "super.setUp()");
        super.setUp();
        Log.e(TAG, "setUp()");
    }

    @Override
    protected void tearDown() throws Exception {
        Log.e(TAG, "tearDown()");
        Log.e(TAG, "super.tearDown()");
        super.tearDown();
    }
}
