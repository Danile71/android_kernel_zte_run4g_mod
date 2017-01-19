package com.mediatek.gallery3d.plugin;

import java.io.FileDescriptor;
import android.content.ContentResolver;
import android.content.Context;
import android.graphics.BitmapFactory;
import android.graphics.BitmapFactory.Options;
import android.net.Uri;
import android.util.Log;

import com.mediatek.dcfdecoder.DcfDecoder;

import com.mediatek.gallery3d.ext.IImageOptionsExt;
import com.mediatek.gallery3d.ext.DefaultImageOptionsExt;

import com.android.gallery3d.ui.PhotoView.Size;
import com.mediatek.gallery3d.ui.MtkBitmapScreenNail;
import com.android.gallery3d.data.MediaObject;

public class Op01ImageOptionsExt extends DefaultImageOptionsExt {
    private static final String TAG = "Gallery2/Op01ImageOptionsExt";
    public static final int THUMBNAIL_TARGET_SIZE = 640;

    @Override
    public int getSubType(int subType, int w, int h) {
        // we think of specified size image as special image
        // such as 1024 x 1, 1600 x 1
        boolean isSpecialImage = false;
        int scale = Math.max(w, h) / THUMBNAIL_TARGET_SIZE;
        if(scale != 0 && (w/scale == 0 || h/scale==0)) {
            isSpecialImage =  true;
            Log.d(TAG, "is special image, w: " + w + ", h: " + h);
        }
        
        // display image at its origin size, MAV and special image 
        // is not regarded as normal image
        if (0 == (subType & MediaObject.SUBTYPE_MPO_MAV) && !isSpecialImage) {
            subType |= SUBTYPE_ORIGIN_SIZE;
        }
        Log.d(TAG, "getSubType subType = " + subType);
        return subType;
    }

    @Override
    public float getImageDisplayScale(int subType) {
        Log.d(TAG, "[OP01]getImageDisplayScale subType = " + subType);
        if ((subType & SUBTYPE_ORIGIN_SIZE) != 0) {
            return 1.0f;
        }
        return -1.0f;
    }

    @Override
    public boolean needMtkScreenNail(int subType) {
        Log.d(TAG, "[OP01]needMtkScreenNail subType = " + subType);
        if (super.needMtkScreenNail(subType)) {
            return true;
        }
        
        if (0 != (subType & SUBTYPE_ORIGIN_SIZE)) {
            return true;
        } else {
            return false;
        }
    }

    @Override
    public void updateSizeForSubtype(Size size, MtkBitmapScreenNail screenNail) {
        if (null == screenNail) return;
        
        if ((screenNail.getSubType() & SUBTYPE_ORIGIN_SIZE) != 0) {
            size.width = screenNail.getOriginWidth();
            size.height = screenNail.getOriginHeight();
        }
        Log.d(TAG, "[OP01]updateSizeForSubtype width = " + size.width + ", height = " + size.height);
    }
    
    
    @Override
    public Size getSizeForSubtype(MtkBitmapScreenNail screenNail) {
        if (null == screenNail) return null;
        
        if ((screenNail.getSubType() & SUBTYPE_ORIGIN_SIZE) != 0 &&
            screenNail.getOriginWidth() > 0 &&
            screenNail.getOriginHeight() > 0) {
            Size size = new Size();
            size.width = screenNail.getOriginWidth();
            size.height = screenNail.getOriginHeight();
            Log.d(TAG, "[OP01]getSizeForSubtype width = " + size.width + ", height = " + size.height);
            return size;
        }

        return null;
    }
    
    @Override
    public Size updateImageSize(boolean isDrmSupported, Uri uri, ContentResolver cr, FileDescriptor fd) {
        Options options = new Options();
        options.inJustDecodeBounds = true;

        if (isDrmSupported && null != uri && null != uri.getPath() && 
            ContentResolver.SCHEME_FILE.equals(uri.getScheme()) &&
            uri.getPath().toLowerCase().endsWith(".dcf")) {
            //when drm file, decode it.
            //Note: currently, only DRM files on sdcard can be decoded
            Uri drmUri = Uri.parse("file:///" + uri.getPath());
            DcfDecoder tempDcfDecoder = new DcfDecoder();
            tempDcfDecoder.forceDecodeUri(cr, drmUri, options, false);
        } else {
            if (fd == null) return null;
            BitmapFactory.decodeFileDescriptor(fd, null, options);
        }
        if (0 != options.outWidth && 0 != options.outHeight) {
            Size size = new Size();
            size.width = options.outWidth;
            size.height = options.outHeight;
            Log.d(TAG,"[OP01]updateImageSize:[" + options.outWidth + "x" + options.outHeight + "]");
            return size;
        }

        return null;
    }
}