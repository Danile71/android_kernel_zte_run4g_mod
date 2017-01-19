package com.mediatek.mediatekdm.wfhs.io;

import com.mediatek.mediatekdm.wfhs.MdmWifi;

public class AAAServerTrustRootIOHandlerInfo extends NodeIOHandlerInfo {

    @Override
    protected void fetchInfos() {
        localUri = "AAAServerTrustRoot";
        handlerName = ".WifiNodeIOHandler";

        preferSessionIds = new Integer[] { 
                MdmWifi.SessionID.PROVISION_CERTIFICATE };

        preferNodes = new String[] { "CertURL", "CertSHA256Fingerprint"};
        preferKeys = new String[] { "aaacerturl", "aaacertsha256fingerprint"};
    }
}
