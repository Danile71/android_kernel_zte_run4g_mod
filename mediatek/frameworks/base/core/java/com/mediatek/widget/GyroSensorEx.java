package com.mediatek.widget;

import android.app.Activity;
import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.util.Log;
import android.view.WindowManager;

public class GyroSensorEx {
    private static final String TAG = "GyroSensorEx";
    private Context mContext;
    protected SensorManager mSensorManager;
    protected Sensor mGyroSensor;
    protected boolean mHasGyroSensor;
    protected Object mSyncObj = new Object();
    protected GyroPositionListener mListener = null;
    public static final float UNUSABLE_ANGLE_VALUE = -1;
    public interface GyroPositionListener {
        public void onCalculateAngle(long newTimestamp, float eventValues0 , float eventValues1, int newRotation);
        public boolean calculate(long newTimestamp, float eventValues0 , float eventValues1, int newRotation);
    }
    
    GyroSensorEx(Context context) {
        mContext = context;
        mSensorManager = (SensorManager)mContext.getSystemService(Context.SENSOR_SERVICE);
        mGyroSensor = mSensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE);
        mHasGyroSensor = (mGyroSensor != null);
        if (!mHasGyroSensor) {
            // TODO 
            // show MavSeekBar if has gyro sensor
            // hide MavSeekBar if no gyro sensor,
            // maybe this action should be do when onProgressChanged()
            Log.d(TAG, "not has gyro sensor");
        } else {

        }
    }
    
    
    private void registerGyroSensorListener() {
        if (mHasGyroSensor) {
            Log.d(TAG, "register gyro sensor listener");
            mSensorManager.registerListener(mPositionListener, mGyroSensor,
                SensorManager.SENSOR_DELAY_GAME);
        }
    }

    private void unregisterGyroSensorListener() {
        if (mHasGyroSensor) {
            Log.d(TAG, "unregister gyro listener");
            mSensorManager.unregisterListener(mPositionListener);
        }
    }
    private PositionListener mPositionListener = new PositionListener();

    public class PositionListener implements SensorEventListener {
        @Override
        public void onAccuracyChanged(Sensor sensor, int accuracy) {
        }

        @Override
        public void onSensorChanged(SensorEvent event) {
            onGyroSensorChanged(event);
        }
    }

    public void setGyroPositionListener(GyroPositionListener gyroPositionListener) {
        synchronized(mSyncObj) {
            registerGyroSensorListener();
            mListener = gyroPositionListener;
            }
    }
    
    public void onGyroSensorChanged(SensorEvent event) {
        synchronized(mSyncObj) {
            if (mListener != null) {
            WindowManager w = (WindowManager)mContext.getSystemService(Context.WINDOW_SERVICE);
            int newRotation = w.getDefaultDisplay().getRotation();
            mListener.calculate(event.timestamp, event.values[0],  event.values[1] ,newRotation);
            }
        }
    }

    

}
