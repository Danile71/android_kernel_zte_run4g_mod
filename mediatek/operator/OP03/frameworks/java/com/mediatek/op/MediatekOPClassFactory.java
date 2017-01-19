package com.mediatek.op;

import java.util.HashMap;
import java.util.Map;

import android.os.SystemClock;
import android.util.Log;

import com.mediatek.common.MediatekClassFactory;
import com.mediatek.common.media.IRCSePriorityExt;
import com.mediatek.common.telephony.ICallerInfoExt;
import com.mediatek.common.telephony.ITetheringExt;
import com.mediatek.common.telephony.IServiceStateExt;
import com.mediatek.common.telephony.ITelephonyExt;
import com.mediatek.common.telephony.IUsimPhoneBookManagerExt;
import com.mediatek.common.wifi.IWifiFwkExt;
import com.mediatek.common.telephony.IGsmDCTExt;

public final class MediatekOPClassFactory {

    private static final boolean DEBUG_PERFORMANCE = true;
    private static final boolean DEBUG_GETINSTANCE = true;
    private static final String TAG = "MediatekOPClassFactory";

    // mediatek-op.jar public interface map used for interface class matching.
    private static Map<Class, String> opInterfaceMap = new HashMap<Class, String>();
    static {
        opInterfaceMap.put(ITetheringExt.class, "com.mediatek.op.telephony.TetheringExtOP03");
        opInterfaceMap.put(IWifiFwkExt.class, "com.mediatek.op.wifi.WifiFwkExtOP03");
        opInterfaceMap.put(IServiceStateExt.class, "com.mediatek.op.telephony.ServiceStateExtOP03");
        opInterfaceMap.put(ITelephonyExt.class, "com.mediatek.op.telephony.TelephonyExtOP03");
        opInterfaceMap.put(ICallerInfoExt.class, "com.mediatek.op.telephony.CallerInfoPlugin");
        opInterfaceMap.put(IUsimPhoneBookManagerExt.class, "com.mediatek.op.telephony.UsimPhoneBookManagerExtOP03");
        opInterfaceMap.put(IRCSePriorityExt.class, "com.mediatek.op.media.RCSePriorityExtOP03");
        opInterfaceMap.put(IGsmDCTExt.class, "com.mediatek.op.telephony.GsmDCTExtOP03");        
    }

    /**
     * Get the op specific class name.
     * 
     * 
     */
    public static String getOpIfClassName(Class<?> clazz) {
        String ifClassName = null;

        if (opInterfaceMap.containsKey(clazz)) {
            ifClassName = opInterfaceMap.get(clazz);
        }

        return ifClassName;
    }
}
