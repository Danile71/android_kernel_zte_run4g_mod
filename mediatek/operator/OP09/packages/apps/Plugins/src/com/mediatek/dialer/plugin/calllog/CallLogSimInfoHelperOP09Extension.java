package com.mediatek.dialer.plugin.calllog;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.drawable.Drawable;
import android.util.Log;

import com.mediatek.dialer.ext.CallLogSimInfoHelperExtension;
import com.mediatek.dialer.plugin.OP09DialerPlugin;
import com.mediatek.op09.plugin.R;
import com.mediatek.telephony.SimInfoManager;


public class CallLogSimInfoHelperOP09Extension extends CallLogSimInfoHelperExtension {
 
    private static final String TAG = "CallLogSimInfoHelperOP09Extension";

    private Context mPluginContext;
    private OP09DialerPlugin mOP09DialerPlugin;

    // sim id of sip call in the call log database
    public static final int CALL_TYPE_SIP = -2;

    public CallLogSimInfoHelperOP09Extension(Context pluginContext, OP09DialerPlugin oP09DialerPlugin) {
        mPluginContext = pluginContext;
        mOP09DialerPlugin = oP09DialerPlugin;
    }

    /**
     * get sim name by sim id
     * 
     * @param simId from datebase
     * @return string sim name
     */
    public boolean getSimDisplayNameById(int simId, StringBuffer callDisplayName) {
        callDisplayName.append(" ");
        return true;
    }

    /**
     * get sim color drawable by sim id
     * 
     * @param simId form datebases
     * @return Drawable sim color
     */
    public boolean getSimColorDrawableById(int simId, Drawable drawableSimColor) {
        if (CALL_TYPE_SIP == simId) {
            // The request is sip color
            if (null == drawableSimColor) {
                Drawable dw = (Drawable) mPluginContext.getResources().getDrawable(R.drawable.dark_small_internet_call);
                if (null != dw) {
                    drawableSimColor = dw;
                } else {
                    drawableSimColor = null;
                }
            }
        } else {
            return false;
        }
        return true;
    }

    public boolean getSimBackgroundDarkResByColorId(int colorId, int[] simBackgroundDarkRes) {
        if (colorId < 0 || colorId > 3) {
            colorId = 0;
        }
        simBackgroundDarkRes[0] = SimInfoManager.SimBackgroundDarkSmallRes[colorId];
        return true;
    }

    private static void log(String msg) {
        Log.d(TAG, msg);
    }
}
