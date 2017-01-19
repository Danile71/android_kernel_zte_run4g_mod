
package com.mediatek.mediatekdm.volte.imsio;

import com.mediatek.common.ims.mo.ImsLboPcscf;
import com.mediatek.ims.ImsManager;
import com.mediatek.mediatekdm.iohandler.PlainStringHandler;

public class PCSCFAddressHandler extends PlainStringHandler {

    private final ImsManager mImsManager;
    private final int mIndex;

    public PCSCFAddressHandler(String uri, ImsManager manager, int index) {
        super(uri);
        mImsManager = manager;
        mIndex = index;
    }

    @Override
    protected String readValue() {
        return mImsManager.readImsLboPcscfMo()[mIndex].getLboPcscfAddress();
    }

    @Override
    protected void writeValue(String value) {
        ImsLboPcscf[] values = mImsManager.readImsLboPcscfMo();
        values[mIndex].setLboPcscfAddress(value);
        mImsManager.writeImsLboPcscfMo(values);
    }

}
