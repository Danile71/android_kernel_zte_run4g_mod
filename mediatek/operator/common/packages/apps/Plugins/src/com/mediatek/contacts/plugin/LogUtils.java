package com.mediatek.contacts.plugin;

import com.mediatek.xlog.Xlog;

public class LogUtils {
    private static final String TAG = "Contacts/AAS_SNE"; /**common tag added for both features*/
    private static final String LOG_MSG_TSTART = "[";
    private static final String LOG_MSG_TEND = "]";

    public static void d(String tag, String msg) {
        Xlog.d(TAG, getLogMsg(tag, msg));
    }

    public static void w(String tag, String msg) {
        Xlog.w(TAG, getLogMsg(tag, msg));
    }

    public static void e(String tag, String msg) {
        Xlog.e(TAG, getLogMsg(tag, msg));
    }

    private static String getLogMsg(String tag, String msg) {
        StringBuilder sb = new StringBuilder();
        sb.append(LOG_MSG_TSTART).append(tag).append(LOG_MSG_TEND).append(msg);
        return sb.toString();
    }
}
