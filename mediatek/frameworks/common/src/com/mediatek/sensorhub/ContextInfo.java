package com.mediatek.sensorhub;


import android.util.Log;

import com.mediatek.sensor.SensorPlus;

/**
 * Lists out sensor context types and their corresponding data slot indices 
 * that can be used to construct {@link Condition}. 
 * 
 * <li>Context type: Context types are used to check whether they are supported 
 * on the device before constructing {@link Condition} based on them.
 * Each context is computed out based on some physical base sensors and
 * different sensors may be needed for different contexts. 
 * Meanwhile, different devices may have different sensor capabilities.
 * So it is necessary to check whether base sensors required by some context types are available on the device. 
 * If any base sensor is not available, the context type will not be supported.
 * </li>
 * 
 * <li>Data index: Each sensor context is formed by some data cells. 
 * And each data cell has a unique data slot index. 
 * You can customize {@link Condition} based on those indices. 
 * </li>
 * 
 * @see Condition
 */
//All data slot index should be sync with native value. 
public class ContextInfo {
    private static final String TAG = "ContextInfo";

    private ContextInfo() {}

    /**
     * Lists out context types. 
     */
    public static class Type {
        private Type() {
        }

        /**
         * A constant describing the clock context. 
         * 
         * @hide
         */
        public static final int CLOCK = 1001;

        /**
         * A constant describing the pedometer context.
         */
        public static final int PEDOMETER = SensorPlus.TYPE_PEDOMETER;

        /**
         * A constant describing the user activity context.
         */
        public static final int USER_ACTIVITY = SensorPlus.TYPE_USER_ACTIVITY;

        /**
         * A constant describing the device's position relative to user context.
         */
        public static final int CARRY = SensorPlus.TYPE_CARRY;

        /**
         * A constant describing device shake state context.
         */
        public static final int SHAKE = SensorPlus.TYPE_SHAKE;

        /**
         * A constant describing device facing state context.
         */
        public static final int FACING = SensorPlus.TYPE_FACING;

        /**
         * A constant describing the device pick up context.
         */
        public static final int PICK_UP = SensorPlus.TYPE_PICK_UP;

        /**
         * A constant describing the gesture context.
         * 
         * @hide
         */
        public static final int GESTURE = 1002;
    }

    /**
     * Data indices for the clock context.
     * 
     * @hide
     */
    public static class Clock {
        private Clock() {
        }

        /**
         * The data slot index for system time. 
         * The time in milliseconds since boot, including time spent in sleep. 
         * Actually, the time is obtained through SystemClock.elapsedRealtime().
         * <p/>
         * Data type: long<br/>
         * Unit: Millisecond
         * 
         * @hide
         */
        public static final int TIME = 12;
    }

    /**
     * Data indices for the pedometer context.
     */
    public static class Pedometer {
        private Pedometer() {
        }

        /**
         * The data slot index for step length.
         * <p/>
         * Data type: Integer<br/>
         * Unit: Millimeter
         */
        public static final int STEP_LENGTH = 14;

        /**
         * The data slot index for step frequency.
         * <p/>
         * Data type: float<br/>
         * Unit: Steps/second
         */
        public static final int STEP_FREQUENCY = 15;

        /**
         * The data slot index for total step count.
         * <p/>
         * Data type: Integer<br/>
         * Unit: Step
         */
        public static final int TOTAL_COUNT = 16;

        /**
         * The data slot index for total step distance.
         * <p/>
         * Data type: Integer<br/>
         * Unit: Millimeter
         */
        public static final int TOTAL_DISTANCE = 17;

        /**
         * The data index for current data's time in millisecond.
         * <p/>
         * Data type: long<br/>
         * Unit: Millisecond
         * 
         * @hide
         */
        public static final int TIMESTAMP = 18;
    }

    /**
     * Data indices for user activity context.
     */
    public static class UserActivity {
        private UserActivity() {
        }

        /**
         * This class lists out the state of user activities.
         */
        public static class State {
            private State() {
            }

            /**
             * A constant describing the device is in a vehicle, such as a car.
             */
            public static final int IN_VEHICLE = 20;

            /**
             * A constant describing the device is on a bicycle.
             */
            public static final int ON_BICYCLE = 21;

            /**
             * A constant describing the device is on a user who is walking or running.
             */
            public static final int ON_FOOT = 22;

            /**
             * A constant describing the device is still (not moving).
             */
            public static final int STILL = 23;

            /**
             * A constant describing unable to detect the current activity.
             */
            public static final int UNKNOWN = 24;

            /**
             * A constant describing the device angle relative to gravity changed significantly. 
             * This often occurs when a device is picked up from a desk or a user who is sitting stands up.
             */
            public static final int TILTING = 25;
        }

        /**
         * The data slot index for the detected user activity.
         * <p/>
         * Value range: Should be one of the following:
         * <ul>
         * <li>{@link State#IN_VEHICLE} 
         * <li>{@link State#ON_BICYCLE} 
         * <li>{@link State#ON_FOOT}
         * <li>{@link State#STILL}
         * <li>{@link State#UNKNOWN}
         * <li>{@link State#TILTING}
         * </ul>
         * Data type: Integer
         */
        public static final int CURRENT_STATE = 31;

        /**
         * The data slot index for the confidence of current user activity.
         * <p/>
         * Value range: [0, 100]<br/>
         * Data type: Integer
         */
        public static final int CONFIDENCE = 32;

         /**
         * The data slot index for the time in millisecond at which the activity was detected.
         * <p/>
         * Data type: long<br/>
         * Unit: Millisecond
         * 
         * @hide
         */
        public static final int TIMESTAMP = 33;

        /**
         * The data slot index for the duration in millisecond the current state lasts for.
         * <p/>
         * Data type: long<br/>
         * Unit: Millisecond
         */
        public static final int DURATION = 34;
    }

    /**
     * Data indices for device's position relative to user context.
     */
    public static class Carry {
        private Carry() {
        }

        /**
         * The data slot index for whether the device is in pocket.
         * <p/>
         * Value range: 1 indicates the device is in pocket; 0 indicates not.<br/>
         * Data type: Integer
         */
        public static final int IN_POCKET = 28;

        /**
        * The data index for the time in millisecond at which the state was detected.
        * <p/>
        * Data type: long<br/>
        * Unit: Millisecond
        * 
        * @hide
        */
        public static final int TIMESTAMP = 29;
    }

    /**
     * Data indices for device picked up context.
     */
    public static class Pickup {
        private Pickup() {
        }

        /**
         * The data slot index for device pick up state.
         * <p/>
         * Value range: 1 indicates the device is picked up; 0 indicates not.<br/>
         * Data type: Integer
         */
        public static final int VALUE = 38;

        /**
         * The data slot index for the time in nanosecond at which the pick up was detected.
         * <p/>
         * Data type: long<br/>
         * Unit: Second
         * 
         * @hide
         */
        public static final int TIMESTAMP = 39;
    }

    /**
     * Data for device facing state context.
     */
    public static class Facing {
        private Facing() {
        }

        /**
         * The data slot index for device facing down value.
         * <p/>
         * Value range: 1 indicates the device is face down; 0 indicates not.<br/>
         * Data type: Integer
         */
        public static final int FACE_DOWN = 41;

        /**
         * The data slot index for the time in millisecond at which device's facing state was detected.
         * <p/>
         * Data type: long<br/>
         * Unit: Millisecond
         * 
         * @hide
         */
        public static final int TIMESTAMP = 42;
    }

    /**
     * Data indices for device shake state context.
     */
    public static class Shake {
        private Shake() {
        }

        /**
         * The data slot index for shake value. 
         * <p/>
         * Value range: 1 indicates the device is shaken; 0 indicates not. <br/>
         * Data type: Integer
         */
        public static final int VALUE = 44;

        /**
         * The data slot index for the the time in millisecond at which the shake was detected.
         * <p/>
         * Data type: long<br/>
         * Unit: Millisecond
         * 
         * @hide
         */
        public static final int TIMESTAMP = 45;
    }

    /**
     * Data slot indices for gesture event.
     * 
     * @hide
     */
    public static class Gesture {
        private Gesture() {
        }

        /**
         * Supported gestures.
         */
        public static class Type {
            /**
             * A constant to describe the double tap gesture.
             */
            public static final int GES_DOUBLE_TAP         = 0;
            /**
             * A constant to describe the slide right gesture.
             */
            public static final int GES_SLIDE_RIGHT        = 1;
            /**
             * A constant to describe the slide left gesture.
             */
            public static final int GES_SLIDE_LEFT         = 2;
            /**
             * A constant to describe the slide up gesture.
             */
            public static final int GES_SLIDE_UP           = 3;
            /**
             * A constant to describe the slide down gesture.
             */
            public static final int GES_SLIDE_DOWN         = 4;
            /**
             * A constant to describe the gesture A.
             */
            public static final int GES_A                  = 5;
            /**
             * A constant to describe the gesture B.
             */
            public static final int GES_B                  = 6;
            /**
             * A constant to describe the gesture C.
             */
            public static final int GES_C                  = 7;
            /**
             * A constant to describe the gesture D.
             */
            public static final int GES_D                  = 8;
            /**
             * A constant to describe the gesture E.
             */
            public static final int GES_E                  = 9;
            /**
             * A constant to describe the gesture F.
             */
            public static final int GES_F                  = 10;
            /**
             * A constant to describe the gesture G.
             */
            public static final int GES_G                  = 11;
            /**
             * A constant to describe the gesture H.
             */
            public static final int GES_H                  = 12;
            /**
             * A constant to describe the gesture I.
             */
            public static final int GES_I                  = 13;
            /**
             * A constant to describe the gesture J.
             */
            public static final int GES_J                  = 14;
            /**
             * A constant to describe the gesture K.
             */
            public static final int GES_K                  = 15;
            /**
             * A constant to describe the gesture L.
             */
            public static final int GES_L                  = 16;
            /**
             * A constant to describe the gesture M.
             */
            public static final int GES_M                  = 17;
            /**
             * A constant to describe the gesture N.
             */
            public static final int GES_N                  = 18;
            /**
             * A constant to describe the gesture O.
             */
            public static final int GES_O                  = 19;
            /**
             * A constant to describe the gesture P.
             */
            public static final int GES_P                  = 20;
            /**
             * A constant to describe the gesture Q.
             */
            public static final int GES_Q                  = 21;
            /**
             * A constant to describe the gesture R.
             */
            public static final int GES_R                  = 22;
            /**
             * A constant to describe the gesture S.
             */
            public static final int GES_S                  = 23;
            /**
             * A constant to describe the gesture T.
             */
            public static final int GES_T                  = 24;
            /**
             * A constant to describe the gesture U.
             */
            public static final int GES_U                  = 25;
            /**
             * A constant to describe the gesture V.
             */
            public static final int GES_V                  = 26;
            /**
             * A constant to describe the gesture W.
             */
            public static final int GES_W                  = 27;
            /**
             * A constant to describe the gesture X.
             */
            public static final int GES_X                  = 28;
            /**
             * A constant to describe the gesture Y.
             */
            public static final int GES_Y                  = 29;
            /**
             * A constant to describe the gesture Z.
             */
            public static final int GES_Z                  = 30;
        }

        /**
         * The data slot index for gesture value.
         * <p/>
         * Data type: Integer
         */
        public static final int VALUE = 47;
    }

    /**
     * Gets the data type of the specified data index.
     * 
     * @param index data index
     * @return data type
     * 
     * @hide
     */
    public static int getDataType(int index) {
        if (index < Clock.TIME || index > Gesture.VALUE) {
            Log.w(TAG, "getDataType: invalid index " + index);
            return DataCell.DATA_TYPE_INVALID;
        }
        int type = DataCell.DATA_TYPE_INT;
        switch(index) {
            case Clock.TIME:
            case Pedometer.TIMESTAMP:
            case UserActivity.TIMESTAMP:
            case UserActivity.DURATION:
            case Carry.TIMESTAMP:
            case Pickup.TIMESTAMP:
            case Facing.TIMESTAMP:
            case Shake.TIMESTAMP:
                type = DataCell.DATA_TYPE_LONG;
                break;
            case Pedometer.STEP_FREQUENCY:
                type = DataCell.DATA_TYPE_FLOAT;
                break;
            default:
                break;
        }
        return type;
    }
}