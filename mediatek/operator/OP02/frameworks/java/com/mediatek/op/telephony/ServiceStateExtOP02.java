package com.mediatek.op.telephony;

import android.content.Context;
import android.content.res.Resources;
import android.telephony.TelephonyManager;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.telephony.TelephonyManagerEx;
import com.android.internal.telephony.PhoneFactory;
import android.telephony.SignalStrength;
import android.util.Log;
/// M: The OP02 implementation of ServiceState.
public class ServiceStateExtOP02 extends ServiceStateExt {
    public ServiceStateExtOP02() {
    }

    public ServiceStateExtOP02(Context context) {
    }

    @Override
    public String getEccPlmnValue() {
        int state = 0;

        //[ALPS01673673]-Start: move this check to AP side to fix display wrong sim info.
        //boolean hasNoSimCard = true;
        //[ALPS01673673]-End

        for (int i = 0; i < PhoneConstants.GEMINI_SIM_NUM; i++) {
            state = TelephonyManagerEx.getDefault().getSimState(i);
            if (state == TelephonyManager.SIM_STATE_READY) {
                return Resources.getSystem().getText(com.android.internal.R.string.emergency_calls_only).toString();
            }
            //[ALPS01673673]-Start: move this check to AP side to fix display wrong sim info.
            //hasNoSimCard = hasNoSimCard && (state == TelephonyManager.SIM_STATE_ABSENT);
        }
        //[ALPS01673673]-Start: move this check to AP side to fix display wrong sim info.
        //if (hasNoSimCard) {
        //    return Resources.getSystem().getText(com.android.internal.R.string.lockscreen_missing_sim_message_short).toString();
        //}
        //[ALPS01673673]-End
        
        return Resources.getSystem().getText(com.android.internal.R.string.lockscreen_carrier_default).toString();
    }

    //[ALPS01558804] MTK-START: send notification for using some spcial icc card    
    @Override
    public boolean needIccCardTypeNotification(String iccCardType){
        if (PhoneFactory.isLteSupport() && (iccCardType.equals("SIM"))){
            return true;      
        }
        return false;
    }
    //[ALPS01558804] MTK-END: send notification for using some spcial icc card    

    //[ALPS01706187]MTK-START: Modify signal level's mapping rule 
    @Override
    public int mapLteSignalLevel(int mLteRsrp, int mLteRssnr, int mLteSignalStrength){        
        int rsrpIconLevel;
        
        if (mLteRsrp < -140 || mLteRsrp > -44) rsrpIconLevel = SignalStrength.SIGNAL_STRENGTH_NONE_OR_UNKNOWN;
        else if (mLteRsrp >= -97) rsrpIconLevel = SignalStrength.SIGNAL_STRENGTH_GREAT;
        else if (mLteRsrp >= -105) rsrpIconLevel = SignalStrength.SIGNAL_STRENGTH_GOOD;
        else if (mLteRsrp >= -113) rsrpIconLevel = SignalStrength.SIGNAL_STRENGTH_MODERATE;
        else if (mLteRsrp >= -120) rsrpIconLevel = SignalStrength.SIGNAL_STRENGTH_POOR;
        else rsrpIconLevel = SignalStrength.SIGNAL_STRENGTH_NONE_OR_UNKNOWN;
        Log.i(TAG,"op02_mapLteSignalLevel=" + rsrpIconLevel); 
        return rsrpIconLevel;
    }
    //[ALPS01706187]MTK-START: Modify signal level's mapping rule 
}
