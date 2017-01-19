
package com.mediatek.mediatekdm.volte.imsio;

import android.util.Log;

import com.mediatek.common.ims.mo.ImsIcsi;
import com.mediatek.ims.ImsManager;
import com.mediatek.mediatekdm.DmConst.TAG;
import com.mediatek.mediatekdm.mdm.MdmException;
import com.mediatek.mediatekdm.mdm.NodeIoHandler;

public class ICSIModeHandler implements NodeIoHandler {

    private final ImsManager mImsManager;
    private byte[] mWriteCache = null;
    private final String mUri;
    private final int mIndex;

    public ICSIModeHandler(String uri, ImsManager manager, int index) {
        mImsManager = manager;
        mUri = uri;
        mIndex = index;
    }

    @Override
    public int read(int offset, byte[] data) throws MdmException {
        Log.d(TAG.NODEIOHANDLER, "+ICSIModeHandler.read(" + mUri + ", " + offset + ", " + data + ")");
        Log.d(TAG.NODEIOHANDLER, "ICSIModeHandler.read: mIndex = " + mIndex);
        ImsIcsi icsi = mImsManager.readImsIcsiMo()[mIndex];
        byte[] result = (icsi.getIsAllocated() ? "true" : "false").getBytes();
        if (data != null) {
            System.arraycopy(result, offset, data, 0, data.length);
        }
        Log.d(TAG.NODEIOHANDLER, "-ICSIModeHandler.read(" + result.length + ")");
        return result.length;
    }

    @Override
    public void write(int offset, byte[] data, int totalSize) throws MdmException {
        Log.d(TAG.NODEIOHANDLER, "+ICSIModeHandler.write(" + mUri + ", " + offset + ", " + data + ", " + totalSize + ")");

        if (offset == 0 && data.length == totalSize) {
            boolean value = Boolean.parseBoolean(new String(data));
            ImsIcsi[] values = mImsManager.readImsIcsiMo();
            values[mIndex].setIsAllocated(value);
            mImsManager.writeImsIcsiMo(values);
        } else {
            if (offset == 0 && data.length == 0 && totalSize != 0) {
                mWriteCache = new byte[totalSize];
            } else {
                System.arraycopy(data, 0, mWriteCache, offset, data.length);
                if (offset + data.length == totalSize) {
                    // NOTE: totalSize maybe a little smaller than mWriteCache at this moment.
                    //       Check document for write() for details.
                    boolean value = Boolean.parseBoolean(new String(mWriteCache, 0, totalSize));
                    ImsIcsi[] values = mImsManager.readImsIcsiMo();
                    values[mIndex].setIsAllocated(value);
                    mImsManager.writeImsIcsiMo(values);
                    mWriteCache = null;
                }
            }
        }
        Log.d(TAG.NODEIOHANDLER, "-ICSIModeHandler.write()");
    }

}
