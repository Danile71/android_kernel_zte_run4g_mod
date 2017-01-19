package com.mediatek.op.telephony;

import android.content.Context;
import android.text.TextUtils;

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.RetryManager;
import com.android.internal.telephony.dataconnection.DcFailCause;
import com.mediatek.common.telephony.IGsmDCTExt;


public class GsmDCTExtOP10 extends GsmDCTExt {
    private Context mContext;

    public GsmDCTExtOP10(Context context) {
    }

    public boolean isDataAllowedAsOff(String apnType) {
        if (TextUtils.equals(apnType, PhoneConstants.APN_TYPE_MMS)) {
            return true;
        }
        return false;
    }
    
    public boolean getFDForceFlag(boolean force_flag) {
        return force_flag;
    }

    @Override
    public boolean needSmRetry(Object cause){
        DcFailCause tmpcause = (DcFailCause) cause;
        log("[OP] Check sm cause:" +  tmpcause);
        if (tmpcause == DcFailCause.USER_AUTHENTICATION ||
            tmpcause == DcFailCause.SERVICE_OPTION_NOT_SUBSCRIBED){            
            return true;                    
        }
                    
        return false;
    }
    @Override
    public boolean doSmRetry(Object cause, Object obj1, Object obj2){
        log("[OP] doSmRetry: setSmRetryConfig");  
        setSmRetryConfig(obj1);
        return true;
    }    
    @Override
    public boolean setSmRetryConfig(Object retryManager){
        RetryManager rm = (RetryManager) retryManager;     
        int maxRetryCount = 2;
        int retryTime = 45000;
        int randomizationTime = 0;
        int retryCount = rm.getRetryCount();
        log("[OP] set SmRetry Config:" + maxRetryCount + "/" + retryTime + "/" + randomizationTime);
        rm.configure(maxRetryCount, retryTime, randomizationTime);
        rm.setRetryCount(retryCount);        
        return true;
    }    
    @Override
    public boolean needRacUpdate(){
        return true;
    }    
    
}
