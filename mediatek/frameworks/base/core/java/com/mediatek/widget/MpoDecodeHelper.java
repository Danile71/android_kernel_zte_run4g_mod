package com.mediatek.widget;

import java.io.File;

import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.MediaStore;
import android.provider.MediaStore.Images.ImageColumns;
import android.util.FloatMath;
import android.util.Log;

import com.mediatek.widget.DataAdapterEx;
import com.mediatek.common.mpodecoder.IMpoDecoder;
import com.mediatek.common.MediatekClassFactory;


import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.BitmapFactory.Options;
public class MpoDecodeHelper {
    private static final String TAG ="MpoDecodeHelper";
    
    private static IMpoDecoder createMpoDecoder(String filePath) {
        if (filePath == null) return null;
        IMpoDecoder mpoDecoder = null;
        try {
            mpoDecoder = MediatekClassFactory.createInstance(IMpoDecoder.class, filePath);
            if (mpoDecoder != null && mpoDecoder.isMpoDecoderValid()) {
                return mpoDecoder;
            }
            return null;
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }
    
    private static Bitmap retrieveMpoMarkFrames (IMpoDecoder mpoDecoder,int frameCount, int imageWidth,int  imgeHeigh, int type,
            DataAdapterEx dataBundle) {

        Options options = new Options();
        options.inSampleSize = calculateSampleSizeByType(imageWidth, imgeHeigh,
                type, QuickContactBadgeEx.Config.getTargetSize(type));
        Bitmap mpoMarkFrames = null;
        if (dataBundle != null && dataBundle.isInterrupte()) return null;
        if (dataBundle.mMarkFrameIndex >= 0 
                && dataBundle.mMarkFrameIndex < dataBundle.getmFrameCount()) {
            Bitmap mBitmap = decodeFrameSafe(mpoDecoder, dataBundle.mMarkFrameIndex, options);
            if (mBitmap == null) {
                return null;
            }
            mpoMarkFrames = postScaleDown(
                    mBitmap, type, QuickContactBadgeEx.Config.getTargetSize(type));
        }
        return mpoMarkFrames;
    }
    
    private static Bitmap retrieveMpoMarkFrames (IMpoDecoder mpoDecoder,int frameCount,
            int imageWidth,int  imgeHeigh, int type, int frameIndex) {

        Options options = new Options();
        options.inSampleSize = calculateSampleSizeByType(imageWidth, imgeHeigh,
                type, QuickContactBadgeEx.Config.getTargetSize(type));
        Bitmap mpoMarkFrames = null;
        Bitmap mBitmap = decodeFrameSafe(mpoDecoder, frameIndex, options);
        if (mBitmap == null) {
            return null;
        }
        mpoMarkFrames = postScaleDown(
                mBitmap, type, QuickContactBadgeEx.Config.getTargetSize(type));
        return mpoMarkFrames;
    }
    
    private static Bitmap[] retrieveMpoFrames (IMpoDecoder mpoDecoder,int frameCount, int imageWidth,int  imgeHeigh, int type,
            DataAdapterEx dataBundle) {

        Options options = new Options();
        options.inSampleSize = calculateSampleSizeByType(imageWidth, imgeHeigh,
                type, QuickContactBadgeEx.Config.getTargetSize(type));
        Bitmap[] mpoFrames = new Bitmap[frameCount];
        for (int i = 0; i < frameCount; i++) {
            if (dataBundle.isInterrupte()) return null;
            Bitmap mBitmap = decodeFrameSafe(mpoDecoder, i, options);
            if (mBitmap == null) {
                return null;
            }
            mpoFrames[i] = postScaleDown(
                    mBitmap, type, QuickContactBadgeEx.Config.getTargetSize(type));
        }
        return mpoFrames;
    }

    public static boolean load(DataAdapterEx dataBundle) {
        if (dataBundle == null ) {
            return false;
        }
        if (dataBundle.isInterrupte() || dataBundle.getState() == DataAdapterEx.STATE_LOADED_ALL_FRAME ) {
            return true;
        }
        String filePath = dataBundle.getFilePath();
        int type = dataBundle.getType();
        Log.i(TAG, "request filePath = "+getfilepath(dataBundle.getContext(), filePath)+"");
        IMpoDecoder mpoDecoder = createMpoDecoder(getfilepath(dataBundle.getContext(), filePath));
        try {
            if (mpoDecoder != null &&  dataBundle.getState() == DataAdapterEx.STATE_QUEUED) {
                dataBundle.setState(DataAdapterEx.STATE_LOADING_MARK_FRAME);
                initData(mpoDecoder, dataBundle);
                Bitmap bitmap = retrieveMpoMarkFrames(mpoDecoder, dataBundle.getmFrameCount(),
                        dataBundle.getImageWidth(), dataBundle.getImageHeight(), type, dataBundle);
                if (bitmap != null) {
                    dataBundle.initMarkFrame(bitmap);
                    QuickContactBadgeEx.sMavRenderThread.setRenderRequester(QuickContactBadgeEx.isOnIdleState());
                    dataBundle.setState(DataAdapterEx.STATE_LOADED_MARK_FRAME);
                } else {
                    dataBundle.setState(DataAdapterEx.STATE_ERROR_MARK_FRAME);
                }
                
            } else if (mpoDecoder != null && dataBundle.getState() == DataAdapterEx.STATE_LOADED_MARK_FRAME) {
                if (dataBundle.getmFrameCount() <= 0) {
                    initData(mpoDecoder, dataBundle);
                }
                dataBundle.setState(DataAdapterEx.STATE_LOADING_ALL_FRAME);
                Bitmap[] mpoFrames = retrieveMpoFrames(mpoDecoder, dataBundle.getmFrameCount(), 
                        dataBundle.getImageWidth(), dataBundle.getImageHeight(), type, dataBundle);
                if (mpoFrames != null) {
                    dataBundle.setFrames(mpoFrames);
                    dataBundle.setState(DataAdapterEx.STATE_LOADED_ALL_FRAME);
                } else {
                    dataBundle.setState(DataAdapterEx.STATE_ERROR_ALL_FRAME);
                }
            }
        } finally {
            // we must close mpo wrapper manually.
            if (null != mpoDecoder) {
                mpoDecoder.close();
            }
        }
        return true;
    }
    
    public static Bitmap loadOneBitmap (Context context, String uri, int type) {
        Log.i(TAG, "request filePath = "+getfilepath(context, uri)+"");
        Bitmap bitmap = null;
        IMpoDecoder mpoDecoder = createMpoDecoder(getfilepath(context, uri));
        try {
            if (mpoDecoder != null ) {
                int frameCount = mpoDecoder.frameCount();
                int imageWidth = mpoDecoder.width();
                int imageHeight = mpoDecoder.height();
                bitmap = retrieveMpoMarkFrames(mpoDecoder, frameCount,
                        imageWidth, imageHeight, type, frameCount/2);  
            }
        } finally {
            // we must close mpo wrapper manually.
            if (null != mpoDecoder) {
                mpoDecoder.close();
            }
        }
        return bitmap;
    }
    
    
    public static void initData(IMpoDecoder mpoDecoder, DataAdapterEx dataBundle) {
        if (mpoDecoder != null && dataBundle != null) {
            int frameCount = mpoDecoder.frameCount();
            int imageWidth = mpoDecoder.width();
            int imageHeight = mpoDecoder.height();
            dataBundle.setDataBundle(frameCount, imageWidth, imageHeight);
        }

    }
    public static Bitmap decodeFrameSafe(IMpoDecoder mpoDecoder,
            int frameIndex, Options options) {
        if (null == mpoDecoder || frameIndex < 0 || null == options) {
            Log.w(TAG, "decodeFrameSafe:invalid paramters");
            return null;
        }
        //As there is a chance no enough dvm memory for decoded Bitmap,
        //Skia will return a null Bitmap. In this case, we have to
        //downscale the decoded Bitmap by increase the options.inSampleSize
        Bitmap bitmap = null;
        final int maxTryNum = 8;
        for (int i=0; i < maxTryNum /*&& (null == jc || !jc.isCancelled())*/; i++) {
            //we increase inSampleSize to expect a smaller Bitamp
            Log.v(TAG,"decodeFrameSafe:try for sample size " +
                      options.inSampleSize);
            try {
                bitmap = decodeFrame(/*jc, */mpoDecoder, frameIndex, options);
            } catch (OutOfMemoryError e) {
                Log.w(TAG,"decodeFrameSafe:out of memory when decoding:"+e);
            }
            if (null != bitmap) break;
            options.inSampleSize *= 2;
        }
        return bitmap;
    }
    
    public static Bitmap decodeFrame(/*JobContext jc,*/ IMpoDecoder mpoDecoder,
            int frameIndex, Options options) {
        if (null == mpoDecoder || frameIndex < 0 || null == options) {
            Log.w(TAG, "decodeFrame:invalid paramters");
            return null;
        }
        Bitmap bitmap = mpoDecoder.frameBitmap(frameIndex, options);
/*        if (null != jc && jc.isCancelled()) {
            bitmap.recycle();
            bitmap = null;
        }*/
        return bitmap;
    }
    
    private static String getfilepath(Context context, String uri) {
        if (uri == null) {
            Log.e(TAG, "srcUri is null.");
            return null;
        }
        Uri srcUri = Uri.parse(uri);
        String scheme = srcUri.getScheme();

        final String[] filePath = new String[1];
        // sourceUri can be a file path or a content Uri, it need to be handled
        // differently.
        if (scheme != null && scheme.equals(ContentResolver.SCHEME_CONTENT)) {
            if (srcUri.getAuthority().equals(MediaStore.AUTHORITY)) {
                querySource(context, srcUri, new String[] {
                        ImageColumns.DATA
                },
                        new ContentResolverQueryCallback() {

                            @Override
                            public void onCursorResult(Cursor cursor) {
                                filePath[0] = cursor.getString(0);
                            }
                        });
            }
        } else {
            filePath[0] = uri;
        }
        return filePath[0];
    }
    
    
    public static interface ContentResolverQueryCallback {
        void onCursorResult(Cursor cursor);
    }
    
    public static int calculateSampleSizeByType(int width, int height, 
                                                int type, int targetSize) {
        int sampleSize = 1;
        if (QuickContactBadgeEx.Config.needCenterCropProcess(type)) {
            // We center-crop the original image as it's micro thumbnail.
            // In this case, we want to make sure the shorter side >= "targetSize".
            float scale = (float) targetSize / Math.min(width, height);
            sampleSize = computeSampleSizeLarger(scale);

            // For an extremely wide image, e.g. 300x30000, we may got OOM
            // when decoding it for TYPE_MICROTHUMBNAIL. So we add a max
            // number of pixels limit here.
            if (QuickContactBadgeEx.Config.isOutOfLimitation((width / sampleSize) * (height / sampleSize), type)) {
                sampleSize = computeSampleSize(QuickContactBadgeEx.Config.caculateScale(type, width, height));
            }
        } else {
            // For screen nail, we only want to keep the longer side >= targetSize.
            float scale = (float) targetSize / Math.max(width, height);
            sampleSize = computeSampleSizeLarger(scale);
        }
        return sampleSize;
    }
    
    // Find the min x that 1 / x >= scale
    public static int computeSampleSizeLarger(float scale) {
        int initialSize = (int) FloatMath.floor(1f / scale);
        if (initialSize <= 1) return 1;

        return initialSize <= 8
                ? prevPowerOf2(initialSize)
                : initialSize / 8 * 8;
    }

    // Find the max x that 1 / x <= scale.
    public static int computeSampleSize(float scale) {
        assertTrue(scale > 0);
        int initialSize = Math.max(1, (int) FloatMath.ceil(1 / scale));
        return initialSize <= 8
                ? nextPowerOf2(initialSize)
                : (initialSize + 7) / 8 * 8;
    }

    // Returns the previous power of two.
    // Returns the input if it is already power of 2.
    // Throws IllegalArgumentException if the input is <= 0
    public static int prevPowerOf2(int n) {
        if (n <= 0) throw new IllegalArgumentException();
        return Integer.highestOneBit(n);
    }
    
    // Returns the next power of two.
    // Returns the input if it is already power of 2.
    // Throws IllegalArgumentException if the input is <= 0 or
    // the answer overflows.
    public static int nextPowerOf2(int n) {
        if (n <= 0 || n > (1 << 30)) throw new IllegalArgumentException("n is invalid: " + n);
        n -= 1;
        n |= n >> 16;
        n |= n >> 8;
        n |= n >> 4;
        n |= n >> 2;
        n |= n >> 1;
        return n + 1;
    }
    
    // Throws AssertionError if the input is false.
    public static void assertTrue(boolean cond) {
        if (!cond) {
            throw new AssertionError();
        }
    }
    
    public static Bitmap postScaleDown(Bitmap bitmap, int type, int targetSize) {
        if (null == bitmap) return null;
        //scale down according to type
        if (QuickContactBadgeEx.Config.needCenterCropProcess(type)) {
            bitmap = resizeAndCropCenter(bitmap, targetSize, true);
        } else {
            bitmap = resizeDownBySideLength(bitmap, targetSize, true);
        }
/*        bitmap = DecodeUtils.ensureGLCompatibleBitmap(bitmap);*/
        return bitmap;
    }
    
    public static Bitmap resizeAndCropCenter(Bitmap bitmap, int size, boolean recycle) {
        int w = bitmap.getWidth();
        int h = bitmap.getHeight();
        if (w == size && h == size) return bitmap;

        // scale the image so that the shorter side equals to the target;
        // the longer side will be center-cropped.
        float scale = (float) size / Math.min(w,  h);
        Bitmap target = Bitmap.createBitmap(size, size, getConfig(bitmap));
        int width = Math.round(scale * bitmap.getWidth());
        int height = Math.round(scale * bitmap.getHeight());
        if (target == null) return null;
        Canvas canvas = new Canvas(target);
        canvas.translate((size - width) / 2f, (size - height) / 2f);
        canvas.scale(scale, scale);
        Paint paint = new Paint(Paint.FILTER_BITMAP_FLAG | Paint.DITHER_FLAG);
        canvas.drawBitmap(bitmap, 0, 0, paint);
        if (recycle) bitmap.recycle();
        return target;
    }
    
    public static Bitmap resizeDownBySideLength(
            Bitmap bitmap, int maxLength, boolean recycle) {
        int srcWidth = bitmap.getWidth();
        int srcHeight = bitmap.getHeight();
        float scale = Math.min(
                (float) maxLength / srcWidth, (float) maxLength / srcHeight);
        if (scale >= 1.0f) return bitmap;
        return resizeBitmapByScale(bitmap, scale, recycle);
    }
    
    public static Bitmap resizeBitmapByScale(
            Bitmap bitmap, float scale, boolean recycle) {
        int width = Math.round(bitmap.getWidth() * scale);
        int height = Math.round(bitmap.getHeight() * scale);
        /// M: fix certain wbmp no thumbnail issue.@{
        if (width < 1 || height < 1) {
            Log.i(TAG, "scaled width or height < 1, no need to resize");
            return bitmap;
        }
        /// @}
        if (width == bitmap.getWidth()
                && height == bitmap.getHeight()) return bitmap;
        Bitmap target = Bitmap.createBitmap(width, height, getConfig(bitmap));
        if (target == null) return bitmap;
        Canvas canvas = new Canvas(target);
        canvas.scale(scale, scale);
        Paint paint = new Paint(Paint.FILTER_BITMAP_FLAG | Paint.DITHER_FLAG);
        canvas.drawBitmap(bitmap, 0, 0, paint);
        if (recycle) bitmap.recycle();
        return target;
    }
    
    private static Bitmap.Config getConfig(Bitmap bitmap) {
        Bitmap.Config config = bitmap.getConfig();
        if (config == null) {
            config = Bitmap.Config.ARGB_8888;
        }
        return config;
    }
    
    public static void querySource(Context context, Uri sourceUri, String[] projection,
            ContentResolverQueryCallback callback) {
        ContentResolver contentResolver = context.getContentResolver();
        querySourceFromContentResolver(contentResolver, sourceUri, projection, callback);
    }

    private static void querySourceFromContentResolver(
            ContentResolver contentResolver, Uri sourceUri, String[] projection,
            ContentResolverQueryCallback callback) {
        Cursor cursor = null;
        try {
            cursor = contentResolver.query(sourceUri, projection, null, null,
                    null);
            if ((cursor != null) && cursor.moveToNext()) {
                callback.onCursorResult(cursor);
            }
        } catch (Exception e) {
            // Ignore error for lacking the data column from the source.
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
    }
}
