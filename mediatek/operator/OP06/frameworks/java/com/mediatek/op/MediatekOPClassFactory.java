
package com.mediatek.op;

import java.util.HashMap; //import java.util.List;
import java.util.Map;

import android.os.SystemClock;
import android.util.Log;

import com.mediatek.common.MediatekClassFactory;
import com.mediatek.common.media.IRCSePriorityExt;
import com.mediatek.common.search.IRegionalPhoneSearchEngineExt;
import com.mediatek.common.telephony.IUsimPhoneBookManagerExt;

public final class MediatekOPClassFactory {

    private static final boolean DEBUG_PERFORMANCE = true;
    private static final boolean DEBUG_GETINSTANCE = true;
    private static final String TAG = "MediatekOP06ClassFactory";

    // mediatek-op.jar public interface map used for interface class matching.
    private static Map<Class, String> opInterfaceMap = new HashMap<Class, String>();
    static {
        opInterfaceMap.put(IRegionalPhoneSearchEngineExt.class,
            "com.mediatek.op.search.RegionalPhoneSearchEngineExt");
        opInterfaceMap.put(IUsimPhoneBookManagerExt.class, "com.mediatek.op.telephony.UsimPhoneBookManagerExtOP06");
        opInterfaceMap.put(IRCSePriorityExt.class, "com.mediatek.op.media.RCSePriorityExtOP06");

    }


    /**
     * Get the op specific class name.
     * 
     * 
     */
    public static String getOpIfClassName(Class<?> clazz) {
        String ifClassName = null;
        Log.d(TAG, clazz.toString());
        if (opInterfaceMap.containsKey(clazz)) {
            ifClassName = opInterfaceMap.get(clazz);
        }

        return ifClassName;
    }
}
