/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.widget;

import android.util.Log;

public class AnimationEx  {
    private static final String TAG = "AnimationEx";
    public static final int TYPE_ANIMATION_CONTINUOUS = 1;
    public static final int TYPE_ANIMATION_PLAYBACK = 2;
    public static final int TYPE_ANIMATION_INTERVAL = 5;
    public static final int TYPE_ANIMATION_SINGLEIMAGE = 3;
    public static final int CONTINUOUS_FRAME_ANIMATION_CHANGE_THRESHOLD = 4;
    private int mCurrentMavFrameIndex = 0xFFFF;
    
    private int mTargetMavFrameIndex = 0xFFFF;
    private boolean right = false;
    public int mType = 0;
    private int FRAME_COUNT;
    private long version = 1;
    private boolean isDisabled = false;
    private boolean mIsInTransitionMode = false;
    private long mBeginTime ;
    private long mEndTime ;
    private int mIntervalTime = 0;
    
    public AnimationEx(int frameCount, int currentMavFrameIndex, int targetFrame
            ,int type, int intervalTime) {
        mIntervalTime = intervalTime;
        initAnimation(frameCount, currentMavFrameIndex, targetFrame, type);
    }
    
    public AnimationEx(int currentMavFrameIndex ,int type) {
        initAnimation(currentMavFrameIndex,type);
    }
    
    public void  initAnimation (int frameCount, int currentMavFrameIndex, int targetFrame
            ,int type) {
        FRAME_COUNT = frameCount;
        mTargetMavFrameIndex = targetFrame;
        mType = type;
        mCurrentMavFrameIndex = currentMavFrameIndex;
    }
    public void initAnimation(int lastIndex, int type) {
        RenderThreadEx.TYPE = type;
        mType = type;
        if (mCurrentMavFrameIndex == 0xFFFF && lastIndex != 0xFFFF) {
            mCurrentMavFrameIndex = lastIndex;
        }
        mTargetMavFrameIndex = lastIndex;
    }
    public void disableAnimation () {
        isDisabled = true;
    }
    
    public void ableAnimation () {
        isDisabled = false;
    }
    
    public boolean isInTranslateMode () {
        return mIsInTransitionMode;
    }
    
    public boolean isDisabled () {
        return isDisabled == true;
    }
    public void resetAnimation(int type) {
        RenderThreadEx.TYPE = type;
        if (type == TYPE_ANIMATION_PLAYBACK) {
            if(isRightDirection()) {
                initAnimation(FRAME_COUNT - 1, TYPE_ANIMATION_PLAYBACK);
            } else {
                initAnimation(0, TYPE_ANIMATION_PLAYBACK);
            }
        } else if (type == TYPE_ANIMATION_INTERVAL) {
            mType = type;
            mBeginTime = System.currentTimeMillis();
            mEndTime = mBeginTime + mIntervalTime;
        }
    }
    
    public boolean advanceAnimation() {
        if (QuickContactBadgeEx.DEBUG) Log.d(TAG, "advanceAnimation||||| mCurrentMavFrameIndex="+mCurrentMavFrameIndex+"   mTargetMavFrameIndex=="+
                mTargetMavFrameIndex+"   mType="+mType+
                "   FRAME_COUNT="+FRAME_COUNT +" mEndTime="+mEndTime
                +" System.currentTimeMillis()="+System.currentTimeMillis() +" mIsInTransitionMode="+mIsInTransitionMode
                +" "+this);
        if (mCurrentMavFrameIndex == 0xFFFF || mTargetMavFrameIndex == 0xFFFF ) {
            return true;
        }
        if (mType == TYPE_ANIMATION_INTERVAL) {
            return false;
        }
        
        int DValue = mCurrentMavFrameIndex - mTargetMavFrameIndex;
        if (DValue > 0) {
            right = false;
        } else if (DValue < 0) {
            right = true;
        }
        mCurrentMavFrameIndex = DValue > 0 ? mCurrentMavFrameIndex-1 : (DValue < 0 ? mCurrentMavFrameIndex + 1 : mCurrentMavFrameIndex);
        if (mType == TYPE_ANIMATION_PLAYBACK && DValue == 0) {
            mTargetMavFrameIndex = (FRAME_COUNT -1) - mTargetMavFrameIndex;
        }
        
        return isFinished();
    }
    public boolean isFinished() {
        if (mType == TYPE_ANIMATION_PLAYBACK) {
            return false;
        } else if (mType == TYPE_ANIMATION_INTERVAL){
            long currentTime = System.currentTimeMillis();
            return currentTime >= mEndTime;
        } else if (mType == TYPE_ANIMATION_CONTINUOUS) {
            return mCurrentMavFrameIndex == mTargetMavFrameIndex;
        }
        return false;
    }
     public int getType() {
         return mType;
     }
    
     public boolean isRightDirection () {
         return right;
     }
     
    public int getCurrentFrame() {
        return mCurrentMavFrameIndex;
    }
    
    public void nextStepAnimation () {
        if (!isFinished()) return ;
        if(getType() == AnimationEx.TYPE_ANIMATION_CONTINUOUS) {
            resetAnimation(AnimationEx.TYPE_ANIMATION_INTERVAL);
        } else if (getType() == AnimationEx.TYPE_ANIMATION_INTERVAL) {
            resetAnimation(AnimationEx.TYPE_ANIMATION_PLAYBACK);
        }
    }
}
