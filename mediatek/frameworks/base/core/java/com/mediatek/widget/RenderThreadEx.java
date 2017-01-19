package com.mediatek.widget;

import com.mediatek.widget.GyroSensorEx.GyroPositionListener;
import android.content.Context;
import android.os.SystemProperties;
import android.os.Process;
import android.util.Log;
import android.view.Surface;
import android.widget.AbsListView.OnScrollListener;
import com.mediatek.widget.QuickContactBadgeEx;

public class RenderThreadEx extends Thread implements GyroPositionListener {
    private static final String TAG = "RenderThreadEx";
    //private Animation mAnimation= null;
    private OnDrawMavFrameListener mOnDrawMavFrameListener= null;
    private Object mRenderLock = new Object();
    public boolean mRenderRequested = false;
    public boolean mIsActive = true;
    public static final int CONTINUOUS_FRAME_ANIMATION_CHANGE_THRESHOLD = 2;
    private GyroSensorEx  mGyroSensor = null;
    private Context mContext;
    public static int TYPE = 1;
    public interface OnDrawMavFrameListener {
        public boolean advanceAnimation(int targetFrame, int type);
        public void drawMavFrame();
        public void initAnimation(int targetFrame, int type);
        public void changeAnimationType();
        public int getSleepTime();
    }
    public RenderThreadEx(Context context, GyroSensorEx gyroSensor) {
        super("MavRenderThread");
        mContext = context;
       /* mGalleryActivity = galleryActivity;*/
        //mAnimation = new Animation();
        mGyroSensor = gyroSensor;
        if (mGyroSensor != null) {
            mGyroSensor.setGyroPositionListener(this);
        }
    }
 
    public void  setRenderRequester (boolean request) {
        synchronized (mRenderLock) {
            mRenderRequested = request;
            mRenderLock.notifyAll();
        }
    }
    
    public void stopActive () {
        mIsActive = false;
    }
    
    private boolean add = false;
    @Override
    public void run() {
        Process.setThreadPriority(Process.THREAD_PRIORITY_DISPLAY);
        while (mIsActive && !Thread.currentThread().isInterrupted()) {
            Log.d(TAG, "run~~~~~~~~~~~~~~~~~"+Thread.currentThread().getId()+"    mRenderRequested=="+mRenderRequested +" mContext="+mContext);
            if (/*mGalleryActivity.hasPausedActivity() || */!mIsActive) {
                Log.v(TAG, "MavRenderThread:run: exit MavRenderThread");
                return;
            }
            boolean isFinished = false;
            synchronized (mRenderLock) {
                if (mRenderRequested) {
                   // if (calculate(newTimestamp, eventValues0, eventValues1, newRotation)) {
                    isFinished = mOnDrawMavFrameListener.advanceAnimation(mLastIndex, AnimationEx.TYPE_ANIMATION_CONTINUOUS);
                    //}
                    mRenderRequested = (!isFinished) ? true : false;
                    mOnDrawMavFrameListener.drawMavFrame();
                    mOnDrawMavFrameListener.changeAnimationType();
                } else {
                    try {
                        mRenderLock.wait();
                    } catch (InterruptedException e) {
                        return;
                    }
                }
            }
            if (!isFinished) {
                try {
                    add = SystemProperties.getInt("MV_POSITIVE", 0) == 1 ? true : false;
                    if (TYPE == AnimationEx.TYPE_ANIMATION_CONTINUOUS && QuickContactBadgeEx.DEBUG_FOR_LOG) {
                        float intervalTime;
                        float factor = SystemProperties.getInt("MV_INT_GYRO", 0);
                        if (factor < 0 || factor > 1000) {
                            factor = 0;
                        }
                        if (!add) {
                            factor = -factor;
                        }
                        int sleepTime = (int) (mOnDrawMavFrameListener.getSleepTime()+factor);
                        if (sleepTime > 0) {
                            Thread.sleep(sleepTime);
                        } else {
                            Thread.sleep(mOnDrawMavFrameListener.getSleepTime());
                        }
                    } else {
                        Thread.sleep(mOnDrawMavFrameListener.getSleepTime());
                    }

                } catch (InterruptedException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
            }
        }
    }

    public void setOnDrawMavFrameListener (OnDrawMavFrameListener  lisenter) {
        mOnDrawMavFrameListener = lisenter;
    }
    
    
    private int mOrientation = -1;
    private float mValue = 0;
    private long timestamp = 0;
    private float angle[] = {0,0,0};
    private boolean mFirstTime = true;
    private static int mLastIndex = 0xFFFF;
    public static final float BASE_ANGLE = 6.5f;
    public static final float NS2S = 1.0f / 1000000000.0f;
    public static final float TH = 0.001f;
    public static final float OFFSET = 0.0f;
    public static final float UNUSABLE_ANGLE_VALUE = -1;
    long newTimestamp;
    float eventValues0;
    float eventValues1;
    int newRotation;
    
    public boolean calculate(long newTimestamp, float eventValues0 , float eventValues1, int newRotation) {
        //workaround for Gyro sensor HW limitation.
        //As sensor continues to report small movement, wrongly
        //indicating that the phone is slowly moving, we should
        //filter the small movement.
        final float xSmallRotateTH = 0.05f;
        //xSmallRotateTH indicating the threshold of max "small
        //rotation". This varible is determined by experiments
        //based on MT6575 platform. May be adjusted on other chips.
        float valueToUse = 0;
        if (mOrientation != newRotation) {
            // orientation has changed, reset calculations
            mOrientation = newRotation;
            mValue = 0;
            angle[0] = 0;
            angle[1] = 0;
            angle[2] = 0;
            mFirstTime = true;
        }
        switch (mOrientation) {
        case Surface.ROTATION_0:
            valueToUse = eventValues1;
            break;
        case Surface.ROTATION_90:
            // no need to re-map
            valueToUse = eventValues0;
            break;
        case Surface.ROTATION_180:
            // we do not have this rotation on our device
            valueToUse = -eventValues1;
            break;
        case Surface.ROTATION_270:
            valueToUse = -eventValues0;
            break;
        default:
            valueToUse = eventValues0;
        }
        mValue = valueToUse + OFFSET;
        if (timestamp != 0 && Math.abs(mValue) > TH) {
            final float dT = (newTimestamp - timestamp) * NS2S;

            angle[1] += mValue * dT * 180 / Math.PI;
            if (mFirstTime) {
                angle[0] = angle[1] - BASE_ANGLE;
                angle[2] = angle[1] + BASE_ANGLE;
                mFirstTime = false;
            } else if (angle[1] <= angle[0]) {
                angle[0] = angle[1];
                angle[2] = angle[0] + 2 * BASE_ANGLE;
            } else if (angle[1] >= angle[2]) {
                angle[2] = angle[1];
                angle[0] = angle[2] - 2 * BASE_ANGLE;
            }
        }
        float mAngle ;
        if (timestamp != 0 && QuickContactBadgeEx.mCount != 0) {
            mAngle = angle[1] - angle[0];
        } else {
            mAngle = UNUSABLE_ANGLE_VALUE;
        }
        timestamp = newTimestamp;
        
        if (mAngle != UNUSABLE_ANGLE_VALUE) {
           return onGyroPositionChanged(mAngle);
        } else { 
            return false;
        }
    }
    
    public void onCalculateAngle(long newTimestamp, float eventValues0 , float eventValues1, int newRotation) {
        this.newTimestamp = newTimestamp;
        this.eventValues0 = eventValues0;
        this.eventValues1 = eventValues1;
        this.newRotation = newRotation;
    }
    
    public boolean onGyroPositionChanged(float angle) {
        boolean isChanged = false;
        if (QuickContactBadgeEx.mCount != 0) {
            int index = (int)(angle * QuickContactBadgeEx.mCount / (2 * BASE_ANGLE));
            if (index >= 0 && index < QuickContactBadgeEx.mCount) {
                
                if ((mLastIndex == 0xFFFF || mLastIndex != index)) {
                    if (QuickContactBadgeEx.DEBUG_FOR_LOG && QuickContactBadgeEx.mAllQuickContactBadgeEx != null) Log.d(TAG, "mCount = "+QuickContactBadgeEx.mCount+" index=="+index+" mLastIndex="+mLastIndex
                            +" size= "+QuickContactBadgeEx.mAllQuickContactBadgeEx.size());
                    if (mLastIndex != 0xFFFF && Math.abs(mLastIndex - index) > 2 && (QuickContactBadgeEx.isOnIdleState())) {
                        mOnDrawMavFrameListener.initAnimation(index, AnimationEx.TYPE_ANIMATION_CONTINUOUS);
                    }
                    mLastIndex = index;
                    isChanged = true;
                }
            }
        }
        return isChanged;
        
    }
}

