package com.mediatek.op.telephony;

import android.content.Context;
import com.mediatek.common.telephony.IGsmDCTExt;


public class GsmDCTExtOP07 extends GsmDCTExt {
    private Context mContext;

    public GsmDCTExtOP07(Context context) {
    }

    public boolean isDomesticRoamingEnabled() {
        return true;
    }
}
