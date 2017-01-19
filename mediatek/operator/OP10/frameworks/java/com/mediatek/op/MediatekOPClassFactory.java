
package com.mediatek.op;

import java.util.HashMap; //import java.util.List;
import java.util.Map;

import android.os.SystemClock;
import android.util.Log;

import com.mediatek.common.MediatekClassFactory;
import com.mediatek.common.telephony.IGsmDCTExt;

public final class MediatekOPClassFactory {

    private static final boolean DEBUG_PERFORMANCE = true;
    private static final boolean DEBUG_GETINSTANCE = true;
    private static final String TAG = "MediatekOP10ClassFactory";

    // mediatek-op.jar public interface map used for interface class matching.
    private static Map<Class, String> opInterfaceMap = new HashMap<Class, String>();
    static {
        opInterfaceMap.put(IGsmDCTExt.class, "com.mediatek.op.telephony.GsmDCTExtOP10");
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
