package com.mediatek.settingsprovider.plugin;

import android.content.Context;
import android.provider.Settings;

import com.mediatek.op09.plugin.R;
import com.mediatek.providers.settings.ext.DefaultDatabaseHelperExt;
import com.mediatek.xlog.Xlog;

public class OP09DatabaseHelperExt extends DefaultDatabaseHelperExt {

    private static final String TAG = "OP09DatabaseHelperExt";
    private Context mContext;

    public OP09DatabaseHelperExt(Context context) {
        super(context);
        mContext = context;
    }

    @Override
    public String getResBoolean(Context context, String name, String defaultValue) {
        String ret = defaultValue;
        if (Settings.Global.DATA_ROAMING.equals(name)) {
            ret = mContext.getResources().getBoolean(
                    R.bool.def_data_roaming_op09) ? "1" : "0";
        } else if (Settings.Global.DATA_ROAMING_2.equals(name)) {
            ret = mContext.getResources().getBoolean(
                    R.bool.def_data_roaming_2_op09) ? "1" : "0";
        }
        Xlog.d(TAG, "getResBoolean, name = " + name + ",ret = " + ret);
        return ret;
    }

    @Override
    public String getResStr(Context context, String name, String defaultValue) {
        String ret = defaultValue;
        if (Settings.Secure.LOCATION_PROVIDERS_ALLOWED.equals(name)) {
            ret = mContext.getResources().getString(R.string.def_location_providers_allowed_op09);
        }
        Xlog.d(TAG,"getResStr name = " + name + " string value = " + ret);
        return ret;
    }
}
