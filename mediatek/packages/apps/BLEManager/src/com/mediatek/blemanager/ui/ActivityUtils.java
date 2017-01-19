package com.mediatek.blemanager.ui;


import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.graphics.RectF;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.provider.MediaStore;
import android.util.Log;

import java.io.ByteArrayOutputStream;
import java.io.File;

import com.mediatek.blemanager.R;
import com.mediatek.blemanager.provider.BLEConstants;
import com.mediatek.storage.StorageManagerEx;

public class ActivityUtils {

    private static final String TAG = BLEConstants.COMMON_TAG + "[ActivityUtils]";

    private static final String ROOT_DIR = StorageManagerEx.getDefaultPath();
    
    private static final String FILE_DIR = ROOT_DIR + "/ble";
    
    /**
     * 
     * @param activity
     * @param which
     */
    public static void startImageChooser(Activity activity, int which) {
        if (activity == null) {
            return;
        }
        Log.d(TAG, "[startImageChooser] which : " + which);
        Intent intent = null;
        switch(which) {
        case 0:
            intent = new Intent(activity, DeviceDefaultImageChooser.class);
            break;
        case 1:
            intent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
            Uri uri = getTempFileUri();
            intent.putExtra(MediaStore.EXTRA_OUTPUT, uri);
            break;
        case 2:
            intent = new Intent(Intent.ACTION_PICK);
            intent.setType("image/*");
            break;
            default:
                throw new IllegalArgumentException("not recognized id!!");
        }
        if (intent != null) {
            activity.startActivityForResult(intent, which);
        }
    }
    
    /**
     * 
     * @param activity
     * @param which
     * @param uri
     * @param photoX
     * @param photoY
     */
    public static void handlePhotoCrop(Activity activity, int which, Uri uri) {
        Intent intent = new Intent("com.android.camera.action.CROP");
        intent.setDataAndType(uri, "image/*");
        intent.putExtra("crop", true);
        intent.putExtra("scale", true);
        intent.putExtra("scaleUpIfNeeded", true);
        intent.putExtra("aspectX", 1);
        intent.putExtra("aspectY", 1);
        intent.putExtra("outputX", 510);
        intent.putExtra("outputY", 565);
        activity.startActivityForResult(intent, which);
    }
    
    /**
     * 
     * @param context
     * @param uri
     * @return
     */
    private static Bitmap getCornorBitmap(File file) {
        Bitmap bitmap = BitmapFactory.decodeFile(file.getAbsolutePath());
        Drawable imageDrawable = new BitmapDrawable(bitmap);

        Bitmap output = Bitmap.createBitmap(510, 565, Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(output);
        RectF outerRect = new RectF(0, 0, 510, 565);
        Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG);
        paint.setColor(Color.RED);
        canvas.drawRoundRect(outerRect, 10, 10, paint);

        paint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.SRC_IN));
        imageDrawable.setBounds(0, 0, 510, 565);
        canvas.saveLayer(outerRect, paint, Canvas.ALL_SAVE_FLAG);
        imageDrawable.draw(canvas);
        canvas.restore();
        return output;
    }
    
    public static Uri getDrawableUri(int drawableId) {
        String uriString = "android.resource://com.mediatek.blelinker/" + drawableId;
        return Uri.parse(uriString);
    }
    
    public static Bitmap getDrawbleBitmap(Context context, int drawableId) {
        if (context == null) {
            Log.d(TAG, "[getDrawbleBitmap] context is null");
            return null;
        }
        Resources r = context.getResources();
        Bitmap map = BitmapFactory.decodeResource(r, drawableId);
        return map;
    }
    
    /**
     * get default image from resource.
     * @param context
     * @return
     */
    public static byte[] getDefaultImage(Context context) {
        if (context == null) {
            Log.d(TAG, "[getDefaultImage] context is null");
            return null;
        }
        Resources r = context.getResources();
        Bitmap map = BitmapFactory.decodeResource(r, R.drawable.image_device);
        if (map == null) {
            Log.d(TAG, "[getDefaultImage] map is null");
            return null;
        }
        
        return comproseBitmapToByteArray(map);
    }
    
    /**
     * 
     * @param bitmap
     * @return
     */
    public static byte[] comproseBitmapToByteArray(Bitmap bitmap) {
        if (bitmap == null) {
            Log.d(TAG, "[comproseBitmapToByteArray] bitmap is null");
            return null;
        }
        ByteArrayOutputStream bos = new ByteArrayOutputStream();
        bitmap.compress(Bitmap.CompressFormat.PNG, 100, bos);
        return bos.toByteArray();
    }
    
    /**
     * 
     * @param bytes
     * @return
     */
    public static Bitmap decodeByteArrayToBitmap(byte[] bytes) {
        if (bytes == null) {
            Log.d(TAG, "[decodeByteArrayToBitmap] bytes is null");
            return null;
        }
        if (bytes.length == 0) {
            Log.d(TAG, "[decodeByteArrayToBitmap] bytes length is 0");
            return null;
        }
        return BitmapFactory.decodeByteArray(bytes, 0, bytes.length);
    }
    
    /**
     * Get drawable icon from resource and copy the resource to the storage/ble/
     * @param context
     * @param drawableId
     * @return
     */
//    public static String getDrawableUri(Context context, int drawableId) {
//        File file = getFile();
//        try {
//            FileOutputStream fos = new FileOutputStream(file);
//            Log.d(TAG, "[getDrawableUri] file : " + file.toString());
//            InputStream is = context.getResources().openRawResource(drawableId);
//            byte[] buffer = new byte[1024];
//            int count = 0;
//            try {
//                while((count = is.read(buffer)) > 0) {
//                    fos.write(buffer, 0, count);
//                }
//                fos.close();
//                is.close();
//            } catch (IOException e) {
//                e.printStackTrace();
//            }
//        } catch (FileNotFoundException e) {
//            e.printStackTrace();
//        }
//        return file.getAbsolutePath();//Uri.fromFile(file);
//    }
    
    /**
     * Map the drawable to be an uri. which can be saved in db.
     * 
     * @param drawableId
     * @return
     */
//    public static Uri getDrawableUri(Context context, int drawableId, String fileName) {
//        File file = getFile(fileName);
//        try {
//            FileOutputStream fos = new FileOutputStream(file);
//            Log.d(TAG, "[getDrawableUri] file : " + file.toString());
//            InputStream is = context.getResources().openRawResource(drawableId);
//            byte[] buffer = new byte[1024];
//            int count = 0;
//            try {
//                while((count = is.read(buffer)) > 0) {
//                    fos.write(buffer, 0, count);
//                }
//                fos.close();
//                is.close();
//            } catch (IOException e) {
//                e.printStackTrace();
//            }
//        } catch (FileNotFoundException e) {
//            e.printStackTrace();
//        }
//        return Uri.fromFile(file);
//    }
    
    /**
     * Save picture from camera(bitmap) to be a file which stored in storage/ble/
     * 
     * @param context
     * @param tagid
     * @param image
     * @return
     */
//    private static Uri saveBitmapToFile(File file, Bitmap image) {
//        Uri uri = null;
//        if (image == null) {
//            Log.d(TAG, "[saveBitmapToUri] image is null!!");
//            return null;
//        }
//
//        FileOutputStream fos = null;
//        try {
//            fos = new FileOutputStream(file);
//            image.compress(Bitmap.CompressFormat.PNG, 100, fos);
//            fos.flush();
//            fos.close();
//        } catch (IOException e) {
//            // TODO Auto-generated catch block
//            e.printStackTrace();
//        }
//        uri = Uri.fromFile(file);
//        return uri;
//    }

    /**
     * Save the image which choose from gallery.
     * Then return the saved uri, which indicate to the new folder.
     * This method try to avoid the image which you choose from gallery and later
     * delete from gallery
     * 
     * @param context
     * @param tagId
     * @param uri
     * @return
     */
    public static Bitmap saveImageFromCustom(Context context, Uri uri, String fileName) {
        if (context == null) {
            Log.d(TAG, "[saveImageFromCustom] context is null!!");
            return null;
        }
        if (uri == null) {
            Log.d(TAG, "[saveImageFromCustom] uri is null!!");
            return null;
        }
        Log.d(TAG, "[saveImageFromCustom] uri : " + uri);
        String srcPath = getUriPath(context, uri);
        Log.d(TAG, "[saveImageFromCustom] srcPath : " + srcPath);
//        File desFile = getFile(fileName);
        File srcFile = new File(srcPath);

        Bitmap bitmap = getCornorBitmap(srcFile);
//        Uri retUri = saveBitmapToFile(desFile, bitmap);
//        context.getContentResolver().delete(uri, null, null);
//        Log.d(TAG, "[saveImageFromCustom] retUri : " + retUri);
        return bitmap;
    }
    
    /**
     * return the device image to be a drawable
     * @param tagId
     * @return
     */
//    public static Drawable getDrawableIcon(Uri uri) {
//        String fileName = uri.toString();
//        File f = getFile(fileName);
//        try {
//            InputStream is = new FileInputStream(f);
//            return new BitmapDrawable(is);
//        } catch (FileNotFoundException e) {
//            e.printStackTrace();
//            return null;
//        }
//    }
    
    /**
     * 
     * @return
     */
    public static Uri getTempFileUri() {
        String fileName = "device_temp.png";
        File f = new File(FILE_DIR);
        if (!f.exists()) {
            f.mkdir();
        }
        File file = new File(FILE_DIR + "/" + fileName);
        return Uri.fromFile(file);
    }
    
    /**
     * check the file exist or not, if not exist, create it and return the file
     * @param tagid
     * @return
     */
//    private static File getFile() {
//        long tagid = System.currentTimeMillis();
//        String fileName = "device_" + tagid + ".png";
//        File f = new File(FILE_DIR);
//        if (!f.exists()) {
//            f.mkdir();
//        }
//        File file = new File(FILE_DIR + "/" + fileName);
//        return file;
//    }
    
    /**
     * 
     * @param fileName
     * @return
     */
//    public static File getFile(String fileName) {
//        File f = new File(FILE_DIR);
//        if (!f.exists()) {
//            f.mkdir();
//        }
//        Uri uri = Uri.parse(fileName);
//        String file1 = fileName.substring(8);
//        File file = new File(file1);
//        return file;
//    }
    
    private static String getUriPath(Context context, Uri uri) {
        if (context == null) {
            Log.d(TAG, "[getUriPath] context is null!!");
            return "";
        }
        if (uri == null) {
            Log.d(TAG, "[getUriPath] uri is null!!");
            return "";
        }
        String fileName = null;

        if (uri.getScheme().toString().compareTo("content") == 0) {
            Cursor cursor = context.getContentResolver().query(uri, null, null,
                    null, null);
            Log.d(TAG, "[getUriPath] cursor : " + cursor);
            if (cursor != null && cursor.moveToFirst()) {
                int columnIndex = cursor
                        .getColumnIndexOrThrow(MediaStore.MediaColumns.DATA);
                Log.d(TAG, "[getUriPath] column_index : " + columnIndex);
                fileName = cursor.getString(columnIndex);
                Log.d(TAG, "[getUriPath] fileName : " + fileName);
                cursor.close();
            }
        } else if (uri.getScheme().compareTo("file") == 0) {
            fileName = uri.toString();
            fileName = uri.toString().replace("file://", "");
            if (!fileName.startsWith("/mnt")) {
                fileName += "/mnt";
            }
        }
        Log.d(TAG, "[getUriPath] fileName : " + fileName);
        return fileName;
    }

}
