package com.mediatek.dialer.plugin.dialpad;

import android.content.Context;
import android.content.res.Resources;
import android.database.Cursor;
import android.text.format.DateFormat;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import com.mediatek.dialer.ext.DialerSearchAdapterExtension;
import com.mediatek.dialer.plugin.OP09DialerPlugin;

public class DialerSearchAdapterOP09Extension extends DialerSearchAdapterExtension {
 
    private static final String TAG = "DialerSearchAdapterOP09Extension";

    private static final String ID = "id";
    private static final String ID_NAME_DATE = "date";
    private static final int CALL_LOG_DATE_INDEX = 2;

    private OP09DialerPlugin mOP09DialerPlugin;

    public DialerSearchAdapterOP09Extension(OP09DialerPlugin oP09DialerPlugin) {
        mOP09DialerPlugin = oP09DialerPlugin;
    }

    public void bindCallLogViewPost(View view, Context context, Cursor cursor) {
        updateCallLogDate(view, context, cursor);
    }

    public void bindContactCallLogViewPost(View view, Context context, Cursor cursor) {
        updateCallLogDate(view, context, cursor);
    }

    private void updateCallLogDate(View view, Context context, Cursor cursor) {
        log("updateCallLogDate()");
        if (0 == mOP09DialerPlugin.getTimezoneRawOffset()) {
            return;
        }
        Resources resource = context.getResources();
        String packageName = context.getPackageName();
        TextView dateView = (TextView) view.findViewById(resource.getIdentifier(ID_NAME_DATE, ID, packageName));
        if (null != dateView) {
            java.text.DateFormat dateFormat = DateFormat.getTimeFormat(context);
            long date = cursor.getLong(CALL_LOG_DATE_INDEX);
            dateView.setText(dateFormat.format(date + mOP09DialerPlugin.getTimezoneOffset(date)));
        }
    }

    private static void log(String msg) {
        Log.d(TAG, msg);
    }
}
