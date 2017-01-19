package com.mediatek.mediatekdm.wfhs.io;

import com.mediatek.mediatekdm.wfhs.MdmWifi;

public class PreferredRoamingPartnerIOHandlerInfo extends NodeIOHandlerInfo {

    @Override
    protected void fetchInfos() {
        localUri = "Policy/PreferredRoamingPartnerList";
        handlerName = ".WifiNodeIOHandler";

        preferSessionIds = new Integer[] { 
                MdmWifi.SessionID.POLICY_UPDATE_USERNAME_PASSWORD };

        preferNodes = new String[] { "FQDN_match", "Priority", "Country" };
        preferKeys = new String[] { "fqdnmatch", "priority", "country" };
    }
}
