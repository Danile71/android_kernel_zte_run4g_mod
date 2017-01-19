package com.mediatek.compatibility.imagetransform;

import android.util.Log;

import java.lang.NoClassDefFoundError;

public class ImagetransformSupport {

    public static final String TAG = "ImagetransformSupport";
    public static boolean isImagetransformFeatureAvaliable() {
        try {
            Class.forName("com.mediatek.imagetransform.ImageTransformFactory");
            return true;
        } catch (ClassNotFoundException e) {
            Log.e(TAG, "NoClassDefFoundError: imagetransform is not available");
            return false;
        }
    }

}
