package com.mediatek.common.ims;

import com.mediatek.common.ims.mo.ImsIcsi;
import com.mediatek.common.ims.mo.ImsAuthInfo;
import com.mediatek.common.ims.mo.ImsLboPcscf;
import com.mediatek.common.ims.mo.ImsPhoneCtx;
import com.mediatek.common.ims.mo.ImsXcapInfo;

public interface IImsManager{

    //MO read/write API

    void writeImsMoBool(int nodeId, boolean nodeValue);
    boolean readImsMoBool(int nodeId);

    void writeImsMoInt(int nodeId, int nodeValue);
    int readImsMoInt(int nodeId);    

    void writeImsMoString(int nodeId, String nodeValue);
    String   readImsMoString(int nodeId);

    void writeImsMoStringArray(int nodeId, String[] nodesValue);
    String[] readImsMoStringArray(int nodeId);

    void writeImsIcsiMo(ImsIcsi[] nodesValue);
    ImsIcsi[] readImsIcsiMo();

    void writeImsAuthInfoMo(ImsAuthInfo nodeValue);
    ImsAuthInfo readImsAuthInfoMo();
    
    void writeImsLboPcscfMo(ImsLboPcscf[] nodesValue);
    ImsLboPcscf[] readImsLboPcscfMo();
    
    void writeImsImsPhoneCtxMo(ImsPhoneCtx[] nodesValue);
    ImsPhoneCtx[] readImsPhoneCtxMo();
    
    void writeImsXcapInfoMo(ImsXcapInfo nodeValue);
    ImsXcapInfo readImsXcapInfoMo();

    void setImsMoImpi(String impi);

}