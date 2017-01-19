package com.mediatek.sensorhub;

import android.app.PendingIntent;
import android.os.Build;
import android.os.Parcel;
import android.os.Parcelable;
import android.util.Log;

/**
 * Represents the general behavior to be acted by sensor hub service.
 * {@link Condition} can be used with it to configure when the behavior should be trigger.
 * 
 * @see com.mediatek.sensorhub.SensorHubManager#requestAction(Condition, Action)
 */
public class Action implements Parcelable {
    private static final String TAG = "Action";
    private static final boolean LOG = !"user".equals(Build.TYPE) && !"userdebug".equals(Build.TYPE);

    /**
     * @hide
     */
    public static final int ACTION_ID_INVALID               = 0x00;

    /**
     * @hide
     */
    public static final int ACTION_ID_AP_WAKEUP             = 0x01;

    /**
     * @hide
     */
    public static final int ACTION_ID_TOUCH_ACTIVE          = 0x03;

    /**
     * @hide
     */
    public static final int ACTION_ID_TOUCH_DEACTIVE        = 0x04;

    /**
     * @hide
     */
    public static final int ACTION_ID_CONSYS_WAKEUP         = 0x05;

    private static final int NO_INTENT = 0;
    private static final int HAS_INTENT = 1;

    private final int mActionId;
    private final boolean mRepeatable;
    private final boolean mOnConditionChanged;
    private final PendingIntent mIntent;

    /**
     * @hide
     */
    public Action(int actionId, boolean repeatable, boolean onConditionChanged) {
        mActionId = actionId;
        mIntent = null;
        mRepeatable = repeatable;
        mOnConditionChanged = onConditionChanged;
    }

    /**
     * Creates a PendingIntent type action:
     * When the action is to be performed, the PendingItent will be sent.
     * 
     * @param intent a PendingIntent to be sent when the action is performed.
     * @param repeatable whether to repeat the action or not
     * @param onConditionChanged whether to perform the action only when its condition's status changed.
     * 
     * @see #isOnConditionChanged
     */
    public Action(PendingIntent intent, boolean repeatable, boolean onConditionChanged) {
        mActionId = ACTION_ID_AP_WAKEUP;
        mIntent = intent;
        mRepeatable = repeatable;
        mOnConditionChanged = onConditionChanged;
    }

    /**
     * Creates a PendingIntent type action:
     * When the action is to be performed, the PendingItent will be sent.
     * 
     * @param intent a PendingIntent to be sent when the action is performed.
     * @param repeatable whether to repeat the action or not
     * 
     */
    public Action(PendingIntent intent, boolean repeatable) {
        this(intent, repeatable, true);
    }

    private Action(Parcel source) {
        mActionId = source.readInt();
        int hasIntent = source.readInt();
        if (hasIntent == HAS_INTENT) {
            mIntent = PendingIntent.CREATOR.createFromParcel(source);
        } else {
            mIntent = null;
        }
        mRepeatable = source.readInt() > 0 ? true : false;
        mOnConditionChanged = source.readInt() > 0 ? true : false;
        if (LOG) {
            Log.v(TAG, "readParcel: size=" + source.dataSize() + "," + this.toString());			
        }
    }

    /**
     * Gets the PendingIntent instance related to this action.
     * 
     * @return The PendingIntent instance of this action or {@code null} if none was specified.
     */
    public PendingIntent getIntent() {
        return mIntent;
    }

    /**
     * Indicates whether this action is repeatable.
     * 
     * @return {@code true} if the action is repeatable, {@code false} otherwise.
     */
    public boolean isRepeatable() {
        return mRepeatable;
    }

    /**
     * Indicates whether this action is performed only when the condition's status is changed. 
     * The condition's status changed means the condition related to this action was not 
     * met at the previous check, but is met at the current check.
     * The default value is {@code true}.
     * 
     * @return {@code true} if this action to be performed only when condition changed, {@code false} otherwise.
     */
    public boolean isOnConditionChanged() {
        return mOnConditionChanged;
    }

    /**
     * @hide
     */
    public int getAction() {
        return mActionId;
    }

    /**
     * @hide
     */
    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder(128);
        sb.append("action[");
        sb.append(mActionId);
        sb.append(",");
        sb.append(mRepeatable);
        sb.append(",");
        sb.append(mOnConditionChanged);
        sb.append(",");
        sb.append(mIntent);
        sb.append(']');
        return sb.toString();
    }

    /**
     * @hide
     */
    public static final Parcelable.Creator<Action> CREATOR = new Creator<Action>() {
        @Override
        public Action createFromParcel(Parcel source) {
            return new Action(source);
        }
        @Override
        public Action[] newArray(int size) {
            return new Action[size];
        }
    };

    /**
     * @hide
     */
    @Override
    public int describeContents() {
        return 0;
    }

    /**
     * @hide
     */
    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(mActionId);
        if (mIntent == null) {
            dest.writeInt(NO_INTENT);
        } else {
            dest.writeInt(HAS_INTENT);
            mIntent.writeToParcel(dest, flags);
        }
        dest.writeInt((mRepeatable ? 1 : 0));
        dest.writeInt((mOnConditionChanged ? 1 : 0));
        if (LOG) {
           Log.v(TAG, "writeToParcel: size=" + dest.dataSize() + ", " + this.toString());
        }
    }
}
