package com.mediatek.backuprestore;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.mediatek.backuprestore.utils.Constants;
import com.mediatek.backuprestore.utils.MyLogger;

import java.util.HashSet;

public class SDCardReceiver extends BroadcastReceiver {

    interface OnSDCardStatusChangedListener {
        void onSDCardStatusChanged(boolean mount);
    }

    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/SDCardReceiver";
    private static SDCardReceiver sInstance;
    private HashSet<OnSDCardStatusChangedListener> mListnerList;

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        MyLogger.logI(CLASS_TAG, "  SDCardReceiver -> onReceive: " + action);
        SDCardReceiver s = getInstance();
        if (action != null && action.equals(Intent.ACTION_MEDIA_UNMOUNTED)) {
            MyLogger.logI(CLASS_TAG, "------  SDCardReceiver -> onReceive: ACTION_MEDIA_UNMOUNTED s = "+s + ", sInstance = "+ sInstance);
            if (s.mListnerList != null) {
                for (OnSDCardStatusChangedListener listener : s.mListnerList) {
                    listener.onSDCardStatusChanged(false);
                }
            } else {
                context.stopService(new Intent("com.mediatek.backuprestore.intent.RestoreService"));
                context.stopService(new Intent("com.mediatek.backuprestore.intent.BackupService"));
                MyLogger.logI(CLASS_TAG, "------  SDCardReceiver -> onReceive: must be stopService "+mListnerList + ", s.mListnerList = "+ s.mListnerList);
            }
        } else if (action != null && action.equals(Intent.ACTION_MEDIA_MOUNTED)) {
            if (s.mListnerList != null) {
                for (OnSDCardStatusChangedListener listener : s.mListnerList) {
                    listener.onSDCardStatusChanged(true);
                }
            }
        } else if (action != null && action.equals(Constants.INTENT_SD_SWAP)) {
            boolean sdCardExist = intent.getBooleanExtra(Constants.ACTION_SD_EXIST, false);
            if (s.mListnerList != null) {
                for (OnSDCardStatusChangedListener listener : s.mListnerList) {
                    listener.onSDCardStatusChanged(sdCardExist);
                }
            }
        }
    }

    public static SDCardReceiver getInstance() {
        if (sInstance == null) {
            sInstance = new SDCardReceiver();
        }
        return sInstance;
    }

    public void registerOnSDCardChangedListener(OnSDCardStatusChangedListener listener) {
        if (mListnerList == null) {
            mListnerList = new HashSet<OnSDCardStatusChangedListener>();
        }
        MyLogger.logV(CLASS_TAG, "registerOnSDCardChangedListener:" + listener);
        mListnerList.add(listener);
    }

    public void unRegisterOnSDCardChangedListener(OnSDCardStatusChangedListener listener) {
        MyLogger.logV(CLASS_TAG, "unRegisterOnSDCardChangedListener:" + listener);
        if (mListnerList != null) {
            mListnerList.remove(listener);
        }
    }
}
