package com.mediatek.sensorhub;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.concurrent.CopyOnWriteArrayList;

import android.app.PendingIntent;
import android.app.PendingIntent.CanceledException;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Binder;
import android.os.Build;
import android.os.Bundle;
import android.os.PowerManager;
import android.os.RemoteException;
import android.os.SystemClock;
import android.util.Log;

import com.mediatek.sensorhub.Action;
import com.mediatek.sensorhub.Condition;
import com.mediatek.sensorhub.ISensorHubService;
import com.mediatek.sensorhub.ParcelableListInteger;

/**
 * The implementation of sensor hub service.
 * 
 * @hide
 */
public class SensorHubService extends ISensorHubService.Stub {
    private static final String TAG = "SensorHubService";

    static final boolean LOG = !"user".equals(Build.TYPE) && !"userdebug".equals(Build.TYPE);

    private int mNativeContext; // accessed by native methods
    private int mListenerContext; // accessed by native methods
    private final Context mContext;
    private int mBroadcastRefCount = 0;
    private PowerManager.WakeLock mWakeLock;
    private final ResultReceiver mResultReceiver = new ResultReceiver();
    private Object mLock = new Object();

    class ResultReceiver implements PendingIntent.OnFinished {
        @Override
        public void onSendFinished(PendingIntent pi, Intent intent, int resultCode,
                String resultData, Bundle resultExtras) {
            synchronized (mLock) {
                mBroadcastRefCount--;
                if (LOG) Log.v(TAG, "onSendFinished: wlCount=" + mBroadcastRefCount);
                if (mBroadcastRefCount == 0) {
                    mWakeLock.release();
                }
            }
        }
    }

    private static class Holder {
        public final int pid;
        public final int uid;
        public Holder() {
            pid = Binder.getCallingPid();
            uid = Binder.getCallingUid();
        }
    }

    private static class ActionHolder extends Holder {
        public final int rid;
        public final boolean repeat;
        public final PendingIntent intent;
        
        public ActionHolder(int requestId, PendingIntent intent, boolean repeat) {
            super();
            this.rid = requestId;
            this.intent = intent;
            this.repeat = repeat;
        }
    }

    private CopyOnWriteArrayList<ActionHolder> mActionIntents = new CopyOnWriteArrayList<ActionHolder>();

    public SensorHubService(Context context) {
        mContext = context;
        nativeSetup(new WeakReference<SensorHubService>(this));
        PowerManager pm = (PowerManager)context.getSystemService(Context.POWER_SERVICE);
        mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, TAG);  
    }

    @Override
    public ParcelableListInteger getContextList() throws RemoteException {
        return new ParcelableListInteger(nativeGetContextList()); 
    }

    @Override
    public int requestAction(Condition condition, Action action) throws RemoteException {
        int permission = mContext.checkCallingOrSelfPermission(SensorHubManager.WAKE_DEVICE_SENSORHUB);
        if (PackageManager.PERMISSION_GRANTED != permission) {
            throw new SensorHubPermissionException("Need permission " + SensorHubManager.WAKE_DEVICE_SENSORHUB);
        }
        //Cannot fix rid before native function return, so we should add it after.
        final long origId = Binder.clearCallingIdentity();
        int rid = nativeRequestAction(condition, action);
        Binder.restoreCallingIdentity(origId);
        if (LOG) {
            Log.v(TAG, "requestAction: rid=" + rid + ", " + action);
        }
        if (rid > 0) {
            ActionHolder ah = new ActionHolder(rid, action.getIntent(), action.isRepeatable());            
            mActionIntents.add(ah);
            if (LOG) {
                Log.v(TAG, "requestAction: add client[rid=" + rid + ", pid=" + ah.pid + ", uid=" + ah.uid + "]");
            }
        }
        return rid;
    }

    @Override
    public boolean cancelAction(int requestId) throws RemoteException {
        ActionHolder find = null;
        for (ActionHolder holder : mActionIntents) {
            if (holder.rid == requestId) {
                find = holder;
                break;
            }
        }
        if (find != null) {
            if (find.pid != Binder.getCallingPid() || find.uid != Binder.getCallingUid()) {
                Log.w(TAG, "cancelAction: current[pid=" + Binder.getCallingPid() + ",uid=" 
                    + Binder.getCallingUid() + "], old[pid=" + find.pid + ",uid=" + find.uid + "]");
            }
        } else {
            if (LOG) {
                Log.v(TAG, "cancelAction: succeed due to no client. rid=" + requestId);
            }
            return true;
        }
        final long origId = Binder.clearCallingIdentity();
        boolean removed = nativeCancelAction(requestId);
        Binder.restoreCallingIdentity(origId);
        if (LOG) {
            Log.v(TAG, "cancelAction: rid=" + requestId + (removed ? " succeed." : " failed!"));
        }
        if (!removed) {
            return false;
        }

        mActionIntents.remove(find);
        return true;
    }

    @Override
    public boolean updateCondition(int requestId, Condition condition) throws RemoteException {
        int permission = mContext.checkCallingOrSelfPermission(SensorHubManager.WAKE_DEVICE_SENSORHUB);
        if (PackageManager.PERMISSION_GRANTED != permission) {
            throw new SensorHubPermissionException("Need permission " + SensorHubManager.WAKE_DEVICE_SENSORHUB);
        }
        final long origId = Binder.clearCallingIdentity();
        boolean result = nativeUpdateCondition(requestId, condition);
        Binder.restoreCallingIdentity(origId);
        if (LOG) {
            Log.v(TAG, "updateCondition: rid=" + requestId + (result ? " succeed." : " failed!"));
        }
        return result;
    }

    @Override
    public boolean enableGestureWakeup(boolean enabled)
            throws RemoteException {
        int permission = mContext.checkCallingOrSelfPermission(SensorHubManager.WAKE_DEVICE_SENSORHUB);
        if (PackageManager.PERMISSION_GRANTED != permission) {
            throw new SensorHubPermissionException("Need permission " + SensorHubManager.WAKE_DEVICE_SENSORHUB);
        }
        boolean result = nativeEnableGestureWakeup(enabled);
        return result;
    }

    private ArrayList<DataCell> buildData(Object[] data) {
        ArrayList<DataCell> list = new ArrayList<DataCell>();
        if (data != null) {
            DataCell previousClock = null;
            DataCell currentClock = null;
            DataCell previousActivityTime = null;
            DataCell currentActivityTime = null;
            for (int i = 0; i < data.length; i++) {
                DataCell item = (DataCell)data[i];
                if (ContextInfo.Clock.TIME == item.getIndex()) {
                    if (item.isPrevious()) {
                        previousClock = item;
                    } else {
                        currentClock = item;
                    }
                } else if (ContextInfo.UserActivity.TIMESTAMP == item.getIndex()) {
                    if (item.isPrevious()) {
                        previousActivityTime = item;
                    } else {
                        currentActivityTime = item;
                    }
                } else {
                    list.add(item);
                }
            }

            if (previousClock != null && previousActivityTime != null) {
                DataCell datacell = new DataCell(ContextInfo.UserActivity.DURATION, true/*previous*/, 
                        previousClock.getLongValue() - previousActivityTime.getLongValue());
                list.add(datacell);
            } else if (currentClock != null && currentActivityTime != null) {
                DataCell datacell = new DataCell(ContextInfo.UserActivity.DURATION, false/*previous*/, 
                        currentClock.getLongValue() - currentActivityTime.getLongValue());
                list.add(datacell);
            } else {
                if (previousClock != null) {
                    list.add(previousClock);
                }
                if (currentClock != null) {
                    list.add(currentClock);
                }
                if (previousActivityTime != null) {
                    list.add(previousActivityTime);
                }
                if (currentActivityTime != null) {
                    list.add(currentActivityTime);
                }
            }
        }
        return list;
    }

    private static final int POST_EVENT_ACTION_DATA = 1;
    private void handleNativeMessage(int msg, int ext1, int ext2, Object[] data) {
        if(LOG) {
            Log.v(TAG, "handleNativeMessage: msg=" + msg + ",arg1=" + ext1 + ", arg2=" + ext2);
        }
        if (msg == POST_EVENT_ACTION_DATA) {
            int rid = ext1;
            for (ActionHolder holder : mActionIntents) {
                if (holder.rid == rid) {
                    if (!holder.repeat) {
                        mActionIntents.remove(holder);
                    }
                    ArrayList<DataCell> list = buildData(data);
                    try {
                        if (null == holder.intent) {
                            Log.w(TAG, "handleNativeMessage: null pendingintent!");
                            return;
                        }
                        synchronized(mLock) {
                            if (mBroadcastRefCount == 0) {
                                mWakeLock.acquire();
                            }
                           mBroadcastRefCount++;
                           if (LOG) Log.v(TAG, "handleNativeMessage: sending intent=" + holder.intent + ", wlCount=" + mBroadcastRefCount);
                        }
                        long elapsed = SystemClock.elapsedRealtime();
                        ActionDataResult result = new ActionDataResult(rid, list, elapsed);
                        Intent intent = new Intent();
                        intent.putExtra(ActionDataResult.EXTRA_ACTION_DATA_RESULT, result);
                        holder.intent.send(mContext, 0, intent, mResultReceiver, null);
                    } catch (CanceledException e) {
                        Log.e(TAG, "handleNativeMessage: exception for rid " + ext1, e);
                    }
                }
            }
        }
    }

    @SuppressWarnings(("UnusedDeclaration"))
    private static void postEventFromNative(Object selfRef, int msg, int ext1, int ext2, Object[] data) {
        @SuppressWarnings("unchecked")
        SensorHubService service = (SensorHubService)((WeakReference<SensorHubService>)selfRef).get();
        if (service == null) {
            Log.e(TAG, "postEventFromNative: Null SensorHubService! msg=" + msg + ", arg1=" + ext1 + ", arg2=" + ext2);
            return;
        }

        service.handleNativeMessage(msg, ext1, ext2, data);
    }

    public native int[] nativeGetContextList();
    private native int nativeRequestAction(Condition condition, Action action);
    private native boolean nativeUpdateCondition(int requestId, Condition condition);
    private native boolean nativeCancelAction(int requestId);
    private native boolean nativeEnableGestureWakeup(boolean enable);

    private static native void nativeInit();
    private native void nativeSetup(Object weakRef);
    private native void nativeFinalize();

    static {
        System.loadLibrary("sensorhub_jni");
        nativeInit();
    }
}
