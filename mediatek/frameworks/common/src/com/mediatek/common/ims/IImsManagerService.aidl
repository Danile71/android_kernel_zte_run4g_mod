package com.mediatek.common.ims;

import android.os.Bundle;

import com.mediatek.common.ims.mo.ImsIcsi;
import com.mediatek.common.ims.mo.ImsAuthInfo;
import com.mediatek.common.ims.mo.ImsLboPcscf;
import com.mediatek.common.ims.mo.ImsPhoneCtx;
import com.mediatek.common.ims.mo.ImsXcapInfo;

interface IImsManagerService {

    //MO read/write API
    void writeImsMoBool(int nodeId, boolean nodeValue);
    boolean readImsMoBool(int nodeId);

    void writeImsMoInt(int nodeId, int nodeValue);
    int readImsMoInt(int nodeId);    

    void writeImsMoString(int nodeId, String nodeValue);
    String   readImsMoString(int nodeId);

    void writeImsMoStringArray(int nodeId, in String[] nodesValue);
    String[] readImsMoStringArray(int nodeId);

    void writeImsIcsiMo(in ImsIcsi[] nodesValue);
    ImsIcsi[] readImsIcsiMo();

    void writeImsAuthInfoMo(in ImsAuthInfo nodeValue);
    ImsAuthInfo readImsAuthInfoMo();
    
    void writeImsLboPcscfMo(in ImsLboPcscf[] nodesValue);
    ImsLboPcscf[] readImsLboPcscfMo();
    
    void writeImsImsPhoneCtxMo(in ImsPhoneCtx[] nodesValue);
    ImsPhoneCtx[] readImsPhoneCtxMo();
    
    void writeImsXcapInfoMo(in ImsXcapInfo nodeValue);
    ImsXcapInfo readImsXcapInfoMo();
    
    void setImsMoImpi(String impi);
}
