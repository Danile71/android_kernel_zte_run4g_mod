package com.mediatek.sensorhub;

import android.app.PendingIntent;

import com.mediatek.sensorhub.Condition;
import com.mediatek.sensorhub.Action;
import com.mediatek.sensorhub.ParcelableListInteger;

/**
 * @hide
 */
interface ISensorHubService {
    ParcelableListInteger getContextList();
    int requestAction(in Condition condition, in Action action);
    boolean cancelAction(in int requestId);
    boolean updateCondition(in int requestId, in Condition condition);
    boolean enableGestureWakeup(in boolean enabled);
}