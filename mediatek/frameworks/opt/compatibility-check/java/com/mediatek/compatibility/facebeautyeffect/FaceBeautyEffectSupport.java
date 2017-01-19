package com.mediatek.compatibility.facebeautyeffect;

import android.util.Log;

import java.lang.NoClassDefFoundError;

public class FaceBeautyEffectSupport {

    public static final String TAG = "EffectSupport";
    public static boolean isFaceBeautyEffectFeatureAvaliable() {
        try {
            Class.forName("com.mediatek.effect.FaceBeautyEffect");
            return true;
        } catch (ClassNotFoundException e) {
            Log.e(TAG, "NoClassDefFoundError: Camera FaceBeautyEffect is not available");
            return false;
        }
    }
}
