package com.mediatek.op.telephony;

import android.content.Context;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.telephony.IUiccControllerExt;

public class UiccControllerExtOP09 extends UiccControllerExt {
    // UIM for CT Display
    public String getMissingTitle(Context context, int simId) {
        String strTitle = null;
        if (FeatureOption.MTK_GEMINI_SUPPORT == true) {
            if(PhoneConstants.GEMINI_SIM_1 == simId ){
                strTitle = context.getText(com.mediatek.internal.R.string.sim_missing_slot1_card).toString();
            }else{
                strTitle = context.getText(com.mediatek.internal.R.string.sim_missing_slot2_card).toString();
            }
        }else{
            strTitle = context.getText(com.mediatek.internal.R.string.sim_missing_card).toString();
        }

        return strTitle;
    }

    public CharSequence getMissingDetail(Context context) {
        CharSequence strTitle = context.getText(com.mediatek.internal.R.string.sim_missing_detail_card);
        return strTitle;
    }
}
