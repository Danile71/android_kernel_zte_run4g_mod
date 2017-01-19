/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.mms.plugin;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SqliteWrapper;
import android.net.Uri;
import android.net.http.AndroidHttpClient;
import android.provider.Telephony.Mms;

import com.mediatek.encapsulation.MmsLog;
import com.mediatek.mms.ext.DefaultMmsCancelDownloadExt;

import org.apache.http.impl.client.DefaultHttpRequestRetryHandler;

import java.util.HashMap;

public class Op09MmsCancelDownloadExt extends DefaultMmsCancelDownloadExt {
    private static final String TAG = "Mms/MmsCancelDownloadExt";
    private static final String STATUS_EXT = "st_ext";

    private Context mContext;
    private HashMap<String, AndroidHttpClient> mClientMap;
    private boolean mEnableCancelToast;
    private boolean mWaitingCnxn;
    private DefaultHttpRequestRetryHandler mHttpRetryHandler;

    public Op09MmsCancelDownloadExt(Context context) {
        super(context);
        mContext = context;
        mClientMap = new HashMap<String, AndroidHttpClient>();
        mEnableCancelToast = false;
        mWaitingCnxn = false;
    }

    public void addHttpClient(String url, AndroidHttpClient client) {
        MmsLog.d(TAG, "setHttpClient(): url = " + url);

        mClientMap.put(url, client);
    }

    public void cancelDownload(final Uri uri) {
        MmsLog.d(TAG, "MmsCancelDownloadExt: cancelDownload()");
        if (uri == null) {
            MmsLog.d(TAG, "cancelDownload(): uri is null!");
            return;
        }

        // Update the download status
        markStateExt(uri, STATE_CANCELLING);

        Thread thread = new Thread(new Runnable() {
            String mContentUrl = null;

            @Override
            public void run() {
                mContentUrl = getContentLocation(uri);

                if (!mClientMap.containsKey(mContentUrl)) {
                    setCancelDownloadState(uri, true);
                } else {
                    abortMmsHttp(mContentUrl, uri);
                }
            }
        });

        thread.start();
    }

    public void removeHttpClient(String url) {
        MmsLog.d(TAG, "removeHttpClient(): url = " + url);

        mClientMap.remove(url);
    }

    public void setCancelToastEnabled(boolean isEnable) {
        MmsLog.d(TAG, "setCancelEnabled(): mEnableCancelToast = " + isEnable);
        mEnableCancelToast = isEnable;
    }

    public boolean getCancelToastEnabled() {
        MmsLog.d(TAG, "getCancelEnabled(): mEnableCancelToast = " + mEnableCancelToast);
        return mEnableCancelToast;
    }

    public void markStateExt(Uri uri, int state) {
        MmsLog.d(TAG, "markStateExt: state = " + state + " uri = " + uri);

        // Use the STATUS field to store the state of downloading process
        ContentValues values = new ContentValues(1);
        values.put(STATUS_EXT, state);
        SqliteWrapper.update(mContext, mContext.getContentResolver(),
                    uri, values, null, null);
    }

    public int getStateExt(Uri uri) {
        MmsLog.d(TAG, "getStateExt: uri = " + uri);
        Cursor cursor = SqliteWrapper.query(mContext, mContext.getContentResolver(),
                            uri, new String[] {STATUS_EXT}, null, null, null);

        if (cursor != null) {
            try {
                if (cursor.moveToFirst()) {
                    return cursor.getInt(0);
                }
            } finally {
                cursor.close();
            }
        }
        return STATE_UNKNOWN;
    }

    public int getStateExt(String url) {
        MmsLog.d(TAG, "getStateExt: url = " + url);

        String where = Mms.CONTENT_LOCATION + " = ?";
        Cursor cursor = SqliteWrapper.query(mContext, mContext.getContentResolver(),
                Mms.CONTENT_URI, new String[] {STATUS_EXT}, where, new String[] {url}, null);

        if (cursor != null) {
            try {
                if (cursor.moveToFirst()) {
                    return cursor.getInt(0);
                }
            } finally {
                cursor.close();
            }
        }
        return STATE_UNKNOWN;
    }

    public void setWaitingDataCnxn(boolean isWaiting) {
        MmsLog.d(TAG, "setWaitingDataCnxn(): mWaitingCnxn = " + isWaiting);
        mWaitingCnxn = isWaiting;
    }

    public boolean getWaitingDataCnxn() {
        MmsLog.d(TAG, "getWaitingDataCnxn(): mWaitingCnxn = " + mWaitingCnxn);
        return mWaitingCnxn;
    }

    public void saveDefaultHttpRetryHandler(DefaultHttpRequestRetryHandler retryHandler) {
        MmsLog.d(TAG, "saveDefaultHttpRetryHandler(): retryHandler = " + retryHandler);
        mHttpRetryHandler = retryHandler;
    }

    private void setCancelDownloadState(Uri uri, boolean isCancelling) {
        MmsLog.d(TAG, "setCancelDownloadState()...");
        this.getHost().setCancelDownloadState(uri, isCancelling);
    }

    private String getContentLocation(final Uri uri) {
        String contentUrl = null;

        Cursor cursor = SqliteWrapper.query(mContext, mContext.getContentResolver(),
            uri, new String[]{Mms.CONTENT_LOCATION}, null, null, null);

        if (cursor != null) {
            try {
                if ((cursor.getCount() == 1) && cursor.moveToFirst()) {
                    contentUrl = cursor.getString(0);
                    MmsLog.d(TAG, "getContentLocation(): contentUrl = " + contentUrl);
                }
            } finally {
                cursor.close();
            }
        }

        return contentUrl;
    }

    private void abortMmsHttp(String contentUrl, Uri uri) {
        if (mHttpRetryHandler != null) {
            MmsLog.d(TAG, "Set Http request retry = 0.");
            mHttpRetryHandler.setRetryCount(0);
        }
        AndroidHttpClient client = mClientMap.get(contentUrl);
        if (client != null) {
            // Abort MMS HTTP download.
            MmsLog.d(TAG, "===before abortMmsHttp");
            markStateExt(uri, STATE_ABORTED);
            client.getConnectionManager().closeExpiredConnections();
            client.getConnectionManager().shutdown();
            MmsLog.d(TAG, "===after abortMmsHttp");
        } else {
            MmsLog.e(TAG, "cancelDownload(): client is null!");
        }
    }

}
