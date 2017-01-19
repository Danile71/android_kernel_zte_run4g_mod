package com.mediatek.ut;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.drawable.ColorDrawable;
import android.graphics.PixelFormat;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.SurfaceControl;
import android.util.Log;
import android.util.DisplayMetrics;
import android.view.Display;
import android.view.Surface;
import android.view.WindowManager;
import android.view.View;
import android.view.ViewGroup.LayoutParams;
import android.app.Presentation;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.SystemProperties;
import android.widget.AbsoluteLayout;
import android.widget.ImageView;

import android.hardware.display.DisplayManager;
import android.hardware.display.VirtualDisplay;

import android.media.ImageReader;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class VirtualBoard extends SurfaceView implements SurfaceHolder.Callback {
	static private final String TAG = "SFT";
	private static final String NAME = TAG;
    private static final int DENSITY = DisplayMetrics.DENSITY_HIGH;

	private SurfaceHolder surfaceHolder = null;
	private DisplayManager mDisplayManager;
	private VirtualDisplay mVirtualDisplay;

	static private final int BORDER_WIDTH = 20;
	static private final int DEFAULT_BUFFER_WIDTH = 64;
	static private final int DEFAULT_BUFFER_HEIGHT = 64;

	boolean isRun;
	boolean isUseSurfaceView;
	int mViewWidth;
	int mViewHeight;

	static private final int TEST_CASE = 2;

	JsonParser mParser;

	// Colors to test (RGB).  These must convert cleanly to and from BT.601 YUV.
    private static final int TEST_COLORS[] = {
        makeColor(10, 100, 200),        // YCbCr 89,186,82
        makeColor(100, 200, 10),        // YCbCr 144,60,98
        makeColor(200, 10, 100),        // YCbCr 203,10,103
        makeColor(10, 200, 100),        // YCbCr 130,113,52
        makeColor(100, 10, 200),        // YCbCr 67,199,154
        makeColor(200, 100, 10),        // YCbCr 119,74,179
    };
	private static final int UI_TIMEOUT_MS = 2000;
    private static final int UI_RENDER_PAUSE_MS = 200;
	private Handler mUiHandler;
	final TestPresentation[] mPresentation = new TestPresentation[1];

	private final Lock mImageReaderLock = new ReentrantLock(true /*fair*/);
    private ImageReader mImageReader;
    private Surface mSurface;

	/* TEST_COLORS static initialization; need ARGB for ColorDrawable */
    private static int makeColor(int red, int green, int blue) {
        return 0xff << 24 | (red & 0xff) << 16 | (green & 0xff) << 8 | (blue & 0xff);
    }

	public VirtualBoard(Context context, JsonParser parser, int viewWidth, int viewHeight, boolean useSurfaceView) {
		super(context);

		mParser = parser;
		mViewWidth = viewWidth;
		mViewHeight = viewHeight;
		isUseSurfaceView = useSurfaceView;
		mUiHandler = new Handler(Looper.getMainLooper());
		mDisplayManager = (DisplayManager)context.getSystemService(Context.DISPLAY_SERVICE);

		if (useSurfaceView) {
			//Retrieve the SurfaceHolder instance associated with this SurfaceView.
			surfaceHolder = getHolder();

			//Specify this class (DrawingBoard) as the class that implements the
			//three callback methods required by SurfaceHolder.Callback.
			surfaceHolder.addCallback(this);
		} else {
			mImageReaderLock.lock();
	        try {
	            mImageReader = ImageReader.newInstance(mViewWidth, mViewHeight, PixelFormat.RGBA_8888, 2);
	            mSurface = mImageReader.getSurface();
	        } finally {
	            mImageReaderLock.unlock();
	        }

			createVirtualDisplay(mSurface);
		}
	}

	public void tearDown() {
		if (!isUseSurfaceView) {
			mImageReaderLock.lock();
	        try {
	            mImageReader.close();
	            mImageReader = null;
	            mSurface = null;
	        } finally {
	            mImageReaderLock.unlock();
	        }

			if (TEST_CASE == 2)
				mPresentation[0].dismiss();
			if (mVirtualDisplay != null) {
	            mVirtualDisplay.release();
	        }
		}
	}

	//SurfaceHolder.Callback callback method.
	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		Log.d(TAG, "Virtual Board surfaceCreated");

		createVirtualDisplay(holder.getSurface());
	}

	@Override public void surfaceDestroyed(SurfaceHolder sh) {
		Log.d(TAG, "Virtual Board surfaceDestroyed");
		isRun = false;
		if (TEST_CASE == 2)
			mPresentation[0].dismiss();
		if (mVirtualDisplay != null) {
            mVirtualDisplay.release();
        }
	}

	//Neither of these two methods are used in this example, however, their definitions
	//are required because SurfaceHolder.Callback was implemented.
	@Override public void surfaceChanged(SurfaceHolder sh, int f, int w, int h) {}

	private void createVirtualDisplay(Surface surface) {
		try {
			mVirtualDisplay = mDisplayManager.createVirtualDisplay(NAME,
                    mViewWidth, mViewHeight, DENSITY, surface, 0);
		}  finally {
        }

		isRun = true;
		new ColorSlideShow(mVirtualDisplay.getDisplay()).start();
//		TestPresentation presentation = new TestPresentation(getContext(), mVirtualDisplay.getDisplay(), 0);
//		presentation.show();
	}
	/**
     * Creates a series of colorful Presentations on the specified Display.
     */
    private class ColorSlideShow extends Thread {
        private Display mDisplay;

        public ColorSlideShow(Display display) {
            mDisplay = display;
        }

        @Override
        public void run() {
        	Log.d(TAG, "ColorSlideShow run");

			if (TEST_CASE == 0) {
	            for (int i = 0; i < TEST_COLORS.length; i++) {
	                showPresentation(TEST_COLORS[i]);
	            }
			} else if (TEST_CASE == 1) {
				int i = 0;
				while (isRun) {
					if (i >= TEST_COLORS.length)
						i = 0;
					showPresentation(TEST_COLORS[i]);
					i++;
				}
			} else if (TEST_CASE == 2) {
				showPresentation(TEST_COLORS[0]);
			}
        }

        private void showPresentation(final int color) {
//            final TestPresentation[] presentation = new TestPresentation[1];
            try {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        // Want to create presentation on UI thread so it finds the right Looper
                        // when setting up the Dialog.
                        Log.d(TAG, "new TestPresentation");
                        mPresentation[0] = new TestPresentation(getContext(), mDisplay, color);
                        Log.d(TAG, "showing color=0x" + Integer.toHexString(color));
                        mPresentation[0].show();
                    }
                });

                // Give the presentation an opportunity to render.  We don't have a way to
                // monitor the output, so we just sleep for a bit.
                try { Thread.sleep(UI_RENDER_PAUSE_MS); }
                catch (InterruptedException ignore) {}
            } finally {
                if (mPresentation[0] != null) {
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                        	if (TEST_CASE == 0 || TEST_CASE == 1) {
                            	mPresentation[0].dismiss();
                        	}
                        }
                    });
                }
            }
        }
    }

    /**
     * Executes a runnable on the UI thread, and waits for it to complete.
     */
    private void runOnUiThread(Runnable runnable) {
        Runnable waiter = new Runnable() {
            @Override
            public void run() {
                synchronized (this) {
                    notifyAll();
                }
            }
        };
        synchronized (waiter) {
            mUiHandler.post(runnable);
            mUiHandler.post(waiter);
            try {
                waiter.wait(UI_TIMEOUT_MS);
            } catch (InterruptedException ex) {
            }
        }
    }

    /**
     * Presentation we can show on a virtual display.  The view is set to a single color value.
     */
    private class TestPresentation extends Presentation {
        private final int mColor;

        public TestPresentation(Context context, Display display, int color) {
            super(context, display);
            mColor = color;
        }

        @Override
        protected void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);

			Log.d(TAG, String.format("[SFT] TestPresentation onCreate"));
            setTitle("Encode Virtual Test");
            getWindow().setType(WindowManager.LayoutParams.TYPE_PRIVATE_PRESENTATION);

			if (TEST_CASE == 0 || TEST_CASE == 1) {
	            // Create a solid color image to use as the content of the presentation.
	            ImageView view = new ImageView(getContext());
	            view.setImageDrawable(new ColorDrawable(mColor));
	            view.setLayoutParams(new LayoutParams(
	                    LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT));
	            setContentView(view);
			} else if (TEST_CASE >= 2) {

				// abosulte layout setting
				AbsoluteLayout layout = new AbsoluteLayout(getContext());

		       	AbsoluteLayout.LayoutParams lparam = new AbsoluteLayout.LayoutParams(
		              AbsoluteLayout.LayoutParams.FILL_PARENT,
		              AbsoluteLayout.LayoutParams.FILL_PARENT,
		              0, 0);
		       	layout.setLayoutParams(lparam);
		       	setContentView(layout);

				// config frame setting
				int surfacenum = SystemProperties.getInt("debug.sftest.vsnum", 3);
				boolean defaultSize = SystemProperties.getBoolean("debug.sftest.vdefault", true);
				int border = BORDER_WIDTH;
				long fixedColor = 0x00000000;
				int bufferWidth = DEFAULT_BUFFER_WIDTH;
				int bufferHeight = DEFAULT_BUFFER_HEIGHT;
				boolean isEnalbeVirtualDisplay = false;
				String c1 = mParser.getVirtualDisplayConfig("SurfaceNumber");
				String c2 = mParser.getVirtualDisplayConfig("isDefaultSize");
				String c3 = mParser.getVirtualDisplayConfig("BorderWidth");
				String c4 = mParser.getVirtualDisplayConfig("FixedColor");
				String c5 = mParser.getVirtualDisplayConfig("BufferWidth");
				String c6 = mParser.getVirtualDisplayConfig("BufferHeight");
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

				Log.i(TAG, String.format("[SFT] VirtualDisplay Layer Num = %d, isDefaultSize=%b, BufferWidth=%d, BufferHeight=%d, BorderWidth=%d, FixedColor=%d",
					surfacenum, defaultSize, bufferWidth, bufferHeight, border, fixedColor));
//				SurfaceFlingerTest.setGlobalConfig(fixedColor);

				// create surface
				for (int i = 0; i < surfacenum; i++) {
					int format = PixelFormat.RGBA_8888;
					int color = -1;
					int api = 2; // 2 is CPU
					boolean useDefaultSize = defaultSize;
					String s1 = mParser.getVirtualDisplayFrameArray(i, "format");
					String s2 = mParser.getVirtualDisplayFrameArray(i, "color");
					String s3 = mParser.getVirtualDisplayFrameArray(i, "api");
					String s4 = mParser.getVirtualDisplayFrameArray(i, "isDefaultSize");
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
					DrawingBoard view = new DrawingBoard(getContext(), useDefaultSize, bufferWidth, bufferHeight, updateScreen, api, format, color);
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

					int w = mViewWidth - 2*boderWidth;
					int h = mViewHeight - 2*boderWidth;
					int x = boderWidth;
					int y = boderWidth;
					String p0 = mParser.getVirtualDisplayFrameArray(i, "w");
					String p1 = mParser.getVirtualDisplayFrameArray(i, "h");
					String p2 = mParser.getVirtualDisplayFrameArray(i, "x");
					String p3 = mParser.getVirtualDisplayFrameArray(i, "y");
					if (p0 != null)
						w = Integer.parseInt(p0);
					if (p1 != null)
						h = Integer.parseInt(p1);
					if (p2 != null)
						x = Integer.parseInt(p2);
					if (p3 != null)
						y = Integer.parseInt(p3);

					// LayoutParams(int width, int height, int x, int y)
					Log.i(TAG, String.format("[SFT] Virtual layout[%d], w=%d, h=%d, x=%d, y=%d", i, w, h, x, y));
					AbsoluteLayout.LayoutParams lp = new AbsoluteLayout.LayoutParams(
						w, h, x, y);
					layout.addView(view, lp);
				}

/*
				DrawingBoard view = new DrawingBoard(getContext(), true, 100, 100, true, 2, 1, -1);
				view.setZOrderMediaOverlay(false);
				view.setFormat(PixelFormat.OPAQUE);
				setContentView(view);
*/
			}
        }
    }
}
