package com.mediatek.sensor;

/**
 * SensorPlus lists out the new sensor types add by Mediatek.
 * 
 * @hide
 */
public final class SensorPlus {

    /**
     * A constant describing the pedometer sensor type. 
     * <p/>
     * <em>Underlying base sensor(s): Accelerometer<br/>
     * Trigger-mode: On-change<br/>
     * Wake-up sensor: No</em>
     * <p/>
     * The {@link android.hardware.SensorEvent#values SensorEvent#values} for this sensor are:
     * <ul>
     * <li>values[0]: Step length in meter</li>
     * <li>values[1]: Step frequency</li>
     * <li>values[2]: Total step count</li>
     * <li>values[3]: Total distance in meter</li>
     * </ul>
     */
    public static final int TYPE_PEDOMETER = 21;

    /**
     * A constant describing the device's position relative to user detector sensor type.
     * <p/>
     * <em>Underlying base sensor(s): Accelerometer,Light and Proximity<br/>
     * Trigger-mode: On-change<br/>
     * Wake-up sensor: No</em>
     * <p/>
     * The {@link android.hardware.SensorEvent#values SensorEvent#values} for this sensor are:
     * <ul>
     * <li>values[0]: Value 1.0 indicates the device is in pocket, and value 0 indicates the device is out of pocket.</li>
     * </ul>
     */
    public static final int TYPE_CARRY = 22;

    /**
     * A constant describing a activity recognition sensor type. 
     * <p/>
     * <em>Underlying base sensor(s): Accelerometer<br/>
     * Trigger-mode: On-change<br/>
     * Wake-up sensor: No</em>
     * <p/>
     * The {@link android.hardware.SensorEvent#values SensorEvent#values} for this sensor are:
     * <ul>
     * <li>values[0]: The confidence of that the user is in vehicle.</li>
     * <li>values[1]: The confidence of that the device is on a user who is on a bicycle.</li>
     * <li>values[2]: The confidence of that the device is on a user who is on foot.</li>
     * <li>values[3]: The confidence of that the device is still.</li>
     * <li>values[4]: The confidence of that the device's activity is unknown.</li>
     * <li>values[5]: The confidence of that the device is in tilting state.</li>
     * </ul>
     */
    public static final int TYPE_USER_ACTIVITY = 23;

    /**
     * A constant describing a device pick up status detector trigger sensor. 
     * <p/>
     * <em>Underlying base sensor(s): Accelerometer and Proximity<br/>
     * Trigger-mode: One-shot<br/>
     * Wake-up sensor: No</em>
     * <p/>
     * When the sensor detects a pick up event, the {@link android.hardware.TriggerEventListener TriggerEventListener} 
     * is called with the {@link android.hardware.TriggerEvent TriggerEvent}. 
     * The sensor is automatically canceled after the trigger. 
     * <p/>
     * The {@link android.hardware.TriggerEvent#values SensorEvent#values} field is of length 1. 
     * values[0] = 1.0 when the sensor triggers. 1.0 is the only allowed value.
     */
    public static final int TYPE_PICK_UP = 24;

    /**
     * A constant describing a device facing status detector trigger sensor.
     * <p/>
     * <em>Underlying base sensor(s): Accelerometer<br/>
     * Trigger-mode: One-shot<br/>
     * Wake-up sensor: No</em>
     * <p/>
     * When the sensor detects the device is facing down, the {@link android.hardware.TriggerEventListener TriggerEventListener} 
     * is called with the {@link android.hardware.TriggerEvent TriggerEvent}. 
     * The sensor is automatically canceled after the trigger. 
     * <p/>
     * The {@link android.hardware.TriggerEvent#values SensorEvent#values} field is of length 1. 
     * values[0] = 1.0 when the sensor triggers. 1.0 is the only allowed value.
     */
    public static final int TYPE_FACING = 25;

    /**
     * A constant describing a device shaking state detector trigger sensor. 
     * <p/>
     * <em>Underlying base sensor(s): Accelerometer<br/>
     * Trigger-mode: One-shot<br/>
     * Wake-up sensor: No</em>
     * <p/>
     * When the sensor detects the device is being shaken, the {@link android.hardware.TriggerEventListener TriggerEventListener} 
     * is called with the {@link android.hardware.TriggerEvent TriggerEvent}. 
     * The sensor is automatically canceled after the trigger. 
     * <p/>
     * The {@link android.hardware.TriggerEvent#values SensorEvent#values} field is of length 1. 
     * values[0] = 1.0 when the sensor triggers. 
     * 1.0 is the only allowed value.
     */
    public static final int TYPE_SHAKE = 26;
}
