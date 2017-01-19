package com.mediatek.mediatekdm.wfhs.io;

import com.mediatek.mediatekdm.wfhs.MdmWifi;

import java.util.LinkedList;
import java.util.List;

public class CredentialIOHandlerInfo extends NodeIOHandlerInfo {

    @Override
    protected void fetchInfos() {
        localUri = "Credential";
        handlerName = ".WifiNodeIOHandler";

        preferSessionIds = new Integer[] {
                MdmWifi.SessionID.PROVISION_USERNAME_PASSWORD,
                MdmWifi.SessionID.PROVISION_CERTIFICATE,
                MdmWifi.SessionID.PROVISION_DEVICE_CERTIFICATE,
                MdmWifi.SessionID.REMEDIATION_MACHINE_USERNAME_PASSWORD,
                MdmWifi.SessionID.REMEDIATION_USER_USERNAME_PASSWORD };

        preferNodes = new String[] { "Realm", "UsernamePassword/Username", "UsernamePassword/Password", "UsernamePassword/EAPMethod/EAPType", "UsernamePassword/EAPMethod/InnerEAPMethod" };
        preferKeys = new String[] { "realm", "username", "newpassword", "eapmethod", "innereapmethod" };
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
