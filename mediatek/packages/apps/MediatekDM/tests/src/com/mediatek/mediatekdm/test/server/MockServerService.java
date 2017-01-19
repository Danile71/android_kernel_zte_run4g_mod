
package com.mediatek.mediatekdm.test.server;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

import java.util.HashMap;
import java.util.Map;

public class MockServerService extends Service {
    private static final String TAG = "MDMTest/MockServerService";

    public static final class CMCCTestRequest {
        public static final String PAIRING_INFO_RETRIEVAL = "PAIRING_INFO_RETRIEVAL";
        public static final String DM_ACCOUNT_RETRIEVAL = "DM_ACCOUNT_RETRIEVAL";
        public static final String PARAMETERS_RETRIEVAL = "PARAMETERS_RETRIEVAL";
        public static final String PARAMETERS_RETRIEVAL_RETRY = "PARAMETERS_RETRIEVAL_RETRY";
        public static final String PARAMETERS_PROVISION = "PARAMETERS_PROVISION";
        public static final String PARAMETERS_PROVISION_RETRY = "PARAMETERS_PROVISION_RETRY";
        public static final String FIRMWARE_UPDATE = "FIRMWARE_UPDATE";
        public static final String FIRMWARE_QUERY = "FIRMWARE_QUERY";
        public static final String SOFTWARE_INSTALL = "SOFTWARE_INSTALL";
        public static final String SOFTWARE_LIST_RETRIEVAL = "SOFTWARE_LIST_RETRIEVAL";
        public static final String LOCK_PHONE = "LOCK_PHONE";
        public static final String UNLOCK_PHONE = "UNLOCK_PHONE";
        public static final String WIPE_PHONE = "WIPE_PHONE";
        public static final String PROBE_PHONE = "PROBE_PHONE";
    }

    // We use integer instead of enumerations to simplify binder communications.
    public static class TestResult {
        public static final int Success = 0;
        public static final int Fail = 1;
        public static final int Unknown = 2;
    }

    public static final String KEY_PORT = "port";
//    public static final String KEY_NONCE = "nonce";

    public static final int DEFAULT_PORT = 2525;
    static final String HOST = "127.0.0.1";

    private Map<Integer, IMockServer.Stub> mServiceBindings;

    @Override
    public void onCreate() {
        Log.d(TAG, "onCreate");
        mServiceBindings = new HashMap<Integer, IMockServer.Stub>();
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy");
        mServiceBindings = null;
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "onBind " + intent);
        int port = intent.getIntExtra(KEY_PORT, DEFAULT_PORT);
        if (mServiceBindings.containsKey(port)) {
            Log.e(TAG, "Port " + port + " is already occupied.");
            return null;
        } else {
            IMockServer.Stub binder = ServerBinderBuilder.buildServerBinder(intent, this);
            if (binder != null) {
                mServiceBindings.put(port, binder);
            }
            return binder;
        }
    }

    @Override
    public boolean onUnbind(Intent intent) {
        Log.d(TAG, "onUnbind " + intent);
        int port = intent.getIntExtra(KEY_PORT, DEFAULT_PORT);
        if (mServiceBindings.containsKey(port)) {
            mServiceBindings.remove(port);
        }
        return false;
    }
}
