
package com.mediatek.op;

import com.mediatek.common.bootanim.IBootAnimExt;
import com.mediatek.common.lowstorage.ILowStorageExt;
import com.mediatek.common.server.IConnectivityRoamingExt;
import com.mediatek.common.sms.IConcatenatedSmsFwkExt;
import com.mediatek.common.telephony.IPhoneNumberExt;
import com.mediatek.common.telephony.IServiceStateExt;
import com.mediatek.common.telephony.ITelephonyExt;
import com.mediatek.common.telephony.ITelephonyProviderExt;
import com.mediatek.common.telephony.IUiccControllerExt;
import com.mediatek.common.telephony.internationalroaming.strategy.ICardStrategy;
import com.mediatek.common.telephony.internationalroaming.strategy.IDataStrategy;
import com.mediatek.common.telephony.internationalroaming.strategy.IGeneralStrategy;
import com.mediatek.common.telephony.internationalroaming.strategy.INetworkSelectionStrategy;
import com.mediatek.common.util.IPatterns;
import com.mediatek.common.util.IWebProtocolNames;

import java.util.HashMap;
import java.util.Map;

public final class MediatekOPClassFactory {

    private static final boolean DEBUG_PERFORMANCE = true;
    private static final boolean DEBUG_GETINSTANCE = true;
    private static final String TAG = "MediatekOP09ClassFactory";

    // mediatek-op.jar public interface map used for interface class matching.
    private static Map<Class, String> sOpInterfaceMap = new HashMap<Class, String>();
    static {
        sOpInterfaceMap.put(IBootAnimExt.class, "com.mediatek.op.bootanim.BootAnimExtOP09");
        sOpInterfaceMap.put(IPatterns.class, "com.mediatek.op.util.OP09Patterns");
        sOpInterfaceMap.put(IWebProtocolNames.class, "com.mediatek.op.util.OP09WebProtocolNames");
        sOpInterfaceMap.put(IServiceStateExt.class, "com.mediatek.op.telephony.ServiceStateExtOP09");
        sOpInterfaceMap.put(IUiccControllerExt.class, "com.mediatek.op.telephony.UiccControllerExtOP09");
        sOpInterfaceMap.put(IConcatenatedSmsFwkExt.class,"com.mediatek.op.sms.ConcatenatedSmsFwkExtOP09");
        sOpInterfaceMap.put(IPhoneNumberExt.class, "com.mediatek.op.telephony.PhoneNumberExtOP09");
        sOpInterfaceMap.put(ICardStrategy.class, 
                "com.mediatek.op.telephony.internationalroaming.strategy.OP09CardStrategy");
        sOpInterfaceMap.put(IDataStrategy.class, 
                "com.mediatek.op.telephony.internationalroaming.strategy.OP09DataStrategy");
        sOpInterfaceMap.put(IGeneralStrategy.class, 
                "com.mediatek.op.telephony.internationalroaming.strategy.OP09GeneralStrategy");
        sOpInterfaceMap.put(INetworkSelectionStrategy.class, 
                "com.mediatek.op.telephony.internationalroaming.strategy.OP09NetworkSelectionStrategy");
        sOpInterfaceMap.put(ILowStorageExt.class,
                "com.mediatek.op.lowstorage.LowStorageExtOP09");
        sOpInterfaceMap.put(ITelephonyProviderExt.class,
                "com.mediatek.op.telephony.OP09TelephonyProviderExt");
        sOpInterfaceMap.put(IConnectivityRoamingExt.class, 
                "com.mediatek.op.server.OP09ConnectivityRoamingExt");
        sOpInterfaceMap.put(ITelephonyExt.class, "com.mediatek.op.telephony.OP09TelephonyExt");
    }

    /**
     * Get the op specific class name.
     * 
     * 
     */
    public static String getOpIfClassName(Class<?> clazz) {
        String ifClassName = null;

        if (sOpInterfaceMap.containsKey(clazz)) {
            ifClassName = sOpInterfaceMap.get(clazz);
        }

        return ifClassName;
    }
}
