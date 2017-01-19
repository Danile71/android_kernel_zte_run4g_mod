package com.mediatek.dialer.plugin;

import android.util.Log;
import android.view.View;

import com.mediatek.dialer.ext.DefaultSpeedDialExtension;

public class Op01SpeedDialExtension extends DefaultSpeedDialExtension {
    private static final String TAG = "OP01SpeedDialExtension";
    
    @Override
    public void setView(View view, int viewId, boolean mPrefNumContactState, int sdNumber) {
        Log.i(TAG, "setView viewId : " + viewId + " | sdNumber : " + sdNumber);
        if (!mPrefNumContactState && viewId == sdNumber) {
            view.setVisibility(View.GONE);
            Log.i(TAG, "[setView] view is gone");
        } else if (viewId == sdNumber) {
            view.setVisibility(View.VISIBLE);
            Log.i(TAG, "[setView] view is visible");
        }
    }
    
    @Override
    public int setAddPosition(int mAddPosition, boolean mNeedRemovePosition) {
        if (mNeedRemovePosition) {
            return -1;
        }
        return mAddPosition;
    }

    @Override
    public boolean showSpeedInputDialog() {
        Log.i(TAG, "[showSpeedInputDialog");
        return true;
    }
    
    @Override
    public boolean needClearSharedPreferences() {
        Log.i(TAG, "[needClearSharedPreferences");
        return false;
    }
   
    public boolean clearPrefStateIfNecessary() {
        Log.i(TAG, "SpeedDialManageActivity: [clearPrefStateIfNecessary()]");
        return false;
    }

    public boolean needCheckContacts() {
        Log.i(TAG, "SpeedDialManageActivity: [needCheckContacts()]");
        return false;
    }

}
