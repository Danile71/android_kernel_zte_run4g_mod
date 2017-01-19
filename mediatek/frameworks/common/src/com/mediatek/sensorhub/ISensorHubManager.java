package com.mediatek.sensorhub;

import java.util.List;

/**
 * @hide
 */
public interface ISensorHubManager {
    /**
     * Sensor hub service name.
     */
    String SENSORHUB_SERVICE = "sensorhubservice";

    List<Integer> getContextList();

    boolean isContextSupported(int type);

    int requestAction(Condition condition, Action action);

    boolean updateCondition(int requestId, Condition condition);

    boolean cancelAction(int requestId);

    /**
     * Enable touch panel gesture wake up.
     * 
     * @param enabled
     * @return
     * 
     * @hide
     */
    boolean enableGestureWakeup(boolean enabled);
}