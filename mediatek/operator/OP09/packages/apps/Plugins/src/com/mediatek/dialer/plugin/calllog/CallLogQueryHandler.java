/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.dialer.plugin.calllog;

import android.content.AsyncQueryHandler;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.database.MergeCursor;
import android.database.sqlite.SQLiteDatabaseCorruptException;
import android.database.sqlite.SQLiteDiskIOException;
import android.database.sqlite.SQLiteException;
import android.database.sqlite.SQLiteFullException;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.provider.CallLog;
import android.provider.CallLog.Calls;
import android.provider.VoicemailContract.Status;
import android.text.TextUtils;
import android.util.Log;

import java.lang.ref.WeakReference;
import java.util.List;
import java.util.concurrent.TimeUnit;

public class CallLogQueryHandler extends AsyncQueryHandler {

    private static final String TAG = "CallLogQueryHandler";

    private final WeakReference<Listener> mListener;

    public CallLogQueryHandler(ContentResolver contentResolver, Listener listener) {
        super(contentResolver);
        mListener = new WeakReference<Listener>(listener);
    }

    public interface Listener {
        void onCallsDeleted();
    }

    private static final int DELETE_CALLS_TOKEN = 59;

    //private static final int QUERY_ALL_CALLS_JOIN_DATA_VIEW_TOKEN = 62;

    protected void onDeleteComplete(int token, Object cookie, int result) {
        final Listener listener = mListener.get();
        if (listener != null) {
            listener.onCallsDeleted();
        }
    }

    public void deleteSpecifiedCalls(String deleteFilter) {
        startDelete(DELETE_CALLS_TOKEN, null, Calls.CONTENT_URI, deleteFilter, null);
    }

    private static void log(final String log) {
        Log.i(TAG, log);
    }
}
