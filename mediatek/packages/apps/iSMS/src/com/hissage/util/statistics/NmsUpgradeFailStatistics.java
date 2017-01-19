package com.hissage.util.statistics;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.json.JSONObject;

import com.hissage.util.log.NmsLog;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.text.TextUtils;

public class NmsUpgradeFailStatistics extends SQLiteOpenHelper {
    public static final int VERSION = 1;
    public static final String DB_NAME = "nms_upgrade_fail.db";
    public static final String TABLE_NAME = "upgradeFail";
    public static SQLiteDatabase mDateBase = null;
    public static final String LOG_TAG = "NmsUpgradeFailStatistics";
    public static final String KEY_NO_NETWORK = "no_network";
    public static final String KEY_WIFI_FAIL = "wifi_fail";
    public static final String KEY_GPRS_FAIL = "gprs_fail";
    private static NmsUpgradeFailStatistics mInstance = null;
    private boolean isInitOK = true;

    public NmsUpgradeFailStatistics(Context context) {
        super(context, DB_NAME, null, VERSION);
    }

    public static void init(Context context) {
        if (null == mInstance) {
            mInstance = new NmsUpgradeFailStatistics(context);
        }
    }
    
    @Override
    public void onCreate(SQLiteDatabase db) {
        try {
            db.execSQL("CREATE TABLE upgradeFail (_id INTEGER PRIMARY KEY AUTOINCREMENT,date TEXT NOT NULL COLLATE NOCASE,no_network INTEGER NOT NULL DEFAULT 0,wifi_fail INTEGER NOT NULL DEFAULT 0,grps_fail INTEGER NOT NULL DEFAULT 0,UNIQUE (date))");
        } catch (Exception e) {
            isInitOK = false;
            NmsLog.error(LOG_TAG, "fatal error that create table got execption: " + e.toString());
        }
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        // TODO Auto-generated method stub

    }

    private synchronized void doClear() {
        try {
            getWritableDatabase().execSQL(String.format("DELETE FROM %s", TABLE_NAME));
        } catch (Exception e) {
            NmsLog.error(LOG_TAG, "fatal error to clear data: " + e.toString());
        }

    }

    private synchronized void doUpdateToDb(boolean noNetwork, boolean wifi_fail, boolean gprs_fail) {
        try {
            String curDate = new SimpleDateFormat("yyyy-MM-dd").format(new Date());
            if (TextUtils.isEmpty(curDate)) {
                NmsLog.trace(LOG_TAG, "error to get date");
            }

            SQLiteDatabase db = getWritableDatabase();
            db.execSQL(String.format("INSERT OR IGNORE INTO %s(date) VALUES('%s');", TABLE_NAME,
                    curDate));
            NmsLog.trace(LOG_TAG, "update db noNetwork:,wifi_fail,gprs_fail" + noNetwork
                    + wifi_fail + gprs_fail);

            if (noNetwork) {
                db.execSQL(String
                        .format("UPDATE OR IGNORE %s SET %s=(SELECT %s FROM %s WHERE date='%s')+1 WHERE date='%s';",
                                TABLE_NAME, KEY_NO_NETWORK, KEY_NO_NETWORK, TABLE_NAME, curDate,
                                curDate));
            }
            if (wifi_fail) {
                db.execSQL(String
                        .format("UPDATE OR IGNORE %s SET %s=(SELECT %s FROM %s WHERE date='%s')+1 WHERE date='%s';",
                                TABLE_NAME, KEY_WIFI_FAIL, KEY_WIFI_FAIL, TABLE_NAME, curDate,
                                curDate));
            }
            if (gprs_fail) {
                db.execSQL(String
                        .format("UPDATE OR IGNORE %s SET %s=(SELECT %s FROM %s WHERE date='%s')+1 WHERE date='%s';",
                                TABLE_NAME, KEY_GPRS_FAIL, KEY_GPRS_FAIL, TABLE_NAME, curDate,
                                curDate));
            }
        } catch (Exception e) {
            NmsLog.error(LOG_TAG, "fatal error that insert or update data: " + e.toString());
        }
    }

    private JSONObject doGetUpgrageFailData() {
        JSONObject upgradeFail = new JSONObject();

        SQLiteDatabase db = getReadableDatabase();
        db.beginTransaction();
        Cursor cursor = db.rawQuery("SELECT * FROM upgradeFail ORDER BY date", null);
        if (cursor == null) {
            NmsLog.error(LOG_TAG, "doToJasonData rawQuery got cursor is null");
            return null;
        }

        try {
            while (cursor.moveToNext()) {
                // 0 _id,1 date
                JSONObject dateObject = new JSONObject();
                dateObject.put(KEY_NO_NETWORK, cursor.getString(2));
                dateObject.put(KEY_WIFI_FAIL, cursor.getString(3));
                dateObject.put(KEY_GPRS_FAIL, cursor.getString(4));
                upgradeFail.put(cursor.getString(1), dateObject);
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
            upgradeFail = null;
        }
        cursor.close();
        db.endTransaction();
        return upgradeFail;
    }
    
    private static boolean isInited(String logStr) {
        if (mInstance == null) {
            NmsLog.error(LOG_TAG, "error that the singleton is NOT init yet for " + logStr);
            return false;
        }

        if (!mInstance.isInitOK) {
            NmsLog.error(LOG_TAG, "error that the singleton is init ERROR for " + logStr);
            return false;
        }

        return true;
    }
    
    public static void Clear() {
        if (!isInited("clear")) {
            return;
        }
        mInstance.doClear();
    }
  

    public static void updateToDb(boolean noNetwork, boolean wifi_fail, boolean gprs_fail) {
        if (!isInited("updateToDb")) {
            return;
        }
        mInstance.doUpdateToDb(noNetwork, wifi_fail, gprs_fail);
    }
    

    public static JSONObject getUpgrageFailData() {
        if (!isInited("getUpgrageFailData")) {
            return null;
        }
        return mInstance.doGetUpgrageFailData();
    }
}
