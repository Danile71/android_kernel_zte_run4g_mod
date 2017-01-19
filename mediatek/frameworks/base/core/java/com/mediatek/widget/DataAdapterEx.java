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

import com.mediatek.widget.QuickContactBadgeEx.Action;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.drawable.BitmapDrawable;
import android.util.Log;


public class DataAdapterEx {
    private final String TAG = "DataAdapterEx";
    private QuickContactBadgeEx mImageview;
    private Context mContext = null;
    private Bitmap[] mMavBitmap = null;
    private int imageWidth;
    private int imageHeight;
    private int mFrameCount = -1;
    private String mFilePath = null;
    private int mType ;
    private boolean mInterrupte = false;
    public static final int STATE_UNLOADED = 0;
    public static final int STATE_QUEUED = 1;
    public static final int STATE_LOADING_MARK_FRAME = 2;
    
    public static final int STATE_LOADED_MARK_FRAME = 3;
    public static final int STATE_LOADING_ALL_FRAME = 4;
    public static final int STATE_LOADED_ALL_FRAME = 5;

    
    public static final int STATE_RELEASE_ALL_FRAME = 6;
    public static final int STATE_RELEASE_MARK_FRAME = 7;
    
    public static final int STATE_ERROR_ALL_FRAME = 8;
    public static final int STATE_ERROR_MARK_FRAME = 9;
    
    public static final int STATE_NO_MARK_FRAME = -1;
    private int mState = STATE_UNLOADED;
    public int mMarkFrameIndex = STATE_NO_MARK_FRAME;
    public Bitmap mMarkFrame = null;
    private int mCurrentIndex;

    public DataAdapterEx(QuickContactBadgeEx imageview, Context context, String filePath, int type){
        mImageview = imageview;
        mContext = context;
        mMavBitmap = null;
        imageWidth = -1;
        imageHeight = -1;
        mFrameCount = -1;
        mFilePath = filePath;
        mType = type;
        mState = STATE_UNLOADED;
        if (!mImageview.useLruCache(mType)) {
            mMarkFrameIndex = mImageview.getCurrentFrame();
        } else {
            Action mAction = QuickContactBadgeEx.sBitmapCache.get(filePath+type);
            String index  = QuickContactBadgeEx.sCacheOfFrameIndex.get(filePath+type);
            if (mAction != null && index != null) {
                mMarkFrame = mAction.getBitmap();
                mMarkFrameIndex = Integer.parseInt(index);
                
            }
        } 
        setMavImageBitmap();
    }

    public void setFilePath(String filePath) {
        mFilePath = filePath;
    }
    public void setImageWidth(int imageWidth) {
        this.imageWidth = imageWidth;
    }
    public void setImageHeight(int imgeHeigh) {
        this.imageHeight = imgeHeigh;
    }
    public void setFrames(Bitmap[] mavBitmap) {
        mMavBitmap = mavBitmap;
    }
    
    public void setFrameCount(int frameCount) {
        mFrameCount = frameCount;
    }
    public void setState(int state) {
        mState = state;
    }
    
    public String getFilePath() {
        return mFilePath;
    }
    public int getImageWidth() {
        return imageWidth;
    }
    public int getImageHeight() {
        return imageHeight ;
    }
    public int getType() {
        return mType;
    }
    public QuickContactBadgeEx getImageView() {
        return mImageview;
    }
    public Bitmap[] getBitmap() {
        return mMavBitmap;
    }
    public int getState() {
        return mState;
    }
    public Context getContext() {
        return mContext;
    }
    public int getmFrameCount() {
        return mFrameCount;
    }
    public boolean isInterrupte () {
        return mInterrupte;
    }
    public void setInterrupte(boolean interrupte) {
        mInterrupte = interrupte;
    }
    public Bitmap getMarkFrame() {
        return mMarkFrame;
    }

    public void initMarkFrame(Bitmap bitmap) {
        mMarkFrame = bitmap;
        if (QuickContactBadgeEx.DEBUG_FOR_LOG) Log.d(TAG, "setMarkFrame="+mFilePath);
        if (mImageview.useLruCache(mType)) {
            QuickContactBadgeEx.sBitmapCache.put(mFilePath+mType, new QuickContactBadgeEx.Action(mImageview, bitmap));
            QuickContactBadgeEx.sCacheOfFrameIndex.put(mFilePath+mType, Integer.toString(mMarkFrameIndex));
        } else {
            QuickContactBadgeEx.sBigBitmapCache.put(mFilePath+mType, new QuickContactBadgeEx.Action(mImageview, bitmap));
        }
        mMarkFrame = null;
    }
    public void freeFrame() {
        if (mMavBitmap != null) {
            if (QuickContactBadgeEx.DEBUG_FOR_LOG) Log.d(TAG," mImageview=="+mImageview +" this.mFilePath=="+this.mFilePath +" mtype="+mType);
            int frameCount = mMavBitmap.length;
            for (int i = 0; i < frameCount; i++) {
                if (mImageview.useLruCache(mType)) {
                    if (mCurrentIndex != i) {
                        Bitmap mBitmap = mMavBitmap[i];
                        if (mBitmap != null) mBitmap.recycle();
                    }
                } else {
                    Bitmap mBitmap = mMavBitmap[i];
                    if (mBitmap != null) mBitmap.recycle();
                }

            }
        }
       // }
        //mImageview.mCurrentFrame = null;
        //mImageview.mCurrentFrameIndex = -1;
        //mImageview.mUri = null;
        mMavBitmap = null;
    }
    public void setDataBundle(int frameCount ,int width ,int height) {
        mFrameCount = frameCount;
        this.imageWidth = width;
        this.imageHeight = height;
        if (mMarkFrameIndex == -1) {
            mMarkFrameIndex = (int) (mFrameCount*Math.random());
        }
    }

    private void setMavImageBitmap() {
        setMavImageBitmap(mFilePath, mType);
    }

    public void setMavImageBitmap (String filePath, int type) {
        mImageview.setMavImageBitmap (filePath, type);
    }

    public Bitmap getCurrentBitmap(int currentIndex) {
        if (getState() == DataAdapterEx.STATE_LOADED_ALL_FRAME && mMavBitmap != null && currentIndex >= 0) {
            return mMavBitmap[currentIndex];
        } else return null;
    }
    
    public void setCurrentFrame() {
        if (getState() == DataAdapterEx.STATE_LOADED_ALL_FRAME && mImageview.mAnimation != null) {
            mCurrentIndex = mImageview.mAnimation.getCurrentFrame();
            Bitmap bitmap = getCurrentBitmap(mCurrentIndex);
            if (bitmap == null) return;
            mImageview.setImage(bitmap);
        } else {
            setMavImageBitmap();
        }
    }
    public int getCurrentFrameIndex() {
        return mCurrentIndex;
    }

    public Bitmap getMiddleFrame() {
        Bitmap bitmap = getCurrentBitmap(getmFrameCount()/2); 
        if (bitmap != null ) {
            return bitmap;
        } else {
            return mMarkFrame;
        }
    }
}
