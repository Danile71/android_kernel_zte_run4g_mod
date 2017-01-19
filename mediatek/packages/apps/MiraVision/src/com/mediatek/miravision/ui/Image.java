package com.mediatek.miravision.ui;

import android.app.KeyguardManager;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Rect;
import android.os.Handler;
import android.os.SystemProperties;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Surface;
import android.view.WindowManager;
import android.widget.ImageView;

import com.mediatek.miravision.setting.MiraVisionJni;

public class Image extends ImageView {
    private static final String TAG = "Miravision/ImagePreference";

    private Handler mHandler;
    private boolean mHostFragmentResumed;
    private KeyguardManager mKeyguardManager;
    private boolean mIsFirstDraw = true;
    private Context context;
    
    private Runnable mRunnable = new Runnable() {
        @Override
        public void run() {
            Log.d(TAG, "runnable mHostFragmentResumed: " + mHostFragmentResumed);
            if (mHostFragmentResumed && mKeyguardManager != null
                    && !mKeyguardManager.inKeyguardRestrictedInputMode()) {
                setPQColorRegion();
            }
        }
    };

    public Image(Context context, AttributeSet attrs) {
        super(context, attrs);
        this.context = context;
    }

    public void init(Handler handler, KeyguardManager keyguardManager) {
        mHandler = handler;
        mKeyguardManager = keyguardManager;
    }

    public void setHostFragmentResumed(boolean isResumed) {
        mHostFragmentResumed = isResumed;
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
        Log.d(TAG, "onFinishInflate()");
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        Log.d(TAG, "onMeasure()");
    }

    @Override
    protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
        super.onLayout(changed, left, top, right, bottom);
        Log.d(TAG, "onLayout()");
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        Log.d(TAG, "onDraw()");
        if (mHandler != null && mHostFragmentResumed && mKeyguardManager != null
                && !mKeyguardManager.inKeyguardRestrictedInputMode()) {
            if (mIsFirstDraw) {
                Log.d(TAG, "isFirstDraw true");
                setPQColorRegion();
                mIsFirstDraw = false;
            } else {
                Log.d(TAG, "isFirstDraw false");
                mHandler.postDelayed(mRunnable, 300);
            }
        }
    }

    @Override
    protected void onFocusChanged(boolean gainFocus, int direction, Rect previouslyFocusedRect) {
        super.onFocusChanged(gainFocus, direction, previouslyFocusedRect);
        Log.d(TAG, "onFocusChanged()");
    }

    @Override
    public void onScreenStateChanged(int screenState) {
        super.onScreenStateChanged(screenState);
        Log.d(TAG, "onScreenStateChanged()");
    }

    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);
    }

    private void setPQColorRegion() {
    	Log.d(TAG, "setPQColorRegion()");
    	int[] location = { -1, -1 };
	getLocationOnScreen(location);
	int x = location[0];
	int y = location[1];
	// Because this api needs the portrait coordinates, but the fragment is
	// in landscape,
	// so converted the location to portrait coordinates and passed it.

	android.view.WindowManager manager = (WindowManager) context
			.getSystemService(Context.WINDOW_SERVICE);
	int rotation = manager.getDefaultDisplay().getRotation();

	int screenWidth = manager.getDefaultDisplay().getWidth();
	int screenHeight = manager.getDefaultDisplay().getHeight();

	int statusBarHeight = context.getResources().getDimensionPixelSize(
			com.android.internal.R.dimen.navigation_bar_height);

	if ("tablet".equals(SystemProperties.get("ro.build.characteristics"))) {
		int startX;
		int startY;
		int endX;
		int endY;


		Log.e(TAG, "tablet  getWidth() " + getWidth() + " getHeight:" + getHeight()
				+ " x:" + x + " y:" + y + " screenWidth:" + screenWidth
				+ " screenHeight:" + screenHeight + " getTop:" + getTop()
				+ " getBottom" + getBottom() + " statusBarHeight:"
				+ statusBarHeight);

		//if com.mediatek.common.featureoption.FeatureOption.MTK_LCM_PHYSICAL_ROTATION has change, code need change
		//startX = x;
		//startY = y;
		//endX = x + getLeft();
		//endY = y + getHeight();
		startX = screenWidth - x - getWidth();
		startY = screenHeight - y + statusBarHeight - getHeight();
		endX = screenWidth - x;
		endY = screenHeight - y + statusBarHeight;
		
		//fix value<0 MiraVisionJni not work
		if (startX < 0) {
			startX = 0;
		}
		if (startY < 0) {
			startY = 0;
		}
		//
		Log.e(TAG, "rotation " + rotation + " startX:" + startX
				+ " startY:" + startY + " endX " + endX + " endY:" + endY);
		MiraVisionJni.nativeSetPQColorRegion(1, startX, startY, endX, endY);
	} else {
		Log.e(TAG, "phone");
		MiraVisionJni.nativeSetPQColorRegion(1, 0, x, 0 + getHeight(), x
				+ getWidth());
	}
    }
}
