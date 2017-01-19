package com.mediatek.dialer.plugin.calllog;

import android.app.Activity;
import android.util.Log;
import android.view.View;
import android.widget.ListView;

import com.mediatek.dialer.ext.CallLogSearchResultActivityExtension;

public class CallLogSearchResultActivityOP09Extension extends CallLogSearchResultActivityExtension {
 
    private static final String TAG = "CallLogSearchResultActivityOP09Extension";

    private Activity mActivity;

    public CallLogSearchResultActivityOP09Extension() {
    }

    public void onCreate(Activity activity) {
        mActivity = activity;
    }

    public void onDestroy() {
        mActivity = null;
    }

    public boolean onListItemClick(ListView l, View v, int position, long id) {
        log("onListItemClick(), view = " + v);
        if (!(v.getTag() instanceof CallLogInfo)) {
            log("onListItemClick(), v.getTag() is not instance of CallLogInfo, just return");
            return false;
        }
        CallLogInfo callLogInfo = (CallLogInfo) v.getTag();
        if (null != callLogInfo && null != mActivity) {
            mActivity.startActivity(callLogInfo.mCallDetailIntent);
        }
        return true;
    }

    private static void log(String msg) {
        Log.d(TAG, msg);
    }
}
