/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2012. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
package com.mediatek.passpoint;

//import com.mediatek.passpoint.SSLSocketTest;
import com.mediatek.common.passpoint.*;

import android.os.*;
import android.util.Log;
import android.net.Uri;
import android.net.wifi.*;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.Context;
import android.content.BroadcastReceiver;
import android.security.Credentials;

import com.android.org.conscrypt.TrustManagerImpl;
// import com.android.org.bouncycastle.ocsp.BasicOCSPResp;
// import com.android.org.bouncycastle.ocsp.OCSPException;
// import com.android.org.bouncycastle.ocsp.OCSPReq;
// import com.android.org.bouncycastle.ocsp.OCSPResp;
// import com.android.org.bouncycastle.asn1.ocsp.OCSPResponseStatus;
// import com.android.org.bouncycastle.ocsp.SingleResp;
import com.android.org.bouncycastle.asn1.*;
import com.android.org.bouncycastle.asn1.util.ASN1Dump;
import com.android.org.bouncycastle.asn1.pkcs.PKCSObjectIdentifiers;
import com.android.org.bouncycastle.asn1.x509.*;
import com.android.org.bouncycastle.jce.PKCS10CertificationRequest;
import com.android.org.bouncycastle.jce.provider.BouncyCastleProvider;
import com.android.org.bouncycastle.jce.exception.ExtCertPathValidatorException;

import java.util.*;
import java.util.zip.GZIPInputStream;

import java.math.BigInteger;

import com.mediatek.ksoap2.*;
import com.mediatek.ksoap2.serialization.*;
import com.mediatek.ksoap2.transport.*;
import com.mediatek.kobjects.base64.Base64;

import org.w3c.dom.*;
import org.xml.sax.*;
import org.apache.harmony.security.x509.OtherName;
import org.apache.http.auth.UsernamePasswordCredentials;
import org.apache.http.HttpResponse;
import org.apache.http.message.BasicHeader;
import org.apache.http.Header;

import java.io.*;
import java.net.*;

import javax.net.ssl.*;
import javax.xml.parsers.*;
import javax.xml.transform.TransformerException;

import java.security.cert.*;
import java.security.*;
import java.security.KeyStore.PasswordProtection;

import javax.security.auth.callback.*;
import javax.security.auth.x500.X500Principal;

public class SoapClient implements PasspointClient {
    private static final String TAG = "SoapClient";
    private static final String TAG2 = "SoapClAdvance";

    //private static final String NAMESPACE_NS = "http://www.wi-fi-org/specifications/hotspot2dot0/spp/1.0/";
    private static final String NAMESPACE_NS = "http://www.wi-fi.org/specifications/hotspot2dot0/v1.0/spp";
    private static final String NAMESPACE_DM = "http://www.openmobilealliance.org/tech/DTD/dm_ddf-v1_2.dtd";
    private static final String WIFI_SOAP_POST_DEV_DATA = "sppPostDevData";
    private static final String WIFI_SOAP_UPDATE_RESPONSE = "sppUpdateResponse";
    private static final String WIFI_SOAP_USER_INPUT_RESPONSE = "sppUserInputResponse";
    private static final String WIFI_SOAP_ERROR_CODE = "errorCode";
    private static final String WIFI_SOAP_MGMTREE = "MgmtTree";        
    private static final String WIFI_SOAP_REQ_REASON = "requestReason";
    private static final String WIFI_SOAP_REDIRECTURL = "redirectURI";
    private static final String WIFI_SOAP_SESSIONID = "sessionID";
    private static final String WIFI_SOAP_SYNCML_DMDDF_1_2 = "syncml:dmddf1.2";
    private static final String WIFI_SOAP_S_SPP_VERSION = "supportedSPPVersions";
    private static final String WIFI_SOAP_S_MOLIST = "supportedMOList";
    private static final String WIFI_SOAP_S_MODEVINFO = "urn:oma:mo:oma-dm-devinfo:1.0";
    private static final String WIFI_SOAP_S_MODEVDETAIL = "urn:oma:mo:oma-dm-devdetail:1.0";
    private static final String WIFI_SOAP_S_MOSUBSCRIPTION = "urn:wfa:mo:hotspot2dot0-perprovidersubscription:1.0";
    //private static final String WIFI_SOAP_S_MOHS20 = "urn:wfa:mo-ext:hotspot2dot0-Wi-Fi:1.0";
    private static final String WIFI_SOAP_S_MOHS20 = "urn:wfa:mo-ext:hotspot2dot0-devdetail-ext:1.0";
    private static final String WIFI_SOAP_MO_CONTAINER = "moContainer";
    private static final String WIFI_SOAP_MO_URN = "moURN";
    private static final String WIFI_SOAP_MO_XMLNS = "xmlns"; 
    private static final String WIFI_SOAP_SPP_VERSION = "sppVersion";
    private static final String WIFI_SOAP_SPP_EXCHANGE_COMPLETE = "sppExchangeComplete";
    private static final String WIFI_SOAP_SPP_ERROR = "sppError";  
    private static final String WIFI_SOAP_SPP_NOMOUPDATE = "noMOUpdate";    
    private static final String WIFI_SOAP_SPP_STATUS = "sppStatus";    
    private static final String WIFI_SOAP_SPP_STATUS_OK = "OK";
    private static final String WIFI_SOAP_SPP_STATUS_PROVISION_COMPLETE = "Provisioning complete, request sppUpdateResponse";
    private static final String WIFI_SOAP_SPP_STATUS_REMEDIATION_COMPLETE = "Remediation complete, request sppUpdateResponse";
    private static final String WIFI_SOAP_SPP_STATUS_UPDATE_COMPLETE = "Update complete, request sppUpdateResponse";
    private static final String WIFI_SOAP_SPP_STATUS_NO_UPDATE_AVAILABLE = "No update available at this time";
    private static final String WIFI_SOAP_SPP_STATUS_EXCHANGE_COMPLETE = "Exchange complete, release TLS connection";
    private static final String WIFI_SOAP_SPP_STATUS_ERROR_OCCURRED = "Error occurred";
    private static final String DEVICE_OBJECT = "The interior node holding all devinfo objects";
    private static final String CONTENT_TYPE_XML_CHARSET_UTF_8 = "text/xml;charset=utf-8";
    private static final String CONTENT_TYPE_SOAP_XML_CHARSET_UTF_8 = "application/soap+xml;charset=utf-8";    
    public static final String REDIRECT_URL = "http://127.0.0.1";
    public int OSU_REDIRECT_PORT;
    public int REM_REDIRECT_PORT;
    //Subscription Provisioning request reason
    public static final String SUB_REGISTER = "Subscription registration";
    public static final String CERT_ENROLL_SUCCESS = "Certificate enrollment completed";    
    public static final String CERT_ENROLL_FAIL = "Certificate enrollment failed";
    public static final String USER_INPUT_COMPLETED = "User input completed";
    public static final String SUB_REMEDIATION = "Subscription remediation";
    public static final String SUB_PROVISION = "Subscription provisioning"; //for EAP-SIM
    public static final String SUB_MO_UPLOAD = "MO upload";

    //Policy Provisioning request reason
    public static final String POL_UPDATE = "Policy update";
    public static final String POL_MO_UPLOAD = "MO upload";

    //send response error message
    public static final String WIFI_SOAP_UPDATE_RESPONSE_OK = "OK";
    public static final String WIFI_SOAP_UPDATE_RESPONSE_PERMISSION_DENY = "Permission denied";
    public static final String WIFI_SOAP_UPDATE_RESPONSE_COMMAND_FAILED = "Command failed";
    public static final String WIFI_SOAP_UPDATE_RESPONSE_MO_ADD_UPDATE_FAIL = "MO addition or update failed";
    public static final String WIFI_SOAP_UPDATE_RESPONSE_DEVICE_FULL = "Device full";
    public static final String WIFI_SOAP_UPDATE_RESPONSE_BAD_MGMTREE_URI = "Bad management tree URI";
    public static final String WIFI_SOAP_UPDATE_RESPONSE_REQUEST_ENTITY_TOO_LARGE = "Requested entity too large";
    public static final String WIFI_SOAP_UPDATE_RESPONSE_COMMAND_NOT_ALLOWED = "Command not allowed";
    public static final String WIFI_SOAP_UPDATE_RESPONSE_COMMAND_NOT_EXEC_DUE_TO_USER = "Command not executed due to user";
    public static final String WIFI_SOAP_UPDATE_RESPONSE_NOT_FOUND = "Not found";
    
    private String soapUrl = "https://10.123.103.1:9443/OnlineSignup/services";
    
    private String CMCServerUrl = "http://10.123.103.103:8180/ejbca/publicweb/enroll/cmc/simpleEnroll";

    private String SOAP_ACTION = "";
    private String METHOD_NAME = "";

    public static final int SUBSCRIPTION_PROVISION = 1;
    public static final int SUBSCRIPTION_REMEDIATION = 2;
    public static final int POLICY_UPDATE = 3;
    public static final int SUBSCRIPTION_SIM_PROVISION = 4;
    long sleepTime = 100;  //MS
    int requestreason = 0;

    private Context    mContext;

    //HTTP digest
    private static String digestUsername;
    private static String digestPassword;

    private static PasspointPpsmoParser ppsmoParser;
    
    // ./Wi-Fi/xxxxx/PerProviderSubscription in managementTreeURI
    private static String wifiSPFQDN;
    
    //Settion ID
    private String SessionID = "";
      
    //SubscriptionUpdate
    private static String SubscriptionDMAccUsername;
    private static String SubscriptionDMAccPassword;

    //Policy
    //  PolicyUpdate
    private static String PolicyDMAccUsername;
    private static String PolicyDMAccPassword;

    //Proto port tuple
    //private static Vector<WifiSubscriptionMo.WifiMORequiredProtoPortTupleList> rpptLists = new Vector<WifiSubscriptionMo.WifiMORequiredProtoPortTupleList>();

    //backhaul
    //private static Vector<WifiSubscriptionMo.WifiMOMinBackhaulThresholdList> bhtLists = new Vector<WifiSubscriptionMo.WifiMOMinBackhaulThresholdList>();
    
    //HomeSP
    private static String  HomeOI;
    private static boolean HomeOIRequired;
    //private static Vector<WifiSubscriptionMo.WifiMOHomeOIList> HomeOILists = new Vector<WifiSubscriptionMo.WifiMOHomeOIList>();    
    //private static Vector<String> OtherHomePartners = new Vector<String>();    

    private String requestReason;
    private String moDevInfo;
    private String moDevDetail;
    private String moSubscription;

    private Handler msgHandler;

    //Credential
    private static String  mSoapWebUrl;
    private static String  mOSUServerUrl;
    private static String  mREMServerUrl;
    private static String  mImsi;
    private String mEnrollmentServerURI;
    private String mEnrollmentServerCert;

    private static String mOSUFriendlyName;
   
    TrustManager[] myTrustManagerArray = new TrustManager[]{new CertTrustManager()};
    private static KeyStore hs20KeyStore;
    private static KeyStore hs20PKCS12KeyStore;
    private String mSPFQDN;
    private String mSPFQDNFromMo;
    private static String mOSULanguage;
    private static String iconFileName;
    private static String iconHash;
    private static String enrollDigestUsername;
    private static String enrollDigestPassword;
    private PasspointCertificate mPasspointCertificate;
    private static final int NO_CLIENT_AUTH = 0;
    private static final int CLIENT_CERT = 1;
    private static final int RENEGOTIATE_CERT = 2;
    private static String providerIssuerName; //pre-provisioned certificate by a provider

    private WifiTree mSoapTree;
    private WifiTreeHelper mTreeHelper;
    private PasspointCredential mCred;
    
    //UploadMO
    private static String mUploadMO;
    
    //soap dump
    private String timeString = "";

    private static int procedureDone;
    private static int managementTreeUpdateCount = 0;
    private static boolean isOsuEmMode = false;

    private BroadcastReceiver mStateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            int responseCode = 0;            
            Log.d(TAG, "<<< onReceive:" + action);

            if ("com.mediatek.soap.provision.end".equals(action)) {
                String cmd = intent.getStringExtra("cmd");                
                int result = intent.getIntExtra("result", -1);

                if("addsubtree".equals(cmd)){
                    if(isOsuEmMode){
                        isOsuEmMode = false;
                        if (result == 0) { //success 
                            Bundle b = intent.getExtras();
                            WifiTree obj = b.getParcelable("wifitree");
                            mTreeHelper.dumpTreeInfo(obj);
                            mSoapTree = obj;                        
                            responseCode = 0;
                        }else{
                            responseCode = 3;
                        }
                        Message msg = msgHandler.obtainMessage(PasspointManager.EVENT_PRE_PROVISION_DONE, responseCode, 0, mSoapTree);//"MO addition or update failed"
                        msg.sendToTarget();                                                                                    
                    }else{
                        if (result == 0) { //success 
                            Bundle b = intent.getExtras();
                            WifiTree obj = b.getParcelable("wifitree");
                            mTreeHelper.dumpTreeInfo(obj);                            
                            mSoapTree = obj;
                            responseCode = 0;                            
                        }else{//fail
                            Log.d(TAG, "DM addsubtree fail !!!"); 
                            responseCode = 1;
                        }
                        Message msg = msgHandler.obtainMessage(PasspointManager.EVENT_PROVISION_DONE, responseCode, 0, mSoapTree);//"MO addition or update failed"
                        msg.sendToTarget();                                                                                    
                    }
                }
            } else if ("com.mediatek.soap.policy.end".equals(action)) {
                String cmd = intent.getStringExtra("cmd");                
                int result = intent.getIntExtra("result", -1);
                
                if("replacesubtree".equals(cmd)){
                    if (result == 0) { //success 
                        managementTreeUpdateCount--;
                        if(managementTreeUpdateCount == 0){
                            Bundle b = intent.getExtras();
                            WifiTree obj = b.getParcelable("wifitree");
                            mSoapTree = obj;
                            responseCode = 0; 

                            Message msg = msgHandler.obtainMessage(PasspointManager.EVENT_POLICY_PROVISION_DONE, responseCode, 0, mSoapTree);
                            msg.sendToTarget();                                                    
                            
                        }else{
                            Log.d(TAG, "managementTreeUpdateCount: " + managementTreeUpdateCount); 
                        }
                    }else{//fail
                        Log.d(TAG, "DM replacesubtree fail !!!"); 
                        responseCode = 3;

                        Message msg = msgHandler.obtainMessage(PasspointManager.EVENT_POLICY_PROVISION_DONE, responseCode, 0, mSoapTree);
                        msg.sendToTarget();                                                                            
                    }
                }
                
            } else if("com.mediatek.soap.remediation.end".equals(action)){
                String cmd = intent.getStringExtra("cmd");                
                int result = intent.getIntExtra("result", -1);
                
                if("replacesubtree".equals(cmd)){
                    if (result == 0) { //success 
                        managementTreeUpdateCount--;
                        if(managementTreeUpdateCount == 0){
                            Bundle b = intent.getExtras();
                            WifiTree obj = b.getParcelable("wifitree");
                            mSoapTree = obj;
                            responseCode = 0;    
                            Message msg = msgHandler.obtainMessage(PasspointManager.EVENT_REMEDIATION_DONE, responseCode, 0, mSoapTree);//"MO addition or update failed"
                            msg.sendToTarget();                                                    
                            
                        }else{
                            Log.d(TAG, "managementTreeUpdateCount: " + managementTreeUpdateCount); 
                        }
                    }else{//fail
                        Log.d(TAG, "DM replacesubtree fail !!!"); 
                        responseCode = 3;

                        Message msg = msgHandler.obtainMessage(PasspointManager.EVENT_REMEDIATION_DONE, responseCode, 0, mSoapTree);//"MO addition or update failed"
                        msg.sendToTarget();                                                    
                        
                    }
                }

            } else if ("com.mediatek.soap.sim.provision.end".equals(action)) {
                String cmd = intent.getStringExtra("cmd");                
                int result = intent.getIntExtra("result", -1);
                
                if("addsubtree".equals(cmd)){
                    if (result == 0) { //success 
                        Bundle b = intent.getExtras();
                        WifiTree obj = b.getParcelable("wifitree");
                        mSoapTree = obj;
                        responseCode = 0;                                                
                    }else{//fail
                        Log.d(TAG, "DM addsubtree fail !!!"); 
                        responseCode = 3;
                    }
                    Message msg = msgHandler.obtainMessage(PasspointManager.EVENT_SIM_PROVISION_DONE, responseCode, 0, mSoapTree);
                    msg.sendToTarget();                                                    
                }
            } else if (PasspointStateMachine.HS20_BROWSER_OSUREDIRECT_MO_REQUEST.equals(action)) {
                startSubscriptionProvision(mOSUServerUrl, SoapClient.USER_INPUT_COMPLETED);
            } else if (PasspointStateMachine.HS20_BROWSER_REMREDIRECT_MO_REQUEST.equals(action)) {
                startRemediation(mREMServerUrl, SoapClient.USER_INPUT_COMPLETED, null);
            }
        };
    };

    private class DataStreamOut {
        public void dump(String data, String file) {
            try { 
                DataOutputStream dataOutputStream = 
                    new DataOutputStream( 
                             new FileOutputStream("/data/" + file)); 
                
                   dataOutputStream.writeUTF(data); 
                   dataOutputStream.flush(); 
                   dataOutputStream.close();                 
            } 
            catch(IOException e) { 
                e.printStackTrace(); 
            } 
        }
    } 

    public SoapClient() {
        init();
    }

    public SoapClient(Context context, Handler handler) {
        mContext = context;
        msgHandler = handler;
        mTreeHelper = new WifiTreeHelper();
        init();

        final IntentFilter stateFilter = new IntentFilter();
        stateFilter.addAction("com.mediatek.soap.provision.end");
        stateFilter.addAction("com.mediatek.soap.remediation.end");
        stateFilter.addAction("com.mediatek.soap.policy.end");
        stateFilter.addAction("com.mediatek.soap.sim.provision.end");   
        stateFilter.addAction("com.mediatek.soap.provision.end");
        stateFilter.addAction(PasspointStateMachine.HS20_BROWSER_OSUREDIRECT_MO_REQUEST);
        stateFilter.addAction(PasspointStateMachine.HS20_BROWSER_REMREDIRECT_MO_REQUEST);        
        mContext.registerReceiver(mStateReceiver, stateFilter);
        mPasspointCertificate = PasspointCertificate.getInstance(null);
        
    }
  
    public void init() {
        Log.d(TAG,"[init]");        
        requestReason = null;
        moDevInfo = null;
        moDevDetail = null;
        moSubscription = null;

        digestUsername = "";
        digestPassword = "";
                       
        //SubscriptionUpdate
        SubscriptionDMAccUsername = "";
        SubscriptionDMAccPassword = "";
        
        //Policy
        // Proto port tuple
        //rpptLists.clear();
        // Backhaul
        //bhtLists.clear();
        
        //  PolicyUpdate
        PolicyDMAccUsername = "";
        PolicyDMAccPassword = "";

        //HomeSP
        HomeOI = "";
        HomeOIRequired = false;
        //HomeOILists.clear();
        //OtherHomePartners.clear();
        
        //Credential
        mSoapWebUrl = "";
        mOSUServerUrl = "";
        mREMServerUrl = "";

        //Certificate
        mPasspointCertificate = PasspointCertificate.getInstance(null);
    }

    public void initProvisioning(String reqReason, String devInfo, String devDetail, String subscription, Handler handler) {
        requestReason = reqReason;
        moDevInfo = devInfo;
        moDevDetail = devDetail;
        moSubscription = subscription;

        //setUpdateHandler(handler);
    }

    public void setWifiTree(WifiTree tree){
        mSoapTree = tree;
    }
    
    private void setRemediationHttpDigest(){
        Log.d(TAG, "[setRemediationHttpDigest]");
        if ("true".equals(SystemProperties.get("persist.service.manual.ut.rem"))) return;
        digestUsername = "";
        digestPassword = "";
        String eapType = mCred.getEapMethod();
        Log.d(TAG, "eapType = " + eapType);
        if (eapType.equals("TTLS")) {
            String subscriptionDMAccUsername = mCred.getSubscriptionDMAccUsername();
            String subscriptionDMAccPassword = mCred.getSubscriptionDMAccPassword();
            String credentialUsername = mCred.getUserName();
            String credentialPassword = mCred.getPassword();  
            
            if (subscriptionDMAccUsername != null) {
                Log.d(TAG, "subscriptionDMAccUsername = " + subscriptionDMAccUsername + ", subscriptionDMAccPassword = " + subscriptionDMAccPassword);
                if(!subscriptionDMAccUsername.isEmpty()){
                    digestUsername = subscriptionDMAccUsername;
                    digestPassword = subscriptionDMAccPassword;            
                    Log.d(TAG, "digest using Subscription Update DMAcc, digestUsername/digestPassword: " + digestUsername + "/" +digestPassword);                                                                
                }else if(credentialUsername != null && !credentialUsername.isEmpty()){
                    Log.d(TAG, "credentialUsername = " + credentialUsername + ", credentialPassword = " + credentialPassword);
                    digestUsername = credentialUsername;
                    digestPassword = credentialPassword;
                    Log.d(TAG, "digest using credential, digestUsername/digestPassword: " + digestUsername + "/" +digestPassword);                                                                        
                }            
            } else if (credentialUsername != null && !credentialUsername.isEmpty()) {
                Log.d(TAG, "credentialUsername = " + credentialUsername + ", credentialPassword = " + credentialPassword);
                digestUsername = credentialUsername;
                digestPassword = credentialPassword;
                Log.d(TAG, "digest using credential, digestUsername/digestPassword: " + digestUsername + "/" +digestPassword);                                                                        
            }
        }            
    }

    private void setPolicyUpdateHttpDigest(){
        digestUsername = "";
        digestPassword = "";

        String eapType = mCred.getEapMethod();
        if (eapType.equals("TTLS")) {
            String policyDMAccUsername = mCred.getPolicyDMAccUsername();
            String policyDMAccPassword = mCred.getPolicyDMAccPassword();
            String credentialUsername = mCred.getUserName();
            String credentialPassword = mCred.getPassword();  

            if(policyDMAccUsername != null){
                if(!policyDMAccUsername.isEmpty()){
                    digestUsername = policyDMAccUsername;
                    digestPassword = policyDMAccPassword;            
                    Log.d(TAG, "digest using Policy Update DMAcc, digestUsername/digestPassword: " + digestUsername + "/" +digestPassword);                                                                            
                }else if(credentialUsername != null && !credentialUsername.isEmpty()){                    
                    digestUsername = credentialUsername;
                    digestPassword = credentialPassword;
                    Log.d(TAG, "digest using credential, digestUsername/digestPassword: " + digestUsername + "/" +digestPassword);                                                                        
                }
            }else if(credentialUsername != null && !credentialUsername.isEmpty()){
                digestUsername = credentialUsername;
                digestPassword = credentialPassword;
                Log.d(TAG, "digest using credential, digestUsername/digestPassword: " + digestUsername + "/" +digestPassword);                                                                        
            }
        }            

    }                

    public void setOSUDefaultLanguage(String lang){
        mOSULanguage = lang;
    }

    public void setOSUIconFilename(String filename){
        iconFileName = filename;
    }
    
    public void setSpFqdn(String fqdn){
        mSoapWebUrl = null;
        mOSUServerUrl = null;
        mREMServerUrl = null;
        Log.d(TAG, "set SPFQDN:" + fqdn);
        mSPFQDN = fqdn;
    }

    public void setOSUFriendlyName(String name){
        Log.d(TAG, "OSU Friendly Name:" + name);
        mOSUFriendlyName = name;
    }

    public String getOSUServerURL(){
        return mOSUServerUrl;
    }

    public String getREMServerURL(){
        return mREMServerUrl;
    }

    public void setIMSI(String imsi){
        mImsi = imsi;
    }
    
    public void setOSUIcon(String iconFilename){
        try{
            iconFileName = iconFilename;                    
            iconHash = mPasspointCertificate.computeHash(FileOperationUtil.Read(new File("/data/misc/wifi/icon/"+iconFileName)), "SHA-256");//ext: image/xxx
            Log.d(TAG,"[getOSUIconHash]:"+iconFileName+":"+iconHash);
        }catch(Exception e){
            e.printStackTrace();
        }
    } 

    private String getDeviceId(){
        TelephonyManager tm =
                (TelephonyManager)(mContext.getSystemService(Context.TELEPHONY_SERVICE));
        if (null == tm) {
            return new String("000000000000000");
        }

        String imei = tm.getDeviceId();
        if (imei == null || imei.isEmpty()) {
            return new String("000000000000000");
        }

        return imei;
    }

    public void startCertificateEnroll(final String operation) {
        Log.d(TAG, "enrollmentServerURI: " + mEnrollmentServerURI + ", operation: " + operation);        
        WifiManager wifiManager = (WifiManager) mContext.getSystemService(Context.WIFI_SERVICE);
        boolean enrollSuccess;
        Message msg = null;
        if (wifiManager != null) {
            WifiInfo wifiInfo = wifiManager.getConnectionInfo();
            if (wifiInfo == null) {
                Log.d(TAG, "no ConnectionInfo found");
                return;
            }            
            mPasspointCertificate.setMacAddress(wifiInfo.getMacAddress());
            mPasspointCertificate.setImeiOrMeid(getDeviceId());
            if(operation.equals(mPasspointCertificate.ENROLL)){
                enrollSuccess = mPasspointCertificate.connectESTServer(mEnrollmentServerURI, operation, enrollDigestUsername, enrollDigestPassword, null);
            }else{
                String credentialCertSHA256Fingerprint = mCred.getCertSha256Fingerprint(); 
                
                String subjectDN = mPasspointCertificate.getSubjectX500PrincipalFromPKCS12Keystore(credentialCertSHA256Fingerprint);
                Log.d(TAG, "subjectDN:" + subjectDN); 
                if (subjectDN == null) {
                    enrollSuccess = false;
                } else {
                    enrollSuccess = mPasspointCertificate.connectESTServer(mEnrollmentServerURI, operation, enrollDigestUsername, enrollDigestPassword, subjectDN);
                }
            }
            Log.d(TAG, "Certificate Enrolled :" + enrollSuccess); 

            if(!enrollSuccess){
                Log.d(TAG, "Certificate Enroll fail!!!");
            }
        }else{
            Log.d(TAG, "Wifi service not exist, OSU stops");
            return;
        }

        if(enrollSuccess){
            //success
            msg = msgHandler.obtainMessage(PasspointManager.EVENT_REGISTER_CERT_ENROLLMENT_DONE, true);
        }else{
            //fail
            msg = msgHandler.obtainMessage(PasspointManager.EVENT_REGISTER_CERT_ENROLLMENT_DONE, false);
        }
        //send
        msg.sendToTarget();
    }

    
    public void startSubscriptionProvision(String serverUrl, final String requestReason) {    
        Log.d(TAG, "Run startSubscriptionProvisioning with " + serverUrl);        
        mOSUServerUrl = serverUrl;
        mSoapWebUrl = serverUrl;
        mSPFQDNFromMo = null;
        
        new Thread(new Runnable() {
            public void run() {
                try{
                    SoapObject request = getSubRegistration(requestReason);
                    String response = "";
                    
                    //test xml parser
                    if("true".equals(SystemProperties.get("persist.service.soaptest"))){
                        Log.e(TAG, "[startSubscriptionProvisioning} test fake xml");                        
                        response = FileOperationUtil.Read("/data/soap.xml");
                        Document doc = ppsmoParser.getDocument(response);
                        String status = checkStatus(doc);
                        Log.e(TAG, "[startSubscriptionProvisioning} status:" + status );  
                        getUploadMO(doc);
                    }
                    /*else if("true".equals(SystemProperties.get("persist.service.clientauthtest"))){
                        SSLSocketTest sslTest = new SSLSocketTest();
                        Debug.startMethodTracing("/data/tracedump");
                        sslTest.testClientAuth();  
                        Debug.stopMethodTracing();                                                
                    }*/
                    else{
                        response = connectSoapServer(request, null, null, NO_CLIENT_AUTH);

                        if(response == null){
                            Log.e(TAG, "[startSubscriptionProvisioning] Fail to get soap resonse");
                            Message msg = msgHandler.obtainMessage(PasspointManager.EVENT_PROVISION_DONE, 9, 0, mSoapTree);//"Not found"
                            msg.sendToTarget();
                            return;
                        }

                        Document doc = PasspointPpsmoParser.getDocument(response);
                        String status = checkStatus(doc);
                        
                        //get OSU server page to fill form
                        //while certificate enrollment fail, go to OSU server page to do user-managed user/pass credential registration again
                        if(WIFI_SOAP_SPP_STATUS_OK.equals(status)){
                            if(getSubscriptionSignUpAndUserUpdate(doc)){
                                Log.d(TAG, "[New] url:" + mSoapWebUrl);
                            //useClientCertTLS
                            }else if(getUseClientCertTLS(doc)){
                                Log.d(TAG, "Provisioning using client certificate through TLS (useClientCertTLS)");
                            }else if(getEnrollmentInfo(doc)){
                                startCertificateEnroll(mPasspointCertificate.ENROLL);
                            }
                        }
                        //addMO from server 
                        //while certificate enrollment success, just send sppUpdateResponse
                        //while certificate enrollment fail, install machine-managed user/pass credential, then send sppUpdateResponse
                        else if(WIFI_SOAP_SPP_STATUS_PROVISION_COMPLETE.equals(status)){
                            if(!checkWifiSPFQDNForAddMo(doc)){
                                Log.d(TAG, "[send sppUpdateResponse] sppStatus Error"); 
                                Message msg = msgHandler.obtainMessage(PasspointManager.EVENT_PROVISION_DONE, 1, 0, mSoapTree);//"Permission denied"
                                msg.sendToTarget();  
                                return;
                            }
                            
                            Document docMgmtTree = PasspointPpsmoParser.extractMgmtTree(response);

                            if("true".equals(SystemProperties.get("persist.service.dmtreesync","true"))) {
                                sendToDmAddMo(false, false, getSPPTreeUri(doc, "addMO"), docMgmtTree);
                            }else{
                                // if(getSubscriptionProvision(docMgmtTree, false)){
                                //     Log.d(TAG, "[send sppUpdateResponse] sppStatus OK");  
                                //                                     
                                // }else{
                                //     Log.d(TAG, "[send sppUpdateResponse] sppStatus Error"); 
                                //     Message msg = msgHandler.obtainMessage(PasspointManager.EVENT_PROVISION_DONE, 3, 0, mSoapTree);
                                //     msg.sendToTarget();                                
                                // }                            
                            }
                            
                            
                        }
                        //abort provisioning
                        //while certificate enrollment fail
                        else if(WIFI_SOAP_SPP_STATUS_EXCHANGE_COMPLETE.equals(status)){
                            String err = checkErrorCode(doc);
                            Log.e(TAG, "[startSubscriptionProvisioning] Exchange complete, release TLS connection error occurred: " + err);
                        }else if(WIFI_SOAP_SPP_STATUS_ERROR_OCCURRED.equals(status)){
                            Log.e(TAG, "[startSubscriptionProvisioning] Error occurred");
                            getErrorCode(doc);                            
                        }else{
                            status = checkStatus(doc, NAMESPACE_NS, WIFI_SOAP_USER_INPUT_RESPONSE); // format type R
                            if(WIFI_SOAP_SPP_STATUS_OK.equals(status)){
                                if(getEnrollmentInfo(doc)){
                                    startCertificateEnroll(mPasspointCertificate.ENROLL);
                                }
                            }else if(WIFI_SOAP_SPP_STATUS_ERROR_OCCURRED.equals(status)){
                                Log.e(TAG, "[startSubscriptionProvisioning] checkStatus of sppUserInputResponse Error occurred");
                                getErrorCode(doc);                            
                            } else {
                                Log.e(TAG, "[startSubscriptionProvisioning] unknown status");
                            }

                        }
                    }
                    //hard code test OSU sign-up page
                    //Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse("https://subscription-server.R2-testbed.wi-fi.org:8443/web/ruckuswireles/home/-/onlinesignup/subscriberDetails?sessionID=" + SessionID));
                    //intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    //mContext.startActivity(intent);
                    return;
                    
                }catch(Exception e){
                    e.printStackTrace();
                }
            }
        }).start();
    }

    public void startRemediation(final String serverUrl, final String requestReason, PasspointCredential cred) {
        Log.d(TAG, "Run startRemediation with " + serverUrl);
        mSoapWebUrl = serverUrl;
        mREMServerUrl = serverUrl;
        mSPFQDNFromMo = null;
        
        if (cred != null) {
            mCred = cred;
        } else {
            Log.d(TAG, "cred is null");        
            return;
        }

        if (!USER_INPUT_COMPLETED.equals(requestReason) && !SUB_MO_UPLOAD.equals(requestReason)) {
            setRemediationHttpDigest();
        }
        
        if ("true".equals(SystemProperties.get("persist.service.manual.ut.rem"))){
            mTreeHelper.configToPpsMoNumber(10, mSoapTree);
        }
        
        new Thread(new Runnable() {
            public void run() {
                try {
                    SoapObject request = getSubRemediation(requestReason);
                    String response = null;

                    String eapType = null;
                    if (mCred != null) {
                        eapType = mCred.getEapMethod();
                    }

                    //test xml parser
                    if ("true".equals(SystemProperties.get("persist.service.soaptest"))) {
                        Log.e(TAG, "[startRemediation} test fake xml");                        
                        response = FileOperationUtil.Read("/data/soap.xml");

                        if (response == null) {
                            Log.e(TAG, "[startRemediation] Fail to get soap resonse");
                            return;
                        }
                        
                        Document doc = PasspointPpsmoParser.getDocument(response);
                        String status = checkStatus(doc);
                        Log.e(TAG, "[startRemediation} status:" + status );                        
                    } else {
                        if ("SIM".equals(eapType)) {
                            Log.d(TAG, "EAP-SIM, no digest or client certifcate");                                                            
                            response = connectSoapServer(request, "", "", NO_CLIENT_AUTH);
                        } else if ("TTLS".equals(eapType)) {
                            if (digestUsername != null && !digestUsername.isEmpty()) {
                                Log.d(TAG, "digest using U/P credential or DMAcc, digestUsername/digestPassword: " + digestUsername + "/" +digestPassword);                                        
                                response = connectSoapServer(request, digestUsername, digestPassword, NO_CLIENT_AUTH);
                            }
                        } else if ("TLS".equals(eapType)) {
                            String credentialCertSHA256Fingerprint = mCred.getCertSha256Fingerprint();                        
                            Log.d(TAG, "digest using client cert credential, SHA256 fingerprint: " + credentialCertSHA256Fingerprint);                                                            
                            hs20PKCS12KeyStore = mPasspointCertificate.getCredentialCertKeyStore(credentialCertSHA256Fingerprint);                                                    

                            if (hs20PKCS12KeyStore != null) {
                                response = connectSoapServer(request, null, null, CLIENT_CERT);
                            } else {
                                Log.d(TAG, "client certifcate not found");                                                          
                            }
                        } else {
                            Log.d(TAG, "no digest or client certifcate");                                                          
                            response = connectSoapServer(request, "", "", NO_CLIENT_AUTH);
                        }

                        if (response == null) {
                            Log.e(TAG, "[startRemediation] Fail to get soap resonse");
                            return;
                        }
                        
                        Document doc = PasspointPpsmoParser.getDocument(response);
                        String status = checkStatus(doc);
                        if (WIFI_SOAP_SPP_STATUS_REMEDIATION_COMPLETE.equals(status)) {
                            if (getNoMoUpdate(doc)) {
                                Log.d(TAG, WIFI_SOAP_SPP_NOMOUPDATE);
                            } else if (getSubscriptionSignUpAndUserUpdate(doc)) { //user remediation
                                Log.d(TAG, "[New] url:" + mSoapWebUrl);
                            } else {                            
                                if (checkWifiSPFQDNForUpdateMo(doc)) {
                                    if ("true".equals(SystemProperties.get("persist.service.dmtreesync","true"))) {
                                        sendToDmUpdateMo(SUBSCRIPTION_REMEDIATION, doc);
                                    }
                                } else {
                                    sendUpdateResponse(false, SUBSCRIPTION_REMEDIATION, WIFI_SOAP_UPDATE_RESPONSE_PERMISSION_DENY);
                                }                            
                            }
                        } else if (WIFI_SOAP_SPP_STATUS_OK.equals(status)) {
                            if (getUploadMO(doc)) {
                                startRemediation(serverUrl, SUB_MO_UPLOAD, null);
                            } else if (getSubscriptionSignUpAndUserUpdate(doc)) {
                                Log.d(TAG, "[New] url:" + mSoapWebUrl);
                            } else if (getEnrollmentInfo(doc)) {
                                startCertificateEnroll(mPasspointCertificate.REENROLL);
                            } else {
                                if (checkWifiSPFQDNForUpdateMo(doc)) {
                                    if ("true".equals(SystemProperties.get("persist.service.dmtreesync","true"))) {
                                        sendToDmUpdateMo(SUBSCRIPTION_REMEDIATION, doc);
                                    }
                                } else {
                                    sendUpdateResponse(false, SUBSCRIPTION_REMEDIATION, WIFI_SOAP_UPDATE_RESPONSE_PERMISSION_DENY);
                                }                            
                            }
                        } else if (WIFI_SOAP_SPP_STATUS_PROVISION_COMPLETE.equals(status)) { // SIM subscription policy provision
                            Document docMgmtTree = ppsmoParser.extractMgmtTree(response);

                            if ("true".equals(SystemProperties.get("persist.service.dmtreesync","true"))) {
                                sendToDmAddMo(false, true, getSPPTreeUri(doc, "addMO"), docMgmtTree);
                            }                                               
                        } else if (WIFI_SOAP_SPP_STATUS_EXCHANGE_COMPLETE.equals(status)){
                            Log.d(TAG, WIFI_SOAP_SPP_STATUS_EXCHANGE_COMPLETE);
                            Message msg = msgHandler.obtainMessage(PasspointManager.EVENT_REMEDIATION_DONE, 0, 0, mSoapTree);//"NoUpdate"
                            msg.sendToTarget();                                
                        } else if (WIFI_SOAP_SPP_STATUS_NO_UPDATE_AVAILABLE.equals(status)) {
                            Log.d(TAG, WIFI_SOAP_SPP_STATUS_NO_UPDATE_AVAILABLE);
                        } else if(WIFI_SOAP_SPP_STATUS_ERROR_OCCURRED.equals(status)){
                            Log.e(TAG, "[startRemediation] Error occurred");
                            getErrorCode(doc);                            
                        } else {
                            Log.e(TAG, "[startRemediation] unknown status");
                        }
                    }
                } catch(Exception e) {
                    e.printStackTrace();
                }
            }
        }).start();
    }

    public void startPolicyProvision(final String serverUrl, final String requestReason, PasspointCredential cred) {
        Log.d(TAG, "Run startPolicyProvision with " + serverUrl);
        mSoapWebUrl = serverUrl;
        mSPFQDNFromMo = null;
        
        if (cred != null) {
            mCred = cred;
        } else {
            Log.d(TAG, "cred is null");        
            return;
        }

        if (POL_UPDATE.equals(requestReason)) {
            setPolicyUpdateHttpDigest();
        }

        new Thread(new Runnable() {
            public void run() {
                try{
                    SoapObject request = getPolicyUpdateRequest(requestReason);
                    String response = "";

                    if (digestUsername != null && !digestUsername.isEmpty()) {
                        Log.d(TAG, "digest using U/P Credential or DMAcc, digestUsername/digestPassword: " + digestUsername + "/" +digestPassword);                    
                        response = connectSoapServer(request, digestUsername, digestPassword, NO_CLIENT_AUTH);
                    }
                    
                    if (response != null && !response.isEmpty()) {
                        Document doc = PasspointPpsmoParser.getDocument(response);
                        String status = checkStatus(doc);
                        if (WIFI_SOAP_SPP_STATUS_OK.equals(status)) {
                            if (getUploadMO(doc)) {
                                if (checkWifiSPFQDNForUploadMo(doc)) {
                                    startPolicyProvision(serverUrl, POL_MO_UPLOAD, null);
                                    return;
                                } else {
                                    sendUpdateResponse(false, POLICY_UPDATE, WIFI_SOAP_UPDATE_RESPONSE_PERMISSION_DENY);
                                    return;
                                }
                            }
                        } else if (WIFI_SOAP_SPP_STATUS_UPDATE_COMPLETE.equals(status)) {
                            if (checkWifiSPFQDNForUpdateMo(doc)) {
                                if ("true".equals(SystemProperties.get("persist.service.dmtreesync","true"))) {
                                    sendToDmUpdateMo(POLICY_UPDATE, doc);
                                } 
                            } else {
                                sendUpdateResponse(false, POLICY_UPDATE, WIFI_SOAP_UPDATE_RESPONSE_PERMISSION_DENY);
                            }                                                    
                        
                            return;
                        } else if (WIFI_SOAP_SPP_STATUS_NO_UPDATE_AVAILABLE.equals(status)) {
                            Log.d(TAG, WIFI_SOAP_SPP_STATUS_NO_UPDATE_AVAILABLE);
                        } else if (WIFI_SOAP_SPP_STATUS_ERROR_OCCURRED.equals(status)) {
                            Log.e(TAG, "[startPolicyProvision] Error occurred");
                            getErrorCode(doc);                            
                        } else {
                            Log.e(TAG, "[startPolicyProvision] unknown status");
                        }
                    } else {
                        Log.e(TAG, "[startPolicyProvision] Fail to get soap resonse");
                    }

                } catch(Exception e) {
                    e.printStackTrace();
                }


                if (msgHandler != null) {
                    Message msg = msgHandler.obtainMessage(PasspointManager.EVENT_POLICY_PROVISION_DONE, 0, 0, mSoapTree);
                    msg.sendToTarget();
                } else {
                    Log.e(TAG, "[startPolicyProvision] send EVENT_POLICY_PROVISION_DONE fail, msgHandler null");
                }
            }
        }).start();
    }
    
    private String connectSoapServer(SoapObject request, final String digestUsername, final String digestPassword, final int clientCertType){
        Log.d(TAG, "[connectSoapServer]: request:" + request);
        String response = null;
        SoapSerializationEnvelope envelope = new SoapSerializationEnvelope(SoapEnvelope.VER12);
        envelope.setAddAdornments(false);
        envelope.implicitTypes = true;
        envelope.dotNet = false;
        envelope.setOutputSoapObject(request);


        Log.d(TAG, "Server url:" + mSoapWebUrl);
                        
        if(mSoapWebUrl.startsWith("HTTPS://") || mSoapWebUrl.startsWith("https://")){            
            try{
                int retryCount = 5;
                boolean isConnected = false;

                PasspointHttpClient hc = null;
                UsernamePasswordCredentials credentials = null;

                if (digestUsername != null && digestPassword != null) {
                    credentials = new UsernamePasswordCredentials(digestUsername, digestPassword);
                    hc = new PasspointHttpClient(null, null);
                    hc.setAuthenticationCredentials(credentials);
                } else {
                    if (clientCertType == CLIENT_CERT) {
                        if (hs20PKCS12KeyStore.aliases().hasMoreElements()) {
                            hc = new PasspointHttpClient(hs20PKCS12KeyStore, mPasspointCertificate.passWord.toCharArray());
                        } else {                            
                            Log.d(TAG, "client cert is not installed in passpoint PKCS12 keystore");   
                            hc = new PasspointHttpClient(null, null);
                        }
                    } else {
                        hc = new PasspointHttpClient(null, null);
                    }                
                }
                
                while(retryCount > 0 && !isConnected){                
                    try{
                        URI requestUri = new URI(mSoapWebUrl);
                        HttpResponse httpResp = null;
                        byte[] requestData = 
                                (new HttpTransportSE(mSoapWebUrl)).getRequestData(envelope, "UTF-8");                         
                        Header[] requestHeaders;
                        List<BasicHeader> basicHeaders = new ArrayList<BasicHeader>();

                        if (requestData == null) {
                            break;
                        }
                        
                        basicHeaders.add(new BasicHeader(hc.CONNECTION, "close"));
                        basicHeaders.add(new BasicHeader(hc.ACCEPT_ENCODING_HEADER, "gzip"));                        
                        basicHeaders.add(new BasicHeader(hc.CONTENT_LENGTH_HEADER, "" +  requestData.length));
                        requestHeaders = basicHeaders.toArray(new Header[basicHeaders.size()]);
                        
                        if (envelope.version == SoapSerializationEnvelope.VER12) {
                            httpResp = hc.post(requestUri, CONTENT_TYPE_SOAP_XML_CHARSET_UTF_8, requestData, requestHeaders);                            
                        } else {
                            httpResp = hc.post(requestUri, CONTENT_TYPE_XML_CHARSET_UTF_8, requestData, requestHeaders);                            
                        }
                        
                        InputStream is = httpResp.getEntity().getContent();

                        ByteArrayOutputStream bos = new ByteArrayOutputStream();
                        byte[] buf = new byte[8192];
                                
                        while (true) {
                            int rd = is.read(buf, 0, 8192);
                            if (rd == -1) {
                                break;
                            }
                            bos.write(buf, 0, rd);
                        }
                                
                        bos.flush();
                        response = bos.toString();
                        isConnected = true;
                        Log.d(TAG, "soap connect by TLS");                        
                    }catch(UnknownHostException ee){
                        retryCount--;
                        Log.d(TAG, "Wait for retry:" + retryCount);
                        Thread.sleep(3 * 1000);
                    }
                } 
                
                if(!isConnected){
                   Log.e(TAG, "Failed to connect");
                   return null;
                }
            }catch(Exception e){
                e.printStackTrace();
            }
        } else {
            try{
                int retryCount = 5;
                boolean isConnected = false;

                PasspointHttpClient hc = null;
                UsernamePasswordCredentials credentials = null;

                /* test site: http://httpbin.org/digest-auth/auth/user/passwd */
                credentials = new UsernamePasswordCredentials("user", "passwd");
                hc = new PasspointHttpClient(null, null);
                hc.setAuthenticationCredentials(credentials);
                
                while(retryCount > 0 && !isConnected){                
                    try{
                        URI requestUri = new URI(mSoapWebUrl);
                        Header[] requestHeaders;
                        List<BasicHeader> basicHeaders = new ArrayList<BasicHeader>();

                        basicHeaders.add(new BasicHeader(hc.CONNECTION, "close"));
                        basicHeaders.add(new BasicHeader(hc.ACCEPT_ENCODING_HEADER, "gzip"));
                        requestHeaders = basicHeaders.toArray(new Header[basicHeaders.size()]);
                                                
                        HttpResponse httpResp = hc.get(requestUri, requestHeaders);
                        InputStream is = httpResp.getEntity().getContent();
                        ByteArrayOutputStream bos = new ByteArrayOutputStream();
                        byte[] buf = new byte[8192];
                                
                        while (true) {
                            int rd = is.read(buf, 0, 8192);
                            if (rd == -1) {
                                break;
                            }
                            bos.write(buf, 0, rd);
                        }
                                
                        bos.flush();
                        response = bos.toString();
                        isConnected = true;
                        Log.d(TAG, "soap connect by TLS");                        
                    }catch(UnknownHostException ee){
                        retryCount--;
                        Log.d(TAG, "Wait for retry:" + retryCount);
                        Thread.sleep(3 * 1000);
                    }
                } 
                
                if(!isConnected){
                   Log.e(TAG, "Failed to connect");
                   return null;
                }            
            }catch(Exception e){
                e.printStackTrace();
            }
        }
        
        return response;
    }

    public void sendUpdateResponse(final boolean success, final int procedureType, final String reason){
        Log.d(TAG, "[sendUpdateResponse] start, success = " + success + ", procedureType = " + procedureType + ", reason = " + reason);
        if(procedureType == SUBSCRIPTION_PROVISION){
            mSoapWebUrl = mOSUServerUrl;
        }else if(procedureType == SUBSCRIPTION_SIM_PROVISION){
            mSoapWebUrl = mREMServerUrl; //SIM provision through remediation procedure
        }
        
        new Thread(new Runnable() {
            public void run() {
                try{
                    SoapObject request = new SoapObject();
                    String response = "";
                    
                    if(success == true){
                        request = getUpdateResponse(false, null);
                    }
                    else{
                        request = getUpdateResponse(true, reason);
                    }

                    if(procedureType == POLICY_UPDATE || procedureType == SUBSCRIPTION_REMEDIATION){
                        String eapType = mCred.getEapMethod();
                        String credentialCertSHA256Fingerprint = mCred.getCertSha256Fingerprint();                                                
                        
                        if(digestUsername != null && !digestUsername.isEmpty()){
                            Log.d(TAG, "digest using U/P credential or DMAcc, digestUsername/digestPassword: " + digestUsername + "/" +digestPassword);                                        
                            response = connectSoapServer(request, digestUsername, digestPassword, NO_CLIENT_AUTH);
                        }                        
                        else if("TLS".equals(eapType)){
                            Log.d(TAG, "digest using client cert credential, SHA256 fingerprint: " + credentialCertSHA256Fingerprint);                                                            
                            response = connectSoapServer(request, null, null, CLIENT_CERT);
                        }
                    }else{//procedureType == SUBSCRIPTION_PROVISION || procedureType == SUBSCRIPTION_SIM_PROVISION
                        Log.d(TAG, "OSU, no need to set digest"); 
                        response = connectSoapServer(request, null, null, NO_CLIENT_AUTH);
                    }
                    
        
                    if(response == null || response.isEmpty()){
                        Log.e(TAG, "[sendUpdateResponse] Fail to get soap resonse");
                        return;
                    }
                    
                    Document doc = PasspointPpsmoParser.getDocument(response);
                    String status = checkStatus(doc, "spp:sppExchangeComplete"); // format type Q
                    
                    if(status == null){
                        Log.e(TAG, "[sendUpdateResponse] format type Q can be removed");
                        status = checkStatus(doc, NAMESPACE_NS, WIFI_SOAP_SPP_EXCHANGE_COMPLETE); // format type R
                    }
                    
                    if(WIFI_SOAP_SPP_STATUS_EXCHANGE_COMPLETE.equals(status)){
                        Log.d(TAG, "[sendUpdateResponse] exchange complete");
                        if(procedureType == SUBSCRIPTION_PROVISION){
                            Log.d(TAG, "[sendUpdateResponse] exchange complete:procedureType == SUBSCRIPTION_PROVISION");

                            boolean result = true;
                            if("false".equals(SystemProperties.get("persist.service.aaacacheck"))){
                                result = true;
                            }else{
                                Collection<WifiTree.CredentialInfo> creds = mTreeHelper.getCredentialInfo(mTreeHelper.getSp(mSoapTree, mSPFQDNFromMo));
                                for (WifiTree.CredentialInfo credInfo : creds) {    
                                    //Save mapping of enrolled certificate alias and SHA256 fingerprint
                                    if (credInfo.credential.digitalCertificate.CertificateType.equals("x509v3")) {
                                        mPasspointCertificate.saveMappingOfEnrollCertAliasAndSha256(mPasspointCertificate.getEnrollCertAlias(), credInfo.credential.digitalCertificate.CertSHA256Fingerprint);
                                    }
                                    
                                    for(WifiTree.AAAServerTrustRoot aaaTrustRoot : credInfo.aAAServerTrustRoot.values()){
                                        result &= mPasspointCertificate.installServerTrustRoot(aaaTrustRoot.CertURL, aaaTrustRoot.CertSHA256Fingerprint, PasspointCertificate.AAA_ROOT, false);                                        
                                    }
                                    result &= mPasspointCertificate.installServerTrustRoot(credInfo.subscriptionUpdate.trustRoot.CertURL, credInfo.subscriptionUpdate.trustRoot.CertSHA256Fingerprint, PasspointCertificate.SUBSCRIPTION_ROOT, false);
                                    result &= mPasspointCertificate.installServerTrustRoot(credInfo.policy.policyUpdate.trustRoot.CertURL, credInfo.policy.policyUpdate.trustRoot.CertSHA256Fingerprint, PasspointCertificate.POLICY_ROOT, false);
                                    result &= success;
                                }
                            }                            

                            Message msg = msgHandler.obtainMessage(PasspointManager.EVENT_PROVISION_UPDATE_RESPONSE_DONE, result);
                            msg.sendToTarget();                     
                        } else if(procedureType == SUBSCRIPTION_SIM_PROVISION) {
                            Log.d(TAG, "[sendUpdateResponse] exchange complete:procedureType == SUBSCRIPTION_SIM_PROVISION");

                            boolean result = true;
                            if("false".equals(SystemProperties.get("persist.service.aaacacheck"))){
                                result = true;
                            }else{     
                                Collection<WifiTree.CredentialInfo> creds = mTreeHelper.getCredentialInfo(mTreeHelper.getSp(mSoapTree, mSPFQDNFromMo));
                                for (WifiTree.CredentialInfo credInfo : creds) {
                                    for(WifiTree.AAAServerTrustRoot aaaTrustRoot : credInfo.aAAServerTrustRoot.values()){
                                        result &= mPasspointCertificate.installServerTrustRoot(aaaTrustRoot.CertURL, aaaTrustRoot.CertSHA256Fingerprint, PasspointCertificate.AAA_ROOT, false);                                        
                                    }
                                    result &= mPasspointCertificate.installServerTrustRoot(credInfo.subscriptionUpdate.trustRoot.CertURL, credInfo.subscriptionUpdate.trustRoot.CertSHA256Fingerprint, PasspointCertificate.SUBSCRIPTION_ROOT, false);
                                    result &= mPasspointCertificate.installServerTrustRoot(credInfo.policy.policyUpdate.trustRoot.CertURL, credInfo.policy.policyUpdate.trustRoot.CertSHA256Fingerprint, PasspointCertificate.POLICY_ROOT, false);
                                    result &= success;
                                }    

                            }                            

                            Message msg = msgHandler.obtainMessage(PasspointManager.EVENT_SIM_PROVISION_UPDATE_RESPONSE_DONE, success);
                            msg.sendToTarget();                     
                        } else if(procedureType == SUBSCRIPTION_REMEDIATION) {
                            Log.d(TAG, "[sendUpdateResponse] exchange complete:procedureType == SUBSCRIPTION_REMEDIATION");                        

                            boolean result = true;
                            if("false".equals(SystemProperties.get("persist.service.aaacacheck"))){
                                result = true;
                            }else{ 
                                WifiTree.CredentialInfo credInfo = mTreeHelper.getCredentialInfo(mSoapTree, mCred.getWifiSPFQDN(), mCred.getCredName());
                                if(credInfo == null){
                                    Log.d(TAG, "credInfo is null while retrieving AAA trust root");

                                    Message msg = msgHandler.obtainMessage(PasspointManager.EVENT_REMEDIATION_UPDATE_RESPONSE_DONE, false);
                                    msg.sendToTarget();                     
                                    return;
                                }else{
                                    for(WifiTree.AAAServerTrustRoot aaaTrustRoot : credInfo.aAAServerTrustRoot.values()){
                                        result &= mPasspointCertificate.installServerTrustRoot(aaaTrustRoot.CertURL, aaaTrustRoot.CertSHA256Fingerprint, PasspointCertificate.AAA_ROOT, true);                                        
                                    }                                    
                                    result &= mPasspointCertificate.installServerTrustRoot(credInfo.subscriptionUpdate.trustRoot.CertURL, credInfo.subscriptionUpdate.trustRoot.CertSHA256Fingerprint, PasspointCertificate.SUBSCRIPTION_ROOT, false);
                                    result &= mPasspointCertificate.installServerTrustRoot(credInfo.policy.policyUpdate.trustRoot.CertURL, credInfo.policy.policyUpdate.trustRoot.CertSHA256Fingerprint, PasspointCertificate.POLICY_ROOT, false);
                                    result &= success;

                                }
                            }  

                            Message msg = msgHandler.obtainMessage(PasspointManager.EVENT_REMEDIATION_UPDATE_RESPONSE_DONE, success);
                            msg.sendToTarget();
                        } else if(procedureType == POLICY_UPDATE) {
                            Log.d(TAG, "[sendUpdateResponse] exchange complete:procedureType == POLICY_UPDATE");                        
                            Message msg = msgHandler.obtainMessage(PasspointManager.EVENT_POLICY_PROVISION_UPDATE_RESPONSE_DONE, success);
                            msg.sendToTarget();
                        }                         
                    } else if(WIFI_SOAP_SPP_STATUS_ERROR_OCCURRED.equals(status)) {
                        Log.e(TAG, "[sendUpdateResponse] Error occurred");
                        getErrorCode(doc);                            
                    } else {
                        Log.e(TAG, "[sendUpdateResponse] unknown status");
                    }
                }catch(Exception e){
                    e.printStackTrace();
                }
            }
        }).start();

    }

    private SoapObject getSubRegistration(String requestReason){
        //Construct sppPostDevData element
        SoapObject request = new SoapObject(NAMESPACE_NS, WIFI_SOAP_POST_DEV_DATA);
        AttributeInfo attributeInfo = new AttributeInfo();
        attributeInfo.setName(WIFI_SOAP_SPP_VERSION);
        attributeInfo.setValue("1.0");
        attributeInfo.setType("PropertyInfo.STRING_CLASS");
        attributeInfo.setNamespace(NAMESPACE_NS);        
        request.addAttribute(attributeInfo);
        //requestReason
        request.addAttribute(WIFI_SOAP_REQ_REASON, requestReason);

        //redirectURL
        request.addAttribute(WIFI_SOAP_REDIRECTURL, REDIRECT_URL+":"+OSU_REDIRECT_PORT);
       
        //sessionID (adding all packages except for the first one)
        if(!SUB_REGISTER.equals(requestReason)){
            AttributeInfo sessionIDattributeInfo = new AttributeInfo();
            sessionIDattributeInfo.setName(WIFI_SOAP_SESSIONID);                                      
            sessionIDattributeInfo.setValue(SessionID);
            sessionIDattributeInfo.setType("PropertyInfo.STRING_CLASS");
            sessionIDattributeInfo.setNamespace(NAMESPACE_NS);
            request.addAttribute(sessionIDattributeInfo);
        }


        //New supportedSPPVersions element
        request.addProperty(WIFI_SOAP_S_SPP_VERSION, "1.0");
        //New supportedMOList
        request.addProperty(WIFI_SOAP_S_MOLIST, WIFI_SOAP_S_MOSUBSCRIPTION 
                                                                + " " + WIFI_SOAP_S_MODEVINFO 
                                                                + " " + WIFI_SOAP_S_MODEVDETAIL 
                                                                + " " + WIFI_SOAP_S_MOHS20);

        //New moContainer
        //Construct moContainer
        SoapObject moInfo = getMoInfo();
        SoapObject moDetail = getMoDetail();
        request.addSoapObject(moInfo);
        request.addSoapObject(moDetail);

        return request;
    }
    
    private SoapObject getSubRemediation(String requestReason){
        //Construct sppPostDevData element
        SoapObject request = new SoapObject(NAMESPACE_NS, WIFI_SOAP_POST_DEV_DATA);
        //sppVersion
        AttributeInfo attributeInfo = new AttributeInfo();
        attributeInfo.setName(WIFI_SOAP_SPP_VERSION);
        attributeInfo.setValue("1.0");
        attributeInfo.setType("PropertyInfo.STRING_CLASS");
        attributeInfo.setNamespace(NAMESPACE_NS);
        request.addAttribute(attributeInfo);
        //requestReason
        request.addAttribute(WIFI_SOAP_REQ_REASON, requestReason);
        //redirectURL
        request.addAttribute(WIFI_SOAP_REDIRECTURL, REDIRECT_URL+":"+REM_REDIRECT_PORT);
       

        //sessionID
        if(!SUB_PROVISION.equals(requestReason) && !SUB_REMEDIATION.equals(requestReason)){
            AttributeInfo sessionIDattributeInfo = new AttributeInfo();
            sessionIDattributeInfo.setName(WIFI_SOAP_SESSIONID);
            sessionIDattributeInfo.setValue(SessionID);
            sessionIDattributeInfo.setType("PropertyInfo.STRING_CLASS");
            sessionIDattributeInfo.setNamespace(NAMESPACE_NS);
            request.addAttribute(sessionIDattributeInfo);
        }


        //New supportedSPPVersions element
        request.addProperty(WIFI_SOAP_S_SPP_VERSION, "1.0");
        //New supportedMOList
        request.addProperty(WIFI_SOAP_S_MOLIST, WIFI_SOAP_S_MOSUBSCRIPTION 
                                                                + " " + WIFI_SOAP_S_MODEVINFO 
                                                                + " " + WIFI_SOAP_S_MODEVDETAIL 
                                                                + " " + WIFI_SOAP_S_MOHS20);

        //New moContainer
        //Construct moContainer
        SoapObject moInfo = getMoInfo();
        SoapObject moDetail = getMoDetail();
        request.addSoapObject(moInfo);
        request.addSoapObject(moDetail);

        //New moContainer
        //Construct moContainer
        if(SUB_MO_UPLOAD.equals(requestReason)){
            SoapObject moSub = getSubscription();
            request.addSoapObject(moSub);
        }

        return request;
    }

    private SoapObject getSubscription(){
        WifiTree.CredentialInfo credInfo = mTreeHelper.getCredentialInfo(mSoapTree, mCred.getWifiSPFQDN(), mCred.getCredName());
        SoapObject nsRequest = new SoapObject(NAMESPACE_NS, WIFI_SOAP_MO_CONTAINER);
        AttributeInfo attributeInfo = new AttributeInfo();
        attributeInfo.setName(WIFI_SOAP_MO_URN);
        attributeInfo.setValue(WIFI_SOAP_S_MOSUBSCRIPTION);
        attributeInfo.setType("PropertyInfo.STRING_CLASS");
        attributeInfo.setNamespace(NAMESPACE_NS);        
        nsRequest.addAttribute(attributeInfo);
     
        SoapObject MgmtTreeRequest = new SoapObject(null, WIFI_SOAP_MGMTREE);
            //xmlns
            AttributeInfo ppsMoattributeInfo = new AttributeInfo();
            ppsMoattributeInfo.setName(WIFI_SOAP_MO_XMLNS);
            ppsMoattributeInfo.setValue(WIFI_SOAP_SYNCML_DMDDF_1_2);
            ppsMoattributeInfo.setType("PropertyInfo.STRING_CLASS");                
            MgmtTreeRequest.addAttribute(ppsMoattributeInfo);              
            MgmtTreeRequest.addProperty("VerDTD","1.2");

            SoapObject PerProviderSubscriptionRequest = new SoapObject(null, "Node");
                PerProviderSubscriptionRequest.addProperty("NodeName", "PerProviderSubscription");

                SoapObject rtPropertiesRequest = new SoapObject(null, "RTProperties");
                    SoapObject typeRequest = new SoapObject(null, "Type");
                        typeRequest.addProperty("DDFName", WIFI_SOAP_S_MOSUBSCRIPTION);
                    rtPropertiesRequest.addSoapObject(typeRequest);
                PerProviderSubscriptionRequest.addSoapObject(rtPropertiesRequest);

                SoapObject x1Request = new SoapObject(null, "Node");
                    x1Request.addProperty("NodeName", "x1");

                    SoapObject SubscriptionPriorityRequest = new SoapObject(null, "Node");
                        SubscriptionPriorityRequest.addProperty("NodeName", "SubscriptionPriority");
                        SubscriptionPriorityRequest.addProperty("Value", credInfo.credentialPriority);
                    x1Request.addSoapObject(SubscriptionPriorityRequest);

                    SoapObject SubscriptionRemediationRequest = new SoapObject(null, "Node");
                        SubscriptionRemediationRequest.addProperty("NodeName", "SubscriptionRemediation");
    
                        SoapObject RemURIRequest = new SoapObject(null, "Node");
                            RemURIRequest.addProperty("NodeName", "URI");
                            RemURIRequest.addProperty("Value", mSoapWebUrl);
                        SubscriptionRemediationRequest.addSoapObject(RemURIRequest);

                        SoapObject certURLRequest = new SoapObject(null, "Node");
                            certURLRequest.addProperty("NodeName", "certURL");
                            certURLRequest.addProperty("Value", credInfo.subscriptionUpdate.trustRoot.CertURL);
                        SubscriptionRemediationRequest.addSoapObject(certURLRequest);

                        SoapObject certSHA256FingerprintRequest = new SoapObject(null, "Node");
                            certSHA256FingerprintRequest.addProperty("NodeName", "certSHA256Fingerprint");
                            certSHA256FingerprintRequest.addProperty("Value", credInfo.subscriptionUpdate.trustRoot.CertSHA256Fingerprint);
                        SubscriptionRemediationRequest.addSoapObject(certSHA256FingerprintRequest);

                    x1Request.addSoapObject(SubscriptionRemediationRequest);

                    SoapObject SubscriptionUpdateRequest = new SoapObject(null, "Node");
                        SubscriptionUpdateRequest.addProperty("NodeName", "SubscriptionUpdate");

                        SoapObject UpdateIntervalRequest = new SoapObject(null, "Node");
                            UpdateIntervalRequest.addProperty("NodeName", "UpdateInterval");
                            UpdateIntervalRequest.addProperty("Value", credInfo.subscriptionUpdate.UpdateInterval);
                        SubscriptionUpdateRequest.addSoapObject(UpdateIntervalRequest);

                        SoapObject UpdateMethodRequest = new SoapObject(null, "Node");
                            UpdateMethodRequest.addProperty("NodeName", "UpdateMethod");
                            UpdateMethodRequest.addProperty("Value", credInfo.subscriptionUpdate.UpdateMethod);
                        SubscriptionUpdateRequest.addSoapObject(UpdateMethodRequest);

                        SoapObject RestrictionRequest = new SoapObject(null, "Node");
                            RestrictionRequest.addProperty("NodeName", "Restriction");
                            RestrictionRequest.addProperty("Value", credInfo.subscriptionUpdate.Restriction);
                        SubscriptionUpdateRequest.addSoapObject(RestrictionRequest);
                        
                        SoapObject UpdURIRequest = new SoapObject(null, "Node");
                            UpdURIRequest.addProperty("NodeName", "URI");
                            UpdURIRequest.addProperty("Value", credInfo.subscriptionUpdate.URI);
                        SubscriptionUpdateRequest.addSoapObject(UpdURIRequest);

                    x1Request.addSoapObject(SubscriptionUpdateRequest);
                    
                    SoapObject HomeSPRequest = new SoapObject(null, "Node");
                        HomeSPRequest.addProperty("NodeName", "HomeSP");

                        SoapObject FriendlyNameRequest = new SoapObject(null, "Node");
                            FriendlyNameRequest.addProperty("NodeName", "FriendlyName");
                            FriendlyNameRequest.addProperty("Value", credInfo.homeSP.FriendlyName);
                        HomeSPRequest.addSoapObject(FriendlyNameRequest);
                        
                        SoapObject FQDNNameRequest = new SoapObject(null, "Node");
                            FQDNNameRequest.addProperty("NodeName", "FQDN");
                            FQDNNameRequest.addProperty("Value", credInfo.homeSP.FQDN);
                        HomeSPRequest.addSoapObject(FQDNNameRequest);

                        SoapObject HomeOIListRequest = new SoapObject(null, "Node");
                            HomeOIListRequest.addProperty("NodeName", "HomeOIList");

                            Collection<WifiTree.HomeOIList> oil = mTreeHelper.getHomeOIList(credInfo);
                            for (WifiTree.HomeOIList oi : oil) {
                                SoapObject HomeOIListx1Request = new SoapObject(null, "Node");
                                    HomeOIListx1Request.addProperty("NodeName", oi.nodeName);
                                
                                    SoapObject HomeOIx1Request = new SoapObject(null, "Node");
                                        HomeOIx1Request.addProperty("NodeName", "HomeOI");
                                        HomeOIx1Request.addProperty("Value", oi.HomeOI);
                                        HomeOIx1Request.addProperty("NodeName", "HomeOIRequired");
                                        HomeOIx1Request.addProperty("Value", Boolean.toString(oi.HomeOIRequired));
                                    HomeOIListx1Request.addSoapObject(HomeOIx1Request);
                                    
                                HomeOIListRequest.addSoapObject(HomeOIListx1Request.newInstance());
                            }

                        HomeSPRequest.addSoapObject(HomeOIListRequest);
                        
                    x1Request.addSoapObject(HomeSPRequest);                        
                    
                    SoapObject SubscriptionParametersRequest = new SoapObject(null, "Node");
                        SubscriptionParametersRequest.addProperty("NodeName", "SubscriptionParameters");
                    x1Request.addSoapObject(SubscriptionParametersRequest);
                    
                    SoapObject CredentialRequest = new SoapObject(null, "Node");
                        CredentialRequest.addProperty("NodeName", "Credential");
                    
                        SoapObject CreationDateRequest = new SoapObject(null, "Node");
                            CreationDateRequest.addProperty("NodeName", "CreationDate");
                            CreationDateRequest.addProperty("Value", credInfo.credential.CreationDate);
                        CredentialRequest.addSoapObject(CreationDateRequest);
                    
                        SoapObject UsernamePasswordRequest = new SoapObject(null, "Node");
                            UsernamePasswordRequest.addProperty("NodeName", "UsernamePassword");
                    
                            SoapObject UsernameRequest = new SoapObject(null, "Node");
                                UsernameRequest.addProperty("NodeName", "Username");                                        
                                UsernameRequest.addProperty("Value", credInfo.credential.usernamePassword.Username);
                            UsernamePasswordRequest.addSoapObject(UsernameRequest);
                    
                            SoapObject PasswordRequest = new SoapObject(null, "Node");
                                PasswordRequest.addProperty("NodeName", "Password");                                        
                                PasswordRequest.addProperty("Value", credInfo.credential.usernamePassword.Password);
                            UsernamePasswordRequest.addSoapObject(PasswordRequest);  
                    
                            SoapObject MachineManagedRequest = new SoapObject(null, "Node");
                                MachineManagedRequest.addProperty("NodeName", "MachineManaged"); 
                                if(credInfo.credential.usernamePassword.MachineManaged){
                                    MachineManagedRequest.addProperty("Value", "TRUE");
                                }else{
                                    MachineManagedRequest.addProperty("Value", "FALSE");
                                }
                            UsernamePasswordRequest.addSoapObject(MachineManagedRequest);
                    
                            SoapObject EAPMethodRequest = new SoapObject(null, "Node");
                                EAPMethodRequest.addProperty("NodeName", "EAPMethod");
                    
                                SoapObject EAPTypeRequest = new SoapObject(null, "Node");
                                    EAPTypeRequest.addProperty("NodeName", "EAPType");                                        
                                    EAPTypeRequest.addProperty("Value", credInfo.credential.usernamePassword.eAPMethod.EAPType);
                                EAPMethodRequest.addSoapObject(EAPTypeRequest);  
                    
                                SoapObject InnerMethodRequest = new SoapObject(null, "Node");
                                    InnerMethodRequest.addProperty("NodeName", "InnerMethod");                                        
                                    InnerMethodRequest.addProperty("Value", credInfo.credential.usernamePassword.eAPMethod.InnerMethod);
                                EAPMethodRequest.addSoapObject(InnerMethodRequest);  
                            UsernamePasswordRequest.addSoapObject(EAPMethodRequest);
                            
                        CredentialRequest.addSoapObject(UsernamePasswordRequest);
                    
                        SoapObject RealmRequest = new SoapObject(null, "Node");
                            RealmRequest.addProperty("NodeName", "Realm");                                        
                            RealmRequest.addProperty("Value", credInfo.credential.Realm);
                        CredentialRequest.addSoapObject(RealmRequest);
                        
                    x1Request.addSoapObject(CredentialRequest);
                    
                PerProviderSubscriptionRequest.addSoapObject(x1Request);
                
            MgmtTreeRequest.addSoapObject(PerProviderSubscriptionRequest);
        
        nsRequest.addSoapObject(MgmtTreeRequest);

        return nsRequest;
    }

    private SoapObject getPolicyUpdateRequest(String requestReason){
        //Construct sppPostDevData element
        SoapObject request = new SoapObject(NAMESPACE_NS, WIFI_SOAP_POST_DEV_DATA);
        //sppVersion
        AttributeInfo attributeInfo = new AttributeInfo();
        attributeInfo.setName(WIFI_SOAP_SPP_VERSION);
        attributeInfo.setValue("1.0");
        attributeInfo.setType("PropertyInfo.STRING_CLASS");
        attributeInfo.setNamespace(NAMESPACE_NS);
        request.addAttribute(attributeInfo);
        //requestReason
        request.addAttribute(WIFI_SOAP_REQ_REASON, requestReason);
        //sessionID        
        if(!POL_UPDATE.equals(requestReason)){
            AttributeInfo sessionIDattributeInfo = new AttributeInfo();        
            sessionIDattributeInfo.setName(WIFI_SOAP_SESSIONID);
            sessionIDattributeInfo.setValue(SessionID);
            sessionIDattributeInfo.setType("PropertyInfo.STRING_CLASS");
            sessionIDattributeInfo.setNamespace(NAMESPACE_NS);
            request.addAttribute(sessionIDattributeInfo);
        }
        
        //New supportedSPPVersions element
        request.addProperty(WIFI_SOAP_S_SPP_VERSION, "1.0");
        //New supportedMOList
        request.addProperty(WIFI_SOAP_S_MOLIST, WIFI_SOAP_S_MOSUBSCRIPTION 
                                                                + " " + WIFI_SOAP_S_MODEVINFO 
                                                                + " " + WIFI_SOAP_S_MODEVDETAIL 
                                                                + " " + WIFI_SOAP_S_MOHS20);



        //New moContainer
        //Construct moContainer
        SoapObject moInfo = getMoInfo();
        SoapObject moDetail = getMoDetail();
        request.addSoapObject(moInfo);
        request.addSoapObject(moDetail);

        //New moContainer
        //Construct moContainer
        if(POL_MO_UPLOAD.equals(requestReason)){
            SoapObject subscription = getSubscription();
            request.addSoapObject(subscription);
        }
        return request;
    }

    private SoapObject getUpdateResponse(boolean errorOccur, String reason){
        //Construct sppUpdateResponse element
        SoapObject request = new SoapObject(NAMESPACE_NS, WIFI_SOAP_UPDATE_RESPONSE);
        //sppVersion
        AttributeInfo attributeInfo = new AttributeInfo();
        attributeInfo.setName(WIFI_SOAP_SPP_VERSION);
        attributeInfo.setValue("1.0");
        attributeInfo.setType("PropertyInfo.STRING_CLASS");
        attributeInfo.setNamespace(NAMESPACE_NS);
        request.addAttribute(attributeInfo);
        //sppStatus
        AttributeInfo attributeSppStatus = new AttributeInfo();
        attributeSppStatus.setName(WIFI_SOAP_SPP_STATUS);
        if(errorOccur == true){
            attributeSppStatus.setValue(WIFI_SOAP_SPP_STATUS_ERROR_OCCURRED);
        }
        else{
            attributeSppStatus.setValue(WIFI_SOAP_SPP_STATUS_OK);
        }
        attributeSppStatus.setType("PropertyInfo.STRING_CLASS");
        attributeSppStatus.setNamespace(NAMESPACE_NS);
        request.addAttribute(attributeSppStatus);
        //sessionID
        AttributeInfo sessionIDattributeInfo = new AttributeInfo();        
        sessionIDattributeInfo.setName(WIFI_SOAP_SESSIONID);
        sessionIDattributeInfo.setValue(SessionID);
        sessionIDattributeInfo.setType("PropertyInfo.STRING_CLASS");
        sessionIDattributeInfo.setNamespace(NAMESPACE_NS);
        request.addAttribute(sessionIDattributeInfo);

        
        if(errorOccur == true){
            //sppError
            SoapObject sppError = new SoapObject(NAMESPACE_NS, WIFI_SOAP_SPP_ERROR);
            //errorCode
            AttributeInfo attributeInfoErrorCode = new AttributeInfo();
            attributeInfoErrorCode.setName(WIFI_SOAP_ERROR_CODE);
            attributeInfoErrorCode.setValue(reason);
            attributeInfoErrorCode.setType("PropertyInfo.STRING_CLASS");
            sppError.addAttribute(attributeInfoErrorCode);
            
            request.addSoapObject(sppError);
        }
        
        return request;
    }

    private SoapObject getMoInfo(){
        //New moContainer
        //Construct moContainer
        SoapObject dmMoRequest = new SoapObject(NAMESPACE_NS, WIFI_SOAP_MO_CONTAINER);
        AttributeInfo attributeInfo = new AttributeInfo();
        attributeInfo.setName(WIFI_SOAP_MO_URN);
        attributeInfo.setValue(WIFI_SOAP_S_MODEVINFO);
        attributeInfo.setType("PropertyInfo.STRING_CLASS");
        attributeInfo.setNamespace(NAMESPACE_NS);
        dmMoRequest.addAttribute(attributeInfo);

        //DevInfo
        /*
        sample:
            v4
                <![CDATA[<DevInfo xmlns="urn:oma:mo:oma-dm-devinfo:1.0">
                    <DevId> urn:acme:00-11-22-33-44-55 </DevId>
                    <Man>ACME</Man>
                    <Mod>HS2.0-01</Mod>
                    <DmV>1.2</DmV>
                    <Lang>en-US</Lang>
                </DevInfo>
                ]]>

            v6
                <![CDATA[ <MgmtTree>
        					<VerDTD>1.2</VerDTD>
        						<Node>
        							<NodeName>DevInfo</NodeName>
        							<RTProperties>
        								<Type>
        									<DDFName>urn:oma:mo:oma-dm-devinfo:1.0</DDFName>
        								</Type>
        							</RTProperties>
        							<Node>
        							     <NodeName>DevID</NodeName>
        									<Value>urn:acme:00-11-22-33-44-55</Value>
        							</Node>
        							<Node>
            							<NodeName>Man</NodeName>
            							<Value>ACME</Value>
        							</Node>
        							<Node>
            							<NodeName>Mod</NodeName>
            							<Value>HS2.0-01</Value>
        							</Node>
        							<Node>
            							<NodeName>DmV</NodeName>
            							<Value>1.2</Value>
        							</Node>
        							<Node>
            							<NodeName>Lang</NodeName>
            							<Value>en-US</Value>
        							</Node>
        						</Node>
        				</MgmtTree> ]]>            
     */     
        if ("true".equals(SystemProperties.get("persist.service.soapv4"))){  // format v4
            Log.d(TAG, "[getMoInfo] v4");
            
            SoapObject devInfoRequest = new SoapObject(null, "DevInfo");
            AttributeInfo MODevInfoattributeInfo = new AttributeInfo();
            MODevInfoattributeInfo.setName(WIFI_SOAP_MO_XMLNS);
            MODevInfoattributeInfo.setValue(WIFI_SOAP_S_MODEVINFO);
            MODevInfoattributeInfo.setType("PropertyInfo.STRING_CLASS");
            devInfoRequest.addAttribute(MODevInfoattributeInfo);
            
            devInfoRequest.addProperty("DevId", "imei:868145012164046");
            devInfoRequest.addProperty("Man", "Mediatek");
            devInfoRequest.addProperty("Mod", "HS20-station");        
            devInfoRequest.addProperty("DmV", "1.2");        
            //devInfoRequest.addProperty("Lang", "en-US");
            devInfoRequest.addProperty("Lang", get2LettersSystemLanguageCode());
            
            //Add to DM tree
            dmMoRequest.addSoapObject(devInfoRequest);            
        }
        else{                                                           //default format v6
            Log.d(TAG, "[getMoInfo] v6");
            
            SoapObject devInfoRequest = new SoapObject(null, WIFI_SOAP_MGMTREE);
                //xmlns
                AttributeInfo MoDetailattributeInfo = new AttributeInfo();
                MoDetailattributeInfo.setName(WIFI_SOAP_MO_XMLNS);
                MoDetailattributeInfo.setValue(WIFI_SOAP_SYNCML_DMDDF_1_2);
                MoDetailattributeInfo.setType("PropertyInfo.STRING_CLASS");                
                devInfoRequest.addAttribute(MoDetailattributeInfo);  

            
                devInfoRequest.addProperty("VerDTD", "1.2");

                SoapObject node1Request = new SoapObject(null, "Node");
                    node1Request.addProperty("NodeName", "DevInfo");
                    
                    SoapObject rtPropertiesRequest = new SoapObject(null, "RTProperties");
                        SoapObject typeRequest = new SoapObject(null, "Type");
                            typeRequest.addProperty("DDFName", WIFI_SOAP_S_MODEVINFO);
                        rtPropertiesRequest.addSoapObject(typeRequest);
                    node1Request.addSoapObject(rtPropertiesRequest);

                    SoapObject node1aRequest = new SoapObject(null, "Node");
                        node1aRequest.addProperty("NodeName", "DevId");
                        node1aRequest.addProperty("Value", "imei:" + getDeviceId());                    
                    node1Request.addSoapObject(node1aRequest);
                    
                    SoapObject node1bRequest = new SoapObject(null, "Node");
                        node1bRequest.addProperty("NodeName", "Man");
                        node1bRequest.addProperty("Value", "Mediatek");                    
                    node1Request.addSoapObject(node1bRequest);

                    SoapObject node1cRequest = new SoapObject(null, "Node");
                        node1cRequest.addProperty("NodeName", "Mod");
                        node1cRequest.addProperty("Value", "HS20-station");                    
                    node1Request.addSoapObject(node1cRequest);

                    SoapObject node1dRequest = new SoapObject(null, "Node");
                        node1dRequest.addProperty("NodeName", "DmV");
                        node1dRequest.addProperty("Value", "1.2");                    
                    node1Request.addSoapObject(node1dRequest);

                    SoapObject node1eRequest = new SoapObject(null, "Node");
                        node1eRequest.addProperty("NodeName", "Lang");
                        //node1eRequest.addProperty("Value", "en-US");  
                        node1eRequest.addProperty("Value", get2LettersSystemLanguageCode());
                    node1Request.addSoapObject(node1eRequest);
                    
                devInfoRequest.addSoapObject(node1Request);

            //Add to DM tree
            dmMoRequest.addSoapObject(devInfoRequest);                
        }


        return dmMoRequest;
    }

    private SoapObject getMoDetail(){
        //New moContainer
        //Construct moContainer
        Log.d(TAG, "[getMoDetail]");        
        SoapObject dmMoRequest = new SoapObject(NAMESPACE_NS, WIFI_SOAP_MO_CONTAINER);
        AttributeInfo attributeInfo = new AttributeInfo();
        attributeInfo.setName(WIFI_SOAP_MO_URN);
        attributeInfo.setValue(WIFI_SOAP_S_MODEVDETAIL);
        attributeInfo.setType("PropertyInfo.STRING_CLASS");
        attributeInfo.setNamespace(NAMESPACE_NS);
        dmMoRequest.addAttribute(attributeInfo);

        //DevDetail
        /*
     sample:
             <![CDATA[ <MgmtTree>
                 <VerDTD>1.2</VerDTD>
                 <Node>
                     <NodeName>DevDetail</NodeName>
                     <RTProperties>
                         <Type>
                             <DDFName>urn:oma:mo:oma-dm-devdetail:1.0</DDFName>
                         </Type>
                     </RTProperties>
                 </Node>
                 <Node>
                     <NodeName>URI</NodeName>
                     <Node>
                         <NodeName>MaxDepth</NodeName>
                         <Value> 32 </Value>
                     </Node>
                     <Node>
                         <NodeName>MaxTotLen</NodeName>
                         <Value> 2048 </Value>
                     </Node>
                     <Node>
                         <NodeName>MaxSegLen</NodeName>
                         <Value> 64 </Value>
                     </Node>
                 </Node>
                 <Node>
                     <NodeName>DevType</NodeName>
                     <Value> Smartphone </Value>
                 </Node>
                 <Node>
                     <NodeName>OEM</NodeName>
                     <Value> ACME </Value>
                 </Node>
                 <Node>
                     <NodeName>FmV</NodeName>
                     <Value> 1.2.100.5 </Value>
                 </Node>
                 <Node>
                     <NodeName>SmV</NodeName>
                     <Value> 9.11.130 </Value>
                 </Node>
                 <Node>
                     <NodeName>HmV</NodeName>
                     <Value> 1.0 </Value>
                 </Node>
                 <Node>
                     <NodeName>LrgObj</NodeName>
                     <Value>FALSE</Value>
                 </Node>
             </MgmtTree> ]]>
        
     */
        SoapObject MgmtTreeRequest = new SoapObject(null, WIFI_SOAP_MGMTREE);
            //xmlns
            AttributeInfo MoDetailattributeInfo = new AttributeInfo();
            MoDetailattributeInfo.setName(WIFI_SOAP_MO_XMLNS);
            MoDetailattributeInfo.setValue(WIFI_SOAP_SYNCML_DMDDF_1_2);
            MoDetailattributeInfo.setType("PropertyInfo.STRING_CLASS");                
            MgmtTreeRequest.addAttribute(MoDetailattributeInfo);  

            //VerDTD
            MgmtTreeRequest.addProperty("VerDTD", "1.2");

            //DevDetail
            SoapObject DevDetailRequest = new SoapObject(null, "Node");
                DevDetailRequest.addProperty("NodeName","DevDetail");
                
                SoapObject rtPerpertiesRequest = new SoapObject(null, "RTProperties");
                    SoapObject typePerpertiesRequest = new SoapObject(null, "Type");
                        typePerpertiesRequest.addProperty("DDFName", WIFI_SOAP_S_MODEVDETAIL);
                    rtPerpertiesRequest.addSoapObject(typePerpertiesRequest);
                DevDetailRequest.addSoapObject(rtPerpertiesRequest);

                //Ext
                SoapObject extRequest = new SoapObject(null, "Node");
                    extRequest.addProperty("NodeName", "Ext");
                    
                    SoapObject wifiOrgRequest = new SoapObject(null, "Node");  
                        wifiOrgRequest.addProperty("NodeName", "org.wi-fi");
                        
                        SoapObject wifiRequest = new SoapObject(null, "Node");
                            wifiRequest.addProperty("NodeName", "Wi-Fi");
                            //EAPMethodList
                            SoapObject eapMethodListRequest = new SoapObject(null, "Node");
                                eapMethodListRequest.addProperty("NodeName", "EAPMethodList");
                                
                                SoapObject eapMethod1Request = new SoapObject(null, "Node");
                                    eapMethod1Request.addProperty("NodeName", "EAPMethod1");

                                    SoapObject eapMethod1 = new SoapObject(null, "Node");
                                        eapMethod1.addProperty("NodeName", "EAPMethod");//TLS
                                        eapMethod1.addProperty("Value", "13");
                                     eapMethod1Request.addSoapObject(eapMethod1);
                                eapMethodListRequest.addSoapObject(eapMethod1Request);

                                SoapObject eapMethod2Request = new SoapObject(null, "Node");
                                    eapMethod2Request.addProperty("NodeName", "EAPMethod2");

                                    SoapObject eapMethod2 = new SoapObject(null, "Node");
                                        eapMethod2.addProperty("NodeName", "EAPMethod");//TTLS
                                        eapMethod2.addProperty("Value", "21");
                                        eapMethod2.addProperty("NodeName", "InnerEAPMethod");//MSCHAPv2
                                        eapMethod2.addProperty("Value", "27");
                                     eapMethod2Request.addSoapObject(eapMethod2);
                                eapMethodListRequest.addSoapObject(eapMethod2Request);                                                                       

                                SoapObject eapMethod3Request = new SoapObject(null, "Node");
                                    eapMethod3Request.addProperty("NodeName", "EAPMethod3");

                                    SoapObject eapMethod3 = new SoapObject(null, "Node");
                                        eapMethod3.addProperty("NodeName", "EAPMethod");//SIM
                                        eapMethod3.addProperty("Value", "18");
                                     eapMethod3Request.addSoapObject(eapMethod3);
                                eapMethodListRequest.addSoapObject(eapMethod3Request); 

                                SoapObject eapMethod4Request = new SoapObject(null, "Node");
                                    eapMethod4Request.addProperty("NodeName", "EAPMethod4");

                                    SoapObject eapMethod4 = new SoapObject(null, "Node");
                                        eapMethod4.addProperty("NodeName", "EAPMethod");//AKA
                                        eapMethod4.addProperty("Value", "23");
                                     eapMethod4Request.addSoapObject(eapMethod4);
                                eapMethodListRequest.addSoapObject(eapMethod4Request); 

                            wifiRequest.addSoapObject(eapMethodListRequest);
                
                            //IMSI                           
                            if ("false".equals(SystemProperties.get("persist.service.manual.ut.rem"))) {
                                WifiTree.CredentialInfo credInfo = mTreeHelper.getCredentialInfo(mSoapTree, mCred.getWifiSPFQDN(), mCred.getCredName());
                                if(credInfo != null && 
                                   credInfo.credential != null &&
                                   credInfo.credential.sim != null){
                                    mImsi = credInfo.credential.sim.IMSI;
                                    SoapObject imsiRequest = new SoapObject(null, "Node");
                                        imsiRequest.addProperty("NodeName", "IMSI");                                    
                                        imsiRequest.addProperty("Value", mImsi);
                                    wifiRequest.addSoapObject(imsiRequest);
                                }
                            } else {
                                SoapObject imsiRequest = new SoapObject(null, "Node");
                                    imsiRequest.addProperty("NodeName", "IMSI");                                    
                                    imsiRequest.addProperty("Value", "40002600000004");
                                wifiRequest.addSoapObject(imsiRequest);
                            }

                            
                            //ManufacturingCertificate
                            SoapObject manufactCertRequest = new SoapObject(null, "Node");
                                manufactCertRequest.addProperty("NodeName", "ManufacturingCertificate");                                    
                                manufactCertRequest.addProperty("Value", "false");
                            wifiRequest.addSoapObject(manufactCertRequest);                                

                            // Wi-FiMACAddress
                            SoapObject wifiMacAddressRequest = new SoapObject(null, "Node");
                                wifiMacAddressRequest.addProperty("NodeName", "Wi-FiMACAddress");
                                WifiManager wifiManager = (WifiManager) mContext.getSystemService(Context.WIFI_SERVICE);                                
                                WifiInfo wifiInfo = wifiManager.getConnectionInfo(); 
                                if (wifiInfo == null) {
                                    Log.d(TAG, "no ConnectionInfo found");
                                    return null;
                                }
                                wifiMacAddressRequest.addProperty("Value", wifiInfo.getMacAddress().replace(":", "").toLowerCase());
                            wifiRequest.addSoapObject(wifiMacAddressRequest); 
                            //ClientTriggerRedirectURI
                            //wifiRequest.addProperty("ClientTriggerRedirectURI", "http://127.0.0.1:54685");
                        wifiOrgRequest.addSoapObject(wifiRequest);
                    extRequest.addSoapObject(wifiOrgRequest);
                DevDetailRequest.addSoapObject(extRequest);

                //URI
                SoapObject URIRequest = new SoapObject(null, "Node");
                    URIRequest.addProperty("NodeName", "URI");
                    
                    SoapObject MaxDepthRequest = new SoapObject(null, "Node");
                        MaxDepthRequest.addProperty("NodeName", "MaxDepth");
                        MaxDepthRequest.addProperty("Value", "32");
                    URIRequest.addSoapObject(MaxDepthRequest);
                
                    SoapObject MaxTotLenRequest = new SoapObject(null, "Node");
                        MaxTotLenRequest.addProperty("NodeName", "MaxTotLen");
                        MaxTotLenRequest.addProperty("Value", "2048");
                    URIRequest.addSoapObject(MaxTotLenRequest);
                
                    SoapObject MaxSegLenRequest = new SoapObject(null, "Node");
                        MaxSegLenRequest.addProperty("NodeName", "MaxSegLen");
                        MaxSegLenRequest.addProperty("Value", "64");
                    URIRequest.addSoapObject(MaxSegLenRequest);
                    
                DevDetailRequest.addSoapObject(URIRequest);

                //Required property
                SoapObject DevTypeRequest = new SoapObject(null, "Node");
                    DevTypeRequest.addProperty("NodeName", "DevType");
                    DevTypeRequest.addProperty("Value", "MobilePhone");
                DevDetailRequest.addSoapObject(DevTypeRequest);

                SoapObject OEMRequest = new SoapObject(null, "Node");
                    OEMRequest.addProperty("NodeName", "OEM");
                    OEMRequest.addProperty("Value", "MEDIATEK");
                DevDetailRequest.addSoapObject(OEMRequest);

                SoapObject FwVRequest = new SoapObject(null, "Node");
                    FwVRequest.addProperty("NodeName", "FwV");
                    FwVRequest.addProperty("Value", "1.0");
                DevDetailRequest.addSoapObject(FwVRequest);

                SoapObject SwVRequest = new SoapObject(null, "Node");
                    SwVRequest.addProperty("NodeName", "SwV");
                    SwVRequest.addProperty("Value", "1.0");
                DevDetailRequest.addSoapObject(SwVRequest);

                SoapObject HwVRequest = new SoapObject(null, "Node");
                    HwVRequest.addProperty("NodeName", "HwV");
                    HwVRequest.addProperty("Value", "1.0");
                DevDetailRequest.addSoapObject(HwVRequest);

                SoapObject LrgObjRequest = new SoapObject(null, "Node");
                    LrgObjRequest.addProperty("NodeName", "LrgObj");
                    LrgObjRequest.addProperty("Value", "FALSE");
                DevDetailRequest.addSoapObject(LrgObjRequest);                        
            MgmtTreeRequest.addSoapObject(DevDetailRequest);

        //Add to DM tree
        dmMoRequest.addSoapObject(MgmtTreeRequest);                
                    
        return dmMoRequest;
    }

    private String checkErrorCode(Document doc){
        return null;
    }
    
    private String checkStatus(Document doc){
        if (doc == null) {
            return null;
        }
        
        NodeList list = doc.getElementsByTagNameNS(NAMESPACE_NS,"sppPostDevDataResponse"); // format type R
        if(list.getLength()!= 0){
            Log.d(TAG, "[checkStatus(Document doc)] format type Q can be removed");
            
            Element element = (Element)list.item(0);
            String sppStatus = element.getAttributeNS(NAMESPACE_NS, WIFI_SOAP_SPP_STATUS);
            Log.d(TAG, "sppStatus: " + sppStatus);
            if(WIFI_SOAP_SPP_STATUS_OK.equals(sppStatus) || WIFI_SOAP_SPP_STATUS_PROVISION_COMPLETE.equals(sppStatus) || WIFI_SOAP_SPP_STATUS_REMEDIATION_COMPLETE.equals(sppStatus) || WIFI_SOAP_SPP_STATUS_UPDATE_COMPLETE.equals(sppStatus)){
                SessionID = element.getAttributeNS(NAMESPACE_NS, "sessionID");
                Log.d(TAG, "sessionID: " + SessionID);
            }
            return sppStatus;            
        }
        else
        {
            list = doc.getElementsByTagName("spp:sppPostDevDataResponse"); // format type Q
            Element element = (Element)list.item(0);
            String sppStatus = element.getAttribute("spp:sppStatus");
            Log.d(TAG, "sppStatus: " + sppStatus);
            if(WIFI_SOAP_SPP_STATUS_OK.equals(sppStatus) || WIFI_SOAP_SPP_STATUS_PROVISION_COMPLETE.equals(sppStatus) || WIFI_SOAP_SPP_STATUS_REMEDIATION_COMPLETE.equals(sppStatus) || WIFI_SOAP_SPP_STATUS_UPDATE_COMPLETE.equals(sppStatus)){
                SessionID = element.getAttributeNS(NAMESPACE_NS, "sessionID");
                Log.d(TAG, "sessionID: " + SessionID);
            }
            return sppStatus;
        }
    }

    private String checkStatus(Document doc, String tagName){
        if (doc == null) {
            return null;
        }
        
        NodeList list = doc.getElementsByTagName(tagName);

        if(list.getLength() == 0){
            return null;
        }
        Element element = (Element)list.item(0);
        String att = element.getAttribute(WIFI_SOAP_SPP_STATUS); // format type Q

        if(att == null || att.isEmpty()){
            Log.d(TAG, "[checkStatus(Document doc, String tagName)] format type Q can be removed");            
            att = element.getAttribute("spp:sppStatus"); // form at type R
        }
        Log.d(TAG, "att:" + att);
        return att;
    }

    private String checkStatus(Document doc, String namespace, String tagName){
        if (doc == null) {
            return null;
        }
        
        NodeList list = doc.getElementsByTagNameNS(namespace, tagName);

        if(list.getLength() == 0){
            return null;
        }
        Element element = (Element)list.item(0);
        String att = element.getAttribute(WIFI_SOAP_SPP_STATUS); // format type Q

        if(att == null || att.isEmpty()){
            Log.d(TAG, "[checkStatus(Document doc, String namespace, String tagName)] format type Q can be removed");                        
            att = element.getAttributeNS(namespace, WIFI_SOAP_SPP_STATUS); // form at type R
        }
        Log.d(TAG, "att:" + att);
        return att;
    }

    private String getErrorCode(Document doc){
        NodeList list = doc.getElementsByTagNameNS(NAMESPACE_NS, WIFI_SOAP_SPP_ERROR); 
        if(list.getLength()!= 0){
            Element element = (Element)list.item(0);
            String errorCode = element.getAttribute(WIFI_SOAP_SPP_ERROR);
            Log.d(TAG, "errorCode: " + errorCode);
            return errorCode;            
        }
        return null;
    }

    private boolean getNoMoUpdate(Document doc){
        Log.d(TAG, "[getNoMoUpdate]");                
        NodeList list = doc.getElementsByTagNameNS(NAMESPACE_NS, WIFI_SOAP_SPP_NOMOUPDATE); 
        if(list.getLength()!= 0){
            Message msg = msgHandler.obtainMessage(PasspointManager.EVENT_REMEDIATION_DONE, 0, 0, mSoapTree);//"DoUpdate"
            msg.sendToTarget();             
            return true;            
        }
        return false;
    }

    private Boolean getUploadMO(Document doc){
        Log.d(TAG, "[getUploadMO]");        
        NodeList list = doc.getElementsByTagNameNS(NAMESPACE_NS, "uploadMO"); // format type R
        if(list.getLength()!= 0){
            Log.d(TAG, "[getUploadMO] format type Q can be removed");        
            
            Element element = (Element)list.item(0);
            mUploadMO = element.getAttributeNS(NAMESPACE_NS, WIFI_SOAP_MO_URN);
            Log.d(TAG, "Upload MO: " + mUploadMO);
            if(mUploadMO != null || !mUploadMO.isEmpty()){
                return true;
            }
        }
        else
        {
            list = doc.getElementsByTagName("spp:uploadMO"); // format type Q
            if(list.getLength()!= 0){
                Element element = (Element)list.item(0);
                mUploadMO = element.getAttribute("spp:" + WIFI_SOAP_MO_URN);
                Log.d(TAG, "Upload MO: " + mUploadMO);
                if(mUploadMO != null || !mUploadMO.isEmpty()){
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * Send XML document to DM service to add MO in the tree
     *
     * @param emMode
     *            switch between real OSU and SIGMA/manual MO installation
     */
    public void sendToDmAddMo(boolean emMode, boolean isSim, String sppTreeUri, Document doc){        
        Log.d(TAG, "[sendToDmAddMo]");
        isOsuEmMode = emMode;
        String moTree = PasspointPpsmoParser.xmlToString(doc);        
        Intent intent;
        if(isSim){
           intent = new Intent("com.mediatek.soap.sim.provision.start");
        }else{
            intent = new Intent("com.mediatek.soap.provision.start");
        }
        String cmd = "addsubtree";
        intent.putExtra("cmd", cmd);
        intent.putExtra("targeturi", sppTreeUri);
        intent.putExtra("data", moTree);
        intent.putExtra("isneedwifitree", true);        
        mContext.sendBroadcast(intent);
    }

    public void sendToDmUpdateMo(int flowtype, Document doc){
        Log.d(TAG, "[sendToDmUpdateMo]");        
        String moTree;
        String sppTreeUri;
        Intent intent;
        if(flowtype == POLICY_UPDATE){
            intent = new Intent("com.mediatek.soap.policy.start");           
        }else if(flowtype == SUBSCRIPTION_REMEDIATION){
            intent = new Intent("com.mediatek.soap.remediation.start");
        }else{
            Log.d(TAG, "err: flow type not defined!!!");
            return;
        }
        String cmd = "replacesubtree";
        Vector<Document> sppUpdateNodes = PasspointPpsmoParser.getSPPNodes(doc, NAMESPACE_NS, "updateNode");
        for(Document docNodes : sppUpdateNodes){
            ++managementTreeUpdateCount;            
            Document docMgmtTree = PasspointPpsmoParser.extractMgmtTree(docNodes);  
            moTree = PasspointPpsmoParser.xmlToString(docMgmtTree); 
            Log.d(TAG2,moTree);
            sppTreeUri = getSPPTreeUri(docNodes, "updateNode");
            intent.putExtra("cmd", cmd);
            intent.putExtra("targeturi", sppTreeUri);
            intent.putExtra("data", moTree);
            if(managementTreeUpdateCount == sppUpdateNodes.size()){
                intent.putExtra("isneedwifitree", true);
            }
            mContext.sendBroadcast(intent);            
        }

    }

    private Boolean getEnrollmentInfo(Document doc){
        Log.d(TAG, "[getEnrollmentInfo]");
        try{
            NodeList list = doc.getElementsByTagNameNS(NAMESPACE_NS,"getCertificate");
            if(list.getLength() != 0){
                Log.d(TAG, "[getEnrollmentInfo] got getCertificate:");
                Element eElement = (Element) list.item(0);
                String att = eElement.getAttribute("enrollmentProtocol");
                if(!"EST".equals(att)){ //EST is allowed only
                    return false;
                }
                mEnrollmentServerURI = PasspointPpsmoParser.getTagValue(NAMESPACE_NS,"enrollmentServerURI", eElement);
                Log.d(TAG, "EnrollmentServerURI: "+mEnrollmentServerURI);
                mEnrollmentServerCert = PasspointPpsmoParser.getTagValue(NAMESPACE_NS,"caCertificate", eElement);
                Log.d(TAG, "caCertificate: "+mEnrollmentServerCert);
                enrollDigestUsername = PasspointPpsmoParser.getTagValue(NAMESPACE_NS,"estUserID", eElement);
                if(enrollDigestUsername == null){
                    enrollDigestUsername = "";
                }
                Log.d(TAG, "enrollDigestUsername: "+enrollDigestUsername);
                String enrollDigestPasswordBase64 = PasspointPpsmoParser.getTagValue(NAMESPACE_NS,"estPassword", eElement);
                if(enrollDigestPasswordBase64 == null){
                    enrollDigestPassword = "";
                }else{
                    enrollDigestPassword = new String(Base64.decode(enrollDigestPasswordBase64));
                }
                Log.d(TAG, "enrollDigestPassword: "+enrollDigestPassword);
            }else{
                return false;
            }
            return true;
        }catch(Exception ee){
            ee.printStackTrace();
            return false;
        }
    }

    private Boolean getUseClientCertTLS(Document doc){
        Log.d(TAG, "[getUseClientCertTLS]");
        try{
            NodeList list = doc.getElementsByTagNameNS(NAMESPACE_NS,"useClientCertTLS"); // form at type R
            if(list.getLength() != 0){
                Element eElement = (Element) list.item(0);
                String attAcceptMfgCerts = eElement.getAttributeNS(NAMESPACE_NS, "acceptMfgCerts");// true/false
                String attAcceptProviderCerts = eElement.getAttributeNS(NAMESPACE_NS, "acceptProviderCerts"); // providerIssuerName 

                // TODO: re-negociate TLS with client certificate
                if(attAcceptProviderCerts.equalsIgnoreCase("true")){
                    providerIssuerName = PasspointPpsmoParser.getTagValue(NAMESPACE_NS,"providerIssuerName", eElement);                   
                }else{
                    providerIssuerName = "";
                }
            }else{
                return false;
            }
            return true;
        }catch(Exception ee){
            //ee.printStackTrace();
            return false;
        }
    }

    private String getSPPTreeUri(Document doc, String execution){
        // ex: spp:addMO spp:managementTreeURI="./Wi-Fi/wi-fi.org/PerProviderSubscription"
        Log.d(TAG, "[getSPPTreeUri]");
        NodeList list = doc.getElementsByTagNameNS(NAMESPACE_NS, execution);
        if(list.getLength()!= 0){
            Element element = (Element)list.item(0);
            String sppTreeUri = element.getAttributeNS(NAMESPACE_NS,"managementTreeURI");
            Log.d(TAG, "managementTreeURI: " + sppTreeUri);
            
            if(sppTreeUri != null && !sppTreeUri.isEmpty()){
                return sppTreeUri;
            }
        }
        
        return null;
    }

    private String getWifiSPFQDNFromMoTree(Document doc, String execution){
        // ex: spp:addMO spp:managementTreeURI="./Wi-Fi/wi-fi.org/PerProviderSubscription"
        Log.d(TAG, "[getWifiSPFQDNFromMoTree]");
        String sppTreeUri = getSPPTreeUri(doc, execution);
        
        if(sppTreeUri != null && !sppTreeUri.isEmpty()){
            String[] words = sppTreeUri.split("/");
            wifiSPFQDN = words[2];
            Log.d(TAG, "wifiSPFQDN: " + wifiSPFQDN);
            return wifiSPFQDN;
        }
        
        return null;
    }

    private boolean checkWifiSPFQDNForAddMo(Document doc){
        Log.d(TAG, "[checkWifiSPFQDNForAddMo]");
        mSPFQDNFromMo = getWifiSPFQDNFromMoTree(doc, "addMO");
        Log.d(TAG, "current wifiSPFQDN: " + mSPFQDN+ ", wifiSPFQDN From MO: " + mSPFQDNFromMo);        
        if(mSPFQDNFromMo != null && mSPFQDN.endsWith(mSPFQDNFromMo)){        
            Log.d(TAG, "[checkWifiSPFQDNForAddMo] pass");            
            return true;            
        }
        Log.d(TAG, "[checkWifiSPFQDNForAddMo] fail");
        return false;
    }

    private boolean checkWifiSPFQDNForUpdateMo(Document doc){
        Log.d(TAG, "[checkWifiSPFQDNForUpdateMo]");
        mSPFQDNFromMo = getWifiSPFQDNFromMoTree(doc, "updateNode");
        Log.d(TAG, "current wifiSPFQDN: " + mSPFQDN+ ", wifiSPFQDN From MO: " + mSPFQDNFromMo);        
        if(mSPFQDNFromMo != null && mSPFQDN.endsWith(mSPFQDNFromMo)){        
            Log.d(TAG, "[checkWifiSPFQDNForUpdateMo] pass");            
            return true;            
        }
        Log.d(TAG, "[checkWifiSPFQDNForUpdateMo] fail");
        return false;
    }

    private boolean checkWifiSPFQDNForUploadMo(Document doc){
        Log.d(TAG, "[checkWifiSPFQDNForUploadMo]");
        mSPFQDNFromMo = getWifiSPFQDNFromMoTree(doc, "uploadMO");
        Log.d(TAG, "current wifiSPFQDN: " + mSPFQDN + ", wifiSPFQDN From MO: " + mSPFQDNFromMo);
        if(mSPFQDNFromMo != null && mSPFQDN.endsWith(mSPFQDNFromMo)){
            Log.d(TAG, "[checkWifiSPFQDNForUploadMo] pass");
            return true;            
        }
        Log.d(TAG, "[checkWifiSPFQDNForUploadMo] fail");
        return false;
    }    
    
    private Boolean getSubscriptionSignUpAndUserUpdate(Document doc){
        Log.d(TAG, "[getSubscriptionSignUpAndUserUpdate]");
        NodeList list = doc.getElementsByTagName("exec"); // format type Q

        if(list.getLength() != 0){
            for(int tmp = 0; tmp < list.getLength(); tmp++){
                Node nNode = list.item(tmp);
                if (nNode.getNodeType() == Node.ELEMENT_NODE) {
                    try{
                        Element eElement = (Element) nNode;
                        Log.d(TAG, "launchBrowserToURI : " + PasspointPpsmoParser.getTagValue("launchBrowserToURI", eElement));
                        mSoapWebUrl = PasspointPpsmoParser.getTagValue("launchBrowserToURI", eElement);
                        if(mSoapWebUrl == null){
                            return false;
                        }
                        Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse(mSoapWebUrl));
                        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                        mContext.startActivity(intent);
    
                        Log.d(TAG, "value : " + mSoapWebUrl);
                        return true;                        
                    }catch(Exception ee){
                        //ee.printStackTrace();
                        return false;
                    }
                }
            }
        }
        else{
            Log.d(TAG, "[getSubscriptionSignUpAndUserUpdate] format type Q can be removed");            
            list = doc.getElementsByTagNameNS(NAMESPACE_NS,"exec"); // format type R
            
            for(int tmp = 0; tmp < list.getLength(); tmp++){
                Node nNode = list.item(tmp);
                if (nNode.getNodeType() == Node.ELEMENT_NODE) {
                    try{
                        Element eElement = (Element) nNode;
                        Log.d(TAG, "launchBrowserToURI : " + PasspointPpsmoParser.getTagValue(NAMESPACE_NS, "launchBrowserToURI", eElement));
                        mSoapWebUrl = PasspointPpsmoParser.getTagValue(NAMESPACE_NS, "launchBrowserToURI", eElement);
                        //for plugfest only
                        /*int start = mSoapWebUrl.indexOf(":",8);
                        String temp = mSoapWebUrl.substring(start);
                        mSoapWebUrl = "https://192.165.15.1" + temp;*/
                        //end plugfest only
                        if(mSoapWebUrl == null){
                            return false;
                        }
                        Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse(mSoapWebUrl));
                        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                        mContext.startActivity(intent);
            
                        Log.d(TAG, "value : " + mSoapWebUrl);
                        return true;
                    }catch(Exception ee){
                        //ee.printStackTrace();
                        return false;
                    }
                }
            }
            
        }
        
        return false;
    }            
    
    private String get2LettersSystemLanguageCode(){
        Locale loc = Locale.getDefault();                    
        return loc.getLanguage();
    }

    private String filenameFromURI(String uri){
        String filename = uri.substring(uri.lastIndexOf("/") + 1);
        return filename;
    }
    
    private boolean checkExtendedKeyUsageIdKpClientAuth(final X509Certificate x509Cert) {
        boolean result = true;
        try {
            List<String> extKeyUsages = x509Cert.getExtendedKeyUsage();
            if(extKeyUsages != null){
                result = false;
                for (String extKeyUsage : extKeyUsages) {
                    Log.d(TAG2,"ExtendedKeyUsage:" + extKeyUsage);
                    if(extKeyUsage.equals(KeyPurposeId.id_kp_serverAuth.toString())){ 
                        Log.d(TAG, "Server certificate EKU includes id_kp_serverAuth, true");
                        result = true;
                        break;
                    }                    
                }               
            }
        
        } catch (CertificateParsingException e) {
            e.printStackTrace();
        }

        if (!result) Log.d(TAG, "Server certificate EKU not includes id_kp_serverAuth, false");
        return result;

    }

    private boolean checkSubjectAltNameOtherNameSPLangFriendlyName(final X509Certificate x509Cert) {
        boolean result = false;

        if(mOSUFriendlyName == null){
            return false;
        }else if(mOSUFriendlyName.isEmpty()){
            return false;
        }

        try {
            Collection c = x509Cert.getSubjectAlternativeNames();
            
            if(c != null){
                Iterator it = c.iterator();
                while (it.hasNext()) {
                    List gn = (List) it.next();
                    Integer tag = (Integer) gn.get(0);
                    if(tag == GeneralName.otherName){
                        Log.d(TAG2,"SubjectAltName OtherName:"
                                + gn.get(1).toString());
                        
                        ByteArrayInputStream bais = new ByteArrayInputStream((byte[]) gn.get(1));
                        ASN1InputStream asn1_is = new ASN1InputStream(bais);
                        Object asn1Obj = (Object)asn1_is.readObject();
                        Log.d(TAG2,ASN1Dump.dumpAsString(asn1Obj, true));

                        DERTaggedObject derTagObj = (DERTaggedObject) asn1Obj;                       
                        DERSequence derSeq = (DERSequence)derTagObj.getObject();
                        Enumeration enu = derSeq.getObjects();
                        DERObjectIdentifier oid = (DERObjectIdentifier) ((ASN1Encodable) enu.nextElement()).toASN1Primitive();
                        Log.d(TAG2,"    OID:" + oid.toString());
                        
                        if("1.3.6.1.4.1.40808.1.1.1".equals(oid.toString())){ //id-wfa-hotspot-friendly-name
                            DERTaggedObject dertagObj = (DERTaggedObject) ((ASN1Encodable) enu.nextElement()).toASN1Primitive();
                            ASN1Object derObj = dertagObj.getObject();
                            String spLangFriendlyName;
                            if(derObj instanceof DERUTF8String){
                                DERUTF8String spLangFriendlyNameDERString = (DERUTF8String)(dertagObj.getObject());
                                spLangFriendlyName = spLangFriendlyNameDERString.toString();
                            }else{
                                DEROctetString spLangFriendlyNameDERString = (DEROctetString)(dertagObj.getObject());                            
                                spLangFriendlyName = spLangFriendlyNameDERString.toString();
                            }
                             
                            Log.d(TAG, "language code and friendly name:" + spLangFriendlyName.toString());

                            //check language code                  
                            Log.d(TAG, "mOSULanguage = "+mOSULanguage);
                            if(spLangFriendlyName.substring(0,3).equals(mOSULanguage.toLowerCase())){ //ISO639
                                Log.d(TAG, "Language code match"); 
                                
                              //check friendly name
                                Log.d(TAG, "mOSUFriendlyName = "+mOSUFriendlyName);
                                if(mOSUFriendlyName != null && !mOSUFriendlyName.isEmpty() && mOSUFriendlyName.length() != 0){
                                    if(spLangFriendlyName.substring(3).equals(mOSUFriendlyName)){
                                        Log.d(TAG, "OSU friendly name match");                     
                                        result =  true;
                                        return result;
                                    }else{
                                        Log.d(TAG, "OSU friendly name not match");                     
                                    }
                                }                                
                            }else{
                                Log.d(TAG, "Language code not match"); 
                            }                                         
                        }
                    }
                }
                Log.d(TAG2,"Subject Alternative Names:" + c.toString());                
            }           
        } catch (CertificateParsingException e) {
            e.printStackTrace();
        } catch(Exception e){
            e.printStackTrace();
        }

        return result;
    }

    private boolean checkSubjectAltNameDNSName(final X509Certificate x509Cert, final String fqdn, boolean suffixMatch) {
        boolean result = false;

        try {
            Collection c = x509Cert.getSubjectAlternativeNames();
            
            if(c != null){
                Log.d(TAG2,"Subject Alternative Names:" + c.toString());                
                
                Iterator it = c.iterator();
                while (it.hasNext()) {
                    List gn = (List) it.next();
                    Integer tag = (Integer) gn.get(0);
                    if(tag == GeneralName.dNSName){
                        Log.d(TAG2,"Subject Alternative Name:" + gn.get(1));
                        if(suffixMatch){
                            String value = (String)(gn.get(1));
                            if(value.endsWith(fqdn)){                            
                                Log.d(TAG, "Subject Alternative DNS Name suffix match SPFQDN"); 
                                result = true;
                            }
                        }else{//complete match
                            if(gn.get(1).equals(fqdn)){                            
                                Log.d(TAG, "Subject Alternative DNS Name complete match SPFQDN"); 
                                result = true;
                            }
                        }

                    }
                }
            }           
        } catch (CertificateParsingException e) {
            e.printStackTrace();
        }

        return result;
    }

    private boolean checkLogotypeExtn(final X509Certificate x509Cert){
            if(iconHash == null){ //icon doesn't successfully downloaded and displayed, bypass the check
                return true;
            }

            boolean result = true;
    
            try {        
                // Extension Value
                
                byte[] logoType = x509Cert.getExtensionValue("1.3.6.1.5.5.7.1.12");
                if(logoType != null){
                    ByteArrayInputStream bais = new ByteArrayInputStream(logoType);
                    ASN1InputStream asn1_is = new ASN1InputStream(bais);
                    DEROctetString derObj = (DEROctetString) asn1_is.readObject(); 
                                          
                    bais = (ByteArrayInputStream)derObj.getOctetStream();
                    asn1_is = new ASN1InputStream(bais);            
                    DERSequence logoTypeExt = (DERSequence) asn1_is.readObject();   
                    Log.d(TAG2,"LogotypeExtn:" + logoTypeExt.toString()); 
                    
                    Enumeration LogotypeExtnSequence = logoTypeExt.getObjects();
                    while(LogotypeExtnSequence.hasMoreElements()){
                        
                        DERTaggedObject LogotypeExtnTaggedObj = (DERTaggedObject) ((ASN1Encodable)LogotypeExtnSequence.nextElement()).toASN1Primitive();
                        Log.d(TAG2,"LogotypeExtnTaggedObj:" + LogotypeExtnTaggedObj.toString());
                        Log.d(TAG2,"LogotypeExtnTaggedObj CHOICE: " + LogotypeExtnTaggedObj.getTagNo());
    
                        /*LogotypeExtn ::= SEQUENCE {
                                communityLogos  [0] EXPLICIT SEQUENCE OF LogotypeInfo OPTIONAL,
                                issuerLogo      [1] EXPLICIT LogotypeInfo OPTIONAL,
                                subjectLogo     [2] EXPLICIT LogotypeInfo OPTIONAL,
                                otherLogos      [3] EXPLICIT SEQUENCE OF OtherLogotypeInfo OPTIONAL }
                     
                                 */                    
                        if(LogotypeExtnTaggedObj.getTagNo() == 0){//communityLogos 
                         
                            Log.d(TAG2,"");
                            DERSequence CommunityLogos = (DERSequence) LogotypeExtnTaggedObj.getObject();
                            Log.d(TAG2,"communityLogos:" + CommunityLogos.toString());                    
                            
                          
                            Enumeration enu;
                            Enumeration CommunityLogosEnu = CommunityLogos.getObjects();
                            while(CommunityLogosEnu.hasMoreElements()){
                                result = true;
                                
                                DERTaggedObject CommunityLogosTaggedObj = (DERTaggedObject) ((ASN1Encodable)CommunityLogosEnu.nextElement()).toASN1Primitive();
                                Log.d(TAG2,"CommunityLogosTaggedObj CHOICE: " + CommunityLogosTaggedObj.getTagNo());
                                /* LogotypeInfo ::= CHOICE {
                                               * direct          [0] LogotypeData,
                                               * indirect        [1] LogotypeReference }                    
                                               */                           
                                if(CommunityLogosTaggedObj.getTagNo() == 0){//direct
                                    /*********************************************
                                                     ************         image      *************
                                                     *********************************************/ 
                                    /* LogotypeData ::= SEQUENCE {
                                                    * image           SEQUENCE OF LogotypeImage OPTIONAL,
                                                    * audio           [1] SEQUENCE OF LogotypeAudio OPTIONAL }                 
                                                    */                             
                                    DERSequence LogotypeData = (DERSequence) CommunityLogosTaggedObj.getObject(); ;
                                    Log.d(TAG2,"LogotypeImage:" + LogotypeData.toString());
                                    Enumeration LogotypeDataEnu = LogotypeData.getObjects();                      
                                    while(LogotypeDataEnu.hasMoreElements()){
                                        /* LogotypeImage ::= SEQUENCE {
                                                          * imageDetails    LogotypeDetails,
                                                          * imageInfo       LogotypeImageInfo OPTIONAL }             
                                                          */                        
                                        DERSequence LogotypeImage = (DERSequence) ((ASN1Encodable)LogotypeDataEnu.nextElement()).toASN1Primitive();
                                        Log.d(TAG2,"LogotypeImage:" + LogotypeImage.toString()); 
                                        Enumeration LogotypeImageEnu = LogotypeImage.getObjects();
                                        while(LogotypeImageEnu.hasMoreElements()){
                                            DERSequence imageDetails = (DERSequence) ((ASN1Encodable)LogotypeImageEnu.nextElement()).toASN1Primitive();                                
                                            /* LogotypeImageInfo ::= SEQUENCE {
                                                                * type            [0] LogotypeImageType DEFAULT color,
                                                                * fileSize        INTEGER,  -- In octets
                                                                * xSize           INTEGER,  -- Horizontal size in pixels
                                                                * ySize           INTEGER,  -- Vertical size in pixels
                                                                * resolution      LogotypeImageResolution OPTIONAL,
                                                                * language        [4] IA5String OPTIONAL }  -- RFC 3066 Language Tag                       
                                                                */
                                            DERSequence imageInfo = (DERSequence) ((ASN1Encodable)LogotypeImageEnu.nextElement()).toASN1Primitive();
                                            Log.d(TAG2,"imageInfo:" + imageInfo.toString()); 
                                            enu = imageInfo.getObjects();
                                            while(enu.hasMoreElements()){
                                                ASN1Object info = ((ASN1Encodable)enu.nextElement()).toASN1Primitive();
                                                Log.d(TAG2,"object:" + info.toString());
                                                if(info instanceof DERTaggedObject){
                                                    if(((DERTaggedObject)info).getTagNo() == 4){
                                                        DEROctetString language = (DEROctetString) ((DERTaggedObject) info).getObject();
                                                        String languageCode = new String(language.getEncoded()).substring(2);
                                                        Log.d(TAG2,"imageInfo language code:" + languageCode);
                                                        /*if(mOSULanguage.equals(languageCode)){
                                                            result = true;
                                                        }else{
                                                            result = false;
                                                        }*/
                                                    }
                                                }
                                            }                                                                                                                             
                                        
                                            /*if(result == false){
                                                continue;
                                            }*/
                                            
                                            /* LogotypeDetails ::= SEQUENCE {
                                                                * mediaType       IA5String, -- MIME media type name and optional
                                                                                       -- parameters
                                                                * logotypeHash    SEQUENCE SIZE (1..MAX) OF HashAlgAndValue,
                                                                * logotypeURI     SEQUENCE SIZE (1..MAX) OF IA5String }
                                                                */                             
                                            Log.d(TAG2,"imageDetails:" + imageDetails.toString());            
                                            enu = imageDetails.getObjects();
                                        
                                            //mediaType                             
                                            DERIA5String mediaType = (DERIA5String) ((ASN1Encodable)enu.nextElement()).toASN1Primitive();
                                            Log.d(TAG2,"mediaType:" + mediaType.toString());
                                            DERSequence logotypeHash = (DERSequence)((ASN1Encodable)enu.nextElement()).toASN1Primitive();
                                            Log.d(TAG2,"logotypeHash:" + logotypeHash.toString());            
                                            DERSequence logotypeURI = (DERSequence)((ASN1Encodable)enu.nextElement()).toASN1Primitive();
                                            Log.d(TAG2,"logotypeURI:" + logotypeURI.toString());
                                        
                                            //logotypeURI
                                            enu = logotypeURI.getObjects();
                                            DERIA5String logotypeURIStr = (DERIA5String) ((ASN1Encodable)enu.nextElement()).toASN1Primitive();
                                            Log.d(TAG2,"logotypeURIStr:" + logotypeURIStr.toString());
                                            Log.d(TAG2,"filename : (" + filenameFromURI(logotypeURI.toString()) + ")");                    
                                            if(iconFileName.equals(filenameFromURI(logotypeURIStr.toString()))){
                                                Log.d(TAG, "Icon filename match");                    
                                                result = true;
                                            }else{
                                                Log.d(TAG, "Icon filename not match");                    
                                                result = false;
                                                continue;
                                            }
                                            
                                            //logotypeHash
                                            enu = logotypeHash.getObjects();
                                            DERSequence HashAlgAndValue = (DERSequence) ((ASN1Encodable)enu.nextElement()).toASN1Primitive();
                                            Log.d(TAG2,"HashAlgAndValue:" + HashAlgAndValue.toString());
                                            enu = HashAlgAndValue.getObjects();
                                            //hashAlg
                                            DERSequence hashAlg = (DERSequence) ((ASN1Encodable)enu.nextElement()).toASN1Primitive();           
                                            Log.d(TAG2,"hashAlg:" + hashAlg.toString());            
                                            //hashValue
                                            DEROctetString hashValue = (DEROctetString) ((ASN1Encodable)enu.nextElement()).toASN1Primitive();            
                                            Log.d(TAG2,"hashValue:" + hashValue.toString());
                                            //hashAlg --> AlgorithmIdentifier
                                            enu = hashAlg.getObjects();
                                            DERObjectIdentifier AlgorithmIdentifier = (DERObjectIdentifier) ((ASN1Encodable)enu.nextElement()).toASN1Primitive();
                                            Log.d(TAG2,"AlgorithmIdentifier:" + AlgorithmIdentifier.toString());            
                                            //hashValue --> OctetString
                                            byte[] hashValueOctetString = hashValue.getOctets();
                                            Log.d(TAG2,"hashValueOctetString:" + hashValueOctetString.toString()); 
                                            //String certIconHash = octetStringToString(hashValue.toString().substring(1));
                                            String certIconHash = hashValue.toString().substring(1);
                                            Log.d(TAG2,"hashValue String:" + certIconHash);
                                            if(iconHash.equals(certIconHash)){
                                                Log.d(TAG, "Icon hash match");                    
                                                return true;
                                            }else{
                                                Log.d(TAG, "Icon hash not match"); 
                                                result = false;
                                                continue;
                                            }
                                        
                                        } 

                                    }
                                      
                                }  
                                
                            }                                   
                        }                    
                    }   
                    Log.d(TAG2,"LogotypeExtn parsing done");
                    return result;                                     
                }
            } catch (SecurityException e) {
                e.printStackTrace();
            } catch (IOException e) {
                e.printStackTrace();
            }                
    
            return false;
        }

    class CertTrustManager implements X509TrustManager {
        public void checkClientTrusted(X509Certificate[] arg0, String arg1){
            Log.d(TAG, "[checkClientTrusted] " + arg0 + arg1);
        }
        public void checkServerTrusted(final X509Certificate[] arg0, String arg1)
                throws CertificateException{
            Log.d(TAG, "[checkServerTrusted] X509Certificate amount:" + arg0.length + ", cryptography: " + arg1);
            if("true".equals(SystemProperties.get("persist.service.ocsprevoke"))){
                throw new RuntimeException("Certificate is revoked (5.9h)");
            }else if("true".equals(SystemProperties.get("persist.service.servercertcheck","true"))){
                try{                                
                    int i;
                    for(i = 0; i < arg0.length; i++)
                    {
                        Log.d(TAG, "X509Certificate: " + arg0[i]);      
                        Log.d(TAG, "====================");                        

                        //check validity (not before and not after)
                        arg0[i].checkValidity();
                    }                    

                    //check root CA chaining
                    KeyStore ks = KeyStore.getInstance("AndroidCAStore");
                    ks.load(null, null);
                    TrustManagerImpl tm = new TrustManagerImpl(ks);
                    tm.checkServerTrusted(arg0, arg0[0].getPublicKey().getAlgorithm(), mSPFQDN);
                    
                    if(mOSUServerUrl != null && !mOSUServerUrl.isEmpty()){ // only check on OSU
                        //check SP friendly name and Language code
                        if("true".equals(SystemProperties.get("persist.service.nonamecheck"))){
                
                        }else if("true".equals(SystemProperties.get("persist.service.test59b"))){
                            if(!checkSubjectAltNameOtherNameSPLangFriendlyName(PasspointCertificateTest.getID_H())){
                                throw new RuntimeException("[certificate test]id-wfa-hotspot-friendly-name check fail");                    
                            }
                        }else if("true".equals(SystemProperties.get("persist.service.test59c"))){
                            if(!checkSubjectAltNameOtherNameSPLangFriendlyName(PasspointCertificateTest.getID_I())){
                                throw new RuntimeException("[certificate test]id-wfa-hotspot-friendly-name check fail");                    
                            }
                        }else if("true".equals(SystemProperties.get("persist.service.test59d"))){
                            if(!checkSubjectAltNameOtherNameSPLangFriendlyName(PasspointCertificateTest.getID_J())){
                                throw new RuntimeException("[certificate test]id-wfa-hotspot-friendly-name check fail");                    
                            }
                        }else if("true".equals(SystemProperties.get("persist.service.test59e"))){
                            if(!checkSubjectAltNameOtherNameSPLangFriendlyName(PasspointCertificateTest.getID_K())){
                                throw new RuntimeException("[certificate test]id-wfa-hotspot-friendly-name check fail");                    
                            }                                
                        }else{
                            if(!checkSubjectAltNameOtherNameSPLangFriendlyName(arg0[0])){
                                throw new RuntimeException("id-wfa-hotspot-friendly-name check fail");                    
                            }
                        }                   
                        
                        //check icon type hash value 
                        if("true".equals(SystemProperties.get("persist.service.test59d"))){
                            if(!checkLogotypeExtn(PasspointCertificateTest.getID_J())){
                                throw new RuntimeException("[certificate test]Certificate Logo icon hash doesn't match");                    
                            }                                
                        }else if(!checkLogotypeExtn(arg0[0])){
                            throw new RuntimeException("Certificate Logo icon hash doesn't match");                    
                        }    
                    }
                
                
                    //check id-kp-clientAuth
                    if(!checkExtendedKeyUsageIdKpClientAuth(arg0[0])){
                        throw new RuntimeException("id-kp-clientAuth found");                    
                    }           
                
                
                    //check OCSP
                    //if(!checkOCSP(arg0[0], CaCert)){
                        //throw new RuntimeException("Certificate revoked");
                    //}
                
                    //check SP-FQDN
                    boolean suffixMatch;
                    if(mOSUServerUrl != null && !mOSUServerUrl.isEmpty()){ // check complete host name on OSU server
                        suffixMatch = false;
                    }else{// check SPFQDN on subscription and policy server
                        suffixMatch = true;
                    }
                    if(!checkSubjectAltNameDNSName(arg0[0], mSPFQDN, suffixMatch)){
                        throw new RuntimeException("Certificate Subject Alternative Name doesn't include SP-FQDN");                    
                    }                                
                } catch (CertificateException e) {
                    throw e;
                } catch(Exception e){
                    e.printStackTrace();
                    throw new RuntimeException(e);                    
                }
                    
            }else {
                Log.d(TAG, "[checkServerTrusted] don't check");
            }

        }

        public X509Certificate[] getAcceptedIssuers() {            
            Log.d(TAG, "[getAcceptedIssuers] ");
            return null;
        }
    }

}

