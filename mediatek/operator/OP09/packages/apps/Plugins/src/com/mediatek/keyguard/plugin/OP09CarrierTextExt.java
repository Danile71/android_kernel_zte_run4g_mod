package com.mediatek.keyguard.plugin;

import com.mediatek.keyguard.ext.DefaultCarrierTextExt;

public class OP09CarrierTextExt extends DefaultCarrierTextExt {

    private static final String CT_PLMN = "china telecom";

    public CharSequence customizeCarrierTextCapital(CharSequence carrierText) {
        if (carrierText.toString().equalsIgnoreCase(CT_PLMN)) {
            return carrierText;
        } else {
            return carrierText.toString().toUpperCase();
        }
    }

    @Override
    public boolean showCarrierTextWhenSimMissing(boolean isSimMissing, int simId) {
        return false;
    }
}
