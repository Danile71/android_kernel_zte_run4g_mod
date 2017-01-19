package com.mediatek.op.telephony;

import android.content.ContentValues;
import android.content.Context;
import android.provider.Telephony;
import android.util.Log;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.internal.R;

public class TelephonyProviderExtOP02 extends TelephonyProviderExt {

    private static final String TAG = "TelephonyProviderExtOP02";
    private static final String CU_NUMERIC_1 = "46001";
    private static final String CU_NUMERIC_2 = "46009";
    private static final String CU_APN_NET = "3gnet";
    private static final String CU_APN_WAP = "3gwap";

    private Context mContext;
    private String mCustomizedApnNet;
    private String mCustomizedApnWap;

    public TelephonyProviderExtOP02(Context context) {
        mContext = context;
        if (FeatureOption.MTK_LTE_SUPPORT) {
            mCustomizedApnNet = mContext.getResources().getString(R.string.cu_3gnet_name_in_lte);
            mCustomizedApnWap = mContext.getResources().getString(R.string.cu_3gwap_name_in_lte);
        } else {
            mCustomizedApnNet = mContext.getResources().getString(R.string.cu_3gnet_name);
            mCustomizedApnWap = mContext.getResources().getString(R.string.cu_3gwap_name);
        }
        Log.d(TAG, "Constructor, mCustomizedApnNet=" + mCustomizedApnNet + ", mCustomizedApnWap=" + mCustomizedApnWap);
    }

    public int onLoadApns(ContentValues row) {
        if (row != null) {
            if ((row.get(Telephony.Carriers.NUMERIC) != null)
                    && (row.get(Telephony.Carriers.NUMERIC).equals(CU_NUMERIC_1)
                            || row.get(Telephony.Carriers.NUMERIC).equals(CU_NUMERIC_2))
                            && row.get(Telephony.Carriers.APN) != null) {
                if (row.get(Telephony.Carriers.APN).equals(CU_APN_NET)) {
                    row.put(Telephony.Carriers.NAME, mCustomizedApnNet);
                    return 1;
                } else if (row.get(Telephony.Carriers.APN).equals(CU_APN_WAP)) {
                    if (!(row.containsKey(Telephony.Carriers.TYPE) && (row.get(Telephony.Carriers.TYPE) != null)
                            && (row.get(Telephony.Carriers.TYPE).equals("mms")))) {
                        row.put(Telephony.Carriers.NAME, mCustomizedApnWap);
                        return 1;
                    }
                }
            }
        }
        return 0;
    }
}
