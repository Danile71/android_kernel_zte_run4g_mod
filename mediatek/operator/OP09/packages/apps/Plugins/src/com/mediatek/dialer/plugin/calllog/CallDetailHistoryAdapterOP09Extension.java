package com.mediatek.dialer.plugin.calllog;

import java.util.Iterator;

import android.content.Context;
import android.content.res.Resources;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.text.format.DateUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.mediatek.dialer.PhoneCallDetailsEx;
import com.mediatek.dialer.ext.CallDetailHistoryAdapterExtension;
import com.mediatek.dialer.plugin.OP09DialerPlugin;
import com.mediatek.op09.plugin.R;

public class CallDetailHistoryAdapterOP09Extension extends CallDetailHistoryAdapterExtension {
 
    private static final String TAG = "CallDetailHistoryAdapterOP09Extension";

    private static final int VIEW_TYPE_ALL_CALL_HEADER = 2;

    private Context mHostContext;
    private Context mPluginContext;
    private OP09DialerPlugin mOP09DialerPlugin;
    private PhoneCallDetailsEx[] mPhoneCallDetails;
    private LayoutInflater mLayoutInflater;

    public CallDetailHistoryAdapterOP09Extension(Context pluginContext, OP09DialerPlugin oP09DialerPlugin) {
        mPluginContext = pluginContext;
        mOP09DialerPlugin = oP09DialerPlugin;
    }

    // Below function not finish
    public void init(Context context, PhoneCallDetailsEx[] phoneCallDetails) {
        mHostContext = context;
        mPhoneCallDetails = phoneCallDetails;
        mLayoutInflater = (LayoutInflater) mPluginContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    }

    public int getViewTypeCount(int currentViewTypeCount) {
        return currentViewTypeCount + 1;
    }

    public int getItemViewType(int position) {
        if (0 != position) {
            if (null != mPhoneCallDetails[position-1]
                    && mPhoneCallDetails[position-1] instanceof OP09PhoneCallDetails) {
                OP09PhoneCallDetails phoneCallDetails = (OP09PhoneCallDetails) mPhoneCallDetails[position-1];
                if (OP09PhoneCallDetails.DISPLAY_TYPE_TEXT == phoneCallDetails.displayType) {
                    return VIEW_TYPE_ALL_CALL_HEADER;
                }
            }
        }
        return -1;
    }

    public View getViewPre(int position, View convertView, ViewGroup parent) {
        if (0 == position) {
            return null;
        }
        if (null == mPhoneCallDetails[position-1]) {
            return null;
        }
        if (mPhoneCallDetails[position-1] instanceof OP09PhoneCallDetails) {
            OP09PhoneCallDetails phoneCallDetails = (OP09PhoneCallDetails) mPhoneCallDetails[position-1];
            if (OP09PhoneCallDetails.DISPLAY_TYPE_TEXT == phoneCallDetails.displayType) {
                final View result  = (convertView == null)
                        ? mLayoutInflater.inflate(R.layout.call_detail_history_all_calls_list_item, parent, false)
                        : convertView;
                TextView allCallsText = (TextView) result.findViewById(R.id.text);
                if (null != allCallsText) {
                    if (null == mPhoneCallDetails[position -1].contactUri) {
                        allCallsText.setText(mPluginContext.getString(R.string.all_calls_with_number));
                    } else {
                        allCallsText.setText(mPluginContext.getString(R.string.all_calls_with_contact));
                    }
                }
                return result;
            }
        }
        return null;
    }

    public View getViewPost(int position, View convertView, ViewGroup parent) {
        if (0 == position) {
            return convertView;
        }
        if (null == mPhoneCallDetails[position-1]) {
            return convertView;
        }
        if (0 != mOP09DialerPlugin.getTimezoneRawOffset()) {
            Resources resource = mHostContext.getResources();
            String packageName = mHostContext.getPackageName();
            PhoneCallDetailsEx phoneCallDetails = (PhoneCallDetailsEx) mPhoneCallDetails[position-1];
            TextView dateView =
                (TextView) convertView.findViewById(resource.getIdentifier("date", "id", packageName));
            if (null != dateView) {
                CharSequence dateValue = DateUtils.formatDateRange(mHostContext,
                        phoneCallDetails.date + mOP09DialerPlugin.getTimezoneOffset(phoneCallDetails.date),
                        phoneCallDetails.date + mOP09DialerPlugin.getTimezoneOffset(phoneCallDetails.date),
                        DateUtils.FORMAT_SHOW_TIME | DateUtils.FORMAT_SHOW_DATE |
                        DateUtils.FORMAT_SHOW_WEEKDAY | DateUtils.FORMAT_SHOW_YEAR);
                dateView.setText(dateValue);
            }
        }
        if (mPhoneCallDetails[position-1] instanceof OP09PhoneCallDetails) {
            OP09PhoneCallDetails phoneCallDetails = (OP09PhoneCallDetails) mPhoneCallDetails[position-1];
            if (OP09PhoneCallDetails.DISPLAY_TYPE_NEW_CALL == phoneCallDetails.displayType) {
                if (null != phoneCallDetails.contactUri) {
                    Resources resource = mHostContext.getResources();
                    String packageName = mHostContext.getPackageName();
                    TextView callTypeTextView =
                        (TextView) convertView.findViewById(resource.getIdentifier("call_type_text", "id", packageName));
                    if (null != callTypeTextView) {
                        callTypeTextView.setText(phoneCallDetails.number + " "
                                + Phone.getTypeLabel(resource, phoneCallDetails.numberType, phoneCallDetails.numberLabel));
                    }
                }
            }
        }
        return convertView;
    }

    private static void log(String msg) {
        Log.d(TAG, msg);
    }
}
