
package com.mediatek.mediatekdm.volte.imsio;

import com.mediatek.common.ims.mo.ImsPhoneCtx;
import com.mediatek.ims.ImsManager;
import com.mediatek.mediatekdm.iohandler.PlainStringHandler;

public class PhoneContextIdentityHandler extends PlainStringHandler {

    private final ImsManager mImsManager;
    private final int mIndex;

    public PhoneContextIdentityHandler(String uri, ImsManager manager, int index) {
        super(uri);
        mImsManager = manager;
        mIndex = index;
    }

    @Override
    protected String readValue() {
        return mImsManager.readImsPhoneCtxMo()[mIndex].getPhoneCtxIpuis()[0];
    }

    @Override
    protected void writeValue(String value) {
        ImsPhoneCtx[] values = mImsManager.readImsPhoneCtxMo();
        values[mIndex].setPhoneCtxIpuis(new String[]{value});
        mImsManager.writeImsImsPhoneCtxMo(values);
    }

}
