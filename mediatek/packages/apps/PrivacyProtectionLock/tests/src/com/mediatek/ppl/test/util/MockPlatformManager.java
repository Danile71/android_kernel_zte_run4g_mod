package com.mediatek.ppl.test.util;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.mediatek.common.ppl.IPplAgent;
import com.mediatek.ppl.PlatformManager;

public class MockPlatformManager extends PlatformManager {
    protected static final String TAG = "PPL/MockPlatformManager";

    private IPplAgent mAgent;
    public boolean usbMassStorageEnabled = false;
    private boolean mIsDataEnabled = false;
    private boolean mHasAcquireLock = false;
    private String mMessageInfo;

    public MockPlatformManager(Context context) {
        super(context);
        mAgent = new MockPplAgent(context);
    }

    @Override
    public IPplAgent getPPLAgent() {
        return mAgent;
    }

    @Override
    public boolean isUsbMassStorageEnabled() {
        return usbMassStorageEnabled;
    }

    @Override
    public void setMobileDataEnabled(boolean enable) {
        mIsDataEnabled = enable;
        Log.i(TAG, "setMobileDataEnabled " + mIsDataEnabled);
    }

    @Override
    public void acquireWakeLock() {
        mHasAcquireLock = true;
        Log.i(TAG, "acquireWakeLock " + mHasAcquireLock);
    }

    @Override
    public void sendTextMessage(String destinationAddress, long id, String text, Intent sentIntent, int simId) {
        super.sendTextMessage(destinationAddress, id, text, sentIntent, simId);

        mMessageInfo = MockPplUtil.buidMessagInfo(text, simId);
    }

    // @Override
    public void stayForeground(Service service) {
        Log.i(TAG, "Bring service to foreground");
    }

    // @Override
    public void leaveForeground(Service service) {
        Log.d(TAG, "Exec stopForeground with para true.");
    }

    public String getMessageInfo() {
        return mMessageInfo;
    }
}
