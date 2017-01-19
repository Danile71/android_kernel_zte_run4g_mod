package com.mediatek.op.telephony;

import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.Uri;
import android.provider.Telephony;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.internal.R;
import com.mediatek.xlog.Xlog;

import java.util.HashMap;

/**
 * OP09TelephonyProviderExt is OP09's implementation of ITelephonyProviderExt,
 * it has the only API onLoadApns() which is executed when telephony db was
 * initialized. Additionally, this plugin class has a BraodcastReceiver to
 * listen Locale Changed and then update the CT related APN Strings when Locale
 * Changed.
 */
public class OP09TelephonyProviderExt extends TelephonyProviderExt {

    private static final String TAG = "OP09TelephonyProviderExt";
    private static final String CT_APN_NET = "ctnet";
    private static final String CT_APN_WAP = "ctwap";
    private static final Uri GEMINI_URI = Telephony.Carriers.SIM1Carriers.CONTENT_URI;
    private static final Uri GEMINI_URI_2 = Telephony.Carriers.SIM2Carriers.CONTENT_URI;

    // onApnsLoad will modify all items with numeric in CT_NUMERIC_ARRAY.
    // when locale change, will update db with mcc in CT_MCC_ARRAY.
    // 460 for china mainland CDMA, 455 for Macao CDMA, 204 for VODA GSM

    private static final String[] CT_NUMERIC_ARRAY = { "46003", "46012", 
        "46013", "45502", "20404" };
    private static final String[] CT_MCC_ARRAY = { "460", "455", "204" };

    private Context mContext;
    private HashMap<String, Integer> mUpdateTable = new HashMap<String, Integer>();

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Xlog.d(TAG, "[op09]received intent ==> " + action);
            if (Intent.ACTION_LOCALE_CHANGED.equals(action)) {
                setOpApnName();
            }
        }
    };

    /**
     * The constructor class has three funcs: 1) pass in Context from
     * PluginManager; 2) initialze APN-String updateTable; 3) register
     * BroadcastReceiver to listen to ACTION_LOCALE_CHANGED.
     */
    public OP09TelephonyProviderExt(Context context) {
        mContext = context;
        
        // Every apn item whose name equals with 'ctnet' or 'ctwap' that meet
        // condition should update its UI String.
        
        mUpdateTable.put(CT_APN_NET, R.string.ct_net_name);
        mUpdateTable.put(CT_APN_WAP, R.string.ct_wap_name);

        // Register a broadcast to listen locale changed for updating APN names.

        IntentFilter filter = new IntentFilter(Intent.ACTION_LOCALE_CHANGED);
        mContext.registerReceiver(mReceiver, filter);
    }

    @Override
    public int onLoadApns(ContentValues row) {
        if (row == null || !checkRowByNumeric(row)) {
            return -1;
        }
        for (String key : mUpdateTable.keySet()) {
            if (key.equals(row.get(Telephony.Carriers.APN))) {
                row.put(Telephony.Carriers.NAME, getApnNameByKey(key));
                Xlog.d(TAG, "[op09]onLoadApns update key = " + key + ", name = "
                        + getApnNameByKey(key));
                break;
            }
        }
        return 1;
    }

    private void setOpApnName() {
        Xlog.d(TAG, "[Op09]setOpApnName for language changed");
        ContentResolver updateResolver = mContext.getContentResolver();

        for (String mcc : CT_MCC_ARRAY) {
            for (String key : mUpdateTable.keySet()) {
                updateApnName(updateResolver, getSelection(key, mcc),
                        getApnNameByKey(key));
                Xlog.d(TAG, "[op09]setOpApnName, sql = " + getSelection(key, mcc)
                        + "name = " + getApnNameByKey(key));
            }
        }
    }

    private void updateApnName(ContentResolver upddateResolver,
            String selection, String apnName) {
        ContentValues updateValues = new ContentValues(1);
        updateValues.put(Telephony.Carriers.NAME, apnName);

        upddateResolver.update(GEMINI_URI, updateValues, selection, null);
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            upddateResolver.update(GEMINI_URI_2, updateValues, selection, null);
        }
    }

    private String getSelection(String apn, String mcc) {
        if (CT_APN_NET.equals(apn)) {
            return "apn = 'ctnet' and mcc = '" + mcc
                    + "' and type like '%default%' and sourcetype <> 1";
        } else if (CT_APN_WAP.equals(apn)) {
            return "apn = 'ctwap' and mcc = '" + mcc
                    + "' and type like '%mms%' and sourcetype <> 1";
        } else {
            return null;
        }
    }

    private boolean checkRowByNumeric(ContentValues row) {
        if (row != null) {
            for (String item : CT_NUMERIC_ARRAY) {
                if (item.equals(row.get(Telephony.Carriers.NUMERIC))) {
                    return true;
                }
            }
        }
        return false;
    }

    private String getApnNameByKey(String key) {
        return mContext.getResources().getString(mUpdateTable.get(key));
    }

}
