package com.mediatek.effect;
import android.app.NativeActivity;
import android.media.Image;
import android.util.Log;

import java.nio.ByteOrder;
import java.nio.ByteBuffer;
import java.nio.IntBuffer;

import java.lang.ref.WeakReference;

/**
 * Effects are high-performance transformations that can be applied to image frames. Typical 
 * frames can be images loaded from disk, frames from the camera or other video streams.
 *
 */
public abstract class Effect {
    private static final String TAG = "Effect";
    
    /**
     * @hide
     */
    protected Effect() {
    }
    
    /**
     * Some effects may issue callbacks to inform the host of changes to the effect state.
     * This is the listener interface for receiving those callbacks.
     */
    public interface EffectUpdateListener {
        /**
         * Called when the effect state is updated. 
         * @param effect
         *     The effect that has been updated.
         * @param info
         *     A value that gives more information on the update. See the effect's document
         *     for more details on what this object is.
         */
        public void onEffectUpdateds(Effect effect, Object info);
    }
    
    
    /**
     * Applies an effect to the target input image.
     */
    public abstract void apply(Image srcImage, Image targetImage);

    /**
     * Gets the effect name.
     */
    public abstract String getName();

    /**
     * Sets up filter parameter. Consult the effect document for a list
     * of supported parameter keys for each effect.
     * 
     */
    public abstract void setParameter(String parameterKey, Object value);

    /**
     * Sets up effect listener.
     * @param listener the listener receive effect
     */
    public void setUpdateListener(EffectUpdateListener listener) {
        native_setUpdateListener(listener);
    }
    
    
    /**
      * Releases an effect.
      *
      * <p>Releases the effect and any resources associated with it. You may call this if you need to
      * make sure the acquired resources are no longer held by the effect. Releasing an effect makes it
      * invalid for reuse.</p>
      *
      */
    public abstract void release();

    /**
     * @hide
     */
    protected native void native_setUpdateListener(EffectUpdateListener listener);

}
