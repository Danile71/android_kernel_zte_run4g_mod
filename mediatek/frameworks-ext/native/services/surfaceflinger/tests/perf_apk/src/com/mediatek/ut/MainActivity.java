package com.mediatek.ut;

import java.io.File;

import android.app.Activity;
import android.app.ActionBar;
import android.os.Bundle;
import android.os.SystemProperties;
import android.view.Window;
import android.view.WindowManager;
import android.view.Display;
import android.widget.AbsoluteLayout;
import android.util.Log;
import android.util.DisplayMetrics;
import android.graphics.Point;
import android.graphics.PixelFormat;

import android.view.animation.AlphaAnimation;
import android.view.animation.Animation;
import android.view.animation.AnimationSet;
import android.view.animation.LayoutAnimationController;
import android.view.animation.TranslateAnimation;

public class MainActivity extends Activity {
	static private final String TAG = "SFT";
	static private final int BORDER_WIDTH = 100;
	static private final int DEFAULT_BUFFER_WIDTH = 1000;
	static private final int DEFAULT_BUFFER_HEIGHT = 2000;
	private boolean isRun;

	VirtualBoard mVirtualBoard = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

		String path = getExternalFilesDir(null).getAbsolutePath();
		File file = new File(path + "/json.conf");
		JsonParser parser = new JsonParser(this, file);

        //Set the content view to a new instance of DrawingBoard.
        Window win = getWindow();
        WindowManager.LayoutParams winParams = win.getAttributes();
        final int bits = WindowManager.LayoutParams.FLAG_FULLSCREEN;
		winParams.flags |=  bits;
        win.setAttributes(winParams);
		win.setFormat(PixelFormat.TRANSLUCENT);
		ActionBar bar = getActionBar();
		if (bar != null)
			bar.hide();

		//DisplayMetrics dm = getResources().getDisplayMetrics();
        //Log.i(TAG, String.format("DisplayMetrics = (%d x %d)", dm.widthPixels, dm.heightPixels));
		final Display display = getWindowManager().getDefaultDisplay();
        final Point size = new Point();
		display.getRealSize(size);
		final int screenWidth = size.x;
        final int screenHeight = size.y;
		Log.i(TAG, String.format("[SFT] Screen Resolution = (%d x %d)", screenWidth, screenHeight));

		// abosulte layout setting
		AbsoluteLayout layout = new AbsoluteLayout(this);

       	AbsoluteLayout.LayoutParams lparam = new AbsoluteLayout.LayoutParams(
              AbsoluteLayout.LayoutParams.FILL_PARENT,
              AbsoluteLayout.LayoutParams.FILL_PARENT,
              0, 0);
       	layout.setLayoutParams(lparam);
       	this.setContentView(layout);

		// config frame setting
		int surfacenum = SystemProperties.getInt("debug.sftest.snum", 3);
		boolean defaultSize = SystemProperties.getBoolean("debug.sftest.default", true);
		boolean isShowFps = SystemProperties.getBoolean("debug.sftest.showfps", false);
		int border = BORDER_WIDTH;
		long fixedColor = 0x00000000;
		int bufferWidth = DEFAULT_BUFFER_WIDTH;
		int bufferHeight = DEFAULT_BUFFER_HEIGHT;
		boolean isEnalbeVirtualDisplay = false;
		String c1 = parser.getGlobalConfig("SurfaceNumber");
		String c2 = parser.getGlobalConfig("isDefaultSize");
		String c3 = parser.getGlobalConfig("BorderWidth");
		String c4 = parser.getGlobalConfig("FixedColor");
		String c5 = parser.getGlobalConfig("BufferWidth");
		String c6 = parser.getGlobalConfig("BufferHeight");
		String c7 = parser.getGlobalConfig("ShowFps");
		String c8 = parser.getGlobalConfig("VirtualDisplay");
		if (c1 != null)
			surfacenum = Integer.parseInt(c1);
		if (c2 != null)
			defaultSize = Boolean.parseBoolean(c2);
		if (c3 != null)
			border = Integer.parseInt(c3);
		if (c4 != null)
			fixedColor = Long.parseLong(c4);
		if (c5 != null)
			bufferWidth = Integer.parseInt(c5);
		if (c6 != null)
			bufferHeight = Integer.parseInt(c6);
		if (c7 != null)
			isShowFps = Boolean.parseBoolean(c7);
		if (c8 != null)
			isEnalbeVirtualDisplay = Boolean.parseBoolean(c8);

		Log.i(TAG, String.format("[SFT] Surface Num = %d, isDefaultSize=%b, BufferWidth=%d, BufferHeight=%d, BorderWidth=%d, FixedColor=%d",
			surfacenum, defaultSize, bufferWidth, bufferHeight, border, fixedColor));
		SurfaceFlingerTest.setGlobalConfig(fixedColor);

		// create surface
		for (int i = 0; i < surfacenum; i++) {
			int format = PixelFormat.RGBA_8888;
			int color = 0;
			int api = 2; // 2 is CPU
			boolean useDefaultSize = defaultSize;
			String s1 = parser.getFrameArray(i, "format");
			String s2 = parser.getFrameArray(i, "color");
			String s3 = parser.getFrameArray(i, "api");
			String s4 = parser.getFrameArray(i, "isDefaultSize");
			if (s1 != null)
				format = Integer.parseInt(s1);
			if (s2 != null)
				color = Integer.parseInt(s2);
			if (s3 != null)
				api = Integer.parseInt(s3);
			if (s4 != null)
				useDefaultSize = Boolean.parseBoolean(s4);

			int boderWidth = border;
			// view 1
			boolean updateScreen = true;
			if (i != 0) updateScreen = false;
			DrawingBoard view = new DrawingBoard(this, useDefaultSize, bufferWidth, bufferHeight, updateScreen, api, format, color);
			if (i == 0) {
				//view.setZOrderOnTop(true);
				view.setZOrderMediaOverlay(false);
				view.setFormat(PixelFormat.OPAQUE);
				boderWidth = 0;
			}
			else {
				//view.setWindowType(WindowManager.LayoutParams.TYPE_APPLICATION_SUB_PANEL);
				view.setZOrderMediaOverlay(true);
				view.setFormat(PixelFormat.RGBA_8888);
				//view.setZOrderOnTop(true);
			}

			int w = screenWidth - 2*boderWidth;
			int h = screenHeight - 2*boderWidth;
			int x = boderWidth;
			int y = boderWidth;
			String p0 = parser.getFrameArray(i, "w");
			String p1 = parser.getFrameArray(i, "h");
			String p2 = parser.getFrameArray(i, "x");
			String p3 = parser.getFrameArray(i, "y");
			if (p0 != null)
				w = Integer.parseInt(p0);
			if (p1 != null)
				h = Integer.parseInt(p1);
			if (p2 != null)
				x = Integer.parseInt(p2);
			if (p3 != null)
				y = Integer.parseInt(p3);

			// LayoutParams(int width, int height, int x, int y)
			AbsoluteLayout.LayoutParams lp = new AbsoluteLayout.LayoutParams(
				//screenWidth - 2*BORDER_WIDTH*i, screenHeight - 2*BORDER_WIDTH*i, BORDER_WIDTH*i, BORDER_WIDTH*i);
				w, h, x, y);
			layout.addView(view, lp);
		}


		// show log information view
		{
			LogBoard board = new LogBoard(this, isShowFps);
			board.setZOrderOnTop(true);
			board.setFormat(PixelFormat.RGBA_8888);
			if (isShowFps) {
				AbsoluteLayout.LayoutParams lp = new AbsoluteLayout.LayoutParams(
						200, 30, 10, 10);
				layout.addView(board, lp);
			} else {
				AbsoluteLayout.LayoutParams lp = new AbsoluteLayout.LayoutParams(
						1, 1, -1, -1);
				layout.addView(board, lp);
			}
		}

		if (isEnalbeVirtualDisplay) {
			boolean isShowVirtualDisplay = true;
			int viewWidth = 720;
			int viewHeight = 1280;
			int posx = 10;
			int posy = 200;
			String v1 = parser.getVirtualDisplayConfig("isShow");
			String v2 = parser.getVirtualDisplayConfig("width");
			String v3 = parser.getVirtualDisplayConfig("height");
			String v4 = parser.getVirtualDisplayConfig("posx");
			String v5 = parser.getVirtualDisplayConfig("posy");
			if (v1 != null)
				isShowVirtualDisplay = Boolean.parseBoolean(v1);
			if (v2 != null)
				viewWidth = Integer.parseInt(v2);
			if (v3 != null)
				viewHeight = Integer.parseInt(v3);
			if (v4 != null)
				posx = Integer.parseInt(v4);
			if (v5 != null)
				posy = Integer.parseInt(v5);

			mVirtualBoard = new VirtualBoard(this, parser, viewWidth, viewHeight, isShowVirtualDisplay);
			if (isShowVirtualDisplay) {
				mVirtualBoard.setZOrderOnTop(true);
				AbsoluteLayout.LayoutParams lp = new AbsoluteLayout.LayoutParams(
						viewWidth, viewHeight, posx, posy);
				layout.addView(mVirtualBoard, lp);
			}
		}


/*
		// view 1
		DrawingBoard view1 = new DrawingBoard(this);
		view1.setTranslucent();
		view1.setZOrderOnTop(true);
//		view1.getBackground().setAlpha(128);
		AbsoluteLayout.LayoutParams lp1 = new AbsoluteLayout.LayoutParams(
			1000, 1600, 0, 0);
		layout.addView(view1, lp1);

		// view 2
		DrawingBoard view2 = new DrawingBoard(this);
		view2.setTranslucent();
		view2.setZOrderOnTop(true);
//		view2.getBackground().setAlpha(128);
		AbsoluteLayout.LayoutParams lp2 = new AbsoluteLayout.LayoutParams(
			800, 1200, 50, 50);

		layout.addView(view2, lp2);

		// view 3
		DrawingBoard view3 = new DrawingBoard(this);
		view3.setTranslucent();
		view3.setZOrderOnTop(true);
//		view3.getBackground().setAlpha(128);
		AbsoluteLayout.LayoutParams lp3 = new AbsoluteLayout.LayoutParams(
			600, 1000, 100, 100);
		layout.addView(view3, lp3);
//		setContentView();
*/
    }

	@Override
    public void onDestroy() {
		Log.i(TAG, "[SFT] onDestroy");
		isRun = false;
//		SurfaceFlingerTest.destroy();
        super.onDestroy();

		if (mVirtualBoard != null) {
			mVirtualBoard.tearDown();
			mVirtualBoard = null;
		}
    }
}
