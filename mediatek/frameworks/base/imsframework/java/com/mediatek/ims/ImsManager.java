package com.mediatek.ims;


import android.content.Context;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.util.Log;

import com.mediatek.common.ims.IImsManager;
import com.mediatek.common.ims.IImsManagerService;
import com.mediatek.common.ims.mo.ImsIcsi;
import com.mediatek.common.ims.mo.ImsAuthInfo;
import com.mediatek.common.ims.mo.ImsLboPcscf;
import com.mediatek.common.ims.mo.ImsPhoneCtx;
import com.mediatek.common.ims.mo.ImsXcapInfo;

public final class ImsManager implements IImsManager {
    private static final String TAG = "ImsManager";
    private Context mContext;
    private IImsManagerService mService = null;
    
    /**
     * @hide
     */    
    public ImsManager(Context context) {
        mContext = context;
        int retryCount = 0;
        
        do{
            try{
                IBinder b = ServiceManager.getService(Context.IMS_SERVICE);
                mService = IImsManagerService.Stub.asInterface(b);
                if (mService == null) {
                    Log.e(TAG, "IBinder is null");
                    Thread.sleep(500); //500 ms
                    retryCount++;
                }
            }catch(Exception e){
                e.printStackTrace();
            }
        }while(mService == null && retryCount < 6);
    }

    public void setImsMoImpi(String impi){
        try {
            mService.setImsMoImpi(impi);
        } catch (RemoteException e) {
            return;
        }        
    }

    public void writeImsMoBool(int nodeId, boolean nodeValue){
        try {
            mService.writeImsMoBool(nodeId, nodeValue);
        } catch (RemoteException e) {
            return;
        }
    }
    
    public boolean readImsMoBool(int nodeId){
        try {
            return mService.readImsMoBool(nodeId);
        } catch (RemoteException e) {
            return false;
        }
    }

    public void writeImsMoInt(int nodeId, int nodeValue){
        try {
            mService.writeImsMoInt(nodeId, nodeValue);
        } catch (RemoteException e) {
            return;
        }        
    }
    
    public int readImsMoInt(int nodeId){
        try {
            return mService.readImsMoInt(nodeId);
        } catch (RemoteException e) {
            return 0;
        }          
    }

    public void writeImsMoString(int nodeId, String nodeValue){
        try {
            mService.writeImsMoString(nodeId, nodeValue);
        } catch (RemoteException e) {
            return;
        }        
    }
    
    public String readImsMoString(int nodeId){
        try {
            return mService.readImsMoString(nodeId);
        } catch (RemoteException e) {
            return null;
        }        
    }

    public void writeImsMoStringArray(int nodeId, String[] nodesValue){
        try {
            mService.writeImsMoStringArray(nodeId, nodesValue);
        } catch (RemoteException e) {
            return;
        }        
    }
    
    public String[] readImsMoStringArray(int nodeId){
        try {
            return mService.readImsMoStringArray(nodeId);
        } catch (RemoteException e) {
            return null;
        }        
    }

    public void writeImsIcsiMo(ImsIcsi[] nodesValue){
        try {
            mService.writeImsIcsiMo(nodesValue);
        } catch (RemoteException e) {
            return;
        }        
    }
    
    public ImsIcsi[] readImsIcsiMo(){
        try {
            return mService.readImsIcsiMo();
        } catch (RemoteException e) {
            return null;
        }        
    }
    
    public void writeImsAuthInfoMo(ImsAuthInfo nodesValue){
        try {
            mService.writeImsAuthInfoMo(nodesValue);
        } catch (RemoteException e) {
            return;
        }        
    }
    
    public ImsAuthInfo readImsAuthInfoMo(){
        try {
            return mService.readImsAuthInfoMo();
        } catch (RemoteException e) {
            return null;
        }        
    } 
    
    public void writeImsLboPcscfMo(ImsLboPcscf[] nodesValue){
        try {
            mService.writeImsLboPcscfMo(nodesValue);
        } catch (RemoteException e) {
            return;
        }        
    }
    
    public ImsLboPcscf[] readImsLboPcscfMo(){
        try {
            return mService.readImsLboPcscfMo();
        } catch (RemoteException e) {
            return null;
        }        
    } 
    
    public void writeImsImsPhoneCtxMo(ImsPhoneCtx[] nodesValue){
        try {
            mService.writeImsImsPhoneCtxMo(nodesValue);
        } catch (RemoteException e) {
            return;
        }        
    }
    
    public ImsPhoneCtx[] readImsPhoneCtxMo(){
        try {
            return mService.readImsPhoneCtxMo();
        } catch (RemoteException e) {
            return null;
        }        
    } 
    
    public void writeImsXcapInfoMo(ImsXcapInfo nodesValue){
        try {
            mService.writeImsXcapInfoMo(nodesValue);
        } catch (RemoteException e) {
            return;
        }        
    }
    
    public ImsXcapInfo readImsXcapInfoMo(){
        try {
            return mService.readImsXcapInfoMo();
        } catch (RemoteException e) {
            return null;
        }        
    }                       
    
}