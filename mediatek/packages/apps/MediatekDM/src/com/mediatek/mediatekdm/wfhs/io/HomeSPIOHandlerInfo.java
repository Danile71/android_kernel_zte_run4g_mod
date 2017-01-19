package com.mediatek.mediatekdm.wfhs.io;

import java.util.LinkedList;
import java.util.List;

import com.mediatek.mediatekdm.wfhs.MdmWifi;

public class HomeSPIOHandlerInfo  extends NodeIOHandlerInfo {
    
    @Override
    protected void fetchInfos() {
        localUri = "HomeSP";
        handlerName = ".WifiNodeIOHandler";

        preferSessionIds = new Integer[] {
                MdmWifi.SessionID.PROVISION_USERNAME_PASSWORD,
                MdmWifi.SessionID.PROVISION_CERTIFICATE,
                MdmWifi.SessionID.PROVISION_DEVICE_CERTIFICATE };

        preferNodes = new String[] { "FQDN" };
        preferKeys = new String[] { "homespfqdn" };
    } 
    
    @Override
    public List<String> getRegisterUris(String hotspot) {
        List<String> uris = new LinkedList<String>();
        for (String node : preferNodes) {
            String uri = String.format("%s/%s/%s", hotspot, localUri, node);
            uris.add(uri);
        }
        return uris;
    }
}
