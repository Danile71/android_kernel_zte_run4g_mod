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

public class LogBoard extends SurfaceView implements SurfaceHolder.Callback, Runnable {
	static private final String TAG = "SFT";
	private SurfaceHolder surfaceHolder = null;
	boolean isRun;
	boolean isShowFps;
	boolean isSurfaceCreated;

	public LogBoard(Context context, boolean showFps) {
		super(context);

		//Retrieve the SurfaceHolder instance associated with this SurfaceView.
		surfaceHolder = getHolder();

		//Specify this class (DrawingBoard) as the class that implements the
		//three callback methods required by SurfaceHolder.Callback.
		surfaceHolder.addCallback(this);

		isShowFps = showFps;
		isSurfaceCreated = false;
		isRun = true;
		new Thread(this).start();
	}

	//SurfaceHolder.Callback callback method.
	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		//Create and start a drawing thread whose Runnable object is defined by
		//this class (DrawingBoard).
		isSurfaceCreated = true;
	}

	@Override public void surfaceDestroyed(SurfaceHolder sh) {
		isRun = false;
		isSurfaceCreated = false;
	}

	//Runnable callback method.
	@Override
	public void run() {
		while (isRun) {
			float fps = SurfaceFlingerTest.getFps();

			if (isShowFps && isSurfaceCreated) {
				//Lock the canvas before drawing.
				Canvas canvas = surfaceHolder.lockCanvas();

				//Perform drawing operations on the canvas.
				render(canvas, fps);

				//After drawing, unlock the canvas and display it.
				surfaceHolder.unlockCanvasAndPost(canvas);
			}

			try {
				//Log.i(TAG, "[SFT] fps = " + fps);
	            Thread.sleep(1000);
	        } catch (Exception e) {
	            e.printStackTrace();
	        } finally {
	        }
		}
	}

	private void render(Canvas c, float fps) {
		//Fill the entire canvas' bitmap with 'black'.
		//c.drawColor(0x00000000);
		c.drawColor(Color.BLACK);
		//c.drawColor(0x00000000);
		//Instantiate a Paint object.
		Paint p = new Paint();
		//Set the paint color to 'white'.
		p.setColor(Color.WHITE);
		p.setTextSize(24f);
		//Draw a white circle at position (100, 100) with a radius of 50.
		Rect r = new Rect(10, 10, 200, 110);
        //c.drawRect(r, p);
        c.drawText("fps: "+fps, 20, 23, p);
	}

	public void setFormat(int format) {
		getHolder().setFormat(format);
	}

	//Neither of these two methods are used in this example, however, their definitions
	//are required because SurfaceHolder.Callback was implemented.
	@Override public void surfaceChanged(SurfaceHolder sh, int f, int w, int h) {}

}
