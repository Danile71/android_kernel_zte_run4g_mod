package com.mediatek.sensorhub;

/**
 * An exception that indicates there was no corresponding permission for calling some sensor hub functions.
 */
public class SensorHubPermissionException extends SecurityException {
    private static final long serialVersionUID = 1L;
    /**
     * @param msg detail message 
     */
    public SensorHubPermissionException(String msg) {
        super(msg);
    }
}
