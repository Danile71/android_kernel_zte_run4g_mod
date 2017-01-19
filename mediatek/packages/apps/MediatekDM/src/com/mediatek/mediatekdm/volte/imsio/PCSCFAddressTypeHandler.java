
package com.mediatek.mediatekdm.volte.imsio;

import com.mediatek.common.ims.mo.ImsLboPcscf;
import com.mediatek.ims.ImsManager;
import com.mediatek.mediatekdm.iohandler.PlainStringHandler;

public class PCSCFAddressTypeHandler extends PlainStringHandler {

    private final ImsManager mImsManager;
    private final int mIndex;

    public PCSCFAddressTypeHandler(String uri, ImsManager manager, int index) {
        super(uri);
        mImsManager = manager;
        mIndex = index;
    }

    @Override
    protected String readValue() {
        return mImsManager.readImsLboPcscfMo()[mIndex].getLboPcscfAddressType();
    }

    @Override
    protected void writeValue(String value) {
        ImsLboPcscf[] values = mImsManager.readImsLboPcscfMo();
        values[mIndex].setLboPcscfAddressType(value);
        mImsManager.writeImsLboPcscfMo(values);
    }

}
