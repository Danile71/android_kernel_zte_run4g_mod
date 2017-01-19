package com.mediatek.op.telephony;

import android.util.Log;
import android.content.Context;
import com.mediatek.common.telephony.IServiceStateExt;
import java.util.Arrays;

public class ServiceStateExtOP03 extends ServiceStateExt {
    private static final short[] OrangeInUK = {0x5B86,  0x5B87,  0x5B88,  0x5B89,  0x5B8A};
    
    public ServiceStateExtOP03() {
    }

    public ServiceStateExtOP03(Context context) {
    }

    public boolean isRegCodeRoaming(boolean originIsRoaming, int mccmnc, String numeric) {
        if(originIsRoaming && numeric != null && numeric != ""){
            if(isInOrange(mccmnc) && isInOrange(Integer.parseInt(numeric))) {
                return false;
            }
        }
        return originIsRoaming;
    }

    private boolean isInOrange(int mcc_mnc){
        int index = Arrays.binarySearch(OrangeInUK, (short)mcc_mnc);
        if(index < 0) return false;
        return true;
    }
}
