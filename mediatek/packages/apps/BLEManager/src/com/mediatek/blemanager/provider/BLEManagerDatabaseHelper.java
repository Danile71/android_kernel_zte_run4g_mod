package com.mediatek.blemanager.provider;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;

public class BLEManagerDatabaseHelper extends SQLiteOpenHelper {

    private static final String TAG = BLEConstants.COMMON_TAG + "[BLEManagerDatabaseHelper]";
    
    private static final String DATABASE_NAME = "blemanager.db";
    
    private static final int DEFAULT_VERSION = 1;
    
    private static BLEManagerDatabaseHelper sInstance;
    
//    private Context mContext;
    
    private BLEManagerDatabaseHelper(Context context) {
        super(context, DATABASE_NAME, null, DEFAULT_VERSION);
//        mContext = context;
        Log.d(TAG, "[Constructor] DATABASE_NAME : " + DATABASE_NAME
                + ", DEFAULT_VERSION :" + DEFAULT_VERSION);
    }
    
    public static BLEManagerDatabaseHelper getInstance(Context context) {
        if (context == null) {
            Log.e(TAG, "[getInstance] WRONG PARAMETER!! Context is null!!");
            return null;
        }
        if (sInstance == null) {
            sInstance = new BLEManagerDatabaseHelper(context);
        }
        return sInstance;
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        // TODO Auto-generated method stub
        createUxTable(db);
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        // TODO Auto-generated method stub
        Log.d(TAG, "[onUpgrade] oldVersion : " + oldVersion + ", newVersion : " + newVersion);
    }

    private void createUxTable(SQLiteDatabase db) {
        Log.d(TAG, "[createUxTable] create UX table enter");
        String uxCreateString = "CREATE TABLE " +
            BLEConstants.DEVICE_SETTINGS.TABLE_NAME + " (" +
            BLEConstants.COLUMN_ID + " INTEGER PRIMARY KEY," +
            BLEConstants.COLUMN_BT_ADDRESS + " TEXT," +
            BLEConstants.DEVICE_SETTINGS.DEVICE_DISPLAY_ORDER + " INTEGER," +
            BLEConstants.DEVICE_SETTINGS.DEVICE_NAME + " TEXT," +
            BLEConstants.DEVICE_SETTINGS.DEVICE_IAMGE_DATA + " TEXT," +
            BLEConstants.DEVICE_SETTINGS.DEVICE_SERVICE_LIST + " TEXT," +
            BLEConstants.PXP_CONFIGURATION.RANGE_ALERT_INFO_DIALOG_ENABLER + " INTEGER," +
            BLEConstants.PXP_CONFIGURATION.RINGTONE_ENABLER + " INTEGER," +
            BLEConstants.PXP_CONFIGURATION.RINGTONE + " TEXT," +
            BLEConstants.PXP_CONFIGURATION.VIBRATION_ENABLER + " INTEGER," +
            BLEConstants.PXP_CONFIGURATION.VOLUME + " INTEGER)";
//            BLEConstants.PXP_CONFIGURATION.IS_SUPPORT_OPTIONAL + " INTEGER)";
        db.execSQL(uxCreateString);
    }

}
