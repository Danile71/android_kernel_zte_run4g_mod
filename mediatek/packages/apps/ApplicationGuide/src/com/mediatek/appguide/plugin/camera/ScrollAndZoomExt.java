/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
package com.mediatek.appguide.plugin.camera;

import android.app.Activity;
import android.app.Dialog;
import android.app.StatusBarManager;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnDismissListener;
import android.content.SharedPreferences;
import android.content.pm.ActivityInfo;
import android.content.res.AssetFileDescriptor;
import android.graphics.Color;
import android.hardware.display.DisplayManager;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnCompletionListener;
import android.media.MediaPlayer.OnErrorListener;
import android.media.MediaPlayer.OnPreparedListener;
import android.os.Bundle;
import android.os.SystemProperties;
import android.view.Gravity;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.TextView;
import com.mediatek.camera.ext.IAppGuideExt;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.appguide.plugin.R;
import com.mediatek.pluginmanager.PluginLayoutInflater;
import com.mediatek.xlog.Xlog;

import java.io.IOException;
import java.util.List;

public class ScrollAndZoomExt implements IAppGuideExt {
    private static final String TAG = "ScrollAndZoomExt";
    private static final String SHARED_PREFERENCE_NAME = "application_guide";
    private static final String KEY_CAMERA_GUIDE = "camera_guide";
    private static final String KEY_CAMERA_MAV_GUIDE = "camera_mav_guide";
    static final String MAV_TYPE = "CAMERA_MAV";
    static final String CAMERA_TYPE = "CAMERA";

    private Context mContext;
    private SharedPreferences mSharedPrefs;
    private AppGuideDialog mAppGuideDialog;
    /**
     * construct method
     */
    public ScrollAndZoomExt(Context context) {
        mContext = context;
        Xlog.d(TAG,"ScrollAndZoomExt");
    }

    /**
     * Called when the app want to show camera guide
     * @param type: The app type, such as "PHONE/CONTACTS/MMS/CAMERA"
     */
    public void showCameraGuide(Activity activity, String type, OnGuideFinishListener onFinishListener) {
        Xlog.d(TAG, "showCameraGuide(), type = " + type);
        mSharedPrefs = activity.getSharedPreferences(SHARED_PREFERENCE_NAME,
                Context.MODE_WORLD_WRITEABLE);
        if (FeatureOption.MTK_LCA_ROM_OPTIMIZE 
                || (CAMERA_TYPE.equals(type) && mSharedPrefs.getBoolean(KEY_CAMERA_GUIDE, false)) 
                || (MAV_TYPE.equals(type) && mSharedPrefs.getBoolean(KEY_CAMERA_MAV_GUIDE, false))) {
            Xlog.d(TAG, "already show camera guide, return");
            onFinishListener.onGuideFinish();
            return;
        }

        mAppGuideDialog = new AppGuideDialog(activity,type);
        mAppGuideDialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
        mAppGuideDialog.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, 
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
                
        final OnGuideFinishListener finishListener = onFinishListener;
        mAppGuideDialog.setOnDismissListener(new OnDismissListener() {
            public void onDismiss(DialogInterface dialog) {
                finishListener.onGuideFinish();
            }
        });
        mAppGuideDialog.show();
        Xlog.d(TAG, "show dialog " + mAppGuideDialog);
    }

    /**
     * add for tablet ratation not in time
     * Called when the app orientation changed
     */
    public void configurationChanged() {
        Xlog.d(TAG, "configurationChanged");           
        if (mAppGuideDialog != null 
            && (mAppGuideDialog.mCurrentStep == mAppGuideDialog.SCROLL_IN_CAMERA ||
                mAppGuideDialog.mCurrentStep == mAppGuideDialog.MAV_IN_CAMERA) 
                && !mAppGuideDialog.mPrepareVideo) {
            mAppGuideDialog.prepareVideo(mAppGuideDialog.mCurrentStep);
        }      
    }

    public void dismiss() {
        Xlog.d(TAG, "dismiss " + mAppGuideDialog);
        if (mAppGuideDialog != null) {
            mAppGuideDialog.dismiss();
            mAppGuideDialog = null;
        }
    }
    
    class AppGuideDialog extends Dialog implements OnCompletionListener, 
        OnPreparedListener, SurfaceHolder.Callback {
        public  static final int SCROLL_IN_CAMERA = 0;
        public  static final int ZOOM_IN_CAMERA = 1;
        private static final int MAV_IN_CAMERA = 2;

        private Activity mActivity;
        private View mView;
        private Button mRightBtn;
        private TextView mTitle;

        private MediaPlayer mMediaPlayer;
        // Key and value for enable ClearMotion
        private static final int CLEAR_MOTION_KEY = 1700;
        private static final int CLEAR_MOTION_DISABLE = 1;

        private SurfaceView mPreview;
        private SurfaceHolder mHolder;

        int mCurrentStep;
        boolean mPrepareVideo = false;
        private int mOrientation = 0;
        private boolean mSetScreenSize = false;
        private boolean mIsSmbPlugged;
		private final String[] mVideoArray = new String[] {
                    "scroll_left_bar.mp4", "zoom_in_and_out.mp4","camera_mav.mp4" };
        boolean mFinished = false;

	    private String mGuideType;
        /**
         * next button listner, show next video.
         */
        private View.OnClickListener mNextListener = new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Xlog.d(TAG, "play next video");
                mTitle.setText("");
                mRightBtn.setVisibility(View.GONE);
                mCurrentStep++;
                if (mCurrentStep <= ZOOM_IN_CAMERA) {
                    prepareVideo(mCurrentStep);
                }
            }
        };
        /**
         * ok button listner, finish app guide.
         */
        private View.OnClickListener mOkListener = new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Xlog.d(TAG, "click ok, finish app guide");
                if (CAMERA_TYPE.equals(mGuideType)) {
                    mSharedPrefs.edit()
                    .putBoolean(KEY_CAMERA_GUIDE, true)
                    .commit();                    
                } else if (MAV_TYPE.equals(mGuideType)) {
                    mSharedPrefs.edit()
                    .putBoolean(KEY_CAMERA_MAV_GUIDE, true)
                    .commit();                     
                }

                releaseMediaPlayer();
            }
        };
        /**
         * error listner, stop play video.
         */
        private OnErrorListener mErrorListener = new OnErrorListener() {
            @Override
            public boolean onError(MediaPlayer mp, int what, int extra) {
                Xlog.d(TAG, "play error: " + what);
                releaseMediaPlayer();
                return false;
            }
        };

        public AppGuideDialog(Activity activity,String type) {
            super(activity, R.style.dialog_fullscreen);
            mActivity = activity;
            mGuideType = type;
        }

        @Override
        protected void onCreate(Bundle savedInstanceState) {
            DisplayManager displayManager = (DisplayManager)mActivity.getSystemService(Context.DISPLAY_SERVICE);
            mIsSmbPlugged = displayManager.isSmartBookPluggedIn();
            mOrientation = mActivity.getRequestedOrientation();
            // when invoke play video, need full screen
            if ((mActivity.getWindow().getAttributes().flags & WindowManager.LayoutParams.FLAG_FULLSCREEN) != 
                    WindowManager.LayoutParams.FLAG_FULLSCREEN) {
                Xlog.d(TAG, " fullscreen = false");
                mActivity.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, 
                    WindowManager.LayoutParams.FLAG_FULLSCREEN);
            }

            PluginLayoutInflater inflater = new PluginLayoutInflater(mContext);
            mView = inflater.inflate(R.layout.video_view, null);

            mRightBtn = (Button) mView.findViewById(R.id.right_btn);
            mRightBtn.setText(android.R.string.ok);
            mRightBtn.setVisibility(View.GONE);
            mTitle = (TextView) mView.findViewById(R.id.guide_title);
            
            if (MAV_TYPE.equals(mGuideType)) {
            	mCurrentStep = MAV_IN_CAMERA;
            } else if (CAMERA_TYPE.equals(mGuideType)){
            	mCurrentStep = SCROLL_IN_CAMERA;
            } else {
            	Xlog.d(TAG,"it's not MAV or common Camera MODE guide");
            }

            Xlog.d(TAG, "mCurrentStep = " + mCurrentStep);

            mPreview = (SurfaceView) mView.findViewById(R.id.surface_view);
            mHolder = mPreview.getHolder();
            mHolder.addCallback(this);
            mHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
            mPreview.setBackgroundColor(Color.BLACK);    
            setContentView(mView);
        }

        @Override
        public void onCompletion(MediaPlayer arg0) {
            Xlog.d(TAG, "onCompletion called");
            mRightBtn.setVisibility(View.VISIBLE);
            if (mCurrentStep == SCROLL_IN_CAMERA) {
                mTitle.setText(R.string.scroll_left_bar_title);
                mRightBtn.setOnClickListener(mNextListener);
            } else if (mCurrentStep == ZOOM_IN_CAMERA) {
                mTitle.setText(R.string.zoome_title);
                mRightBtn.setOnClickListener(mOkListener);
            } else if (mCurrentStep == MAV_IN_CAMERA) {
            	// camera mav guild, we should change title and textview top position
                int cameraMavTitleTop = (int)mContext.getResources().getDimension(R.dimen.camera_mav_title_margin_top);
                Xlog.d(TAG, "onCompletion, cameraMavTitleTop = " + cameraMavTitleTop);
                mTitle.setY(cameraMavTitleTop);
                mTitle.setText(R.string.camera_mav_title);
                mRightBtn.setOnClickListener(mOkListener);
            }
        }

        @Override
        public void onPrepared(MediaPlayer mediaplayer) {
            Xlog.d(TAG, "onPrepared called");
            mPreview.setBackgroundColor(android.R.color.transparent);
            mMediaPlayer.start();
        }

        @Override
        public void surfaceCreated(SurfaceHolder holder) {
            Xlog.d(TAG, "surfaceCreated called");
            int orientation = mActivity.getRequestedOrientation();
            Xlog.d(TAG, "mOrientation is " + mOrientation + " orientation = " + orientation);
            // add for surface create again
            if (MAV_TYPE.equals(mGuideType)) {
            	mCurrentStep = MAV_IN_CAMERA;
                if (!mIsSmbPlugged && 
                    orientation != ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE &&
                    orientation != ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE) {
                    mActivity.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
                }
            } else if (CAMERA_TYPE.equals(mGuideType)){
            	mCurrentStep = SCROLL_IN_CAMERA;
                if (!mIsSmbPlugged && 
                     orientation != ActivityInfo.SCREEN_ORIENTATION_PORTRAIT &&
                     orientation != ActivityInfo.SCREEN_ORIENTATION_REVERSE_PORTRAIT) {
                    mActivity.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
                }
            } else {
            	Xlog.d(TAG,"it's not MAV or common Camera MODE guide");
            }

            this.mHolder = holder;
            mMediaPlayer = new MediaPlayer();
            mMediaPlayer.setOnCompletionListener(this);
            mMediaPlayer.setOnPreparedListener(this);
            mMediaPlayer.setOnErrorListener(mErrorListener);
            mMediaPlayer.setDisplay(this.mHolder);
            mTitle.setText("");
            mRightBtn.setVisibility(View.GONE);

            if (MAV_TYPE.equals(mGuideType)) {
                if (mIsSmbPlugged
                    || orientation == ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE
                    || orientation == ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE) {
                    Xlog.d(TAG, "start to prepare vedio for MAV camera on Phone");
                    prepareVideo(mCurrentStep);
                }
            } else if (CAMERA_TYPE.equals(mGuideType)){
                if (mIsSmbPlugged 
                    || orientation == ActivityInfo.SCREEN_ORIENTATION_PORTRAIT
                    || orientation == ActivityInfo.SCREEN_ORIENTATION_REVERSE_PORTRAIT) {
                    Xlog.d(TAG, "start to prepare vedio for common camera on Phone");
                    prepareVideo(mCurrentStep);
                }
            }

        }

        @Override
        public void surfaceChanged(SurfaceHolder surfaceholder, int i, int j, int k) {
            Xlog.d(TAG, "surfaceChanged called");
        }

        @Override
        public void surfaceDestroyed(SurfaceHolder surfaceholder) {
            Xlog.d(TAG, "surfaceDestroyed called, mOrientation:" +
                   mOrientation + " mFinished = " + mFinished);
            if (surfaceholder != mHolder) {
                Xlog.d(TAG, "surfaceholder != mHolder, return");
                return;
            }

            if (mFinished) {
                if (MAV_TYPE.equals(mGuideType)) {
                    if (!mIsSmbPlugged &&
                            mOrientation != ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE &&
                            mOrientation != ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE) {
                        mActivity.setRequestedOrientation(mOrientation);
                    }
                } else if (CAMERA_TYPE.equals(mGuideType)) {
                    if (!mIsSmbPlugged &&
                            mOrientation != ActivityInfo.SCREEN_ORIENTATION_PORTRAIT &&
                            mOrientation != ActivityInfo.SCREEN_ORIENTATION_REVERSE_PORTRAIT) {
                        mActivity.setRequestedOrientation(mOrientation);
                    }
                }
            }

            if (mMediaPlayer != null) {
                mMediaPlayer.pause();
                mMediaPlayer.release();
                mMediaPlayer = null;
            }
            mPrepareVideo = false;
        }

        private void prepareVideo(int step) {
            Xlog.d(TAG, "prepareVideo step = " + step);
            // add for tablet ratation not in time
            mPrepareVideo = true;
            try {
                if (mMediaPlayer != null) {
                    mMediaPlayer.reset();
                    AssetFileDescriptor afd = mContext.getAssets().openFd(mVideoArray[step]);
                    Xlog.d(TAG, "video path = " + afd.getFileDescriptor());
                    mMediaPlayer.setDataSource(afd.getFileDescriptor(), afd.getStartOffset(), afd.getLength());
                    afd.close();
                    // Disable ClearMotion
                    mMediaPlayer.setParameter(CLEAR_MOTION_KEY, CLEAR_MOTION_DISABLE);

                    mMediaPlayer.prepare();
                    resizeSurfaceView();
                    Xlog.d(TAG, "mMediaPlayer prepare()");
                } else {
                    Xlog.d(TAG, "mMediaPlayer is null");
                }
            } catch (IOException e) {
                Xlog.e(TAG, "unable to open file; error: " + e.getMessage(), e);
                releaseMediaPlayer();
            } catch (IllegalStateException e) {
                Xlog.e(TAG, "media player is in illegal state; error: " + e.getMessage(), e);
                releaseMediaPlayer();
            }
        }

        private void releaseMediaPlayer() {
            Xlog.d(TAG, "releaseMediaPlayer");
            if (mMediaPlayer != null) {
                mMediaPlayer.release();
                mMediaPlayer = null;
            }

            onBackPressed();
        }

        public void onBackPressed() {
            mFinished = true;
            super.onBackPressed();
        }

        public void resizeSurfaceView() {
            Xlog.d(TAG, "resizeSurfaceView()");
            
            if (mSetScreenSize) {
                return;
            } else {
                mSetScreenSize = true;
            }
            
            int rotation = mActivity.getWindowManager().getDefaultDisplay().getRotation();
            Xlog.d(TAG, "resizeSurfaceView  Display.getRotation() is " + rotation);
                        
            int videoW= mMediaPlayer.getVideoWidth();
            int videoH = mMediaPlayer.getVideoHeight();
            int screenW = mActivity.getWindowManager().getDefaultDisplay().getWidth();
            int screenH = mActivity.getWindowManager().getDefaultDisplay().getHeight();
            Xlog.d(TAG, "mActivity.getWindowManager().getDefaultDisplay().getHeight() ----  screenW = " 
                                     + screenW + " ,screenH = " + screenH);
                        
            if ("tablet".equals(SystemProperties.get("ro.build.characteristics")) &&
                mContext.getApplicationInfo().getMultiWindowStatus()) {
                screenW = getWindow().getDecorView().getMeasuredWidth();
                screenH = getWindow().getDecorView().getMeasuredHeight();
            }
            Xlog.d(TAG, "videoW = " + videoW + " ,videoH = " + videoH);
            Xlog.d(TAG, "screenW = " + screenW + " ,screenH = " + screenH);

            android.view.ViewGroup.LayoutParams lp = mPreview.getLayoutParams();

            float videoScale = (float)videoH / (float)videoW;
            float screenScale = (float)screenH / (float)screenW;
            if (screenScale > videoScale) {
                lp.width = screenW;
                lp.height = (int)(videoScale * (float)screenW) + 1;
                Xlog.d(TAG, "screenScale > videoScale");
            } else {
                lp.height = screenH;
                lp.width = (int)((float)screenH / videoScale) + 1;
                Xlog.d(TAG, "screenScale < videoScale");
            }
            mPreview.setLayoutParams(lp);
            Xlog.d(TAG, "lp.height = " + lp.height + " ,lp.width = " + lp.width);
        }
    }
}

