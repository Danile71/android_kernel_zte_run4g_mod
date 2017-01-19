
package com.mediatek.mediatekdm.test.server;

import com.mediatek.mediatekdm.test.SimpleXmlAccessor;

public class SimpleSyncMLAccessor extends SimpleXmlAccessor {
    
    public String getCredential() {
        return getNodeContent("/SyncML/SyncHdr/Cred/Data");
    }
    
    public void setCredential(String value) {
        setNodeContent("/SyncML/SyncHdr/Cred/Data", value);
    }
    
    public String getSourceLocURI() {
        return getNodeContent("/SyncML/SyncHdr/Source/LocURI");
    }
    
    public void setTargetLocURI(String value){
        setNodeContent("/SyncML/SyncHdr/Target/LocURI", value);
    }
}
