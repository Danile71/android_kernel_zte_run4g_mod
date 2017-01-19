package com.mediatek.imsframework.tests;

import android.content.Context;
import android.test.AndroidTestCase;
import android.util.Log;

import com.mediatek.common.ims.IImsManagerService;
import com.mediatek.common.ims.mo.ImsIcsi;
import com.mediatek.common.ims.mo.ImsAuthInfo;
import com.mediatek.common.ims.mo.ImsLboPcscf;
import com.mediatek.common.ims.mo.ImsPhoneCtx;
import com.mediatek.common.ims.mo.ImsXcapInfo;
import com.mediatek.ims.ImsManager;
import com.mediatek.ims.ImsConstants;

public class ImsManagerTest extends AndroidTestCase {
    private static final String TAG = "ImsManagerTest/IMSMO";
    
    private ImsManager mImsManager = null;
    
    @Override
    protected void setUp() throws Exception {        
        super.setUp();
        
        
        Thread.sleep(2000);
        mImsManager = (ImsManager) getContext().getSystemService(Context.IMS_SERVICE);        
            
        Log.i(TAG, "Get ImsManager");
        
        assertNotNull(mImsManager);
    }
    
    @Override
    protected void tearDown() throws Exception{
        super.tearDown();
    }
    
    public void testImsManagerMoActions() throws Exception {
        boolean bValue = true;
        int     iValue = 30;
        String  sValue = "test str";
        
        mImsManager.writeImsMoBool(ImsConstants.IMS_MO_SMS, bValue);
        assertEquals(bValue, mImsManager.readImsMoBool(ImsConstants.IMS_MO_SMS));

        mImsManager.writeImsMoBool(ImsConstants.IMS_MO_KEEPALIVE, !bValue);
        assertEquals(!bValue, mImsManager.readImsMoBool(ImsConstants.IMS_MO_KEEPALIVE));
        
        mImsManager.writeImsMoInt(ImsConstants.IMS_MO_TIMER_1, iValue);
        assertEquals(iValue, mImsManager.readImsMoInt(ImsConstants.IMS_MO_TIMER_1));

        mImsManager.writeImsMoInt(ImsConstants.IMS_MO_TIMER_2, iValue * 2);
        assertEquals(iValue * 2, mImsManager.readImsMoInt(ImsConstants.IMS_MO_TIMER_2));

        mImsManager.writeImsMoInt(ImsConstants.IMS_MO_TIMER_4, iValue * 3);
        assertEquals(iValue * 3, mImsManager.readImsMoInt(ImsConstants.IMS_MO_TIMER_4));
                        
        mImsManager.writeImsMoString(ImsConstants.IMS_MO_PCSCF, sValue);
        assertEquals(sValue, mImsManager.readImsMoString(ImsConstants.IMS_MO_PCSCF));

        mImsManager.writeImsMoBool(ImsConstants.IMS_MO_RESOURCE, bValue);
        assertEquals(bValue, mImsManager.readImsMoBool(ImsConstants.IMS_MO_RESOURCE));

        mImsManager.writeImsMoInt(ImsConstants.IMS_MO_VOICE_U, iValue * 5);
        assertEquals(iValue * 5, mImsManager.readImsMoInt(ImsConstants.IMS_MO_VOICE_U));

        mImsManager.writeImsMoInt(ImsConstants.IMS_MO_REG_BASE, iValue * 6);
        assertEquals(iValue * 6, mImsManager.readImsMoInt(ImsConstants.IMS_MO_REG_BASE));
        
        mImsManager.writeImsMoInt(ImsConstants.IMS_MO_REG_MAX, iValue * 7);
        assertEquals(iValue * 7, mImsManager.readImsMoInt(ImsConstants.IMS_MO_REG_MAX));        

        mImsManager.writeImsMoBool(ImsConstants.IMS_MO_MOBILITY, !bValue);
        assertEquals(!bValue, mImsManager.readImsMoBool(ImsConstants.IMS_MO_MOBILITY));

        ImsAuthInfo authObj = new ImsAuthInfo();
        authObj = new ImsAuthInfo("digest", "google.com", "gmail", "pass");        
        mImsManager.writeImsAuthInfoMo(authObj);
        
        ImsXcapInfo xcapObj = new ImsXcapInfo();
        xcapObj = new ImsXcapInfo("www.xcap.com", "gba", "gmail", "pass");        
        mImsManager.writeImsXcapInfoMo(xcapObj);
        
    }    

    public void testMobility(){
        boolean bValue = true;        
        mImsManager.writeImsMoBool(ImsConstants.IMS_MO_MOBILITY, !bValue);
        assertEquals(!bValue, mImsManager.readImsMoBool(ImsConstants.IMS_MO_MOBILITY));
    }

    public void testVoiceU(){                
        int     iValue = 1;
        mImsManager.writeImsMoInt(ImsConstants.IMS_MO_VOICE_U, iValue * 5);
        assertEquals(iValue * 5, mImsManager.readImsMoInt(ImsConstants.IMS_MO_VOICE_U));
    }

    public void testVoiceE(){
        int     iValue = 1;
        mImsManager.writeImsMoInt(ImsConstants.IMS_MO_VOICE_E, iValue * 4);
        assertEquals(iValue * 4, mImsManager.readImsMoInt(ImsConstants.IMS_MO_VOICE_E));
    }

    public void testIcsList() throws Exception {        
        String icsi = "ICSI";
        boolean bValue = true;
        int count = 4;
        int i = 0;

        ImsIcsi[] objs = new ImsIcsi[count];
        ImsIcsi[] readObjs = new ImsIcsi[count];

        for(i = 0; i < count; i++){
            icsi = "ICSI" + "*" + i;
            objs[i] = new ImsIcsi(icsi, bValue);
        }

        mImsManager.writeImsIcsiMo(objs);
        readObjs = mImsManager.readImsIcsiMo();
        
        i = 0;
        for(ImsIcsi sObj:readObjs){ 
            icsi = "ICSI" + "*" + i;
            Log.d(TAG, "ICSI:" + sObj);
            assertEquals(icsi, sObj.getIcsi());
            assertEquals(bValue, sObj.getIsAllocated());
            i++;
    }
    
        for(i = 0; i < count; i++){
            icsi = "ICSI" + "00" + i;
            objs[i].setIcsi(icsi);
            objs[i].setIsAllocated(!bValue);
        }
        mImsManager.writeImsIcsiMo(objs);
        readObjs = mImsManager.readImsIcsiMo();

        i = 0;
        for(ImsIcsi sObj:readObjs){
            icsi = "ICSI" + "00" + i;
            Log.d(TAG, "ICSI:" + sObj);
            assertEquals(icsi, sObj.getIcsi());
            assertEquals(!bValue, sObj.getIsAllocated());
            i++;
        }
    }
    
    public void testLboPcscf() throws Exception {        
        String pcscf = "pcscf";        
        int count = 4;
        int i = 0;

        ImsLboPcscf[] objs = new ImsLboPcscf[count];
        ImsLboPcscf[] readObjs = new ImsLboPcscf[count];

        readObjs = mImsManager.readImsLboPcscfMo();
        if(readObjs.length == 0){
           assertNotNull(null);
           return;
        }
        

        for(i = 0; i < count; i ++){
            pcscf = "pcscf" + "*" + i;
            objs[i] = new ImsLboPcscf(pcscf, pcscf + "_type");
        }

        mImsManager.writeImsLboPcscfMo(objs);
        readObjs = mImsManager.readImsLboPcscfMo();

        i = 0;
        for(ImsLboPcscf sObj:readObjs){ 
            Log.d(TAG, "ImsLboPcscf:" + sObj);
            pcscf = "pcscf" + "*" + i;
            assertEquals(pcscf, sObj.getLboPcscfAddress());
            assertEquals(pcscf + "_type", sObj.getLboPcscfAddressType());
            i++;
        }

        for(i = 0; i < count; i++){
            pcscf = "pcscf" + "00" + i;
            objs[i].setLboPcscfAddress(pcscf);
            objs[i].setLboPcscfAddressType(pcscf + "00type");
        }
        mImsManager.writeImsLboPcscfMo(objs);
        readObjs = mImsManager.readImsLboPcscfMo();

        i = 0;
        for(ImsLboPcscf sObj:readObjs){
            Log.d(TAG, "ImsLboPcscf:" + sObj);
            pcscf = "pcscf" + "00" + i;          
            assertEquals(pcscf, sObj.getLboPcscfAddress());
            assertEquals(pcscf + "00type", sObj.getLboPcscfAddressType());
            i++;
        }
    }

    public void testImsAuthInfo() throws Exception {        
        int i = 0;

        ImsAuthInfo objs = new ImsAuthInfo("IMS Security" + i, "sip doamin" + i, "user" + i, "password" + i);
        mImsManager.writeImsAuthInfoMo(objs);

        ImsAuthInfo sObj = mImsManager.readImsAuthInfoMo();
                
        Log.d(TAG, "testImsAuthInfo:" + sObj);            
        assertEquals("IMS Security" + i, sObj.getAuthType());
        assertEquals("sip doamin" + i,   sObj.getRelam());
        assertEquals("user" + i,         sObj.getUserName());
        assertEquals("password" + i,     sObj.getUserPwd());


        objs.setAuthType(i + "IMS Security");
        objs.setRelam(i + "sip doamin");
        objs.setUserName(i + "user");
        objs.setUserPwd(i + "password");

        mImsManager.writeImsAuthInfoMo(objs);
        sObj = mImsManager.readImsAuthInfoMo();

        assertEquals(i + "IMS Security", sObj.getAuthType());
        assertEquals(i + "sip doamin",   sObj.getRelam());
        assertEquals(i + "user",         sObj.getUserName());
        assertEquals(i + "password",     sObj.getUserPwd());
        }

    public void testImsXcapInfo() throws Exception {
        int i = 0;

        ImsXcapInfo objs = new ImsXcapInfo("XCAP_ROOT_" + i, "XCAP_USER_" + i, "XCAP_PASSWD_" + i, "XCAP_AUTH_" + i);
        mImsManager.writeImsXcapInfoMo(objs);
        ImsXcapInfo sObj = mImsManager.readImsXcapInfoMo();

        Log.d(TAG, "testImsXcapInfo:" + sObj);            
        assertEquals("XCAP_ROOT_" + i,   sObj.getXcapRootUri());
        assertEquals("XCAP_USER_" + i,  sObj.getXcapAuth());
        assertEquals("XCAP_PASSWD_" + i,   sObj.getXcapAuthSecret());
        assertEquals("XCAP_AUTH_" + i, sObj.getXcapAuthType());

        objs.setXcapRootUri(i + "XCAP_ROOT_");
        objs.setXcapAuth(i + "XCAP_USER_");
        objs.setXcapAuthSecret(i + "XCAP_PASSWD_");
        objs.setXcapAuthType(i + "XCAP_AUTH_");

        mImsManager.writeImsXcapInfoMo(objs);
        sObj = mImsManager.readImsXcapInfoMo();

        Log.d(TAG, "testImsXcapInfo:" + sObj);            
        assertEquals(i + "XCAP_ROOT_",   sObj.getXcapRootUri());
        assertEquals(i + "XCAP_USER_",  sObj.getXcapAuth());
        assertEquals(i + "XCAP_PASSWD_",   sObj.getXcapAuthSecret());
        assertEquals(i + "XCAP_AUTH_", sObj.getXcapAuthType());
    }
    
    public void testImsPhoneCtx() throws Exception {
        
        ImsPhoneCtx[] phoneCtxArray = mImsManager.readImsPhoneCtxMo();
        if (phoneCtxArray == null){
            Log.e(TAG,"[PhoneCtx][write] readImsPhoneCtxMo is NULL!!!");
            return;
        }
        
        ImsPhoneCtx phoneCtx = null;
        String phoneCtxStr = null;
        String [] userID = null;
        int index = 0;
        int pcLength = phoneCtxArray.length;
        
        Log.i(TAG, "[write]pcLength: " + pcLength);
        
        for(;index < pcLength; index++){
            Log.i(TAG, "write PhoneCtx " + index);
            phoneCtxStr = "phoneCtx-" + index;                                        
            userID = phoneCtxArray[index].getPhoneCtxIpuis();
            int userIdLength = userID.length;
            for (int j = 0; j < userIdLength; ++j) {                 
                Log.i(TAG, "write userID " + j);
                userID[j] = "userId-" + j;                                 
                
            }
            phoneCtx = new ImsPhoneCtx(phoneCtxStr, userID);
            Log.i(TAG, "phoneCtx:" + phoneCtx);
            phoneCtxArray[index] = phoneCtx;
            mImsManager.writeImsImsPhoneCtxMo(phoneCtxArray);                
        }

        ImsPhoneCtx[] newPhoneCtxArray = mImsManager.readImsPhoneCtxMo();
        pcLength = newPhoneCtxArray.length;
        
        for(;index < pcLength; index++){
            Log.i(TAG, "read PhoneCtx " + index);
            phoneCtxStr = "phoneCtx-" + index;         
            assertEquals(phoneCtxStr, newPhoneCtxArray[index].getPhoneCtx());
            userID = newPhoneCtxArray[index].getPhoneCtxIpuis();
            int userIdLength = userID.length;
            for (int j = 0; j < userIdLength; ++j) {
                Log.i(TAG, "read userID " + j);
                userID[j] = "userId-" + j;
                assertEquals("userId-" + j, userID[j]);
            }            
        }
    }
    
    private void getImsInfo(String impi){
        mImsManager.setImsMoImpi(impi);

        String pcscf = mImsManager.readImsMoString(ImsConstants.IMS_MO_PCSCF);
        Log.d(TAG, "PCSCF:" + pcscf);
        ImsAuthInfo authObj = mImsManager.readImsAuthInfoMo();
        Log.d(TAG, "ImsAuthInfo:" + authObj);
        ImsXcapInfo xcapObj = mImsManager.readImsXcapInfoMo();
        Log.d(TAG, "ImsXcapInfo:" + xcapObj);
        ImsLboPcscf[] pcscfs = mImsManager.readImsLboPcscfMo();

        for(ImsLboPcscf lbopcscf:pcscfs){
            Log.d(TAG, "ImsLboPcscf:" + lbopcscf);
        }        
    }
}