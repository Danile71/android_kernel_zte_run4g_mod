
package com.mediatek.mediatekdm.volte.imsio;

import com.mediatek.ims.ImsManager;
import com.mediatek.mediatekdm.iohandler.PlainStringHandler;

public class RCSRealmHandler extends PlainStringHandler {

    private final ImsManager mImsManager;

    public RCSRealmHandler(String uri, ImsManager manager) {
        super(uri);
        mImsManager = manager;
    }

    @Override
    protected String readValue() {
        return mImsManager.readImsAuthInfoMo().getRelam();
    }

    @Override
    protected void writeValue(String value) {
        mImsManager.readImsAuthInfoMo().setRelam(value);
    }

}
