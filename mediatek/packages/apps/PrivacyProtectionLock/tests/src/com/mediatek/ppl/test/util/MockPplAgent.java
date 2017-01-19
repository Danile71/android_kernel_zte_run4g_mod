package com.mediatek.ppl.test.util;

import android.content.Context;
import android.os.RemoteException;

import com.mediatek.common.ppl.IPplAgent;
import com.mediatek.ppl.ControlData;

public class MockPplAgent extends IPplAgent.Stub {
    private static final String TAG = "PPL/MockPplAgent";

    private static ControlData mControlData = null;

    public MockPplAgent(Context context) {
        mControlData = new ControlData();
    }

    @Override
    public byte[] readControlData() throws RemoteException {
        MockPplUtil.formatLog(TAG, "readControlData: " + mControlData);
        return mControlData.encode();
    }

    @Override
    public int writeControlData(byte[] data) throws RemoteException {
        mControlData.decode(data);
        MockPplUtil.formatLog(TAG, "writeControlData: " + mControlData);
        return 1;
    }

}
