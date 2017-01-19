
package com.mediatek.mediatekdm.test;

import android.content.ComponentName;
import android.content.Intent;
import android.os.Bundle;
import android.test.AndroidTestCase;
import android.util.Log;

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.mediatek.mediatekdm.DmApplication;
import com.mediatek.mediatekdm.DmConst;
import com.mediatek.mediatekdm.DmFeatureSwitch;
import com.mediatek.mediatekdm.DmOperation;
import com.mediatek.mediatekdm.DmOperationManager;
import com.mediatek.mediatekdm.DmReceiver;
import com.mediatek.mediatekdm.util.Path;

import java.lang.reflect.Field;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.TimeUnit;

public class ReceiverTest extends AndroidTestCase {
    private class ReceiverMockContext extends MockRenamingDelegatingContext {
        public ReceiverMockContext() {
            super(DmApplication.getInstance());
            Log.w(TAG, "ReceiverMockContext.<init>()");
        }

        @Override
        public ComponentName startService(Intent intent) {
            notifyStartService(intent);
            return null;
        }
    }

    public static final String TAG = "MDMTest/ReceiverTest";
    private static final int REGISTERED_SIM = 0;
    private static final int WAIT_TIMEOUT = 5;

    private BlockingQueue<Intent> mBlockingQueue;
    private DmReceiver mDmReceiver;
    private ReceiverMockContext mMockContext;

    public void testcase00() {
        Intent intent = new Intent(TelephonyIntents.ACTION_SIM_INDICATOR_STATE_CHANGED);
        intent.putExtra(TelephonyIntents.INTENT_KEY_ICC_STATE, PhoneConstants.SIM_INDICATOR_NORMAL);
        // Has no extra TelephonyIntents.INTENT_KEY_ICC_SLOT
        mDmReceiver.onReceive(mMockContext, intent);
        assertTrue(
                "SIM_INDICATOR_NORMAL should not start service",
                waitStartService(null));
    }

    public void testcase01() {
        Intent resultIntent = new Intent(DmConst.IntentAction.DM_KICK_OFF);
        Intent intent = new Intent(TelephonyIntents.ACTION_SIM_INDICATOR_STATE_CHANGED);
        intent.putExtra(TelephonyIntents.INTENT_KEY_ICC_STATE, PhoneConstants.SIM_INDICATOR_NORMAL);
        intent.putExtra(TelephonyIntents.INTENT_KEY_ICC_SLOT, REGISTERED_SIM);
        mDmReceiver.onReceive(mMockContext, intent);
        assertTrue(
                "SIM_INDICATOR_NORMAL should start service with intent " + resultIntent,
                waitStartService(resultIntent));
    }

    public void testcase02() {
        Intent resultIntent = new Intent(DmConst.IntentAction.DM_KICK_OFF);
        Intent intent = new Intent(TelephonyIntents.ACTION_SIM_INDICATOR_STATE_CHANGED);
        intent.putExtra(TelephonyIntents.INTENT_KEY_ICC_STATE, PhoneConstants.SIM_INDICATOR_ROAMING);
        intent.putExtra(TelephonyIntents.INTENT_KEY_ICC_SLOT, REGISTERED_SIM);
        mDmReceiver.onReceive(mMockContext, intent);
        assertTrue(
                "SIM_INDICATOR_ROAMING should start service with intent " + resultIntent,
                waitStartService(resultIntent));
    }

    public void testcase03() {
        if (DmFeatureSwitch.MTK_GEMINI_SUPPORT) {
            DmOperationManager mockOperationManager =
                    new DmOperationManager(Path.getPathInData(DmApplication.getInstance(), Path.DM_OPERATION_FOLDER));
            mockOperationManager.notifyCurrentAborted();
            setOperationManager(mockOperationManager);

            Intent resultIntent = new Intent(DmConst.IntentAction.GEMINI_DATA_RECOVERED);
            Intent intent = new Intent(TelephonyIntents.ACTION_SIM_INDICATOR_STATE_CHANGED);
            intent.putExtra(TelephonyIntents.INTENT_KEY_ICC_STATE, PhoneConstants.SIM_INDICATOR_CONNECTED);
            intent.putExtra(TelephonyIntents.INTENT_KEY_ICC_SLOT, REGISTERED_SIM);
            mDmReceiver.onReceive(mMockContext, intent);
            assertTrue(
                    "SIM_INDICATOR_CONNECTED should start service with intent " + resultIntent,
                    waitStartService(resultIntent));
        }
    }

    public void testcase04() {
        if (DmFeatureSwitch.MTK_GEMINI_SUPPORT) {
            DmOperationManager mockOperationManager =
                    new DmOperationManager(Path.getPathInData(DmApplication.getInstance(), Path.DM_OPERATION_FOLDER));
            mockOperationManager.notifyCurrentAborted();
            setOperationManager(mockOperationManager);

            Intent resultIntent = new Intent(DmConst.IntentAction.GEMINI_DATA_RECOVERED);
            Intent intent = new Intent(TelephonyIntents.ACTION_SIM_INDICATOR_STATE_CHANGED);
            intent.putExtra(TelephonyIntents.INTENT_KEY_ICC_STATE, PhoneConstants.SIM_INDICATOR_ROAMINGCONNECTED);
            intent.putExtra(TelephonyIntents.INTENT_KEY_ICC_SLOT, REGISTERED_SIM);
            mDmReceiver.onReceive(mMockContext, intent);
            assertTrue(
                    "SIM_INDICATOR_ROAMINGCONNECTED should start service with intent " + resultIntent,
                    waitStartService(resultIntent));
        }
    }

    public void testcase05() {
        if (DmFeatureSwitch.MTK_GEMINI_SUPPORT) {
            DmOperationManager mockOperationManager =
                    new DmOperationManager(Path.getPathInData(DmApplication.getInstance(), Path.DM_OPERATION_FOLDER));
            DmOperation operation = new DmOperation();
            operation.initSI(new byte[16]);
            mockOperationManager.enqueue(operation);
            setOperationManager(mockOperationManager);

            Intent resultIntent = new Intent(DmConst.IntentAction.GEMINI_DATA_RECOVERED);
            Intent intent = new Intent(TelephonyIntents.ACTION_SIM_INDICATOR_STATE_CHANGED);
            intent.putExtra(TelephonyIntents.INTENT_KEY_ICC_STATE, PhoneConstants.SIM_INDICATOR_CONNECTED);
            intent.putExtra(TelephonyIntents.INTENT_KEY_ICC_SLOT, REGISTERED_SIM);
            mDmReceiver.onReceive(mMockContext, intent);
            assertTrue(
                    "SIM_INDICATOR_CONNECTED should start service with intent " + resultIntent,
                    waitStartService(resultIntent));
        }
    }

    public void testcase06() {
        if (DmFeatureSwitch.MTK_GEMINI_SUPPORT) {
            DmOperationManager mockOperationManager =
                    new DmOperationManager(Path.getPathInData(DmApplication.getInstance(), Path.DM_OPERATION_FOLDER));
            DmOperation operation = new DmOperation();
            operation.initSI(new byte[16]);
            mockOperationManager.enqueue(operation);
            setOperationManager(mockOperationManager);

            Intent resultIntent = new Intent(DmConst.IntentAction.GEMINI_DATA_RECOVERED);
            Intent intent = new Intent(TelephonyIntents.ACTION_SIM_INDICATOR_STATE_CHANGED);
            intent.putExtra(TelephonyIntents.INTENT_KEY_ICC_STATE, PhoneConstants.SIM_INDICATOR_ROAMINGCONNECTED);
            intent.putExtra(TelephonyIntents.INTENT_KEY_ICC_SLOT, REGISTERED_SIM);
            mDmReceiver.onReceive(mMockContext, intent);
            assertTrue(
                    "SIM_INDICATOR_ROAMINGCONNECTED should start service with intent " + resultIntent,
                    waitStartService(resultIntent));
        }
    }

    public void testcase07() {
        Intent intent = new Intent("other.intent");
        mDmReceiver.onReceive(mMockContext, intent);
        Intent resultIntent = new Intent(intent);
        assertTrue(
                "Intent should be forwarded as is: " + resultIntent,
                waitStartService(resultIntent));
    }

    private void notifyStartService(Intent intent) {
        try {
            Log.d(TAG, "notifyStartService intent is " + intent);
            mBlockingQueue.put(intent);
            Log.d(TAG, "intent has been put to queue");
        } catch (InterruptedException e) {
            throw new Error(e);
        }
    }

    private void setOperationManager(DmOperationManager manager) {
        try {
            Field field = DmOperationManager.class.getDeclaredField("sInstance");
            field.setAccessible(true);
            field.set(DmOperationManager.class, manager);
            field.setAccessible(false);
        } catch (Exception e) {
            throw new Error(e);
        }
    }

    private boolean waitStartService(Intent expectedIntent) {
        Intent receivedIntent = null;
        try {
            receivedIntent = mBlockingQueue.poll(WAIT_TIMEOUT, TimeUnit.SECONDS);
        } catch (InterruptedException e) {
            new Error(e);
        }
        Log.d(TAG, "waitStartService expectedIntent is " + expectedIntent);
        Log.d(TAG, "waitStartService receivedIntent is " + receivedIntent);
        if (receivedIntent == null && expectedIntent == null) {
            return true;
        }
        if (receivedIntent == null || !expectedIntent.getAction().equals(receivedIntent.getAction())) {
            Log.e(TAG, "Actions do not match.");
            return false;
        }
        Bundle expectedExtras = expectedIntent.getExtras();
        Bundle extras = receivedIntent.getExtras();
        if (extras == null && expectedExtras == null) {
            return true;
        } else if (extras == null && expectedExtras != null) {
            Log.e(TAG, "Extras is missing.");
            return false;
        } else if (extras != null && expectedExtras == null) {
            return true;
        } else {
            for (String key : expectedExtras.keySet()) {
                if (!extras.containsKey(key)) {
                    Log.e(TAG, "Key " + key + " is missing.");
                    return false;
                }
                if (!extras.get(key).equals(expectedExtras.get(key))) {
                    Log.e(TAG, "Value of " + key + " is wrong: " + extras.get(key));
                    return false;
                }
            }
            return true;
        }
    }

    @Override
    protected void setUp() throws Exception {
        Log.d(TAG, "super.setUp()");
        super.setUp();
        Log.d(TAG, "ReceiverTest.setUp()");
        mMockContext = new ReceiverMockContext();
        MockPlatformManager.setUp();
        mDmReceiver = new DmReceiver();
        mBlockingQueue = new ArrayBlockingQueue<Intent>(1);
    }

    @Override
    protected void tearDown() throws Exception {
        Log.d(TAG, "ReceiverTest.tearDown()");
        MockPlatformManager.tearDown();
        mMockContext = null;
        mDmReceiver = null;
        setOperationManager(null);
        Log.d(TAG, "super.tearDown()");
        super.tearDown();
    }
}
