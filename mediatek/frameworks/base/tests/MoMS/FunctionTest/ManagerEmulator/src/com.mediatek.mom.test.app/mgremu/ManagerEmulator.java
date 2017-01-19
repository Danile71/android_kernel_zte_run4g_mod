/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.mom.test.app.mgremu;

import java.util.List;
import java.util.ArrayList;

import android.app.Notification;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.ResolveInfo;
import android.os.RemoteException;
import android.os.Bundle;
import android.os.IBinder;
import android.os.ServiceManager;
import android.util.Log;
import android.widget.Toast;

import com.mediatek.common.mom.IMobileManager;
import com.mediatek.common.mom.IMobileManagerService;
import com.mediatek.common.mom.IPermissionListener;
import com.mediatek.common.mom.IRequestedPermissionCallback;
import com.mediatek.common.mom.IMobileConnectionCallback;
import com.mediatek.common.mom.Permission;
import com.mediatek.common.mom.PermissionRecord;
import com.mediatek.common.mom.ReceiverRecord;

public class ManagerEmulator extends Service {
    private IMobileManager mMobileManager = null;
    private IMobileManagerService mMoMS = null;
    private int mOperation = MGR_OP_NONE;
    private boolean mAttached = false;
    private static final String TAG = "ManagerEmulator";
    myConnectionCallback mConnectionCB = new myConnectionCallback();

    // Operation Result action
    public static final String OP_DONE_ACTION = "com.mediatek.mom.test.app.mgremu.operation.done";

    // Intent extra keys
    public static final String EXTRA_MGR_OP = "extra_operation";
    public static final String EXTRA_UID = "extra_uid";
    public static final String EXTRA_ENABLE = "extra_enable";
    public static final String EXTRA_PACKAGE = "extra_package";
    public static final String EXTRA_STATUS = "extra_status";
    public static final String EXTRA_RESULT = "extra_result";
    public static final String EXTRA_SUBPERMISSION = "extra_sub_permission";
    public static final String EXTRA_PARAM_1 = "extra_param_1";
    public static final String EXTRA_INTENT = "extra_intent";
    public static final String EXTRA_USERID = "extra_userid";

    // Result code
    public static final int RESULT_NONE = -1;
    public static final int RESULT_SUCCESS = 1;
    public static final int RESULT_FAILED = 0;
    public static final int RESULT_WAIT = -999;
    public static final int RESULT_TIMEOUT = -998;

    // Emualted manager operations
    public static final int MGR_OP_NONE = 0;
    public static final int MGR_OP_CB_TIMEOUT = -1;
    // Permission Controller (PMC)
    public static final int MGR_OP_PMC_ATTACH = 1;
    public static final int MGR_OP_PMC_ATTACH_NULL_CB = 2;
    public static final int MGR_OP_PMC_DETACH = 3;
    public static final int MGR_OP_PMC_REGISTER_CB = 4;
    public static final int MGR_OP_PMC_ENABLE_CONTROLLER = 5;
    public static final int MGR_OP_PMC_SET_RECORD = 6;
    public static final int MGR_OP_PMC_GET_RECORD = 7;
    public static final int MGR_OP_PMC_GET_INSTALLED_PACKAGES = 8;
    public static final int MGR_OP_PMC_CHECK_PERMISSION = 9;
    public static final int MGR_OP_PMC_CHECK_PERMISSION_ASYNC = 10;
    public static final int MGR_OP_PMC_CB_CONNECTION_ENDED = 11;
    public static final int MGR_OP_PMC_CB_CONNECTION_RESUME = 12;
    public static final int MGR_OP_PMC_CB_PERMISSION_CHECK = 13;

    // Receiver Controller (REC)
    public static final int MGR_OP_REC_OFFSET = 100;
    public static final int MGR_OP_REC_GET_RECEIVER_LIST = MGR_OP_REC_OFFSET;
    public static final int MGR_OP_REC_SET_RECORD = MGR_OP_REC_OFFSET + 1;
    public static final int MGR_OP_REC_GET_RECORD = MGR_OP_REC_OFFSET + 2;
    public static final int MGR_OP_REC_FILTER_RECEIVER = MGR_OP_REC_OFFSET + 3;
    public static final int MGR_OP_REC_START_MONITOR = MGR_OP_REC_OFFSET + 4;
    public static final int MGR_OP_REC_STOP_MONITOR = MGR_OP_REC_OFFSET + 5;

    // Permission Callback Parameters
    public static final int CB_PARAM_NULL = 0;
    public static final int CB_PARAM_GRANTED = 1;
    public static final int CB_PARAM_DENIED = 2;

    // Callback for attachment
    private class myConnectionCallback extends IMobileConnectionCallback.Stub {
        @Override
        public void onConnectionEnded()
                throws RemoteException {
                mAttached = false;
                Log.d(TAG, "onConnectionEnded()");
                sendResult(MGR_OP_PMC_CB_CONNECTION_ENDED, RESULT_SUCCESS);
        }

        @Override
        public void onConnectionResume()
                throws RemoteException {
                mAttached = false;
                Log.d(TAG, "onConnectionResume()");
                sendResult(MGR_OP_PMC_CB_CONNECTION_RESUME, RESULT_SUCCESS);
        }
    }

    // Callback for permission checking
    private class GrantPermissionListener extends IPermissionListener.Stub {
        @Override
        public boolean onPermissionCheck(PermissionRecord record, int flag, int uid, Bundle data)
                throws RemoteException {
            return true;
        }

        @Override
        public void onPermissionChange(PermissionRecord record)
                throws RemoteException {
        }
    }
    private class DeniedPermissionListener extends IPermissionListener.Stub {
        @Override
        public boolean onPermissionCheck(PermissionRecord record, int flag, int uid, Bundle data)
                throws RemoteException {
            return false;
        }

        @Override
        public void onPermissionChange(PermissionRecord record)
                throws RemoteException {
        }
    }

    // Callback for async permission checking
    private class RequestPermissionCB extends IRequestedPermissionCallback.Stub {
        @Override
        public void onPermissionCheckResult(String permissionName, int uid, int result) {
            sendResult(MGR_OP_PMC_CHECK_PERMISSION_ASYNC, result);
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        int operation = MGR_OP_NONE;

        if (intent != null) {
            operation = intent.getIntExtra(EXTRA_MGR_OP, MGR_OP_NONE);
        } else {
            operation = MGR_OP_NONE;
        }
        executeOperation(operation, intent);
        // Not support bind service
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        mMobileManager = (IMobileManager) getSystemService(Context.MOBILE_SERVICE);
        mMoMS = IMobileManagerService.Stub.asInterface(
                ServiceManager.getService(Context.MOBILE_SERVICE));
        // Start itself as a foreground service
        Notification notification = new Notification.Builder(this).
                setSmallIcon(android.R.drawable.ic_dialog_alert).
                setContentTitle("MoMS Manager Emulator").
                setContentText("Start debugging").
                getNotification();
        startForeground(999, notification);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        stopForeground(true);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        super.onStartCommand(intent, flags, startId);
        int operation = MGR_OP_NONE;

        if (intent != null) {
            operation = intent.getIntExtra(EXTRA_MGR_OP, MGR_OP_NONE);
        } else {
            operation = MGR_OP_NONE;
        }
        executeOperation(operation, intent);

        return START_STICKY;
    }

    void executeOperation(int operation, Intent intent) {
        // Execute operation according to intent
        Log.d(TAG, "Start with operation: " + operation + " intent: " + intent);

        // Handle operations for each module
        handlePermissionControlOperation(intent, operation);
        handleReceiverControlOperation(intent, operation);
    }

    private void handleReceiverControlOperation(Intent intent, int operation) {
        int result = RESULT_NONE;
        switch (operation) {
        case MGR_OP_REC_GET_RECEIVER_LIST: {
            ArrayList<ReceiverRecord> list = (ArrayList<ReceiverRecord>)mMobileManager.getBootReceiverList();
            sendListResult(operation, list);
            } break;
        case MGR_OP_REC_SET_RECORD: {
            String pkg = intent.getStringExtra(EXTRA_PACKAGE);
            int data = intent.getIntExtra(EXTRA_ENABLE, 0);
            boolean enabled = (data > 0) ? true : false;
            mMobileManager.setBootReceiverEnabledSetting(pkg, enabled);
            sendResult(operation, RESULT_SUCCESS);
            } break;
        case MGR_OP_REC_GET_RECORD: {
            String pkg = intent.getStringExtra(EXTRA_PACKAGE);
            boolean enabled = mMobileManager.getBootReceiverEnabledSetting(pkg);
            int data = (enabled) ? 1 : 0;
            sendResult(operation, data);
            } break;
        case MGR_OP_REC_FILTER_RECEIVER: {
            Intent action = (Intent)intent.getParcelableExtra(EXTRA_INTENT);
            int userId = intent.getIntExtra(EXTRA_USERID, 0);
            ArrayList<ResolveInfo> resolveList = intent.getParcelableArrayListExtra(EXTRA_PARAM_1);
            if (resolveList != null) {
                try {
                    mMoMS.filterReceiver(action, resolveList, userId);
                } catch (Exception e) {
                    Log.e(TAG, "checkPermission() failed!");
                }
            }
            sendListResult(operation, resolveList);
            } break;
        case MGR_OP_REC_START_MONITOR: {
            try {
                mMoMS.startMonitorBootReceiver("Start monitor for debug");
            } catch (Exception e) {
                Log.e(TAG, "startMonitorBootReceiver() failed!");
            }
            sendResult(operation, RESULT_SUCCESS);
        } break;
        case MGR_OP_REC_STOP_MONITOR: {
            try {
                mMoMS.stopMonitorBootReceiver("Stop monitor for debug");
            } catch (Exception e) {
                Log.e(TAG, "stopMonitorBootReceiver() failed!");
            }
            sendResult(operation, RESULT_SUCCESS);
        } break;
        default:
        }
    }

    private void handlePermissionControlOperation(Intent intent, int operation) {
        int result = RESULT_NONE;
        switch (operation) {
        case MGR_OP_PMC_ENABLE_CONTROLLER: {
            if (requestAttach()) {
                int data = intent.getIntExtra(EXTRA_ENABLE, 0);
                boolean enabled = (data > 0) ? true : false;
                mMobileManager.enablePermissionController(enabled);
                sendResult(operation, RESULT_SUCCESS);
            } else {
                Log.e(TAG, "requestAttach Failed!()");
            }
            } break;
        case MGR_OP_PMC_REGISTER_CB: {
            if (requestAttach()) {
                int data = intent.getIntExtra(EXTRA_PARAM_1, 0);
                switch (data) {
                    case CB_PARAM_NULL:
                        mMobileManager.registerPermissionListener(null);
                        break;
                    case CB_PARAM_GRANTED: // Always return GRANTED
                        mMobileManager.registerPermissionListener(new GrantPermissionListener());
                        break;
                    case CB_PARAM_DENIED: // Always return DENIED
                        mMobileManager.registerPermissionListener(new DeniedPermissionListener());
                        break;
                }
                sendResult(operation, RESULT_SUCCESS);
            } else {
                Log.e(TAG, "requestAttach Failed!()");
            }
            } break;
        case MGR_OP_PMC_SET_RECORD: {
            if (requestAttach()) {
                String pkg = intent.getStringExtra(EXTRA_PACKAGE);
                String subPermission = intent.getStringExtra(EXTRA_SUBPERMISSION);
                int status = intent.getIntExtra(EXTRA_STATUS, IMobileManager.PERMISSION_STATUS_GRANTED);
                mMobileManager.setPermissionRecord(new PermissionRecord(pkg, subPermission, status));
                sendResult(operation, RESULT_SUCCESS);
            } else {
                Log.e(TAG, "requestAttach Failed!()");
            }
            } break;
        case MGR_OP_PMC_GET_RECORD: {
            String pkg = intent.getStringExtra(EXTRA_PACKAGE);
            ArrayList<Permission> list = (ArrayList<Permission>)mMobileManager.getPackageGrantedPermissions(pkg);
            sendListResult(operation, list);
            } break;
        case MGR_OP_PMC_GET_INSTALLED_PACKAGES: {
            ArrayList<PackageInfo> list = (ArrayList<PackageInfo>)mMobileManager.getInstalledPackages();
            sendListResult(operation, list);
            } break;
        case MGR_OP_PMC_DETACH: {
            if (mAttached) {
                mMobileManager.detach();
                sendResult(operation, RESULT_SUCCESS);
            } else {
                sendResult(operation, RESULT_FAILED);
            }
            } break;
        case MGR_OP_PMC_ATTACH: {
            boolean withCallback = intent.getBooleanExtra(EXTRA_PARAM_1, false);
            if (withCallback) {
                mAttached = requestAttach();
            } else {
                mAttached = mMobileManager.attach(null);
            }
            Log.d(TAG, "MGR_OP_PMC_ATTACH, mAttached: " + mAttached);
            result = (mAttached) ? RESULT_SUCCESS : RESULT_FAILED;
            sendResult(operation, result);
            } break;
        case MGR_OP_PMC_CHECK_PERMISSION: {
            int status = -1;
            int uid = intent.getIntExtra(EXTRA_UID, -1);
            String subPermission = intent.getStringExtra(EXTRA_SUBPERMISSION);
            if (uid != -1) {
                try {
                    status = mMoMS.checkPermission(subPermission, uid);
                } catch (Exception e) {
                    Log.e(TAG, "checkPermission() failed!");
                }
            } else {
                Log.e(TAG, "Invalid uid!");
            }
            sendResult(operation, status);
            } break;
        case MGR_OP_PMC_CHECK_PERMISSION_ASYNC: {
            int status = -1;
            int uid = intent.getIntExtra(EXTRA_UID, -1);
            String subPermission = intent.getStringExtra(EXTRA_SUBPERMISSION);
            if (uid != -1) {
                try {
                    mMoMS.checkPermissionAsync(subPermission, uid, new RequestPermissionCB());
                } catch (Exception e) {
                    Log.e(TAG, "checkPermission() failed!");
                }
            } else {
                Log.e(TAG, "Invalid uid!");
            }
            } break;
        default:
            // Do nothing
        }
    }

    private boolean requestAttach() {
        if (!mAttached) {
            Log.d(TAG, "Not attached yet, start attach");
            mAttached = mMobileManager.attach(mConnectionCB);
            if (!mAttached) {
                Log.d(TAG, "Request attach failed!");
                return false;
            }
        } else {
            Log.d(TAG, "Already attach to MoMS");
        }
        return true;
    }

    private void sendListResult(int operation, ArrayList list) {
        Intent resultIntent = new Intent();
        resultIntent.setAction(OP_DONE_ACTION);
        resultIntent.putExtra(EXTRA_MGR_OP, operation);
        resultIntent.putParcelableArrayListExtra(EXTRA_RESULT, list);
        sendBroadcast(resultIntent);
    }

    private void sendResult(int operation, int result) {
        // Send back the result
        Intent resultIntent = new Intent();
        resultIntent.setAction(OP_DONE_ACTION);
        resultIntent.putExtra(EXTRA_MGR_OP, operation);
        resultIntent.putExtra(EXTRA_RESULT, result);
        sendBroadcast(resultIntent);
    }

    private void makeToast(String message) {
        Toast toast = Toast.makeText(this, message, Toast.LENGTH_SHORT);
        toast.show();
    }
}
