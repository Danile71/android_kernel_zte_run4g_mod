
package com.mediatek.mediatekdm.volte.imsio;

import com.mediatek.common.ims.mo.ImsPhoneCtx;
import com.mediatek.ims.ImsManager;
import com.mediatek.mediatekdm.iohandler.PlainStringHandler;

public class PhoneContextHandler extends PlainStringHandler {

    private final ImsManager mImsManager;
    private final int mIndex;

    public PhoneContextHandler(String uri, ImsManager manager, int index) {
        super(uri);
        mImsManager = manager;
        mIndex = index;
    }

    @Override
    protected String readValue() {
        return mImsManager.readImsPhoneCtxMo()[mIndex].getPhoneCtx();
    }

    @Override
    protected void writeValue(String value) {
        ImsPhoneCtx[] values = mImsManager.readImsPhoneCtxMo();
        values[mIndex].setPhoneCtx(value);
        mImsManager.writeImsImsPhoneCtxMo(values);
    }

}
