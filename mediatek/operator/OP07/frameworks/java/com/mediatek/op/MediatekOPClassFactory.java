package com.mediatek.op;

import java.util.HashMap;
import java.util.Map;

import android.os.SystemClock;
import android.util.Log;

import com.mediatek.common.MediatekClassFactory;
import com.mediatek.common.telephony.IGsmConnectionExt;
import com.mediatek.common.telephony.IGsmDCTExt;
//import com.mediatek.common.telephony.ITetheringExt;
import com.mediatek.common.telephony.IServiceStateExt;
import com.mediatek.common.telephony.ITelephonyExt;

public final class MediatekOPClassFactory {

    private static final boolean DEBUG_PERFORMANCE = true;
    private static final boolean DEBUG_GETINSTANCE = true;
    private static final String TAG = "MediatekOPClassFactory";

    // mediatek-op.jar public interface map used for interface class matching.
    private static Map<Class, String> opInterfaceMap = new HashMap<Class, String>();
    static {
        opInterfaceMap.put(IGsmConnectionExt.class, "com.mediatek.op.telephony.GsmConnectionExtOP07");
        opInterfaceMap.put(IServiceStateExt.class, "com.mediatek.op.telephony.ServiceStateExtOP07");
        opInterfaceMap.put(ITelephonyExt.class, "com.mediatek.op.telephony.TelephonyExtOP07");                
        opInterfaceMap.put(IGsmDCTExt.class, "com.mediatek.op.telephony.GsmDCTExtOP07");
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
