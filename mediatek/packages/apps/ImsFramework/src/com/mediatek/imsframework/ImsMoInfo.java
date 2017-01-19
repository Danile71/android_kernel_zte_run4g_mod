package com.mediatek.imsframework;

import android.util.Log;

import com.mediatek.common.ims.mo.ImsLboPcscf;
import com.mediatek.common.ims.mo.ImsAuthInfo;
import com.mediatek.common.ims.mo.ImsXcapInfo;

import java.lang.StringBuilder;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;

/*
 * @hide
 */

public final class ImsMoInfo {
    static final String TAG = "ImsFrameworkService";

    private static final int MAX_MO_COUNT = 4;
    private String mImpi;
    private String mPcscf;
    private Collection<ImsLboPcscf> mImsLboPcscfList = new ArrayList<ImsLboPcscf>();
    private ImsAuthInfo mImsAuthInfo;
    private ImsXcapInfo mImsXcapInfo;

    public static final ImsMoInfo[] EMPTY_ARRAY = new ImsMoInfo[0];

    public ImsMoInfo() {
        clear();
    }

    public ImsMoInfo(String impi) {
        clear();
        mImpi = impi;
    }

    public void setPcscf(String pcscf) {
        mPcscf = pcscf;
    }

    public void addLboPcscfList(ImsLboPcscf imsLobPcscf) {

        if (imsLobPcscf != null) mImsLboPcscfList.add(imsLobPcscf);
    }

    public void removeLboPcscfList() {
        mImsLboPcscfList.clear();
    }

    public void setImsAuthInfo(ImsAuthInfo imsAuthInfo) {
        mImsAuthInfo = imsAuthInfo;
    }

    public void setImsXcapInfo(ImsXcapInfo imsXcapInfo) {
        mImsXcapInfo = imsXcapInfo;
    }

    public String toString() {
        StringBuilder sBuffer = new StringBuilder("ImsMoInfo:");
        sBuffer.append("mImpi:" + mImpi + " mPcscf:" + mPcscf + " " + "mImsLboPcscfList:" + mImsLboPcscfList.size());

        return sBuffer.toString();
    }

    public String getImpi() {
        return mImpi;
    }

    public String getPcscf() {
        return mPcscf;
    }

    public Collection<ImsLboPcscf> getLboPcscfList() {
        return Collections.unmodifiableCollection(mImsLboPcscfList);
    }

    public ImsAuthInfo getImsAuthInfo() {
        return mImsAuthInfo;
    }

    public ImsXcapInfo getImsXcapInfo() {
        return mImsXcapInfo;
    }

    public void clear() {
        int i = 0;

        mImpi = "";
        mPcscf = "";

        mImsLboPcscfList.clear();
        for(i = 0; i < MAX_MO_COUNT; i++) {
            mImsLboPcscfList.add(new ImsLboPcscf());
        }

        mImsAuthInfo = new ImsAuthInfo();
        mImsXcapInfo = new ImsXcapInfo();
    }

}

