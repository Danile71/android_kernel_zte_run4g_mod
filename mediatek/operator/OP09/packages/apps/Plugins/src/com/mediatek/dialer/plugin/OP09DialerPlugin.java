package com.mediatek.dialer.plugin;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.provider.Settings;
import android.util.Log;

import com.mediatek.dialer.ext.CallDetailExtension;
import com.mediatek.dialer.ext.CallDetailHistoryAdapterExtension;
import com.mediatek.dialer.ext.CallListExtension;
import com.mediatek.dialer.ext.CallLogAdapterExtension;
import com.mediatek.dialer.ext.CallLogSearchResultActivityExtension;
import com.mediatek.dialer.ext.CallLogSimInfoHelperExtension;
import com.mediatek.dialer.ext.DialerPluginDefault;
import com.mediatek.dialer.ext.ContactsCallOptionHandlerFactoryExtension;
import com.mediatek.dialer.ext.DialPadExtension;
import com.mediatek.dialer.ext.DialerSearchAdapterExtension;
import com.mediatek.dialer.plugin.calllog.CallDetailHistoryAdapterOP09Extension;
import com.mediatek.dialer.plugin.calllog.CallDetailOP09Extension;
import com.mediatek.dialer.plugin.calllog.CallListOP09Extension;
import com.mediatek.dialer.plugin.calllog.CallLogAdapterOP09Extension;
import com.mediatek.dialer.plugin.calllog.CallLogSearchResultActivityOP09Extension;
import com.mediatek.dialer.plugin.calllog.CallLogSimInfoHelperOP09Extension;
import com.mediatek.dialer.plugin.calloption.ContactsCallOptionHandlerFactoryOP09Extension;
import com.mediatek.dialer.plugin.dialpad.DialPadOP09Extension;
import com.mediatek.dialer.plugin.dialpad.DialerSearchAdapterOP09Extension;
import com.mediatek.phone.SIMInfoWrapper;

import java.util.Date;
import java.util.TimeZone;

public class OP09DialerPlugin extends DialerPluginDefault {

    private static final String TAG = "OP09DialerPlugin";

    public static final String BEIJING_TIME_ZONE_ID = "Asia/Shanghai";
    public static final String BROADCAST_TIME_DISPLAY_MODE = "com.mediatek.ct.TIME_DISPLAY_MODE";

    private Context mPluginContext;
    private int mTimezoneRawOffset = 0;

    public OP09DialerPlugin(Context context) {
        mPluginContext = context;
        SIMInfoWrapper.getDefault().init(mPluginContext);
        updateTimezoneRawOffset();
        mPluginContext.registerReceiver(mTimeDisplayModeReceiver, new IntentFilter(BROADCAST_TIME_DISPLAY_MODE));
    }

    public DialPadExtension createDialPadExtension() {
        return new DialPadOP09Extension(mPluginContext);
    }

    public ContactsCallOptionHandlerFactoryExtension createContactsCallOptionHandlerFactoryExtension() {
        return new ContactsCallOptionHandlerFactoryOP09Extension(mPluginContext);
    }

    public CallListExtension createCallListExtension() {
        return new CallListOP09Extension(mPluginContext);
    }

    public CallLogAdapterExtension createCallLogAdapterExtension() {
        return new CallLogAdapterOP09Extension(mPluginContext, this);
    }

    public CallDetailExtension createCallDetailExtension() {
        return new CallDetailOP09Extension(mPluginContext);
    }

    public CallDetailHistoryAdapterExtension createCallDetailHistoryAdapterExtension() {
        return new CallDetailHistoryAdapterOP09Extension(mPluginContext, this);
    }

    public DialerSearchAdapterExtension createDialerSearchAdapterExtension() {
        return new DialerSearchAdapterOP09Extension(this);
    }

    public CallLogSearchResultActivityExtension createCallLogSearchResultActivityExtension() {
        return new CallLogSearchResultActivityOP09Extension();
    }

    public CallLogSimInfoHelperExtension createCallLogSimInfoHelperExtension() {
        return new CallLogSimInfoHelperOP09Extension(mPluginContext, this);
    }

    private BroadcastReceiver mTimeDisplayModeReceiver = new BroadcastReceiver() {

        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals(BROADCAST_TIME_DISPLAY_MODE)) {
                log("received time display mode broadcast message");
                updateTimezoneRawOffset();
            }
        }
    };

    private void updateTimezoneRawOffset() {
        if (0 == Settings.System.getInt(mPluginContext.getContentResolver(), "Settings.System.CT_TIME_DISPLAY_MODE", 1)) {
            mTimezoneRawOffset = TimeZone.getTimeZone(OP09DialerPlugin.BEIJING_TIME_ZONE_ID).getRawOffset()
                - TimeZone.getDefault().getRawOffset();
        } else {
            mTimezoneRawOffset = 0;
        } 
    }

    public int getTimezoneRawOffset() {
        return mTimezoneRawOffset;
    }

    public int getTimezoneOffset(long date) {
        if (0 == mTimezoneRawOffset) {
            return 0;
        } else {
            if (TimeZone.getDefault().inDaylightTime(new Date(date))) {
                return mTimezoneRawOffset - TimeZone.getDefault().getDSTSavings();
            } else {
                return mTimezoneRawOffset;
            }
        }
    }

    private static void log(String msg) {
        Log.d(TAG, msg);
    }
}
