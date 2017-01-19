package com.mediatek.sensorhub;

import java.util.ArrayList;
import java.util.List;

import android.content.Intent;
import android.os.Parcel;
import android.os.Parcelable;
import android.util.Log;

/**
 * Represents the data snapshot of the time when an action is performed.
 * <p/>
 * When a requested action is performed, sensor hub service will snapshot 
 * all data of the condition.
 * The application can use this class to extract the data from the callback intent.
 * <code><pre>
 *      ActionDataResult result = ActionDataResult.extractResult(intent);
 * </pre></code>
 * Please see {@linkplain com.mediatek.sensorhub.SensorHubManager#requestAction(Condition, Action) 
 * SensorHubManager.requestAction(Condition, Action)} about how to request an action to be performed 
 * when some conditions are met.
 */
public class ActionDataResult implements Parcelable {
    private static final String TAG = "ActionDataResult";
    private static final boolean LOG = true;

    /**
     * Extra key name in PendingIntent.
     * 
     * @hide
     */
    public static final String EXTRA_ACTION_DATA_RESULT = "com.mediatek.sensorhub.EXTRA_ACTION_DATA_RESULT";
//    public static final String EXTRA_ACTION_DATA_RESULT_ARRAY = "com.mediatek.sensorhub.EXTRA_ACTION_DATA_RESULT_ARRAY";

    /**
     * Extracts the ActionDataResult from the Intent.
     * It is a utility function which extracts the ActionDataResult 
     * from the extras of an Intent that was sent by the sensor hub service.
     * 
     * @param intent The Intent instance the ActionDataResult to be extracted from.
     * 
     * @return The ActionDataResult contained in the intent if any, 
     * or {@code null} if the intent doesn't contain one.
     */
    public static ActionDataResult extractResult(Intent intent) {
        if (hasResult(intent)) {
            return intent.getParcelableExtra(EXTRA_ACTION_DATA_RESULT);
        }
        return null;
    }

    /**
     * Checks whether an intent contains an ActionDataResult instance.
     * 
     * @param intent The intent to be checked.
     * 
     * @return {@code true} if the intent contains an ActionDataResult, {@code false} otherwise.
     */
    public static boolean hasResult(Intent intent) {
        boolean has = false;
        if (intent != null && intent.hasExtra(EXTRA_ACTION_DATA_RESULT)) {
            has = true;
        }
        if (LOG) {
            Log.v(TAG, "hasResult(" + intent + ") return " + has);
        }
        return has;
    }

    private final int mRequestId;
    private final long mElapsedRealtimeMillis;
    private final List<DataCell> mData; 

    /**
     * @hide
     */
    public ActionDataResult(int requestId, DataCell[] data, long elapsed) {
        mRequestId = requestId;
        mData = new ArrayList<DataCell>();
        if (data != null) {
            for (int i = 0, size = data.length; i < size; i++) {
                mData.add(data[i]);
            }
        }
        mElapsedRealtimeMillis = elapsed;
    }

    /**
     * @hide
     */
    public ActionDataResult(int requestId, List<DataCell> data, long elapsed) {
        mRequestId = requestId;
        mData = new ArrayList<DataCell>(data);
        mElapsedRealtimeMillis = elapsed;
    }

    private ActionDataResult(Parcel in) {
        mRequestId = in.readInt();
        mElapsedRealtimeMillis = in.readLong();
        int size = in.readInt();
        mData = new ArrayList<DataCell>(size);
        for (int i = 0; i < size; i++) {
            mData.add(DataCell.CREATOR.createFromParcel(in));
        }
    }

    /**
     * Gets the request ID of this ActionDataResult. 
     * 
     * @return The global unique request id of this ActionDataResult.
     * 
     */
    public int getRequestId() {
        return mRequestId;
    }

    /**
     * Gets the data snapshot from ActionDataResult.
     * 
     * @return The DataCell list, with one DataCell for one data index.
     */
    public List<DataCell> getData() {
        return mData;
    }

    /**
     * Gets the elapsed real time of this snapshot in milliseconds since boot, 
     * including the time spent in sleep as obtained by SystemClock.elapsedRealtime().
     * 
     * @return The elapsed real time.
     */
    public long getElapsedRealtimeMillis() {
        return mElapsedRealtimeMillis;
    }

    /**
     * @hide
     */
    @Override
    public String toString() {
        return new StringBuilder()
        .append("ActionDataResult(mRequestId=")
        .append(mRequestId)
        .append(", mData=")
        .append(mData)
        .append(", mElapsedRealtimeMillis=")
        .append(mElapsedRealtimeMillis)
        .append(")")
        .toString();
    }

    /**
     * @hide
     */
    public static final Parcelable.Creator<ActionDataResult> CREATOR
            = new Parcelable.Creator<ActionDataResult>() {
        @Override
        public ActionDataResult createFromParcel(Parcel source) {
            return new ActionDataResult(source);
        }

        @Override
        public ActionDataResult[] newArray(int size) {
            return new ActionDataResult[size];
        }
    };

    /**
     * @hide
     */
    @Override
    public void writeToParcel(Parcel out, int flags) {
        out.writeInt(mRequestId);
        out.writeLong(mElapsedRealtimeMillis);
        if (mData != null) {
            int size = mData.size();
            out.writeInt(size);
            for (int i = 0; i < size; i++) {
                mData.get(i).writeToParcel(out, flags);
            }
        } else {
            out.writeInt(0);
        }
    }

    /**
     * @hide
     */
    @Override
    public int describeContents() {
        return 0;
    }
}
