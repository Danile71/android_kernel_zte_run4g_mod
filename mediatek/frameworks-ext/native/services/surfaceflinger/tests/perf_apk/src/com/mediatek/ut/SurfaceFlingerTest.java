package com.mediatek.ut;

public class SurfaceFlingerTest {

     static {
         System.loadLibrary("surfaceut");
     }

	 public static native int connect(Object native_window,
	 	boolean useDefaultSize, int w, int h, boolean updateScreen, int api, int format, int colorIndex);
	 public static native void disconnect(int id);
	 public static native int setGlobalConfig(long color);
	 public static native float getFps();
	 public static native void destroy();
}
