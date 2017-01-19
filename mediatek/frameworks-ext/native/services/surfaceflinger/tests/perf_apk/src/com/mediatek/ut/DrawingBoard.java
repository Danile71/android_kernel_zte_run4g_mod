package com.mediatek.ut;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.util.Log;
import android.view.View;

public class DrawingBoard extends SurfaceView implements SurfaceHolder.Callback,
														 View.OnSystemUiVisibilityChangeListener {
	static private final String TAG = "SFT";
	private SurfaceHolder surfaceHolder = null;
	private int mId;
	private boolean isUpdateScreen = false;
	private boolean isDefaultSize = true;

	private int mBufferWidth;
	private int mBufferHeight;
	private int mApi;
	private int mFormat;
	private int mColorIndex;

	private int systemUiVisibility = View.SYSTEM_UI_FLAG_FULLSCREEN |
				      				 View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
				      				 View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY;

	public DrawingBoard(Context context,
		boolean useDefaultSize, int bufferWidth, int bufferHeight, boolean updateScreen, int api, int format, int colorIndex) {
		super(context);

		//Retrieve the SurfaceHolder instance associated with this SurfaceView.
		surfaceHolder = getHolder();

		//Specify this class (DrawingBoard) as the class that implements the
		//three callback methods required by SurfaceHolder.Callback.
		surfaceHolder.addCallback(this);

		isDefaultSize = useDefaultSize;
		isUpdateScreen = updateScreen;

		mBufferWidth = bufferWidth;
		mBufferHeight = bufferHeight;
		mApi = api;
		mFormat = format;
		mColorIndex = colorIndex;

		setOnSystemUiVisibilityChangeListener(this);
	}

	//SurfaceHolder.Callback callback method.
	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		synchronized (this) {
			connect(holder);
        }

		setSystemUiVisibility(systemUiVisibility);
	}

	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
		setSystemUiVisibility(systemUiVisibility);
    }

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		synchronized (this) {
			disconnect(holder);
		}
	}

	@Override
    public void onSystemUiVisibilityChange(int visibility) {
        Log.i(TAG, "onSystemUiVisibilityChange, visibility=" + visibility);
//		systemUIVisibility = visibility;
    }

	/** @hide */
	/*
	@Override
    protected boolean setFrame(int left, int top, int right, int bottom) {
        boolean result = super.setFrame(left, top, right, bottom);
        return result;
    }
	*/

	@Override
	protected boolean onSetAlpha(int alpha) {
		Log.i(TAG, "onSetAlpha, alpha=" + alpha);
        return true;
    }

	public void setFormat(int format) {
		getHolder().setFormat(format);
	}

	private void connect(SurfaceHolder holder) {
		Surface s = null;
        if (holder != null) {
            s = holder.getSurface();
        }
		mId = SurfaceFlingerTest.connect(s, isDefaultSize, mBufferWidth, mBufferHeight, isUpdateScreen, mApi, mFormat, mColorIndex);

		Log.i(TAG, "connect, id=" + mId);
	}

	private void disconnect(SurfaceHolder holder) {
		Surface s = null;
        if (holder != null) {
            s = holder.getSurface();
        }
		SurfaceFlingerTest.disconnect(mId);
	}
}
