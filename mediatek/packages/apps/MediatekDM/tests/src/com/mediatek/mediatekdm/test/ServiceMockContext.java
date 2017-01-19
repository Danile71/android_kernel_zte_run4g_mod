package com.mediatek.mediatekdm.test;

import android.content.ComponentName;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.Intent;
import android.content.ServiceConnection;
import android.util.Log;

import com.mediatek.mediatekdm.test.server.IServiceTest;

public class ServiceMockContext extends ContextWrapper {
    private static final String TAG = "MDMTest/ServiceMockContext";
    private final IServiceTest mTestCase;

    public ServiceMockContext(Context base, IServiceTest testCase) {
        super(base);
        mTestCase = testCase;
    }

    @Override
    public ComponentName startService(Intent intent) {
        Log.d(TAG, "startService: " + intent);
        return mTestCase.startServiceEmulation(intent);
    }

    @Override
    public boolean bindService(Intent intent, ServiceConnection conn, int flags) {
        Log.d(TAG, "bindService: " + intent);
        return mTestCase.bindServiceEmulation(intent, conn, flags);
    }
}
