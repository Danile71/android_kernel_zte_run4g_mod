
package com.mediatek.mediatekdm.volte.imsio;

import com.mediatek.common.ims.mo.ImsIcsi;
import com.mediatek.ims.ImsManager;
import com.mediatek.mediatekdm.iohandler.PlainStringHandler;

public class ICSIHandler extends PlainStringHandler {

    private final ImsManager mImsManager;
    private final int mIndex;

    public ICSIHandler(String uri, ImsManager manager, int index) {
        super(uri);
        mImsManager = manager;
        mIndex = index;
    }

    @Override
    protected String readValue() {
        return mImsManager.readImsIcsiMo()[mIndex].getIcsi();
    }

    @Override
    protected void writeValue(String value) {
        ImsIcsi[] values = mImsManager.readImsIcsiMo();
        values[mIndex].setIcsi(value);
        mImsManager.writeImsIcsiMo(values);
    }

}
