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

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ContentResolver;
import android.database.ContentObserver;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.SystemProperties;
import android.os.Message;
import android.os.Debug;
import android.os.Process;
import android.net.NetworkInfo;
import android.net.wifi.ScanResult;
import android.net.wifi.SupplicantState;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiInfo;
import android.net.Uri;
import android.net.ConnectivityManager;
import android.security.KeyStore;
import android.security.Credentials;
import android.security.KeyChain;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;
import android.widget.Toast;
import android.provider.Settings;

import com.android.internal.os.AtomicFile;
import com.android.internal.util.State;
import com.android.internal.util.StateMachine;
import com.android.internal.telephony.Phone;
import com.google.android.collect.Sets;

import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.net.URL;
import java.net.CookieHandler;
import java.net.CookieManager;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.Long;
import java.lang.Thread;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;
import java.security.PublicKey;
import java.security.cert.X509Certificate;
import java.security.KeyStore.PasswordProtection;
import java.security.cert.CertificateFactory;
import java.security.cert.CertificateException;

import javax.xml.xpath.XPathFactory;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;

import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.IccCardConstants;
//import android.net.wifi.passpoint.PasspointCredentialManager;
//import android.net.wifi.passpoint.PasspointPpsmoParser;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.passpoint.*;
import com.mediatek.internal.R;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import org.xml.sax.InputSource;

public class PasspointStateMachine extends StateMachine {

    private final static String TAG = "PasspointSM";

    private TreeSet<PasspointPolicy> mNetworkPolicy = new TreeSet<PasspointPolicy>();
    private TreeSet<PasspointPolicy> mSmartBlackPolicy = new TreeSet<PasspointPolicy>();
    private PasspointPolicy mLastUsedPolicy;
    private PasspointPolicy mCurrentUsedPolicy;

    private ArrayList<PasspointCredential> mCredentialList = new ArrayList<PasspointCredential>();
    private List<WifiConfiguration> mWifiConfiguration;
    private Context mContext;
    private NetworkInfo.State mWifiState = NetworkInfo.State.UNKNOWN;
    private Object mStateSync;
    private Object mPolicySync;
    private PasspointCertificate mPasspointCertificate;
    private PasspointPpsmoParser mPpsmoParser = new PasspointPpsmoParser();

    private SoapClient mSoapClient;
    private DmClient mDmClient;
    private Handler mTarget;
    private WifiTree mWifiTree;
    private WifiManager mWifiMgr;
	private TelephonyManager mTeleMgr;
    private WifiTreeHelper mTreeHelper = new WifiTreeHelper();
    private boolean mIsNeedForRemediation;
    private String mRemediationUrlFromWNM;//WNM: Wireless Netwrok Management
    private String mOsuUrlFromWNM;
    private ArrayList<PasspointOSUStorage> mPasspointOSUStorage = new ArrayList<PasspointOSUStorage>();
    private boolean mIsPmfRequired;
    private long mStartTime;
    private State mConfigState;
    private State mDiscoveryState;
    private State mUserInputState;
    private State mSubscriptionProvisionState;
    private State mAccessState;
    private ServerSocket mServerSocketForOSU;
    private ServerSocket mServerSocketForREM;
    private boolean mIsOsuDefaultLanguage;
    private static final String OSU_DEFAULT_LANGUAGE = "zxx";
    private String mNai;
    private boolean mIsEapFailureOverCount;
    private String mIconFilename;
    private boolean mIsIgnoreBlackList;
    private String mMcc;
    private String mMnc;
    private String mUpdateMethod;
    ///M: for SIGMA used @{
    private boolean mIsConnectionEvent;
    private boolean mIsConnectionNoEvent; 
	private String mSigmaTestFriendlyName;
    private boolean mIsOsuSuccess;
    ///M: @}
    //PasspointCredentialManager mPasspointCredentialManager;

    // EAP definition
    private final String INT_PRIVATE_KEY = "private_key";
    private final String INT_PHASE2 = "phase2";
    private final String INT_PASSWORD = "password";
    private final String INT_IDENTITY = "identity";
    private final String INT_EAP = "eap";
    private final String INT_CLIENT_CERT = "client_cert";
    private final String INT_CA_CERT = "ca_cert";
    private final String INT_ANONYMOUS_IDENTITY = "anonymous_identity";
    private final String INT_SIM_SLOT = "sim_slot";
    private final String INT_ENTERPRISEFIELD_NAME ="android.net.wifi.WifiConfiguration$EnterpriseField";
    private final String ISO8601DATEFORMAT = "yyyy-MM-dd'T'HH:mm:ss'Z'";

    private final String ENTERPRISE_PHASE2_MSCHAPV2 = "auth=MSCHAPV2";
    private final String ENTERPRISE_PHASE2_MSCHAP = "auth=MSCHAP";
    
    private static final int EVENT_PASSPOINT_ON = 8;
    private static final int EVENT_PASSPOINT_OFF = 9;
    private static final int EVENT_PROVISION_START = 10;
    private static final int EVENT_PROVISION_DONE = 11;
    private static final int EVENT_REMEDIATION_START = 12;
    private static final int EVENT_REMEDIATION_DONE = 13;
    private static final int EVENT_POLICY_PROVISION_START = 14;
    private static final int EVENT_POLICY_PROVISION_DONE = 15;
    private static final int EVENT_POLICY_CHANGE = 16;
    private static final int EVENT_SIM_PROVISION_DONE = 17;
    private static final int EVENT_PRE_PROVISION_DONE = 18;

    private static final int EVENT_WIFI_CONNECTED = 20;
    private static final int EVENT_WIFI_DISCONNECTED = 21;
    private static final int EVENT_WIFI_ENABLED = 23;
    private static final int EVENT_WIFI_DISABLED = 24;
    private static final int EVENT_DATA_CONNECTED = 25;
    private static final int EVENT_DATA_DISCONNECTED = 26;

    private static final int EVENT_ANQP_DONE = 40;
    private static final int EVENT_OSU_USER_SLCT_DONE = 42;
    private static final int EVENT_PROVISION_UPDATE_RESPONSE_DONE = 43;
    private static final int EVENT_REMEDIATION_UPDATE_RESPONSE_DONE = 44;
    private static final int EVENT_POLICY_PROVISION_UPDATE_RESPONSE_DONE = 45;
    private static final int EVENT_REGISTER_CERT_ENROLLMENT_DONE = 46;
    private static final int EVENT_PROVISION_FROM_FILE_DONE = 47;
    private static final int EVENT_SIM_PROVISION_UPDATE_RESPONSE_DONE = 48;
    private static final int EVENT_SUBSCRIPTION_PROVISION_READY = 49;
    private static final int EVENT_SUBSCRIPTION_PROVISION_NOT_READY = 50;
    
    //intent    
    public static final String HS20_BROWSER_OSUREDIRECT_MO_REQUEST = "com.mediatek.passpoint.browser.redirect";
    public static final String HS20_BROWSER_REMREDIRECT_MO_REQUEST = "com.mediatek.passpoint.browser.remredirect";
    public static final String HS20_BROWSER_OSUREDIRECT_MO_REQUEST_DM = "com.mediatek.passpoint.browser.redirect.dm";
    public static final String HS20_BROWSER_REMREDIRECT_MO_REQUEST_DM = "com.mediatek.passpoint.browser.remredirect.dm";

    private static final int HS_OSU_LISTEN_PORT = 54685;
    private static final int HS_REM_LISTEN_PORT = 54686;

    private boolean mMncLengthAssigned = false;
    private int mMncLength = 3;  
    private String mManagementTreeUri; 

    private ContentObserver mPasspointEnablerObserver = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange) {
            boolean passpointEnable = Settings.Global.getInt(mContext.getContentResolver(), Settings.Global.WIFI_PASSPOINT_ON, 0) == 1;
            Log.d(TAG,"mPasspointEnablerObserver omChanged, passpointEnable = " + passpointEnable);
            if (passpointEnable && mWifiMgr.isWifiEnabled()) {
                mWifiMgr.enableHS(true);
                sendMessage(EVENT_PASSPOINT_ON);                
            } else {
                mWifiMgr.enableHS(false);
                sendMessage(EVENT_PASSPOINT_OFF);
            }
        }
    }; 

    public enum MatchMethod {
        REALM,
        HOMEFQDN,
        PARTNERFQDN,
        OTHERHOMEPARTNER,
        PLMN,
        NOVALUE;

        public static MatchMethod toMatchMethod(String str) {
            try {
                return valueOf(str);
            }

            catch (Exception ex) {
                return NOVALUE;
            }
        }
    }

    public PasspointStateMachine(String name, Handler handler, Context context) {
        super(name, handler.getLooper());
        mContext = context;
        mTarget = handler;
        mWifiMgr = (WifiManager)mContext.getSystemService(Context.WIFI_SERVICE);
        mTeleMgr = (TelephonyManager)mContext.getSystemService(Context.TELEPHONY_SERVICE);

        mStateSync = new Object();
        mPolicySync = new Object();

        mWifiConfiguration = new ArrayList<WifiConfiguration>();
        final IntentFilter stateFilter = new IntentFilter();
        stateFilter.addAction(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION);
        stateFilter.addAction(WifiManager.WIFI_STATE_CHANGED_ACTION);
        stateFilter.addAction(WifiManager.NETWORK_STATE_CHANGED_ACTION);
        stateFilter.addAction(WifiManager.HS20_ANQP_DONE_ACTION);
        stateFilter.addAction(WifiManager.HS20_REM_URL_ACTION);
        stateFilter.addAction(PasspointManager.HTTP_PASSPOINT_BROWSER_RESPONSE);
        stateFilter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);
        stateFilter.addAction(WifiManager.HS20_IP_DAD_ACTION);
        stateFilter.addAction(WifiManager.HS20_POLICY_UPDATE_ACTION);
        stateFilter.addAction(WifiManager.HS20_INSTALL_PPSMO_ACTION);
        stateFilter.addAction(WifiManager.HS20_RESET_ACTION);
        stateFilter.addAction(WifiManager.HS20_DUMP_TOAST_ACTION);
        stateFilter.addAction(WifiManager.HS20_ADD_CREDENTIAL_ACTION);
        stateFilter.addAction(WifiManager.HS20_BTM_URL_ACTION);
        stateFilter.addAction(WifiManager.HS20_WEB_INSTALL_PPSMO_ACTION);
        stateFilter.addAction(WifiManager.HS20_SET_PROV_PROTOCOL_ACTION);
        stateFilter.addAction(WifiManager.HS20_SET_OSU_CONNECT_ACTION);
        stateFilter.addAction(WifiManager.HS20_SET_OSU_CONNECT_ACTION_NO);
        stateFilter.addAction(WifiManager.HS20_DEAUTH_URL_ACTION);
        stateFilter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        stateFilter.addAction(WifiManager.HS20_SERVER_METHOD_ACTION);
        stateFilter.addAction(WifiManager.HS20_WEB_INSTALL_AAA_ROOT_CA_ACTION);  
        stateFilter.addAction(WifiManager.HS20_RETRIEVE_TREE_RESPONSE);
        stateFilter.addAction(Intent.ACTION_BOOT_COMPLETED);
        stateFilter.addAction("com.mediatek.soap.provision.end");
        stateFilter.addAction("com.mediatek.hs20.over.retry");
        stateFilter.addAction("com.mediatek.hs20.ignore.blacklist");
        stateFilter.addAction(WifiManager.HS20_WEB_INSTALL_PPSMO_MNC_LENGTH_ACTION);
        stateFilter.addAction(WifiManager.HS20_PPSMO_MANAGEMENT_TREE_URI);
        
        mContext.registerReceiver(mStateReceiver, stateFilter);

        mConfigState = new ConfigState();
        addState(mConfigState);
        mDiscoveryState = new DiscoveryState();
        addState(mDiscoveryState);
        mSubscriptionProvisionState = new SubscriptionProvisionState();
        addState(mSubscriptionProvisionState);
        mAccessState = new AccessState();
        addState(mAccessState);

        setInitialState(mConfigState);

        mWifiTree = new WifiTree();

        mSoapClient = new SoapClient(mContext, mTarget);
        mSoapClient.setWifiTree(mWifiTree);
        mSoapClient.OSU_REDIRECT_PORT = HS_OSU_LISTEN_PORT;
        mSoapClient.REM_REDIRECT_PORT = HS_REM_LISTEN_PORT;

        mDmClient = new DmClient(mContext, mTarget);
        mDmClient.setWifiTree(mWifiTree);

        mCurrentUsedPolicy = null;
        mIsNeedForRemediation = false;
        mIsPmfRequired = true;

        mPasspointCertificate = PasspointCertificate.getInstance (mContext);//singleton
        mUpdateMethod = SystemProperties.get("persist.service.hs20.upmethod", "SPP-ClientInitiated");

        mContext.getContentResolver().registerContentObserver(Settings.Global.getUriFor(Settings.Global.WIFI_PASSPOINT_ON),
                true, mPasspointEnablerObserver);
    }

    private void startHttpServer(final int port){
        HttpServer httpServer = new HttpServer(port);
        httpServer.startListener();
    }

    private long convertMillisFromDataString(String datestring) {
        long result = 0;
        Log.d(TAG, "convertMillisFromDataString:["+datestring+"]");

        try {
            //TimeZone tz = TimeZone.getTimeZone("UTC");
            SimpleDateFormat df = new SimpleDateFormat(ISO8601DATEFORMAT);
            //df.setTimeZone(tz);
            result = df.parse( datestring ).getTime();
            Log.d(TAG, "convertMillisFromDataString result:"+result);
        } catch( ParseException pe ) {
            pe.printStackTrace();
        } finally {
            return result;
        }
    }

    private void createCredentialFromTree(final WifiTree tree) {
        Log.d(TAG, "createCredentialFromTree");
        PasspointCredential pc = null;
        String digiCertType = null;
        String credUnPwEapType = null;
        String digiCredSha256FingerPrint = null;
        String aaaRootCertSha256FingerPrint = null;
        String simEapType = null;

        if (tree == null) return;

        WifiTree.CredentialInfo info = null;
        Set spfqdnSet = tree.spFqdn.entrySet();
        Iterator spfqdnItr = spfqdnSet.iterator();

        while(spfqdnItr.hasNext()) {
            Map.Entry entry1 = (Map.Entry)spfqdnItr.next();
            WifiTree.SpFqdn sp = (WifiTree.SpFqdn)entry1.getValue();
            Log.d(TAG, "SPFQDN:" + sp.nodeName);

            Set credInfoSet = sp.perProviderSubscription.credentialInfo.entrySet();
            Iterator credinfoItr = credInfoSet.iterator();
            String preferCred = Settings.Global.getString(mContext.getContentResolver(), Settings.Global.WIFI_PASSPOINT_PREFER_CREDENTIAL);

            while(credinfoItr.hasNext()) {
                Map.Entry entry2 = (Map.Entry)credinfoItr.next();
                info = (WifiTree.CredentialInfo)entry2.getValue();
                if (info == null) {
                    return;
                }
                Log.d(TAG, "Credential:" + info.nodeName);

                boolean isUserPreferred = false;
                if (preferCred != null && sp.nodeName.equals(preferCred.split(":")[0]) && info.nodeName.equals(preferCred.split(":")[1])) {
                    isUserPreferred = true;
                }                
                credUnPwEapType = info.credential.usernamePassword.eAPMethod.EAPType;
                digiCertType = info.credential.digitalCertificate.CertificateType;
                digiCredSha256FingerPrint = info.credential.digitalCertificate.CertSHA256Fingerprint;
                simEapType = info.credential.sim.EAPType;
                
                WifiTree.AAAServerTrustRoot aaa = null;
                Set set = info.aAAServerTrustRoot.entrySet();
                Iterator i = set.iterator();
                if(i.hasNext()) {
                    Map.Entry entry = (Map.Entry)i.next();
                    aaa = (WifiTree.AAAServerTrustRoot)entry.getValue();
                }                
                
                if (aaa == null) {
                    Log.d(TAG, "AAAServerTrustRoot is empty");
                    aaaRootCertSha256FingerPrint = null;
                } else {
                    aaaRootCertSha256FingerPrint = aaa.CertSHA256Fingerprint;
                }
                
                Log.d(TAG, "credCertType: "+digiCertType+", credSha256FingerPrint: "+digiCredSha256FingerPrint);
                Log.d(TAG, "aaaRootCertSha256FingerPrint: "+aaaRootCertSha256FingerPrint);
                String clientCert = "";

                KeyStore mKeyStore = KeyStore.getInstance();

                if (mKeyStore == null) {
                    Log.d(TAG, "mKeyStore is null");
                    return;
                }

                //TTLS
                if ("21".equals(credUnPwEapType)) {
                    String aaaRootCertSha1FingerPrint = null;
                    
                    if (!mKeyStore.contains(Credentials.WIFI + aaaRootCertSha256FingerPrint)) {                                
                        Log.e(TAG, "AAA trust root is not existed in keystore");
                        return;
                    } else {
                        aaaRootCertSha1FingerPrint = new String(mKeyStore.get(Credentials.WIFI + aaaRootCertSha256FingerPrint));
                    }
                    
                    pc = new PasspointCredential("TTLS",
                                                 aaaRootCertSha1FingerPrint,
                                                 null,
                                                 tree.PpsMoId,
                                                 sp,
                                                 info);

                    pc.setUserPreference(isUserPreferred);
                    mCredentialList.add(pc);
                //TLS
                }else if("x509v3".equals(digiCertType)){
                    // get certificate credential by searching Public Key SHA-256 FingerPrint                    
                    if(mKeyStore.contains(Credentials.WIFI + digiCredSha256FingerPrint)){
                        Log.d(TAG, "load client cert");
                        
                        String creSha1FingerPrint = new String(mKeyStore.get(Credentials.WIFI + digiCredSha256FingerPrint));                        
                        String aaaRootCertSha1FingerPrint;
                        
                        if(aaaRootCertSha256FingerPrint != null && !aaaRootCertSha256FingerPrint.isEmpty()){
                            Log.d(TAG, "AAA trust root is exclusive");
                            if (!mKeyStore.contains(Credentials.WIFI + aaaRootCertSha256FingerPrint)) {                                
                                Log.e(TAG, "AAA trust root is not existed in keystore");
                                return;
                            } else {
                                aaaRootCertSha1FingerPrint = new String(mKeyStore.get(Credentials.WIFI + aaaRootCertSha256FingerPrint));
                            }
                        }else{
                            Log.d(TAG, "AAA trust root is the same as client cert");                                
                            aaaRootCertSha1FingerPrint = creSha1FingerPrint;
                        }    
                        
                        pc = new PasspointCredential("TLS",
                                                     aaaRootCertSha1FingerPrint,
                                                     creSha1FingerPrint,/*(clientCert.isEmpty()? ENTERPRISE_CLIENT_CERT : clientCert )*/
                                                     tree.PpsMoId,
                                                     sp,
                                                     info);                        
                        pc.setUserPreference(isUserPreferred);
                        mCredentialList.add(pc);
                    }else {
                        Log.d(TAG, "client cert doesn't exist");
                    }
                //SIM
                }else if(simEapType != null){
                    String credSimEapType = IANAEapMethod.sIANA_EAPmethod[Integer.valueOf(simEapType)];
                    Log.d(TAG, "credSimEapType: "+credSimEapType);                    
                    if ("SIM".equals(credSimEapType)) {
                    
                        //workaround for ID#26 multiple credential case
                        String mccMnc = mTeleMgr.getSimOperator();
                        Log.d(TAG, "mccMnc: " + mccMnc);
                        if(mccMnc != null && !mccMnc.isEmpty()){ 
                            Log.d(TAG, "[createCredentialFromMO] real SIM");
                            if(mccMnc.length() > 3) {
                                mMcc = mccMnc.substring(0,3);
                                mMnc = mccMnc.substring(3); 
                            }else{
                                Log.d(TAG, "[createCredentialFromMO] fail due to not getting MCC MNC");
                                return;
                            }                       
                        } else {
                            Log.d(TAG, "[createCredentialFromMO] simulate SIM");                                                
                            info.credential.usernamePassword.Password = "90dca4eda45b53cf0f12d7c9c3bc6a89:cb9cccc4b9258e6dca4760379fb82581";
                            if(mMncLengthAssigned == true){
                                String iMsi = info.credential.sim.IMSI;
                                mMcc = iMsi.substring(0,3);
                                mMnc = iMsi.substring(3,3 + mMncLength);                                
                                mMncLengthAssigned = false;
                                Log.d(TAG, "Get PLMN from IMSI, MCC = " + mMcc + ", MNC = " + mMnc);                                                                                
                            }
                        }
                        //end of workaround for ID#26 multiple credential case and SIM credential(ID#0)
                        pc = new PasspointCredential("SIM",
                                                     null,
                                                     null,
                                                     mMcc,
                                                     mMnc,
                                                     tree.PpsMoId,
                                                     sp,
                                                     info);
                        pc.setUserPreference(isUserPreferred);
                        mCredentialList.add(pc);
                    }
                
                }
                
            }
        }

        
    }

    // for SIGMA
    private void createCredentialFromEvent(String command) {
        //command sample:-tTYPE -uUSERNAME -pPASSWORD -iIMSI -nPLMN_MNC -cPLMN_MCC -aROOT_CA -rREALM -fPREFER -qFQDN -lCLIENT_CA -w1
        WifiTree.SpFqdn spfqdn = new WifiTree.SpFqdn();
        WifiTree.CredentialInfo credInfo = new WifiTree.CredentialInfo();

        String eapType = null;
        String rootCA = null;
        String clientCA = null;
        int priority = 128;
        StringTokenizer tokens = new StringTokenizer(command.trim(), " ");
        int count = 0;
        if (tokens != null) {
            count = tokens.countTokens();
        }

        boolean setIeee80211w = false;
        for(int i = 0; i < count; i++){
            String token = tokens.nextToken().trim();
            char param = token.charAt(1);
            String paramValue = token.substring(2);

            switch(param) {
                case 't':
                    if(paramValue.toLowerCase().equals("uname_pwd")){                          
                        credInfo.credential.usernamePassword.eAPMethod.EAPType = "21";
                        eapType = "TTLS";
                    }else if(paramValue.toLowerCase().equals("sim")){                          
                        credInfo.credential.sim.EAPType = "18";
                        eapType = "SIM";
                    }else if(paramValue.toLowerCase().equals("cert")){                         
                        credInfo.credential.digitalCertificate.CertificateType = "x509v3";                                                     
                        eapType = "TLS";
                    }
                    break;
                case 'u':
                    credInfo.credential.usernamePassword.Username = paramValue;
                    break;
                case 'p':
                    credInfo.credential.usernamePassword.Password = paramValue;
                    break;
                case 'i':                
                    credInfo.credential.sim.IMSI = paramValue;
                    break;
                case 'n':
                    mMnc = paramValue;                    
                    break;
                case 'c':
                    mMcc = paramValue;
                    break;
                case 'a':
                    break;
                case 'r':
                    credInfo.credential.Realm = paramValue;                
                    break;
                case 'f':
                    if(paramValue.equals("1")){
                        credInfo.credentialPriority = "0";                        
                    }
                    break;
                case 'q':
                    credInfo.homeSP.FQDN = paramValue;
                    break;
                case 'l':
                    clientCA = paramValue;
                    break;
                case 'w':
                    Set8211W(Integer.valueOf(paramValue));
                    setIeee80211w = true;
                    break;
                default:
                    break;
            }
        }

        if(setIeee80211w == false){
            Set8211W(1); //default value
        }

        PasspointCredential pc = null;
        pc = new PasspointCredential(eapType,
                                     rootCA,
                                     clientCA,
                                     mMcc,
                                     mMnc,
                                     0,
                                     spfqdn,
                                     credInfo);
            if( pc != null ) {
                mCredentialList.add(pc);
            }
    }

    private boolean findDuplicateSIMCredential(String imsi) {
        for (PasspointCredential cred : mCredentialList) {
            if (imsi.equals(cred.getImsi())) {
                return true;
            }
        }
        return false;
    }
    
    private void createCredentialFromSIM(String imsi) { 
        Log.d(TAG, "createCredentialFromSIM, IMSI = " + imsi);
        if (findDuplicateSIMCredential(imsi)) {
            return;
        } else {    
            WifiTree.SpFqdn spfqdn = new WifiTree.SpFqdn();
            WifiTree.CredentialInfo info = new WifiTree.CredentialInfo();
            String plmnMCC;
            String plmnMNC;
            String mccMnc = mTeleMgr.getSimOperator();
            Log.d(TAG, "mccMnc: " + mccMnc);
            if((mccMnc != null) &&
                (mccMnc.length() > 3)) {
                    plmnMCC = mccMnc.substring(0,3);
                    plmnMNC = mccMnc.substring(3); 
            }else{
                Log.d(TAG, "createCredentialFromSIM fail due to not getting MCC MNC");
                return;
            }
            
            String rootCA = null;
            String clientCA = null;
            String eapSimType = null;            
            if(mTeleMgr.getNetworkType() == TelephonyManager.NETWORK_TYPE_UMTS){
                eapSimType = IANAEapMethod.sIANA_EAPmethod[Integer.valueOf("23")];
            }else{
                eapSimType = IANAEapMethod.sIANA_EAPmethod[Integer.valueOf("18")];
            }
            info.credential.sim.IMSI = imsi;
            info.credentialPriority = "128";
            spfqdn.perProviderSubscription.UpdateIdentifier = "0";
                        
            PasspointCredential pc = null;
            pc = new PasspointCredential(eapSimType,
                                         rootCA,
                                         clientCA,
                                         plmnMCC,
                                         plmnMNC,
                                         0,
                                         spfqdn,
                                         info);
            
            if( pc != null ) {
                mCredentialList.add(pc);
            }
        }
    }	

    private void onCredentialExpired() {
        Log.d(TAG, "onCredentialExpired");
        /* TS v5.0 6.4.1 Subscription Expiry
         * In the PerProviderSubscription MO, when the DataLimit (SubscriptionParameters/UsageLimits/DataLimit)
         * or TimeLimit (SubscriptionParameters/UsageLimits/TimeLimit) is reached or passed,
         * the subscription expires (SubscriptionParameters/ExpirationDate) or credential expires (Credential/ExpirationDate),
         * the mobile device should not immediately disassociate itself from the Wi-Fi AN;
         * rather it should wait for the network to take this action and then select an alternative network.
         * Note: this alternative network may be the same as the original Wi-Fi AN.
         */
    }

    public void clearPolicies() {
        Log.d(TAG, "clearPolicies");
        mCurrentUsedPolicy = null;
        mNetworkPolicy.clear();
        mSmartBlackPolicy.clear();
        mLastUsedPolicy = null;
    }


    /*public void installPpsMo(int ppsmoid) {
        Log.d(TAG, "installPpsMo:"+ppsmoid);
        WifiSubscriptionMo mo;       

        mo = new WifiSubscriptionMo();
        mo.configToScenario(ppsmoid);
        mWifiSubscriptionMoList.add(mo);        

        //Additional MO
        Iterator iter = mWifiSubscriptionMoList.values().iterator();

        while (iter.hasNext()) {
            WifiSubscriptionMo nextMo = (WifiSubscriptionMo)iter.next();
            createCredentialFromMO(nextMo);            
        }
    }*/
    public void installPpsMo(int ppsmoid) {
        Log.d(TAG, "installPpsMo:"+ppsmoid);

        mTreeHelper.configToPpsMoNumber(ppsmoid, mWifiTree);
        mTreeHelper.dumpTreeInfo(mWifiTree);
        mSoapClient.setWifiTree(mWifiTree);
        mDmClient.setWifiTree(mWifiTree);
        createCredentialFromTree(mWifiTree);
    }

    public void Set8211W(int index) {
        Log.d(TAG, "Set8211W:"+index);
        if( index == 0 ) {
            mIsPmfRequired = false;
        } else { //index == 1
            mIsPmfRequired = true;
        }
    }

    private void dumpPolicies(TreeSet<PasspointPolicy> policyCollection) {
        Log.d(TAG, "===== dumpPolicies =====");
        for (PasspointPolicy policy : policyCollection) {
            Log.d(TAG, " Policy:" + policy);
        }
        Log.d(TAG, "===== End =====");
    }

    private void dumpCredentials() {
        /*Log.d(TAG, "===== dumpCredentials =====");
        for (PasspointCredential credential : mCredentialList) {
            Log.d(TAG, " Credential:" + credential);

            WifiSubscriptionMo mo = (WifiSubscriptionMo)mWifiSubscriptionMoList.get((Object)credential.getSubscriptionMoIndex());
            if(mo.getMoItemStr(mo.MO_ITEM_POL_SPELIST_SSID) != null){
                Log.d(TAG, " BlackSSID:" + mo.getMoItemStr(mo.MO_ITEM_POL_SPELIST_SSID));
            }                
        }
        Log.d(TAG, "===== End =====");*/
        mTreeHelper.dumpTreeInfo(mWifiTree);
    }

    private BroadcastReceiver mStateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "onReceive:" + action);

            if (action.equals(WifiManager.WIFI_STATE_CHANGED_ACTION)) {
                int State = intent.getIntExtra(WifiManager.EXTRA_WIFI_STATE, WifiManager.WIFI_STATE_UNKNOWN);
                Log.d(TAG, "State:" + State);

                switch (State) {
                case WifiManager.WIFI_STATE_DISABLED:
                case WifiManager.WIFI_STATE_UNKNOWN:
                    sendMessage(EVENT_WIFI_DISABLED);
                    break;

                case WifiManager.WIFI_STATE_ENABLED:
                    sendMessage(EVENT_WIFI_ENABLED);
                    break;
                default:
                    break;
                }
            } else if (action.equals(WifiManager.NETWORK_STATE_CHANGED_ACTION)) {
                NetworkInfo wifiInfo =
                    (NetworkInfo) intent.getParcelableExtra(WifiManager.EXTRA_NETWORK_INFO);
                String bssid = intent.getStringExtra(WifiManager.EXTRA_BSSID);
                Log.d(TAG, "wifiInfo: " + wifiInfo);
                Log.d(TAG, "bssid: " + bssid);

                synchronized (mStateSync) {
                    mWifiState = wifiInfo.getState();
                }

                if (mWifiState == NetworkInfo.State.CONNECTED) {
                    sendMessage(EVENT_WIFI_CONNECTED);

                } else if (mWifiState == NetworkInfo.State.DISCONNECTED) {
                    sendMessage(EVENT_WIFI_DISCONNECTED);
                }
            } else if (action.equals(WifiManager.HS20_ANQP_DONE_ACTION)) {
                Log.d(TAG, "WifiManager.HS20_ANQP_DONE_ACTION");
                sendMessage(EVENT_ANQP_DONE);
            } else if (action.equals(WifiManager.HS20_REM_URL_ACTION)) {
                String remediationURL = intent.getStringExtra(WifiManager.EXTRA_URL);
                Log.d(TAG, "WifiManager.HS20_REM_URL_ACTION " + remediationURL);
                sendMessage( obtainMessage(EVENT_REMEDIATION_START, remediationURL) );
            } else if (action.equals(ConnectivityManager.CONNECTIVITY_ACTION)) {
                Log.d(TAG, "Receive:" + ConnectivityManager.CONNECTIVITY_ACTION);
                NetworkInfo info = (NetworkInfo)intent.getExtra(ConnectivityManager.EXTRA_NETWORK_INFO);
                if(info != null &&
                info.getType() == ConnectivityManager.TYPE_WIFI) {
                    if ( info.isConnected() ) {
                        sendMessage(EVENT_DATA_CONNECTED);
                    } else {
                        sendMessage(EVENT_DATA_DISCONNECTED);
                    }
                }
            } else if (action.equals(WifiManager.HS20_IP_DAD_ACTION)) {
                int type = intent.getIntExtra(WifiManager.EXTRA_DAD_TYPE, 0);
                String data = intent.getStringExtra(WifiManager.EXTRA_DAD_DATA);
                Log.d(TAG, "WifiManager.HS20_IP_DAD_ACTION["+ type +"]["+ data + "]");
                String result = "";
                if ( type == WifiManager.DAD_TYPE_IPV4 ) {
                    result += "IPV4 ";
                } else if ( type == WifiManager.DAD_TYPE_IPV6 ) {
                    result += "IPV6 ";
                } else {
                    result += "IP_UNKNOWN ";
                }

                result = result + data +" DAD Happened!!!";
                showToast(result);
            } else if (action.equals(WifiManager.HS20_POLICY_UPDATE_ACTION)) {
                int interval = intent.getIntExtra(WifiManager.EXTRA_INTERVAL, 2);
                long l_interval = (long)interval;
                Log.d(TAG, "WifiManager.HS20_POLICY_UPDATE_ACTION["+ l_interval + "]");
                if (l_interval == 1) {
                    sendMessage(EVENT_POLICY_PROVISION_START);
                }                
            } else if (action.equals(WifiManager.HS20_INSTALL_PPSMO_ACTION)) {
                int index = intent.getIntExtra(WifiManager.EXTRA_INDEX, 0);
                installPpsMo(index);
            } else if (action.equals(WifiManager.HS20_RESET_ACTION)) {
                delAllCredential();
                clearPolicies();
            } else if (action.equals(WifiManager.HS20_DUMP_TOAST_ACTION)) {
               String data = intent.getStringExtra(WifiManager.EXTRA_TOAST);
               Log.d(TAG, "WifiManager.HS20_DUMP_TOAST_ACTION["+ data + "]");
                if (data == null) return;
               showToast(data);
            } else if (action.equals(WifiManager.HS20_ADD_CREDENTIAL_ACTION)) { //SIGMA
                String data = intent.getStringExtra(WifiManager.EXTRA_PARAM);
                Log.d(TAG, "WifiManager.HS20_ADD_CREDENTIAL_ACTION["+ data + "]");
                if (data == null) return;
                createCredentialFromEvent(data);
            } else if (action.equals(TelephonyIntents.ACTION_SIM_STATE_CHANGED)) { //Read IMSI
                // TODO: Dual SIM support
               String unlockSIMStatus = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
               if ((IccCardConstants.INTENT_VALUE_ICC_LOADED).equals(unlockSIMStatus)) {
                   String imsi = mTeleMgr.getSubscriberId();
	Log.d(TAG, "Read IMSI["+imsi+"] then create credential");
                   if (imsi != null) {
                       mSoapClient.setIMSI(imsi);
                       createCredentialFromSIM(imsi);
                   } else {
                       Log.d(TAG, "Can't read IMSI");
                   }
                }
            } else if (action.equals(WifiManager.HS20_BTM_URL_ACTION)) {
                String data = intent.getStringExtra(WifiManager.EXTRA_URL);
                if (data == null) return;
                if (!data.startsWith("HTTPS://") && !data.startsWith("https://") && !data.startsWith("http://")) {
                    data = "http://" + data;
                }
                Log.d(TAG, "WifiManager.HS20_BTM_URL_ACTION["+ data + "]");
                
                Intent intentBrowser = new Intent(Intent.ACTION_VIEW, Uri.parse(data));
                intentBrowser.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                mContext.startActivity(intentBrowser);                    
            } else if (action.equals(WifiManager.HS20_DEAUTH_URL_ACTION)) {
                String data = intent.getStringExtra(WifiManager.EXTRA_URL);
                if (data == null) return;
                if (!data.startsWith("HTTPS://") && !data.startsWith("https://") && !data.startsWith("http://")) {
                    data = "http://" + data;
                }
                Log.d(TAG, "WifiManager.HS20_DEAUTH_URL_ACTION["+ data + "]");
                
                Intent intentBrowser = new Intent(Intent.ACTION_VIEW, Uri.parse(data));
                intentBrowser.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                mContext.startActivity(intentBrowser);                    
            } else if (action.equals(WifiManager.HS20_SERVER_METHOD_ACTION)) {
                String method = intent.getStringExtra(WifiManager.EXTRA_METHOD);
                
                Log.d(TAG, "WifiManager.HS20_SERVER_METHOD_ACTION["+ method + "]");
                
                if ("0".equals(method)) {
                    mUpdateMethod = "OMA-DM-ClientInitiated";
                } else if("1".equals(method)) {
                    mUpdateMethod = "SPP-ClientInitiated";                    
                }
            } else if (action.equals(WifiManager.HS20_PPSMO_MANAGEMENT_TREE_URI)) {
                String mgntTreeUri = intent.getStringExtra(WifiManager.EXTRA_MGNTURI);
                Log.d(TAG, "WifiManager.HS20_PPSMO_MANAGEMENT_TREE_URI[" + mgntTreeUri + "]");
                if (mgntTreeUri != null && !mgntTreeUri.isEmpty()) {
                    mManagementTreeUri = mgntTreeUri;
                }
            } else if (action.equals(WifiManager.HS20_WEB_INSTALL_PPSMO_ACTION )) {
                String data = intent.getStringExtra(WifiManager.EXTRA_FILENAME);
                String filePath = "/mnt/asec/"+data;
                
                if (mWifiTree != null) {
                    String XML = FileOperationUtil.Read(filePath);
                    Document doc = mPpsmoParser.extractMgmtTree(XML);
                    String sprovider = null;
                    
                    if ("true".equals(SystemProperties.get("persist.service.dmtreesync","true"))) {
                        if (mManagementTreeUri == null) {
                            mSoapClient.sendToDmAddMo(true, false, "./Wi-Fi/wi-fi.org/PerProviderSubscription", doc);
                        } else {
                            mSoapClient.sendToDmAddMo(true, false, mManagementTreeUri, doc);
                        }                        
                    }else{
                        createCredentialFromTree(mWifiTree);   
                        Log.d(TAG, "Send EVENT_PROVISION_FROM_FILE_DONE");                    
                        Message msg = mTarget.obtainMessage(PasspointManager.EVENT_PROVISION_FROM_FILE_DONE, null);
                        msg.sendToTarget(); 
                    }
                } else {
                    Log.d(TAG, "mWifiTree is null, HS20_WEB_INSTALL_PPSMO_ACTION error");                    
                }
            } else if (action.equals(WifiManager.HS20_WEB_INSTALL_PPSMO_MNC_LENGTH_ACTION)) {
                String data = intent.getStringExtra(WifiManager.EXTRA_LENGTH);
                mMncLength = Integer.valueOf(data);
                mMncLengthAssigned = true;
                Log.d(TAG, "WifiManager.HS20_WEB_INSTALL_PPSMO_MNC_LENGTH_ACTION["+ data + "]"); 
            } else if (action.equals(WifiManager.HS20_WEB_INSTALL_CLIENT_CERT_ACTION )) {
                SystemProperties.set("persist.service.clientcert.pass","wifi@123");
            } else if (action.equals(WifiManager.HS20_SET_PROV_PROTOCOL_ACTION)) {
                // for SIGMA test, to do remove
                /*String data = intent.getStringExtra(WifiManager.EXTRA_OSU_METHOD);
                if ("SOAP".equals(data)) {
                    SystemProperties.set("persist.service.hs20.method", "SPP-ClientInitiated");
                } else if ("OMADM".equals(data)) {
                    SystemProperties.set("persist.service.hs20.method", "OMA-DM-ClientInitiated");                
                } else {
                    Log.d(TAG, "do nothing for set method");
                }*/
            } else if (action.equals(WifiManager.HS20_SET_OSU_CONNECT_ACTION)) {
                // for sigma test, for simulate user choose osu provider
                // and response osu completed if procedure completed.
                mIsConnectionEvent = true;
                mSigmaTestFriendlyName = intent.getStringExtra(WifiManager.EXTRA_FRIENDLYNAME);
                Log.d(TAG, "mSigmaTestFriendlyName:" + mSigmaTestFriendlyName);
            } else if (action.equals(WifiManager.HS20_SET_OSU_CONNECT_ACTION_NO)) {
                // for sigma test, for directly response osu completed to supplicant
                // after open ssid is connected.
                mIsConnectionNoEvent = true;
                mSigmaTestFriendlyName = intent.getStringExtra(WifiManager.EXTRA_FRIENDLYNAME);
                Log.d(TAG, "mSigmaTestFriendlyName:" + mSigmaTestFriendlyName);
            } else if (action.equals(Intent.ACTION_BOOT_COMPLETED)) {
                /** To get WifiTree from DM tree.xml */
                Intent intentGetTree = new Intent(WifiManager.HS20_RETRIEVE_TREE_REQUEST);
                mContext.sendBroadcast(intentGetTree);
            } else if (action.equals(WifiManager.HS20_RETRIEVE_TREE_RESPONSE)) {
                Bundle b = intent.getExtras();
                mWifiTree = b.getParcelable("wifitree");
                createCredentialFromTree(mWifiTree);
            } else if (action.equals("com.mediatek.soap.provision.end")) {
                String cmd = intent.getStringExtra("cmd");                
                int result = intent.getIntExtra("result", -1);

                if("deletesubtree".equals(cmd)){
                    if (result == 0) { //success 
                        Log.d(TAG, "DM deletesubtree success !!!"); 
                    }else{
                        Log.d(TAG, "DM deletesubtree fail !!!"); 
                    }
                }
            } else if (action.equals(WifiManager.HS20_EAP_FAIL_OVER_RETRY)) {
                mIsEapFailureOverCount = intent.getBooleanExtra("isoverlimit", false);
                Log.d(TAG, "mIsEapFailureOverCount:"+mIsEapFailureOverCount); 
            } else if (action.equals(WifiManager.HS20_IGNORE_BLACK_LIST)) {
                mIsIgnoreBlackList = true;
            }
        };
    };

    public void startNetworkPolicyPoll() {
        Log.d(TAG, "startNetworkPolicyPoll");
        sendMessage(EVENT_POLICY_PROVISION_START);
    }

    public void startRemediationPoll() {
        Log.d(TAG, "startNetworkPolicyPoll");
        sendMessage(EVENT_REMEDIATION_START);
    }

    public void setPasspointUserPolicies(PasspointPolicy[] policies) {
        Log.d(TAG, "setPasspointUserPolicies");
        /*mUserPolicy.clear();
        for (PasspointPolicy policy : policies) {
            mUserPolicy.add(policy);
        }
        dumpPolicies(mUserPolicy);
        sendMessage(EVENT_POLICY_CHANGE);*/
    }

    public PasspointPolicy[] getPasspointUserPolicies() {
        //return mUserPolicy.toArray(new PasspointPolicy[mUserPolicy.size()]);
        return null;
    }

    private void setKeyMgmt(WifiConfiguration config) {
        // if mIsPmfRequired==1 and the flag (capability) of the AP selected includes ...SHA256...{
        //     Set WifiConfiguration.KeyMgmt.WPA_EAP_SHA256}
        
        if (config.SSID != null) {
            List<ScanResult> scanResults = mWifiMgr.getScanResults();
            if (scanResults != null) {
                for (ScanResult scanresult : scanResults) {
                    if (scanresult.SSID != null) {
                        if(config.SSID.equals("\"" + scanresult.SSID + "\"")){
                            if (scanresult.capabilities != null) {
                                if (scanresult.capabilities.contains("SHA256") && (config.ieee80211w.equals("1"))) {
                                    config.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.WPA_EAP_SHA256);
                                    Log.d(TAG, "setKeyMgmt WPA_EAP_SHA256");
                                } else {
                                    config.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.WPA_EAP);
                                    Log.d(TAG, "setKeyMgmt WPA_EAP");
                                }
                                break;
                            } else {
                                config.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.WPA_EAP);
                                Log.d(TAG, "setKeyMgmt scanresult no capability set WPA_EAP");
                            }
                        } else {
                            Log.d(TAG, "scanResult.SSID not match to config.SSID");
                        }                        
                    } else {
                        config.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.WPA_EAP);
                        Log.d(TAG, "setKeyMgmt scanresult no match set WPA_EAP");
                    }
                }
            }else{
                Log.d(TAG, "scanResults = null");
            }
            
        }else{
            Log.d(TAG, "config.SSID = null");
        }
    }

    private WifiConfiguration CreateOsenConfig(PasspointPolicy pp) {
        Log.d(TAG, "[CreateOsenConfig]");
        WifiConfiguration wfg = new WifiConfiguration();
        if (pp.getBssid() != null) {
            Log.d(TAG, "create bssid:" + pp.getBssid());
            wfg.BSSID = pp.getBssid();
        } else {
            Log.d(TAG, "create ssid:" + pp.getSsid());
            wfg.SSID = "\"" + pp.getSsid() + "\"";
        }
        wfg.simSlot = "\"-1\"";
        wfg.imsi = "\"none\"";
        wfg.ieee80211w = "1";


        wfg.pcsc = "\"none\"";            
        wfg.status = WifiConfiguration.Status.ENABLED;

        wfg.allowedKeyManagement.clear();
        wfg.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.WPA_OSEN);
        setKeyMgmt(wfg);

        /*Group Ciphers*/
        wfg.allowedGroupCiphers.clear();
        wfg.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.CCMP);

        /*Protocols*/
        wfg.allowedProtocols.clear();
        wfg.allowedProtocols.set(WifiConfiguration.Protocol.OSEN);

        Class[] enterpriseFieldArray  = WifiConfiguration.class.getClasses();
        Class<?> enterpriseFieldClass = null;


        for(Class<?> myClass : enterpriseFieldArray) {
            if(myClass.getName().equals(INT_ENTERPRISEFIELD_NAME)) {
                enterpriseFieldClass = myClass;
                break;
            }
        }
        if (enterpriseFieldClass == null) return null;
        Log.d(TAG, "class chosen " + enterpriseFieldClass.getName() );


        Field anonymousId = null, caCert = null, clientCert = null,
              eap = null, identity = null, password = null,
              phase2 = null, privateKey =  null;

        Field[] fields = WifiConfiguration.class.getFields();


        for (Field tempField : fields) {
            if (tempField.getName().trim().equals(INT_ANONYMOUS_IDENTITY)) {
                anonymousId = tempField;
                Log.d(TAG, "field " + anonymousId.getName() );
            } else if (tempField.getName().trim().equals(INT_CA_CERT)) {
                caCert = tempField;
            } else if (tempField.getName().trim().equals(INT_CLIENT_CERT)) {
                clientCert = tempField;
                Log.d(TAG, "field " + clientCert.getName() );
            } else if (tempField.getName().trim().equals(INT_EAP)) {
                eap = tempField;
                Log.d(TAG, "field " + eap.getName() );
            } else if (tempField.getName().trim().equals(INT_IDENTITY)) {
                identity = tempField;
                Log.d(TAG, "field " + identity.getName() );
            } else if (tempField.getName().trim().equals(INT_PASSWORD)) {
                password = tempField;
                Log.d(TAG, "field " + password.getName() );
            } else if (tempField.getName().trim().equals(INT_PHASE2)) {
                phase2 = tempField;
                Log.d(TAG, "field " + phase2.getName() );

            } else if (tempField.getName().trim().equals(INT_PRIVATE_KEY)) {
                privateKey = tempField;
            }
        }


        Method setValue = null;


        for(Method m: enterpriseFieldClass.getMethods()) {
            if(m.getName().trim().equals("setValue")) {
                Log.d(TAG, "method " + m.getName() );
                setValue = m;
                break;
            }
        }

        try {
            // EAP
            setValue.invoke(eap.get(wfg), "AN-TLS");
                       
            // EAP Anonymous Id
            String anonymousid = mNai;
            Log.d(TAG, "anonymousid:" + anonymousid);
            setValue.invoke(anonymousId.get(wfg), anonymousid);                
            
        } catch (Exception e) {
            Log.d(TAG, "CreateEapTlsConfig err:" + e);
        }

        return wfg;
    }
    private WifiConfiguration CreateOpenConfig(PasspointPolicy pp) {

        WifiConfiguration wfg = new WifiConfiguration();
        if (pp.getBssid() != null) {
            wfg.BSSID = pp.getBssid();
        } else {
            wfg.SSID = "\"" + pp.getSsid() + "\"";
        }
        wfg.simSlot = null;
        if (mIsPmfRequired) {
            wfg.ieee80211w = "1";
        }
        wfg.allowedKeyManagement.clear();
        wfg.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE);

        return wfg;
    }


    private WifiConfiguration CreateEapTlsConfig(PasspointPolicy pp) {
        Log.d(TAG, "[CreateEapTlsConfig]");
        WifiConfiguration wfg = new WifiConfiguration();
        if("true".equals(SystemProperties.get("persist.service.manual.rem"))){
            Log.d(TAG, "create bssid:" + pp.getBssid());
            wfg.BSSID = "mtkemp";
        } else if (pp.getBssid() != null) {
            Log.d(TAG, "create bssid:" + pp.getBssid());
            wfg.BSSID = pp.getBssid();
        } else {
            Log.d(TAG, "create ssid:" + pp.getSsid());
            wfg.SSID = "\"" + pp.getSsid() + "\"";
        }
        wfg.simSlot = "\"-1\"";
        wfg.imsi = "\"none\"";

        if (mIsPmfRequired) {
            wfg.ieee80211w = "1";
        }
        
        int ocsplevel = 0;
        if (pp.getCredential().getCheckAaaServerCertStatus() == true) {
            ocsplevel = 2;
        }

        Log.d(TAG, "config ocsp to:" + ocsplevel);
        wfg.ocsp = Integer.toString(ocsplevel);

        wfg.pcsc = "\"none\"";            
        wfg.status = WifiConfiguration.Status.ENABLED;

        wfg.allowedKeyManagement.clear();
        wfg.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.WPA_EAP);
        wfg.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.IEEE8021X);
        setKeyMgmt(wfg);

        /*Group Ciphers*/
        /*wfg.allowedGroupCiphers.clear();
        wfg.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.CCMP);
        wfg.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.TKIP);*/

        /*Protocols*/
        /*wfg.allowedProtocols.clear();
        wfg.allowedProtocols.set(WifiConfiguration.Protocol.RSN);
        wfg.allowedProtocols.set(WifiConfiguration.Protocol.WPA);*/

        Class[] enterpriseFieldArray  = WifiConfiguration.class.getClasses();
        Class<?> enterpriseFieldClass = null;


        for(Class<?> myClass : enterpriseFieldArray) {
            if(myClass.getName().equals(INT_ENTERPRISEFIELD_NAME)) {
                enterpriseFieldClass = myClass;
                break;
            }
        }
        if (enterpriseFieldClass == null) return null;
        Log.d(TAG, "class chosen " + enterpriseFieldClass.getName() );


        Field anonymousId = null, caCert = null, clientCert = null,
              eap = null, identity = null, password = null,
              phase2 = null, privateKey =  null;

        Field[] fields = WifiConfiguration.class.getFields();


        for (Field tempField : fields) {
            if (tempField.getName().trim().equals(INT_ANONYMOUS_IDENTITY)) {
                anonymousId = tempField;
                Log.d(TAG, "field " + anonymousId.getName() );
            } else if (tempField.getName().trim().equals(INT_CA_CERT)) {
                caCert = tempField;
            } else if (tempField.getName().trim().equals(INT_CLIENT_CERT)) {
                clientCert = tempField;
                Log.d(TAG, "field " + clientCert.getName() );
            } else if (tempField.getName().trim().equals(INT_EAP)) {
                eap = tempField;
                Log.d(TAG, "field " + eap.getName() );
            } else if (tempField.getName().trim().equals(INT_IDENTITY)) {
                identity = tempField;
                Log.d(TAG, "field " + identity.getName() );
            } else if (tempField.getName().trim().equals(INT_PASSWORD)) {
                password = tempField;
                Log.d(TAG, "field " + password.getName() );
            } else if (tempField.getName().trim().equals(INT_PHASE2)) {
                phase2 = tempField;
                Log.d(TAG, "field " + phase2.getName() );

            } else if (tempField.getName().trim().equals(INT_PRIVATE_KEY)) {
                privateKey = tempField;
            }
        }


        Method setValue = null;


        for(Method m: enterpriseFieldClass.getMethods()) {
            if(m.getName().trim().equals("setValue")) {
                Log.d(TAG, "method " + m.getName() );
                setValue = m;
                break;
            }
        }

        try {
                // EAP
                String eapmethod = pp.getCredential().getEapMethod();
                Log.d(TAG, "eapmethod:" + eapmethod);
                setValue.invoke(eap.get(wfg), eapmethod);
                
                // Username, password, EAP Phase 2
                /*setValue.invoke(phase2.get(wfg), null);
                setValue.invoke(identity.get(wfg), null);
                setValue.invoke(password.get(wfg), null);*/
                
                // EAP Anonymous Id
                //setValue.invoke(anonymousId.get(wfg), anonymousid);
                
                // EAP CA Certificate
                String cacertificate = null;
                String rootCA = pp.getCredential().getCaRootCert();
                if("true".equals(SystemProperties.get("persist.service.aaacacert.on"))) {
                    setValue.invoke(caCert.get(wfg), SystemProperties.get("persist.service.aaacacert"));
                } else if (rootCA == null){
                    //cacertificate = ENTERPRISE_CA_CERT;
                } else {
                    cacertificate = "keystore://" + Credentials.WIFI + "HS20" + Credentials.CA_CERTIFICATE + rootCA;
                }
                Log.d(TAG, "cacertificate:" + cacertificate);
                setValue.invoke(caCert.get(wfg), cacertificate);
                
                //User certificate
                String usercertificate = null;
                String privatekey = null;
                String clientCertPath = pp.getCredential().getClientCertPath();
                if (clientCertPath != null){
                    privatekey = "keystore://" + Credentials.WIFI + "HS20" + Credentials.USER_PRIVATE_KEY + clientCertPath;
                    usercertificate = "keystore://" + Credentials.WIFI + "HS20" + Credentials.USER_CERTIFICATE + clientCertPath;
                }
                Log.d(TAG, "privatekey:" + privatekey);
                Log.d(TAG, "usercertificate:" + usercertificate);
                if (privatekey != null && usercertificate != null) {
                    setValue.invoke(privateKey.get(wfg), privatekey);
                    setValue.invoke(clientCert.get(wfg), usercertificate);
                }
            } catch (Exception e) {
                Log.d(TAG, "CreateEapTlsConfig fail to set:" + e);
            }

        return wfg;
    }

    private WifiConfiguration CreateEapTtlsConfig(PasspointPolicy pp) {

        WifiConfiguration wfg = new WifiConfiguration();
        if("true".equals(SystemProperties.get("persist.service.manual.rem"))){
            Log.d(TAG, "create bssid:" + pp.getBssid());
            wfg.BSSID = "mtkemp";
        }else if (pp.getBssid() != null) {
            Log.d(TAG, "create bssid:" + pp.getBssid());
            wfg.BSSID = pp.getBssid();
        } else {
            Log.d(TAG, "create ssid:[" + pp.getSsid()+"]");
            wfg.SSID = "\"" + pp.getSsid() + "\"";
        }
        wfg.simSlot = null;

        int ocsplevel = 0;
        if (pp.getCredential().getCheckAaaServerCertStatus() == true) {
            ocsplevel = 2;
        }

        Log.d(TAG, "config ocsp to:" + ocsplevel);
        wfg.ocsp = Integer.toString(ocsplevel);

        wfg.status = WifiConfiguration.Status.ENABLED;

        wfg.allowedKeyManagement.clear();
        wfg.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.WPA_EAP);
        setKeyMgmt(wfg);

        /*Group Ciphers*/
        wfg.allowedGroupCiphers.clear();
        wfg.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.CCMP);
        wfg.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.TKIP);

        /*Protocols*/
        wfg.allowedProtocols.clear();
        wfg.allowedProtocols.set(WifiConfiguration.Protocol.RSN);
        wfg.allowedProtocols.set(WifiConfiguration.Protocol.WPA);

        Class[] enterpriseFieldArray  = WifiConfiguration.class.getClasses();
        Class<?> enterpriseFieldClass = null;


        for(Class<?> myClass : enterpriseFieldArray) {
            if(myClass.getName().equals(INT_ENTERPRISEFIELD_NAME)) {
                enterpriseFieldClass = myClass;
                break;
            }
        }
        if (enterpriseFieldClass == null) return null;
        Log.d(TAG, "class chosen " + enterpriseFieldClass.getName() );


        Field anonymousId = null, caCert = null, clientCert = null,
              eap = null, identity = null, password = null,
              phase2 = null, privateKey =  null;

        Field[] fields = WifiConfiguration.class.getFields();


        for (Field tempField : fields) {
            if (tempField.getName().trim().equals(INT_ANONYMOUS_IDENTITY)) {
                anonymousId = tempField;
                Log.d(TAG, "field " + anonymousId.getName() );
            } else if (tempField.getName().trim().equals(INT_CA_CERT)) {
                caCert = tempField;
            } else if (tempField.getName().trim().equals(INT_CA_CERT)) {
            } else if (tempField.getName().trim().equals(INT_CLIENT_CERT)) {
                clientCert = tempField;
                Log.d(TAG, "field " + clientCert.getName() );
            } else if (tempField.getName().trim().equals(INT_EAP)) {
                eap = tempField;
                Log.d(TAG, "field " + eap.getName() );
            } else if (tempField.getName().trim().equals(INT_IDENTITY)) {
                identity = tempField;
                Log.d(TAG, "field " + identity.getName() );
            } else if (tempField.getName().trim().equals(INT_PASSWORD)) {
                password = tempField;
                Log.d(TAG, "field " + password.getName() );
            } else if (tempField.getName().trim().equals(INT_PHASE2)) {
                phase2 = tempField;
                Log.d(TAG, "field " + phase2.getName() );

            } else if (tempField.getName().trim().equals(INT_PRIVATE_KEY)) {
                privateKey = tempField;
            }
        }


        Method setValue = null;


        for(Method m: enterpriseFieldClass.getMethods()) {
            if(m.getName().trim().equals("setValue")) {
                Log.d(TAG, "method " + m.getName() );
                setValue = m;
                break;
            }
        }

        try {
                // EAP
                setValue.invoke(eap.get(wfg), pp.getCredential().getEapMethod());
                
                // EAP Phase 2
                setValue.invoke(phase2.get(wfg), ENTERPRISE_PHASE2_MSCHAPV2);
                
                // EAP Anonymous Id
                if("true".equals(SystemProperties.get("persist.service.noanonymousid"))){
                    //setValue.invoke(anonymousId.get(wfg), ENTERPRISE_ANON_IDENT);
                }else{
                    setValue.invoke(anonymousId.get(wfg), "anonymous@" + pp.getCredential().getRealm());                
                }
                
                // EAP CA Certificate
                String rootCA = pp.getCredential().getCaRootCert();//sha1
                if("true".equals(SystemProperties.get("persist.service.aaacacert.on"))) {
                    setValue.invoke(caCert.get(wfg), SystemProperties.get("persist.service.aaacacert"));
                }else if(rootCA == null){
                    //setValue.invoke(caCert.get(wfg), ENTERPRISE_CA_CERT);
                }else{
                    setValue.invoke(caCert.get(wfg), "keystore://" + Credentials.WIFI + "HS20" + Credentials.CA_CERTIFICATE + rootCA);
                }
                
                // Private Key
                //setValue.invoke(privateKey.get(wfg), ENTERPRISE_PRIV_KEY);
                
                // EAP Identity
                setValue.invoke(identity.get(wfg), pp.getCredential().getUserName());
                
                // EAP Password
                setValue.invoke(password.get(wfg), pp.getCredential().getPassword());
                
                // EAP Client certificate
                setValue.invoke(clientCert.get(wfg), pp.getCredential().getClientCertPath());         
            } catch (Exception e) {
                Log.d(TAG, "CreateEapTtlsConfig fail to set:" + e);
            }

        return wfg;
    }

    private WifiConfiguration CreateEapSimConfig(PasspointPolicy pp) {

        WifiConfiguration wfg = new WifiConfiguration();
        boolean realSIM = false;
        String imsi = pp.getCredential().getImsi();

        if (mTeleMgr.getSimState() == TelephonyManager.SIM_STATE_READY) {
            realSIM = true;
        }

        String eapType = pp.getCredential().getEapMethod();
        String plmnMCC = pp.getCredential().getMcc();
		String plmnMNC = pp.getCredential().getMnc();        
        if (pp.getBssid() != null) {
            wfg.BSSID = pp.getBssid();
        } else {
            wfg.SSID = "\"" + pp.getSsid() + "\"";
        }

        wfg.ieee80211w = "1";
        
        //SIM SLOT
        // TODO: ask AF9 for simsolt function
        /*if(realSIM){
            wfg.simSlot = "\"" + Integer.toString(SystemProperties.getInt(
                Phone.GEMINI_DEFAULT_SIM_PROP, 
                Phone.GEMINI_SIM_1)) + "\"";
        }else{
            wfg.simSlot = "\"0\"";
        }*/
        //wfg.imsi = "\"1232010000000000@wlan.mnc026.mcc310.3gppnetwork.org\"";
        String eapSimIMSI;
        if(plmnMNC.length() == 2){
            eapSimIMSI = imsi + "@wlan.mnc0" + plmnMNC + ".mcc" + plmnMCC + ".3gppnetwork.org";
        }else{
            eapSimIMSI = imsi + "@wlan.mnc" + plmnMNC + ".mcc" + plmnMCC + ".3gppnetwork.org";
        }

        if(eapType.equals("AKA")){
            wfg.imsi = "\"" + "0" + eapSimIMSI + "\"";
        }else if(eapType.equals("SIM")){
            wfg.imsi = "\"" + "1" + eapSimIMSI + "\"";
        }

        Log.d(TAG, "wfg.imsi: " + wfg.imsi );        
        wfg.pcsc = "\"rild\"";
        wfg.status = WifiConfiguration.Status.ENABLED;

        wfg.allowedKeyManagement.clear();
        wfg.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.WPA_EAP);
        setKeyMgmt(wfg);

        /*Group Ciphers*/
        wfg.allowedGroupCiphers.clear();
        wfg.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.CCMP);
        wfg.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.TKIP);

        /*Protocols*/
        wfg.allowedProtocols.clear();
        wfg.allowedProtocols.set(WifiConfiguration.Protocol.RSN);
        wfg.allowedProtocols.set(WifiConfiguration.Protocol.WPA);

        Class[] enterpriseFieldArray  = WifiConfiguration.class.getClasses();
        Class<?> enterpriseFieldClass = null;


        for(Class<?> myClass : enterpriseFieldArray) {
            if(myClass.getName().equals(INT_ENTERPRISEFIELD_NAME)) {
                enterpriseFieldClass = myClass;
                break;
            }
        }

        if (enterpriseFieldClass == null) return null;
        Log.d(TAG, "class chosen " + enterpriseFieldClass.getName() );


        Field anonymousId = null, caCert = null, clientCert = null,
              eap = null, identity = null, password = null,
              phase2 = null, privateKey =  null;

        Field[] fields = WifiConfiguration.class.getFields();


        for (Field tempField : fields) {
            if (tempField.getName().trim().equals(INT_ANONYMOUS_IDENTITY)) {
                anonymousId = tempField;
                Log.d(TAG, "field " + anonymousId.getName() );
            } else if (tempField.getName().trim().equals(INT_CA_CERT)) {
                caCert = tempField;
            } else if (tempField.getName().trim().equals(INT_CA_CERT)) {
            } else if (tempField.getName().trim().equals(INT_CLIENT_CERT)) {
                clientCert = tempField;
                Log.d(TAG, "field " + clientCert.getName() );
            } else if (tempField.getName().trim().equals(INT_EAP)) {
                eap = tempField;
                Log.d(TAG, "field " + eap.getName() );
            } else if (tempField.getName().trim().equals(INT_IDENTITY)) {
                identity = tempField;
                Log.d(TAG, "field " + identity.getName() );
            } else if (tempField.getName().trim().equals(INT_PASSWORD)) {
                password = tempField;
                Log.d(TAG, "field " + password.getName() );
            } else if (tempField.getName().trim().equals(INT_PHASE2)) {
                phase2 = tempField;
                Log.d(TAG, "field " + phase2.getName() );

            } else if (tempField.getName().trim().equals(INT_PRIVATE_KEY)) {
                privateKey = tempField;
            } 
        }


        Method setValue = null;


        for(Method m: enterpriseFieldClass.getMethods()) {
            if(m.getName().trim().equals("setValue")) {
                Log.d(TAG, "method " + m.getName() );
                setValue = m;
                break;
            }
        }

        try {
            // EAP
            setValue.invoke(eap.get(wfg), pp.getCredential().getEapMethod());

            // EAP Anonymous Id
            //setValue.invoke(anonymousId.get(wfg), ENTERPRISE_ANON_IDENT);

            // EAP CA Certificate
            //setValue.invoke(caCert.get(wfg), ENTERPRISE_CA_CERT);

            // Private Key
            //setValue.invoke(privateKey.get(wfg), ENTERPRISE_PRIV_KEY);

            // EAP Identity
            if(eapType.equals("AKA")){
                setValue.invoke(identity.get(wfg), "0" + eapSimIMSI);
            }else if(eapType.equals("SIM")){
                setValue.invoke(identity.get(wfg), "1" + eapSimIMSI);
            }            
            
            
            // EAP Password
            if(!realSIM){
                setValue.invoke(password.get(wfg), pp.getCredential().getPassword());
            }

            // EAP Client certificate
            setValue.invoke(clientCert.get(wfg), pp.getCredential().getClientCertPath());

        } catch (Exception e) {
            Log.d(TAG, "CreateEapSimConfig fail to set:" + e);
        }

        return wfg;
    }

    private void RemoveCurrentNetwork() {
        WifiInfo info = mWifiMgr.getConnectionInfo();
        if (info != null) {
            int networkId = info.getNetworkId();
            Log.d(TAG, "RemoveCurrentNetwork from conn_info:" + networkId);
            mWifiMgr.removeNetwork(networkId);
            mWifiMgr.saveConfiguration();
            return;
        } else {
            String ssid = mCurrentUsedPolicy.getSsid();
            List<WifiConfiguration> networks = mWifiMgr.getConfiguredNetworks();
            if (networks == null ) {
                Log.d(TAG, "RemoveCurrentNetwork getConnectionInfo null");
                return;
            } else {
                for (WifiConfiguration config : networks) {
                    if (ssid.equals(config.SSID)) {
                        int networkId = config.networkId;
                        Log.d(TAG, "RemoveCurrentNetwork: from configuration" + networkId);
                        mWifiMgr.removeNetwork(networkId);
                        mWifiMgr.saveConfiguration();
                        return;
                    }
                }
            }
        }
    }

    private void ConnectToPasspoint(PasspointPolicy pp) {
        Log.d(TAG, "ConnectToPasspoint:" + pp );

        // 1.Connect to passpoint with the credential
        // 2.Connect to passpoint with no credential	

        WifiInfo info = mWifiMgr.getConnectionInfo();
        if (info != null) {
            String ssid = info.getSSID();
            Log.d(TAG, "getConnectionInfo:" + info);
            if (ssid != null && ssid.equals(pp.getSsid())) {
                Log.d(TAG, "The passpoint is already connected");
                return;
            }
        }

        //Query bssid by fqdn of the policy
        /*String bssid = mWifiMgr.getHsMatchNai(pp.getCredential().getRealm(), "EAP_TTLS");
        if ("NO_MATCH".equals(bssid)) {
            Log.d(TAG, "Connect fail due to fqdn qurey NO_MATCH:" + pp.getCredential().getFqdn());
            return;
        } else {
            pp.setBssid(bssid);
        }*/

        WifiConfiguration wfg = null;

        PasspointCredential currentCredential = pp.getCredential();
        if(currentCredential != null){
            String updateIdentifier = currentCredential.getUpdateIdentifier();
            Log.d(TAG, "[ConnectToPasspoint] updateIdentifier = " + updateIdentifier);                            
            
            if(updateIdentifier != null && !updateIdentifier.isEmpty()){
                Log.d(TAG, "[ConnectToPasspoint] set updateidentifier to supplicant");                        
                mWifiMgr.setHsUpdateIdentifier(updateIdentifier);        
            }else{
                mWifiMgr.setHsUpdateIdentifier("0");        
            }
        }else{
            mWifiMgr.setHsUpdateIdentifier("0");        
        }
 
        List<ScanResult> results = mWifiMgr.getScanResults();
        if(results == null){
            Log.d(TAG, "Scan result is null.");            
            return;
        }
        
        boolean osen = false;
        for (ScanResult result : results) {
            //Log.d(TAG, "[ConnectToPasspoint] scanresult:" + result);
            if(result.SSID.equals(pp.getSsid())) {
                if (result.capabilities.contains("OSEN")) {
                    osen = true;
                }
            }
        }

        if( osen ){
            Log.d(TAG, "CreateOsenConfig");  
            mWifiMgr.disableAllNetworkAtOnce();
            wfg = CreateOsenConfig(pp);
        } else if (null == currentCredential || null == currentCredential.getEapMethod()) {
            Log.d(TAG, "CreateOpenConfig");    
            mWifiMgr.disableAllNetworkAtOnce();
            wfg = CreateOpenConfig(pp);
        } else if ("TTLS".equals(currentCredential.getEapMethod())) {
            Log.d(TAG, "CreateEapTtlsConfig");
            wfg = CreateEapTtlsConfig(pp);
        } else if ("SIM".equals(currentCredential.getEapMethod())) {
            Log.d(TAG, "CreateEapSimConfig");
            //mWifiMgr.staEnableSwSim();
            wfg = CreateEapSimConfig(pp);
        } else if("TLS".equals(currentCredential.getEapMethod())) {
            Log.d(TAG, "CreateEapTlsConfig");
            wfg = CreateEapTlsConfig(pp);
        }        

        int netid = 0;
        WifiConfiguration configuredWfg = findConfiguredNetworks(wfg);
        if(configuredWfg != null) {
            netid = configuredWfg.networkId;
            Log.d(TAG, "The passpoint is configed but disconnected, netid:"+netid+" ssid:"+configuredWfg.SSID);
        } else {
            netid = mWifiMgr.addNetwork(wfg);
            Log.d(TAG, "The passpoint is not configed, addNetwork:" + netid + " ssid:" + wfg.SSID);
        }

        // Set highest priority while OSU or using preferred credential
        if (pp.getSubscriptionPriority() == 0 ||
            (osen || null == currentCredential || null == currentCredential.getEapMethod())) {
            mWifiMgr.setNetworkPriority(netid, findHighestPriorityFromNetwork()+ 1);
        }
        //smart blacklist
        /*final int SMART_TIMEOUT = 5*60*1000;
        if (pp.compareTo(mLastUsedPolicy) != 0) {
            mStartTime = System.currentTimeMillis();
            mLastUsedPolicy = pp;
            Log.d(TAG, "[ConnectToPasspoint] new policy");
        } else {
            long endTime = System.currentTimeMillis();
            Log.d(TAG, "[ConnectToPasspoint] old policy" + 
                " timeout:" + ((endTime - mStartTime)/1000) + "/" + (SMART_TIMEOUT/1000));
            if ((endTime - mStartTime) > SMART_TIMEOUT) {
                mSmartBlackPolicy.add(pp);
            }
        }*/

        /* For applying WifiStateTracker design change by Mediatek.
         * The device can't reconnect if auth. failed over max retry count.("mediatek.wlan.hs20.reauth")
         * If the network was configured and disabled, don't enable it again.
         */
        if (configuredWfg != null && mIsEapFailureOverCount) {
            Log.d(TAG, "[ConnectToPasspoint] mIsEapFailureOverCount:true");
            List<WifiConfiguration> configuredList = mWifiMgr.getConfiguredNetworks();
            for (WifiConfiguration config : configuredList) {
                Log.d(TAG, "[ConnectToPasspoint] check configured netid:"+config.networkId+" status:"+config.status); 
                if (config.networkId == netid && config.status == WifiConfiguration.Status.DISABLED) {
                    Log.d(TAG, "[ConnectToPasspoint] return due to disabled"); 
                    return;
                }
            }
        }


        mWifiMgr.enableAllNetworkAtOnce();
    }

    private WifiConfiguration findConfiguredNetworks(WifiConfiguration wfg) {

        List<WifiConfiguration> networks = mWifiMgr.getConfiguredNetworks();
        if (wfg == null || wfg.SSID == null || networks == null) {
            return null;
        }

        for (WifiConfiguration config : networks) {
            if (wfg.SSID.equals(config.SSID)) {
                Log.d(TAG, "findConfiguredNetworks:"+config.SSID);
                return config;
            }
        }
        Log.d(TAG, "findConfiguredNetworks:empty");
        return null;
    }

    private int findHighestPriorityFromNetwork() {
        List<WifiConfiguration> networks = mWifiMgr.getConfiguredNetworks();
        int maxPriority = 0;
        WifiConfiguration foundWfg = null;
        
        if (networks == null) return 0;

        for (WifiConfiguration config : networks) {
            if (config.priority > maxPriority) {
                maxPriority = config.priority;
            }
        }
        Log.d(TAG, "findHighestPriorityFromNetwork: " + maxPriority);
        return maxPriority;
    }
    
    private HashSet<String> parseSsidString( String srcString, String tag) {
        //MATCH:<ssid string> or NO_MATCH
        int srclength = srcString.length();
        //final String eol = System.getProperty("line.separator");
        final String eol = "/n";
        if( tag.length() == 0 || srclength == 0 ) {
            return null;
        };
        HashSet<String> sResultSet = new HashSet<String>();
        //Log.d(TAG, "parseSsidString["+srcString+"]");
        String ssid = null;
        int pos =0;
        int posend =0;

        pos = srcString.indexOf( tag );
        while (pos >= 0) {
            pos += tag.length();
            posend = srcString.indexOf( tag, pos )-1;
            if( posend < 0) {
                posend = srclength;
            }
            if ( posend <= srclength) {
                ssid = srcString.substring(pos, posend);
                sResultSet.add( ssid );
                pos = srcString.indexOf( tag, posend );
            } else {
                pos = -1;
            }
        }
        //Dump result for check
        Log.d(TAG, "parseSsidString:" + sResultSet.size() );
        for (String result : sResultSet ) {
            Log.d(TAG, "result:" + result);
        }

        return sResultSet;
    }
    
    public void sendProvisionFromFileDone() {
        sendMessage(EVENT_PROVISION_FROM_FILE_DONE);
    }

    public void sendProvisionDone(int result, WifiTree tree) {
        Message msg = obtainMessage(EVENT_PROVISION_DONE, result, 0, tree);
        sendMessage(msg);
    }

    public void sendPreProvisionDone(int result, WifiTree tree) {
        Message msg = obtainMessage(EVENT_PRE_PROVISION_DONE, result, 0, tree);
        sendMessage(msg);
    }

    public void sendRemediationDone(int result, WifiTree tree) {
        Message msg = obtainMessage(EVENT_REMEDIATION_DONE, result, 0, tree);
        sendMessage(msg);
    }

    public void sendPolicyProvisionDone(int result, WifiTree tree) {
        Message msg = obtainMessage(EVENT_POLICY_PROVISION_DONE, result, 0, tree);
        sendMessage(msg);
    }
    
    public void sendSimProvisionDone(int result, WifiTree tree){
        Message msg = obtainMessage(EVENT_SIM_PROVISION_DONE, result, 0, tree);
        sendMessage(msg);
    }

    public void sendProvisionUpdateResponseDone(boolean success) {
        sendMessage(EVENT_PROVISION_UPDATE_RESPONSE_DONE, success);
    }

    public void sendSimProvisionUpdateResponseDone(boolean success) {
        sendMessage(EVENT_SIM_PROVISION_UPDATE_RESPONSE_DONE, success);
    }

    public void sendRemediationUpdateResponseDone(boolean success) {
        sendMessage(EVENT_REMEDIATION_UPDATE_RESPONSE_DONE, success);
    }

    public void sendPolicyProvisionUpdateResponseDone(boolean success) {
        sendMessage(EVENT_POLICY_PROVISION_UPDATE_RESPONSE_DONE, success);
    }

    public void sendRegisterCertEnrollmentDone(boolean success){
        sendMessage(EVENT_REGISTER_CERT_ENROLLMENT_DONE, success);
    }

    public void setOsuServiceProvider(String bssid) {
        sendMessage( obtainMessage(EVENT_OSU_USER_SLCT_DONE, bssid) );
    }

    private HashSet<String> getSsidSet( String method,PasspointCredential credential, String PartnerFQDN) {
        String sResult = null;
        String parsedPartnerFQDN;
        String parsePartnerType; //includeSubdomains or exactMatch
        Log.d(TAG, "[createPolicyByCredential] getSsidSet:" + method);
        switch( MatchMethod.toMatchMethod(method.toUpperCase()) ) {
        case REALM:
            sResult = mWifiMgr.getHsMatchNai( credential.getRealm(), credential.getEapMethod() );
            break;
        case HOMEFQDN:
            sResult = mWifiMgr.getHsMatchFqdn( credential.getFqdn() );
            break;
        case PARTNERFQDN:
            if( PartnerFQDN != null && PartnerFQDN.length()!=0 ) {
                int srclength = PartnerFQDN.length();
                int pos =0;
                int posend =0;
                posend = PartnerFQDN.indexOf( "," );
                if (posend >= 0) {
                    parsedPartnerFQDN = PartnerFQDN.substring(pos, posend);
                    if(PartnerFQDN.substring(posend + 1).equals("includeSubdomains")){
                        sResult = mWifiMgr.getHsMatchFqdn( parsedPartnerFQDN + " -i" );
                    }else if(PartnerFQDN.substring(posend + 1).equals("exactMatch")){
                        sResult = mWifiMgr.getHsMatchFqdn( parsedPartnerFQDN );
                    }
                }
            }

            break;
        case PLMN:
            sResult = mWifiMgr.getHsMatchPlmn( credential.getMcc()+" "+credential.getMnc());
            break;

        case OTHERHOMEPARTNER:
            sResult = mWifiMgr.getHsMatchFqdn( credential.getOtherhomepartners());
            break;
            
        case NOVALUE:
            break;
        }

        if( sResult != null ) {
            return parseSsidString(sResult,"MATCH:");
        }

        Log.d(TAG, "getSsidSet = null");
        return null;
    }

    private PasspointPolicy getPolicyUserPreferred(Collection<PasspointPolicy> policyCollection) {
        PasspointPolicy choosedpolicy = null;
        for (PasspointPolicy policy : policyCollection ) {
            if( policy.getSubscriptionPriority() == 0 ) {
                choosedpolicy = policy;
                // TODO: compare others if we have two/more user preferred policy
            }
        }
        Log.d(TAG, "getPolicyUserPreferred:" + choosedpolicy);
        return choosedpolicy;
    }

    private PasspointPolicy getPolicyMinimumBackhaulThreshold(Collection<PasspointPolicy> policyCollection, Collection<Integer> moIndexCollection) {
        HashSet<String> ssidSet = null;
        ArrayList<String> checkPoint = new ArrayList<String>();
        
        for (PasspointPolicy policy : policyCollection) {
            Collection<WifiTree.MinBackhaulThresholdNetwork> list = mTreeHelper.getMinBackhaulThreshold(
                mTreeHelper.getCredentialInfo(mWifiTree,
                    policy.getCredential().getWifiSPFQDN(), 
                    policy.getCredential().getCredName()));
            
            if (list == null) continue;

            for (WifiTree.MinBackhaulThresholdNetwork item : list) {
            
                ssidSet = getSsidSetMinimumBackhaulThreshold(item.DLBandwidth, item.ULBandwidth);
            
                if (ssidSet == null) continue;

                if ("Home".equals(item.NetworkType)) {
                    for (String ssid : ssidSet) {
                        if (ssid.equals(policy.getSsid()) && policy.getHomeSp() == true) {
                            return policy;
                        }
                    }
                }
            
                if ("Roaming".equals(item.NetworkType)) {
                    for (String ssid : ssidSet) {
                        if (ssid.equals(policy.getSsid()) && policy.getHomeSp() == false) {
                            return policy;
                        }
                    }
                }
            }

        }
        
        Log.d(TAG, "getPolicyMinimumBackhaulThreshold: null");
        return null;
    }

    HashSet<String> getSsidSetMinimumBackhaulThreshold(String dlBandwidth, String ulBandwidth) {
		String sResult = null;
        Log.d(TAG, "getSsidSetMinimumBackhaulThreshold dlBandwidth: " + dlBandwidth + " ulBandwidth: " + ulBandwidth);
        sResult = mWifiMgr.getHsMinBackhaul( dlBandwidth, ulBandwidth);

        if( sResult != null ) {
            return parseSsidString(sResult,"MATCH:");
        }

        Log.d(TAG, "getSsidSetMinimumBackhaulThreshold null");
        return null;
    }

    private PasspointPolicy getPolicyProtoPortTuple(Collection<PasspointPolicy> policyCollection, Collection<Integer> moIndexCollection) {
        HashSet<String> ssidSet = null;

        for (PasspointPolicy policy : policyCollection) {
            Collection<WifiTree.RequiredProtoPortTuple> list = mTreeHelper.getRequiredProtoPortTuple(
                mTreeHelper.getCredentialInfo(mWifiTree,
                    policy.getCredential().getWifiSPFQDN(), 
                    policy.getCredential().getCredName()));
            
            for (WifiTree.RequiredProtoPortTuple item : list) {
            
                ssidSet = getSsidSetProtoPortTuple(item.IPProtocol, item.PortNumber);

                if (ssidSet == null) continue;

                for (String ssid : ssidSet) {
                    if (ssid.equals(policy.getSsid())) {
                        return policy;
                    }
                }
            }
        }        

        Log.d(TAG, "getPolicyProtoPortTuple null");
        return null;
    }

    HashSet<String> getSsidSetProtoPortTuple(String proto, String port) {
        String sResult = null;
        Log.d(TAG, "[getProtoQualifiedSsidSet] proto: " + proto + " port: " + port);
        sResult = mWifiMgr.getHsProtoPort( proto, port);

        if( sResult != null ) {
            return parseSsidString(sResult,"MATCH:");
        }

        Log.d(TAG, "getSsidSetProtoPortTuple null");
        return null;
    }

    private PasspointPolicy getPolicyBssLoad(Collection<PasspointPolicy> policyCollection, Collection<Integer> moIndexCollection) {
        HashSet<String> ssidSet = null;

        for (PasspointPolicy policy : policyCollection) {
            String maxvalue = mTreeHelper.getMaximumBSSLoadValue(
                mTreeHelper.getCredentialInfo(mWifiTree,
                    policy.getCredential().getWifiSPFQDN(), 
                    policy.getCredential().getCredName()));
            
            if (maxvalue != null) {
                ssidSet = getSsidSetBssLoad(maxvalue);
                if (ssidSet == null) return null;
                for (String ssid : ssidSet) {
                    if (ssid.equals(policy.getSsid())) {
                        return policy;
                    }
                }
            }
        }       


        Log.d(TAG, "getPolicyBssLoad null");
        return null;
    }

    HashSet<String> getSsidSetBssLoad(String maxvalue) {
        String sResult = null;
        Log.d(TAG, "getSsidSetBssLoad maxvalue: " + maxvalue);
        sResult = mWifiMgr.getHsBssload(maxvalue);

        if( sResult != null ) {
            return parseSsidString(sResult,"MATCH:");
        }

        Log.d(TAG, "getSsidSetBssLoad null");
        return null;
    }

    private PasspointPolicy getPolicyDefault(Collection<PasspointPolicy> policyCollection) {
        PasspointPolicy choosedpolicy = null;
        for (PasspointPolicy policy : policyCollection ) {
            if( policy.compareTo(choosedpolicy) < 0 ) {
                choosedpolicy = policy;
            }
        }
        Log.d(TAG, "getPolicyDefault:" + choosedpolicy);
        return choosedpolicy;
    }

    private void createPolicyByCredential() {
        HashSet<String> ssidSet = new HashSet<String>();

        for (PasspointCredential credential : mCredentialList ) {
            Log.d(TAG, "[createPolicyByCredential] a credential starts - type:"+ credential.getEapMethod()
                        + " priority:" + credential.getPriority() + " fqdn:" + credential.getFqdn());

            //Match realm/PLMN  first
            if("SIM".equals(credential.getEapMethod())) { //MATCH PLMN
                Log.d(TAG, "[createPolicyByCredential] match PLMN:");               
                ssidSet = getSsidSet("PLMN", credential, null);   
                createDefaultPolicies(ssidSet, credential);         
            } else if( credential.getRealm() != null ) { //MATCH REALM (realm + eap type)
                Log.d(TAG, "[createPolicyByCredential] match REALM");               
                ssidSet = getSsidSet("REALM", credential, null);
                createDefaultPolicies(ssidSet, credential);

            } else {
                Log.d(TAG, "[createPolicyByCredential] a credential fail to create policy");
                continue;
            }
          

            int foundHomeSP = 0;
            int foundHomeSPOtherHomePartner = 0;

            //Match HomeSP FQDN
            if (credential.getFqdn() != null) {
                foundHomeSP = matchHomeSPFQDN(credential);
                Log.d(TAG, "[createPolicyByCredential] matchHomeSPFQDN:" + foundHomeSP);
            } else {
                Log.d(TAG, "[createPolicyByCredential] skip inspection (HomeSP.FQDN)");
            }

            //MATCH HomeSP OtherHomePartner
            if (credential.getOtherhomepartners() != null) {
                foundHomeSPOtherHomePartner = matchHomeSPOtherHomePartner(credential);
                Log.d(TAG, "[createPolicyByCredential] matchHomeSPOtherHomePartner:" + foundHomeSPOtherHomePartner);
            } else {
                Log.d(TAG, "[createPolicyByCredential] skip inspection (HomeSP.OtherHomePartner)");
            }

            //MATCH HomeOI                
            if (foundHomeSP > 0 || foundHomeSPOtherHomePartner > 0) {
                if (matchHomeOI(credential)) {
                    continue;
                }
            } else {
                Log.d(TAG, "[createPolicyByCredential] skip inspection (HomeSP.HomeOI)");
            }

            //MATCH Roaming Partner                
            if(credential.getPrpList() != null){
                Log.d(TAG, "[createPolicyByCredential] matchRoamingPartner");
                matchRoamingPartner(credential);
            } else {
                Log.d(TAG, "[createPolicyByCredential] skip inspection (Policy.PreferredRoamingPartenerList)");
            }
        }
    }

    private void createDefaultPolicies (HashSet<String> ssidset, PasspointCredential credential) {
        if (ssidset == null || ssidset.size() == 0 ) {
            Log.d(TAG, "createPolicies ssidset is empty");
            return;
        }

        synchronized (mPolicySync) {
            PasspointPolicy policy;
            for (String ssid : ssidset) {
                policy = buildPolicy(ssid, credential.getPriority(), ssid, null, credential, PasspointPolicy.UNRESTRICTED, false);
                Log.d(TAG, "mNetworkPolicy.add(policy)");
                mNetworkPolicy.add(policy);
            }
        }
        dumpPolicies(mNetworkPolicy);
    }

    private PasspointPolicy findPolicy(PasspointPolicy policy) {
        PasspointPolicy foundpolicy = null;
    
        synchronized (mPolicySync) {
            if(mNetworkPolicy.contains(policy)) {
                foundpolicy = policy;
            }
        }
        Log.d(TAG, "findPolicy:" + foundpolicy);
        return foundpolicy;
    }

    private PasspointPolicy buildPolicy(String name, int priority, String ssid,
                                         String bssid, PasspointCredential pc,
                                         int restriction, boolean ishomesp) {

        PasspointPolicy policy = new PasspointPolicy(name, priority, ssid, bssid, pc, restriction, ishomesp);
        Log.d(TAG, "buildPolicy:" + policy);
        return policy;
    }

    private boolean addPolicy(PasspointPolicy newpolicy) {
        boolean ret = false;
        synchronized (mPolicySync) {
            if(!mNetworkPolicy.contains(newpolicy)) {
                ret = mNetworkPolicy.add(newpolicy);
            }
        }
        Log.d(TAG, "addPolicy:" + ret + " ssid:" + newpolicy.getSsid());
        return ret;
    }

    private boolean deletePolicy(PasspointPolicy oldpolicy) {
        /**
         * this method will delete ONE policy
         */
        if(oldpolicy == null){
            return false;
        }
        
        boolean ret = false;
        synchronized (mPolicySync) {
            if(mNetworkPolicy.contains(oldpolicy)) {
                ret = mNetworkPolicy.remove(oldpolicy);
            }
        }
        Log.d(TAG, "deletePolicy:" + ret + " ssid:" + oldpolicy.getSsid());
        return ret;
    }

    private boolean updatePolicy( PasspointPolicy newpolicy) {
        boolean found = false;
        synchronized (mPolicySync) {
            Log.d(TAG, "updatePolicy:" + newpolicy);
            for ( PasspointPolicy policy : mNetworkPolicy) {
                if( newpolicy.getSsid() != null && policy.getSsid() != null && newpolicy.getSsid().equals(policy.getSsid()) ) {
                    found = true;
                    //Update Restriction info
                    if( newpolicy.getRestriction() < policy.getRestriction() ) {
                        policy.setRestriction( newpolicy.getRestriction() );
                        Log.d(TAG, "updatePolicy policy.setRestriction:" +newpolicy.getRestriction());
                    }

                    //Update Homesp info
                    policy.setHomeSp(newpolicy.getHomeSp());
                    Log.d(TAG, "updatePolicy policy.setHomeSp:" +newpolicy.getHomeSp());

                    //Force update roaming priority for PartnerList/FQDN
                    policy.setRoamingPriority(newpolicy.getRoamingPriority());
                    Log.d(TAG, "updatePolicy policy.setRoamingPriority:" +newpolicy.getRoamingPriority());

                    //Replace with a higher credential
                    Log.d(TAG, "updatePolicy compareTo" );
                    if(newpolicy.getCredential().compareTo( policy.getCredential() ) < 0) {
                        policy.setCredential( newpolicy.getCredential() );
                        Log.d(TAG, "updatePolicy policy.setCredential:" +newpolicy.getCredential());
                    }
                }
            }
        }
        Log.d(TAG, "updatePolicy found?" + found);
        return found;
    }

    private boolean matchHomeOI(PasspointCredential credential){

        boolean found = false;
        TreeSet<PasspointPolicy> newNetworkPolicies = new TreeSet<PasspointPolicy>();
        newNetworkPolicies = (TreeSet)mNetworkPolicy.clone();
        Collection<WifiTree.HomeOIList> homeOIList = credential.getHomeOIList();

        if(homeOIList == null || homeOIList.isEmpty()) {
            Log.d(TAG,"[matchHomeOI]credential.getHomeOIList is null");
            return false;
        }

        // remove policy if the HomeOI of credential is not match
        for (WifiTree.HomeOIList homeoiListItem : homeOIList ) {
            Log.d(TAG, "[matchHomeOI]HomeOI:" + homeoiListItem.HomeOI);
            Log.d(TAG, "[matchHomeOI]HomeOIRequired:" + homeoiListItem.HomeOIRequired);

            if( homeoiListItem.HomeOIRequired && !homeoiListItem.HomeOI.isEmpty() ) {
                Log.d(TAG, "[matchHomeOI]getHsMatchHomeOi:" + homeoiListItem.HomeOI);
                String sResult = mWifiMgr.getHsMatchHomeOi( homeoiListItem.HomeOI );
                HashSet<String> sResultSet = null;
                if( sResult != null ) {
                    sResultSet = parseSsidString(sResult,"MATCH:");
                }

                if (sResultSet == null) {
                    Log.d(TAG, "[matchHomeOI]HomeOI(" + homeoiListItem.HomeOI + ") not found");
                    continue;
                }

                if (mNetworkPolicy.isEmpty()) {
                    Log.d(TAG,"[matchHomeOI]mNetworkPolicy.isEmpty");
                    continue;
                }

                for ( String ssid : sResultSet ) {
                    Log.d(TAG,"[matchHomeOI]mNetworkPolicy size:" + mNetworkPolicy.size());
                    for (PasspointPolicy policy : mNetworkPolicy ) {
                        if( policy.getSsid().equals(ssid) ) {
                            found = true;
                            policy.setHomeSp(true);
                            updatePolicy(policy);
                            Log.d(TAG,"keep policy in list:" + ssid);
                        } else {
                            boolean r = newNetworkPolicies.remove(policy);
                            Log.d(TAG,"delete policy in list:" + policy.getSsid() + " ret:" + r);
                        }
                    }
                }
            }
        }
        mNetworkPolicy = newNetworkPolicies;
        dumpPolicies(mNetworkPolicy);
        Log.d(TAG,"matchHomeOI:" + found);
        return found;
    }

    private int matchHomeSPFQDN(PasspointCredential credential){
        HashSet<String> ssidSet;
        PasspointPolicy policy = null;
        
        ssidSet = getSsidSet("HOMEFQDN", credential, null); 
        if(ssidSet == null){
            return 0;
        }

        for (String ssid : ssidSet) {
            policy = buildPolicy(ssid, credential.getPriority(), ssid, null, credential, PasspointPolicy.HOME_SP, true);
            updatePolicy(policy);
        }

        return ssidSet.size();
    }
    
    private int matchHomeSPOtherHomePartner(PasspointCredential credential){
        HashSet<String> ssidSet;
        PasspointPolicy policy = null;

        ssidSet = getSsidSet("OTHERHOMEPARTNER", credential, null);
        if(ssidSet == null){
            return 0;
        }

        for (String ssid : ssidSet) {
            policy = buildPolicy(ssid, credential.getPriority(), ssid, null, credential, PasspointPolicy.HOME_SP, true);
            updatePolicy(policy);
        }

        return ssidSet.size();
    }
    
    private int matchRoamingPartner(PasspointCredential credential){
        HashSet<String> ssidSet;
        int roamingPartners = 0;
        String prpFqdnMatch;
        PasspointPolicy policy = null;
        
        for (WifiTree.PreferredRoamingPartnerList prpListItem : credential.getPrpList() ) { 
            prpFqdnMatch = prpListItem.FQDN_Match;
            if( prpFqdnMatch != null && prpFqdnMatch.length() != 0 ) {
                ssidSet = getSsidSet("PARTNERFQDN", credential, prpFqdnMatch);
                if(ssidSet == null){
                    continue;
                }

                for (String ssid : ssidSet) {
                    policy = buildPolicy(ssid, credential.getPriority(), ssid, null, credential, PasspointPolicy.ROAMING_PARTNER, false);
                    policy.setRoamingPriority(Integer.valueOf(prpListItem.Priority));
                    updatePolicy(policy);
                }

                roamingPartners += ssidSet.size();
                Log.d(TAG, "[matchRoamingPartner] match Roaming Partner:" + prpFqdnMatch);                                                     
            }
        }
        Log.d(TAG, "[createPolicyByCredential] matchRoamingPartner end:" + roamingPartners);
        return roamingPartners;
    }
    
    private void tryConnectToPasspoint() {

        PasspointPolicy choosedpolicy = choosePolicy(mNetworkPolicy); 

        if (choosedpolicy == null) {
            Log.d(TAG, "tryConnectToPasspoint - Policy not found");
            return;
        }

        Collection<WifiTree.SPExclusionList> list = mTreeHelper.getSPExclusionList(
            mTreeHelper.getCredentialInfo(mWifiTree,
                    choosedpolicy.getCredential().getWifiSPFQDN(), 
                    choosedpolicy.getCredential().getCredName()));
        
        if (list != null && !mIsIgnoreBlackList) {
            for (WifiTree.SPExclusionList item : list) {
                Log.d(TAG, "SPExclusionList item.SSID:" + item.SSID);
                if( item.SSID.equals(choosedpolicy.getSsid()) ) {
                    Log.d(TAG, "tryConnectToPasspoint - blackSsid match");
                    mNetworkPolicy.remove(choosedpolicy);
                    tryConnectToPasspoint();
                    return;
                }
            }
        }

        mCurrentUsedPolicy = choosedpolicy;
        ConnectToPasspoint(choosedpolicy);
    }

    private PasspointPolicy choosePolicy( Collection<PasspointPolicy> policyCollection ) {
        Log.d(TAG, "choosePolicy");
        PasspointPolicy choosedpolicy = null;
        dumpPolicies(mNetworkPolicy);

        /* 
         * Following selection has no priority description in the TSv3.08
         * We implement by test plan.
         */

        //Choose ssid by user preferred
        choosedpolicy = getPolicyUserPreferred(policyCollection);
        if (choosedpolicy != null) {
            Log.d(TAG, "choosePolicy, getPolicyUserPreferred");
            return choosedpolicy;
        }

        //Collect all policy related mo
        HashSet<Integer> moIndexes = new HashSet<Integer>();
        for (PasspointPolicy policy : policyCollection ) {
            moIndexes.add(policy.getCredential().getSubscriptionMoIndex());
        }
        
        //Choose ssid by Minimum Backhaul Threshold if MO present
        choosedpolicy = getPolicyMinimumBackhaulThreshold(policyCollection, moIndexes);
        if (choosedpolicy != null) {
            Log.d(TAG, "choosePolicy, getPolicyMinimumBackhaulThreshold");
            return choosedpolicy;
        }

        //Choose ssid by Proto Port Tuple if MO present
        choosedpolicy = getPolicyProtoPortTuple(policyCollection, moIndexes);
        if (choosedpolicy != null) {
            Log.d(TAG, "choosePolicy, getPolicyProtoPortTuple");
            return choosedpolicy;
        }

        //Choose ssid by BSS load if MO present
        choosedpolicy = getPolicyBssLoad(policyCollection, moIndexes);
        if (choosedpolicy != null) {
            Log.d(TAG, "choosePolicy, getPolicyBssLoad");
            return choosedpolicy;
        }

        //Choose ssid by HomeSP and Priority
        choosedpolicy = getPolicyDefault(policyCollection);
        Log.d(TAG, "choosePolicy, getPolicyDefault");

        return choosedpolicy;            
    }

    private void handleEventProvisionDone(int result, WifiTree tree) {
        PasspointClient client = null;

        Log.d(TAG, "EventProvisionDone:" + result);
        showToast("EventProvisionDone:" + result);
        if (result == 0) {
            mWifiTree = tree;
            mIsOsuSuccess = true;
        } else {
            updateNotificationProvisionFail();
            mIsOsuSuccess = false;
            //for SIGMA
            mWifiMgr.setHsOsuCompleted(mIsOsuSuccess);
            //Soap used only
            if (PasspointManager.USING_SOAP.equals(mUpdateMethod)) {
                String responseStatus = null;
                
                switch (result){
                    case 1:
                        responseStatus = SoapClient.WIFI_SOAP_UPDATE_RESPONSE_PERMISSION_DENY;
                        break;

                    case 2:
                        responseStatus = SoapClient.WIFI_SOAP_UPDATE_RESPONSE_COMMAND_FAILED;
                        break;

                    case 3:
                        responseStatus = SoapClient.WIFI_SOAP_UPDATE_RESPONSE_MO_ADD_UPDATE_FAIL;
                        break;

                    case 4:
                        responseStatus = SoapClient.WIFI_SOAP_UPDATE_RESPONSE_DEVICE_FULL;
                        break;
                        
                    case 5:
                        responseStatus = SoapClient.WIFI_SOAP_UPDATE_RESPONSE_BAD_MGMTREE_URI;
                        break;

                    case 6:
                        responseStatus = SoapClient.WIFI_SOAP_UPDATE_RESPONSE_REQUEST_ENTITY_TOO_LARGE;
                        break;
                    
                    case 7:
                        responseStatus = SoapClient.WIFI_SOAP_UPDATE_RESPONSE_COMMAND_NOT_ALLOWED;
                        break;
                    
                    case 8:
                        responseStatus = SoapClient.WIFI_SOAP_UPDATE_RESPONSE_COMMAND_NOT_EXEC_DUE_TO_USER;
                        break;
                    
                    case 9:
                        responseStatus = SoapClient.WIFI_SOAP_UPDATE_RESPONSE_NOT_FOUND;
                        break;

                    default:
                        break;
                
                }
                mSoapClient.sendUpdateResponse(mIsOsuSuccess, SoapClient.SUBSCRIPTION_PROVISION, responseStatus);
            }

            if (PasspointManager.USING_DM.equals(mUpdateMethod)) {
                //provision failed, disconnect current network
                disAssociate();
            }
            return;
        }
       
        //Provision MO value

        Log.d(TAG, "handleEventProvisionDone:" + mUpdateMethod);
        if (PasspointManager.USING_SOAP.equals(mUpdateMethod)) {
            client = mSoapClient;                
        } else if (PasspointManager.USING_DM.equals(mUpdateMethod)) {
            client = mDmClient;
        }

        if(mCurrentUsedPolicy.getCredential() != null 
            && mCurrentUsedPolicy.getCredential().getEapMethod().equals("SIM")){ // SIM (replacement)
            //mo has been updated in client            
        }else{//TTLS and TLS (add)
            //mo has been updated in client  
        }

        if (PasspointManager.USING_SOAP.equals(mUpdateMethod)) {
            if(mCurrentUsedPolicy.getCredential() != null && "SIM".equals(mCurrentUsedPolicy.getCredential().getEapMethod())) {
                mSoapClient.sendUpdateResponse(true, SoapClient.SUBSCRIPTION_SIM_PROVISION, SoapClient.WIFI_SOAP_UPDATE_RESPONSE_OK);
            } else {
                mSoapClient.sendUpdateResponse(true, SoapClient.SUBSCRIPTION_PROVISION, SoapClient.WIFI_SOAP_UPDATE_RESPONSE_OK);
            }
        }

        if (PasspointManager.USING_DM.equals(mUpdateMethod)) {
            if(mCurrentUsedPolicy.getCredential() != null && "SIM".equals(mCurrentUsedPolicy.getCredential().getEapMethod())) {
                sendSimProvisionUpdateResponseDone(true);
            } else {                
                sendProvisionUpdateResponseDone(true);
            }
        }         
    }

    private void reAssociate() {
        Log.d(TAG, "reAssociate");
        //Clear all credential
        mCredentialList.clear();
        //Create credential from MO
        createCredentialFromTree(mWifiTree);
        //installPpsMoFromFile();
        //createCredentialFromMO( mWifiSubscriptionMo, mCredentialList, client_ca );

        //Clear all policy
        RemoveCurrentNetwork();
        mCurrentUsedPolicy = null;
        mNetworkPolicy.clear();

        //C.K. modified-->
        mWifiMgr.disconnect();
        
        dumpCredentials();
        //sleep 300 ms  to help re-connect?
        /*try {
            Thread.sleep(300);
        } catch (InterruptedException x) {}*/
        //For SIGMA OSU test
        mWifiMgr.setHsOsuCompleted(mIsOsuSuccess);

    }

    private void disAssociate() {
        Log.d(TAG, "disAssociate");

        //Clear all policy
        RemoveCurrentNetwork();
        mCurrentUsedPolicy = null;

        mWifiMgr.disconnect();
        
        dumpCredentials();
        //sleep 300 ms  to help re-connect?
        /*try {
            Thread.sleep(300);
        } catch (InterruptedException x) {}*/
        //For SIGMA OSU test
        mWifiMgr.setHsOsuCompleted(mIsOsuSuccess);

    }

    private static File getSystemDir() {
        return new File(Environment.getDataDirectory(), "system");
    }

    //For R1 used only
    public void setSpExLis(String exSplist) {
        Log.d(TAG, "setSpExLis:"+ exSplist);
        SystemProperties.set("persist.service.spelist", exSplist);
    }

    public void resetDmTree(){
        Log.d(TAG, "[resetDmTree]");        
        Intent intent = new Intent("com.mediatek.soap.provision.start");
        intent.putExtra("cmd", "deletesubtree");
        intent.putExtra("targeturi", "./Wi-Fi/wi-fi.org/PerProviderSubscription");
        mContext.sendBroadcast(intent);

        Intent intent2 = new Intent("com.mediatek.soap.provision.start");
        intent2.putExtra("cmd", "deletesubtree");
        intent2.putExtra("targeturi", "./Wi-Fi/example.com/PerProviderSubscription");
        mContext.sendBroadcast(intent2);        
    }
    
    public void addCredential(PasspointCredential pc) {
        mCredentialList.add(pc);
    }
    public void delCredential(PasspointCredential pc) {
        mCredentialList.remove(pc);
    }
    public void delCredentialbyindex(int index) {
        mCredentialList.remove(index);
    }
    public void delAllCredential() {
        mCredentialList.clear();
        mPasspointCertificate.cleanClientCertStore();
        //mPasspointCredentialManager.clear();
        mWifiTree = new WifiTree();
        mSoapClient.setWifiTree(mWifiTree);
        mDmClient.setWifiTree(mWifiTree);
        mManagementTreeUri = null;
        if("true".equals(SystemProperties.get("persist.service.dmtreesync","true"))) {
            resetDmTree();        
        }
    }
    public boolean addCredentialbyValue(String type, String username, String passwd, String imsi, String mnc, String mcc, String root_ca, String realm, int preferred, String fqdn, String client_ca) {
        WifiTree.SpFqdn spfqdn = new WifiTree.SpFqdn();
        WifiTree.CredentialInfo credInfo = new WifiTree.CredentialInfo();
        
        if("SIM".equals(type)){
            spfqdn.perProviderSubscription.UpdateIdentifier = "0";
        }

        credInfo.credential.usernamePassword.Username = username;        
        credInfo.credential.usernamePassword.Password = passwd;
        credInfo.credential.sim.IMSI = imsi;
        credInfo.credential.Realm = realm;
        credInfo.credentialPriority = Integer.toString(preferred);
        credInfo.homeSP.FQDN = fqdn;

        mWifiTree.PpsMoId = 0;
        PasspointCredential pc = new PasspointCredential(type, root_ca, client_ca, mcc, mnc, 0,  spfqdn, credInfo);
        
        return mCredentialList.add(pc);
    }
    //call by EM tool
    public String dumpCredential() {//Type:" + credential.getType() 
        String resultString = "";
        int index = 0;
        resultString = resultString+"Total credentials: "+mCredentialList.size()+"\n";
        for (PasspointCredential credential : mCredentialList) {
           resultString = resultString+"#"+String.valueOf(index++)+" ----------\n";
           resultString = resultString+credential+"\n";
           /*WifiSubscriptionMo mo = (WifiSubscriptionMo)mWifiSubscriptionMoList.get((Object)credential.getSubscriptionMoIndex());
           if(mo.getMoItemStr(mo.MO_ITEM_POL_SPELIST_SSID) != null){
              resultString = resultString+" BlackSSID: " + mo.getMoItemStr(mo.MO_ITEM_POL_SPELIST_SSID);
           }*/
           
        }
        return resultString;
    }

    private String getOsuProviderList() {
        //String result = "OSU#1\nssid:[Legacy-OSU]\nbssid:[00:0c:43:44:0d:00]\nosu_friendly_name_1:[eng][57692d466920416c6c69616e6365204f5355]\nosu_friendly_name_2:[kor][57692d466920416c6c69616e6365204f5355]\nosu_uri:[osu-server.R2-testbed.wi-fi.org]\nosu_method:[0][1]]\nicon_1:[128][128][eng][image/png][1357161475_wifi]";
        String result = mWifiMgr.getHsServiceProviderList();
        return result;
    }
    
    private String getStringInBraket(String s) {
        return getStringInBraket(s, 0);
    }
    
    private String getStringInBraket(String s, int index) {
        /*  example, 
            String s = "abc[def]gh[ij]k[lmn]";
            String s0 = getStringInBraket(s,0);
            String s1 = getStringInBraket(s,1);
            String s2 = getStringInBraket(s,2);
            s0 --> def
            s1 --> ij
            s2 --> lmn
        */
        if (s == null) {
            return null;
        }
    
        int pos = 0;
        for (int i = 0; i < index ; i++) {
            pos = s.indexOf("]", pos);
            pos++;
        }
        int start = s.indexOf("[", pos);
        int end = s.indexOf("]", pos);
        String result = null;
    
        if (start > 0 && end > 0 && end > start) {
            result = s.substring(++start, end);
        }
    
        return result;
    }
    
    private void parseOsuProviderList(String content, Collection<PasspointOSUStorage> osuStorageCollection) {
        BufferedReader br = new BufferedReader(new StringReader(content));
        String tempLine;
        PasspointOSUStorage tmpOsuStorage = null;
        int noOfProvider = 0;
        try {
            while ((tempLine = br.readLine()) != null) {
                if(tempLine.startsWith("OSU#")) {
                    if(tmpOsuStorage != null ) {
                        Log.d(TAG, " ");
                        osuStorageCollection.add(tmpOsuStorage);
                        tmpOsuStorage = null;
                        noOfProvider++;
                    }
                    Log.d(TAG, "creating provider#" + noOfProvider);
                    tmpOsuStorage = new PasspointOSUStorage();
    
                } else if(tempLine.startsWith("ssid:")) {
                    String ssid = getStringInBraket(tempLine);
                    tmpOsuStorage.mSSID = ssid;
                    Log.d(TAG, " mSSID:"+tmpOsuStorage.mSSID);

                } else if(tempLine.startsWith("bssid:")) { //Must be prior to "ssid" !!
                    String bssid = getStringInBraket(tempLine);
                    tmpOsuStorage.mBssid = bssid;
                    Log.d(TAG, " mBssid:"+tmpOsuStorage.mBssid);
    
                } else if(tempLine.startsWith("osu_uri:")) {
                    String osu_uri = getStringInBraket(tempLine);
                    tmpOsuStorage.mOsuUrl = osu_uri;
                    Log.d(TAG, " mOsuUrl:"+tmpOsuStorage.mOsuUrl);
    
                } else if(tempLine.startsWith("osu_method:")) {
                    String method;
                    for (int i = 0; (method = getStringInBraket(tempLine, i))!= null; i++) {
                        int value = Integer.parseInt(method);
                        Log.d(TAG, " mOsuMethod:"+value);
                        tmpOsuStorage.mOsuMethod.add(value);
                    }
    
                } else if(tempLine.startsWith("osu_friendly_name_")) {
                    String lang_name;
                    String fri_name_binary;
    
                    lang_name = getStringInBraket(tempLine, 0);
                    fri_name_binary = getStringInBraket(tempLine, 1);
    
                    if (lang_name != null && fri_name_binary != null) {
                        String fri_name = new String( parseStringToBinary(fri_name_binary), "UTF-8");
                        tmpOsuStorage.mFriendlyName.put(lang_name,fri_name);
                        Log.d(TAG, " mFriendlyName:"+lang_name+"," + fri_name);
                    }
    
                } else if(tempLine.startsWith("osu_nai")) {
                    String nai;
    
                    nai = getStringInBraket(tempLine);
    
                    if (nai != null) {
                        String nai_name = new String( parseStringToBinary(nai), "UTF-8");
                        tmpOsuStorage.mNai = nai_name;
                        Log.d(TAG, " mNai:"+nai_name);
                    }
    
                } else if(tempLine.startsWith( "icon_" )) {
                    PasspointOSUIcon osuicon = new PasspointOSUIcon();
    
                    String tmp;
                    tmp = getStringInBraket(tempLine, 0);
                    if (tmp != null) {
                        int width = Integer.parseInt(tmp);
                        osuicon.mWidth = width;
                        Log.d(TAG, " icon width:" + width);
                    }
    
                    tmp = getStringInBraket(tempLine, 1);
                    if (tmp != null) {
                        int height = Integer.parseInt(tmp);
                        osuicon.mHeight = height;
                        Log.d(TAG, " icon height:" + height);
                    }
    
                    osuicon.mLangCode = getStringInBraket(tempLine, 2);
                    Log.d(TAG, " icon lang_name:"+osuicon.mLangCode);
    
                    osuicon.mIconType = getStringInBraket(tempLine, 3);
                    Log.d(TAG, " icon type:"+osuicon.mIconType);
    
                    osuicon.mIconFileName = getStringInBraket(tempLine, 4);
                    Log.d(TAG, " icon name:"+osuicon.mIconFileName);
    
                    tmpOsuStorage.mOsuIcon.add(osuicon);
    
                } else if(tempLine.startsWith("osu_service_description_")) {
                    String lang_name = getStringInBraket(tempLine, 0);
                    String osu_dsc_binary = getStringInBraket(tempLine, 1);
                    String osu_dsc = new String( parseStringToBinary(osu_dsc_binary), "UTF-8");
                    if (osu_dsc != null && lang_name != null) {
                        tmpOsuStorage.mOsuServiceDescription.put(lang_name,osu_dsc);
                        Log.d(TAG, " mOsuServiceDescription:"+lang_name+"," + osu_dsc);
                    }
                }
            }
    
            if(tmpOsuStorage != null ) {
                Log.d(TAG, " ");
                osuStorageCollection.add(tmpOsuStorage);
            }
    
        } catch(Exception ex) {
            ex.printStackTrace();
        } finally {
            try {
                br.close();
            } catch(IOException ex) {
                ex.printStackTrace();
            }
        }
    }
    
    private byte[] parseStringToBinary(String input) {
        String tmpByteString;
        byte tmpByte;
        byte[] bytearray;
        bytearray = new byte[input.length()/2];
        for( int i = 0; i < input.length()/2; i++ ) {
            tmpByteString = input.substring(2*i, 2*(i+ 1) );
            tmpByte = Integer.valueOf( tmpByteString, 16 ).byteValue();
            bytearray[i] = tmpByte;
        }
        return bytearray;
    }

    public List<PasspointServiceProviderInfo> getServiceProviderInfoList() {
        Log.d(TAG, "getOsuProviderList");
        //Query SP list from supplicant
        //Get Name + SSID + ICON + OSU url
        //Send Name + SSID + ICON (OSU url optional?) to UI, UI will have a timer
        String listFromSupplicant = null;
        Locale loc = Locale.getDefault();
        String system_lang = loc.getISO3Language();
        Log.d(TAG, "system_lang:"+system_lang);
        mWifiMgr.setHsLanguageFilter( system_lang );

        try {
            listFromSupplicant = getOsuProviderList();
            //for debug only, too many log
            //Log.d(TAG, "-----OSU list from supplicant.start-----");
            //Log.d(TAG, listFromSupplicant);
            //Log.d(TAG, "-----OSU list from supplicant.end-----");
            mPasspointOSUStorage.clear();
            if( listFromSupplicant != null ) {
                parseOsuProviderList(listFromSupplicant, mPasspointOSUStorage);
            } else { // add default language code 'zxx' icon query
                mWifiMgr.setHsLanguageFilter(OSU_DEFAULT_LANGUAGE);
                listFromSupplicant = getOsuProviderList();
                mPasspointOSUStorage.clear();
                if( listFromSupplicant != null ) {
                    parseOsuProviderList(listFromSupplicant, mPasspointOSUStorage);
                    mIsOsuDefaultLanguage = true;
                }
            }
        } catch(Exception ex) {
            ex.printStackTrace();
        }

        int totalsize = mPasspointOSUStorage.size();
        if (listFromSupplicant == null || totalsize == 0) {
            Log.e(TAG, "getServiceProviderInfoList is empty");
            return null;
        }

        Log.d(TAG, "totalsize:" + totalsize);
        
        ArrayList<PasspointServiceProviderInfo> spInfoList = new ArrayList<PasspointServiceProviderInfo>(totalsize);

        PasspointServiceProviderInfo cancelmsg = new PasspointServiceProviderInfo();
        cancelmsg.mSSID = "Cancel Registration";
        cancelmsg.mFriendlyName = "Cancel Registration";
        spInfoList.add(cancelmsg);

        for( int i = 0; i < totalsize ; i++) {
            PasspointServiceProviderInfo spinfo = new PasspointServiceProviderInfo();
            spinfo.mSSID = mPasspointOSUStorage.get(i).mSSID;
            spinfo.mBssid = mPasspointOSUStorage.get(i).mBssid;
            spinfo.mFriendlyName = mPasspointOSUStorage.get(i).mFriendlyName.get(system_lang);
            if(!mPasspointOSUStorage.get(i).mOsuIcon.isEmpty()) {
                String langCode;
                if(mIsOsuDefaultLanguage == true){
                    langCode = OSU_DEFAULT_LANGUAGE;
                }else{
                    langCode = system_lang;
                }

                for(PasspointOSUIcon osuIcon : mPasspointOSUStorage.get(i).mOsuIcon){
                    if(osuIcon.mLangCode.equals(langCode) || osuIcon.mLangCode.equals(OSU_DEFAULT_LANGUAGE)){
                        spinfo.mIconType = osuIcon.mIconType;
                        spinfo.mIconPath = osuIcon.mIconFileName;
                        mIconFilename = osuIcon.mIconFileName;
                        break;
                    }
                }
                
            } 
            spInfoList.add(spinfo);
        }
        return spInfoList;
    }

    public void setPreferCredential(String credname, String wifispfqdn) {
        Settings.Global.putString(mContext.getContentResolver(), Settings.Global.WIFI_PASSPOINT_PREFER_CREDENTIAL, wifispfqdn + ":" + credname);
        reAssociate();
    }
    
    public List<PasspointCredentialInfo> getCredentialInfoList() {  
        ArrayList<PasspointCredentialInfo> credInfoList = new ArrayList<PasspointCredentialInfo>(mCredentialList.size());

        for (PasspointCredential cred : mCredentialList) {
            int userPreferred = cred.getUserPreference() ? 1 : 0;
            PasspointCredentialInfo credInfo = new PasspointCredentialInfo(userPreferred, cred.getFriendlyName(), cred.getCredName(), cred.getWifiSPFQDN());
            credInfoList.add(credInfo);
        }

        return credInfoList;
    }

    public void notifySubscriptionProvisionStatus(boolean isReady) {
        if (isReady) {
            sendMessage(EVENT_SUBSCRIPTION_PROVISION_READY);                
        } else {
            sendMessage(EVENT_SUBSCRIPTION_PROVISION_NOT_READY);                
        }
    }
    
    private void updateNotificationServiceProvider(boolean isAvailable) {
        int available = isAvailable? 1 : 0 ;
        int numberOfSp = 0;
        if (isAvailable) {
            List<PasspointServiceProviderInfo> spInfoList = getServiceProviderInfoList();
            if (spInfoList != null) {
                numberOfSp = spInfoList.size();
            }
        }
        Message msg = mTarget.obtainMessage(PasspointManager.EVENT_UPDATE_NOTIFICATION_SPINFO, available, numberOfSp);
        msg.sendToTarget();
    }
    
    private void updateNotificationConnected(boolean isConnected, String info) {
        int connected = isConnected? 1 : 0 ;
        int numberOfSp = 0;

        Message msg = mTarget.obtainMessage(PasspointManager.EVENT_UPDATE_NOTIFICATION_CONNEDTED, connected, 0, (String)info);
        msg.sendToTarget();
    }

    private void updateNotificationProvisionFail() {
        Message msg = mTarget.obtainMessage(PasspointManager.EVENT_UPDATE_NOTIFICATION_PROVISION_FAIL);
        msg.sendToTarget();
    }

    class HttpServer {
        private int mServerPort;
    
        HttpServer(int port){
            mServerPort = port;                    
        }       
    
        public void startListener() {
            new Thread(new Runnable() {
                
                public void run() {
                    Log.d(TAG, "[HttpServer] >> enter");
                    if(mServerPort == HS_OSU_LISTEN_PORT){
                        try {
                            if (mServerSocketForOSU == null) {
                                mServerSocketForOSU = new ServerSocket(mServerPort);
                                Log.d(TAG, "[HttpServer] The server is running on " + mServerPort + " mServerSocketForOSU:" + mServerSocketForOSU);
                            } else {
                                Log.d(TAG, "[HttpServer] The server is running already mServerSocketForOSU:" + mServerSocketForOSU);
                                return;
                            }                        
                        } catch (SocketException e) {
                            e.printStackTrace();
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                        
                        try{
                            // Accept incoming connections.
                            Log.d(TAG, "[HttpServer] accepting");
                            Socket clientSocket;
                            clientSocket = mServerSocketForOSU.accept();
                            Log.d(TAG, "[HttpServer] accepted clientSocket:" + clientSocket);
                        
                            handleResponseToClient(clientSocket, mServerPort);
                            clientSocket.close();
                        }catch(Exception ioe){
                            Log.d(TAG, "[HttpServer] Exception encountered on accept. Ignoring. Stack Trace :");
                            ioe.printStackTrace();
                        }                   
                        
                        try{
                            mServerSocketForOSU.close();
                            Log.d(TAG, "[HttpServer] ServerSocket closed");
                        }catch(Exception ioe){
                            Log.d(TAG, "[HttpServer] Problem stopping server socket"); 
                            ioe.printStackTrace();
                        }
                        mServerSocketForOSU = null;
                        Log.d(TAG, "[HttpServer] << exit"); 
                    }else if(mServerPort == HS_REM_LISTEN_PORT){
                        try {
                            if (mServerSocketForREM == null) {
                                mServerSocketForREM = new ServerSocket(mServerPort);
                                Log.d(TAG, "[HttpServer] The server is running on " + mServerPort + " mServerSocketForREM:" + mServerSocketForREM);
                            } else {
                                Log.d(TAG, "[HttpServer] The server is running already mServerSocketForREM:" + mServerSocketForREM);
                                return;
                            }                        
                        } catch (SocketException e) {
                            e.printStackTrace();
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                        
                        try{
                            // Accept incoming connections.
                            Log.d(TAG, "[HttpServer] accepting");
                            Socket clientSocket;
                            clientSocket = mServerSocketForREM.accept();
                            Log.d(TAG, "[HttpServer] accepted clientSocket:" + clientSocket);
                        
                            handleResponseToClient(clientSocket, mServerPort);
                            clientSocket.close();
                        }catch(Exception ioe){
                            Log.d(TAG, "[HttpServer] Exception encountered on accept. Ignoring. Stack Trace :");
                            ioe.printStackTrace();
                        }                   
                        
                        try{
                            mServerSocketForREM.close();
                            Log.d(TAG, "[HttpServer] ServerSocket closed");
                        }catch(Exception ioe){
                            Log.d(TAG, "[HttpServer] Problem stopping server socket"); 
                            ioe.printStackTrace();
                        }
                        mServerSocketForREM = null;
                        Log.d(TAG, "[HttpServer] << exit"); 
                    }
                    
                }//end of run()
            }).start();
        }

        private void handleResponseToClient(Socket cs, int sp) {
            final String HTTP_RESPNOSE = "<html><body><font face=\"Arial\" color=\"#4ea000\" size=\"7\">" + mContext.getResources().getString(R.string.passpoint_osu_finished) + "</font></body></html>";
            String redirectIntent = null;
            BufferedReader in = null;
            PrintWriter out = null;
            String updateMethod = null;
    
            if (HS_OSU_LISTEN_PORT == sp) {
                updateMethod = mUpdateMethod;
            } else if (HS_REM_LISTEN_PORT == sp) {
                updateMethod = mCurrentUsedPolicy.getCredential().getUpdateMethod();
            }
    
            if (PasspointManager.USING_SOAP.equals(updateMethod)) {
                if(HS_OSU_LISTEN_PORT == sp) {
                    redirectIntent = HS20_BROWSER_OSUREDIRECT_MO_REQUEST;
                } else if(HS_REM_LISTEN_PORT == sp){
                    redirectIntent = HS20_BROWSER_REMREDIRECT_MO_REQUEST;
                }
            }
            if (PasspointManager.USING_DM.equals(updateMethod)) {
                if(HS_OSU_LISTEN_PORT == sp) {
                    redirectIntent = HS20_BROWSER_OSUREDIRECT_MO_REQUEST_DM;
                } else if(HS_REM_LISTEN_PORT == sp) {
                    redirectIntent = HS20_BROWSER_REMREDIRECT_MO_REQUEST_DM;
                }
            }
    
            Log.d(TAG, "[HttpServer] Accepted Client Address - " + cs.getInetAddress().getHostName());
            
            try{
                in = new BufferedReader(new InputStreamReader(cs.getInputStream()));
                out = new PrintWriter(new OutputStreamWriter(cs.getOutputStream()));
                
                
                Log.d(TAG, "[HttpServer] Start to send http response to browser");
                out.write("HTTP/1.1 200 OK\r\n");
                out.write("Content-Type: text/html; charset=UTF-8\r\n");
                out.write("Content-Length: " + HTTP_RESPNOSE.length() + "\r\n\r\n");
                out.write(HTTP_RESPNOSE);
                out.flush();
                Log.d(TAG, "[HttpServer] End to send http response to browser");
    
                Intent intent = new Intent(redirectIntent);
                mContext.sendBroadcast(intent);
    
            }catch(Exception e){
                Log.d(TAG, "[HttpServer] write response error");
                e.printStackTrace();
            }finally{
                try{
                    if (in != null) in.close();
                    if (out != null) out.close();
                    Log.d(TAG, "[HttpServer] handleResponseToClient r/w closed");
                }catch(Exception ioe){
                    Log.d(TAG, "[HttpServer] r/w close error");
                    ioe.printStackTrace();
                }
            }
        }
    }

    class ConfigState extends State {
        @Override
        public void enter() {
            Log.d(TAG, "[State]ConfigState enter");

            //readNetwrokPolicy();
            mSmartBlackPolicy.clear();
            mIsNeedForRemediation = false;
            mIsConnectionEvent = false;
            mIsConnectionNoEvent = false;
            mIsEapFailureOverCount = false;
            mIsIgnoreBlackList = false;
            updateNotificationServiceProvider(false);
            updateNotificationConnected(false, null);
            mMcc = null;
            mMnc = null;
            
            Log.d(TAG, "[Systemproperty] persist.service.dmtreesync = " + SystemProperties.get("persist.service.dmtreesync"));
            Log.d(TAG, "[Systemproperty] persist.service.servercertcheck = " + SystemProperties.get("persist.service.servercertcheck"));            
            Log.d(TAG, "[Systemproperty] persist.service.clientcert.pass = " + SystemProperties.get("persist.service.clientcert.pass"));
            Log.d(TAG, "[Systemproperty] persist.service.hs20.cert.dbg = " + SystemProperties.get("persist.service.hs20.cert.dbg"));            
        }

        @Override
        public boolean processMessage(Message message) {
            Log.d(TAG, "[State]ConfigState.processMessage what=" + readableEvent(message.what));
            boolean retValue = true;

            if (FeatureOption.MTK_PASSPOINT_R2_SUPPORT) {
                switch (message.what) {
                case EVENT_WIFI_DISABLED:
                    break;

                case EVENT_PASSPOINT_ON:
                    transitionTo(mDiscoveryState);
                    break;
                    
                case EVENT_WIFI_ENABLED:
                    boolean passpointEnable = Settings.Global.getInt(mContext.getContentResolver(), Settings.Global.WIFI_PASSPOINT_ON, 0) == 1;
                    if (passpointEnable) {
                        mWifiMgr.enableHS(true);
                        transitionTo(mDiscoveryState);                        
                    }
                    break;

                default:
                    Log.d(TAG, "unknow event");
                    retValue = false;
                    break;
                }
            }
            return retValue;
        }
    }

    class DiscoveryState extends State {
        @Override
        public void enter() {
            Log.d(TAG, "[State]DiscoveryState enter");
            dumpPolicies(mNetworkPolicy);
        }

        @Override
        public boolean processMessage(Message message) {
            Log.d(TAG, "[State]DiscoveryState.processMessage what=" + readableEvent(message.what));
            boolean retValue = true;

            switch (message.what) {
            case EVENT_WIFI_DISABLED:
            case EVENT_PASSPOINT_OFF:                                    
                transitionTo(mConfigState);
                break;

            case EVENT_ANQP_DONE:
                handleEventAnqpDone();
                break;

            case EVENT_WIFI_CONNECTED:
                handleEventWifiConnected();
                break;

            case EVENT_REMEDIATION_START:
                mIsNeedForRemediation = true;
                mRemediationUrlFromWNM = (String)message.obj;
                break;
                
            case EVENT_PROVISION_FROM_FILE_DONE:                
                showToast("EVENT_PROVISION_FROM_FILE_DONE");                
                break;

            case EVENT_PRE_PROVISION_DONE:
                int result = message.arg1;
                showToast("EVENT_PRE_PROVISION_DONE: result = " + result);                                    
                if(result == 0){
                    mWifiTree = (WifiTree)message.obj;
                    /*mWifiTree has all credentials, so clear credential list 
                              * to prevent from adding duplicate credential
                              */
                    mCredentialList.clear();                     
                    createCredentialFromTree(mWifiTree);                       
                }
                break;

            case EVENT_SUBSCRIPTION_PROVISION_READY:
                transitionTo(mSubscriptionProvisionState);
                break;
            default:
                Log.d(TAG, "unknow event");
                retValue = false;
                break;
            }
            return retValue;
        }

        private void handleEventAnqpDone() {
            Log.d(TAG, "handleEventAnqpDone");
            dumpCredentials();
            mNetworkPolicy.clear();   
            createPolicyByCredential();
            //changePolicyBySmartBlackList();
        
            if (!mNetworkPolicy.isEmpty()) {
                tryConnectToPasspoint();
            } else {
                //enable all network letting non-HS20 APs have chance to be connected
                mWifiMgr.enableAllNetworkAtOnce();            
                mIsOsuDefaultLanguage = false;
                updateNotificationServiceProvider(true);
            }
        }

        private void handleEventWifiConnected() {
            if ("true".equals(SystemProperties.get("persist.service.manual.ut.osu"))){
                transitionTo(mSubscriptionProvisionState);
            } else if ("true".equals(SystemProperties.get("persist.service.manual.ut.rem"))){
                transitionTo(mAccessState);
            } else if (mCurrentUsedPolicy == null) {
                // sometimes we get wifi connect before get scan result.
                // skip connected event if no policy is used.
                Log.d(TAG, "skip EVENT_WIFI_CONNECTED");

            } else if (mCurrentUsedPolicy.getCredential() == null) {
                Log.d(TAG, "The policy credential is null");
                transitionTo(mSubscriptionProvisionState);
            } else {
                transitionTo(mAccessState);
            }
        }

        private void changePolicyBySmartBlackList() {
            if (!mSmartBlackPolicy.isEmpty()) {
                Log.d(TAG, "checking SmartBlackList:" + mSmartBlackPolicy.size());
                for (PasspointPolicy policy : mSmartBlackPolicy) {
                    PasspointPolicy p = findPolicy(policy);
                    deletePolicy(p);
                }
                if (mNetworkPolicy.isEmpty()) {
                    Log.d(TAG, "all policies are delete by SmartBlackList");
                }
            }
        }
    }

    class SubscriptionProvisionState extends State {

        @Override
        public void enter() {
            Log.d(TAG, "[State]SubscriptionProvisionState enter");
            
            //for SIGMA test
            if (mIsConnectionEvent || mIsConnectionNoEvent) {
                if (mSigmaTestFriendlyName != null) {
                    int index = getIndexInOsuStorage(mSigmaTestFriendlyName);
                    if (index >= 0) {
                        index++;
                        showToast("auto selection in 5 sec");
                        sendMessageDelayed(obtainMessage(EVENT_OSU_USER_SLCT_DONE, "test"), 5000);
                    }
                } else {
                    Log.d(TAG, "mSigmaTestFriendlyName is null");
                }
            }
        }

        @Override
        public void exit() {
            Log.d(TAG, "[State]SubscriptionProvisionState exit");
        }

        @Override
        public boolean processMessage(Message message) {
            Log.d(TAG, "[State]SubscriptionProvisionState.processMessage what=" + readableEvent(message.what));
            boolean retValue = true;
            boolean success = false;
            String reason;
            switch (message.what) {
            case EVENT_WIFI_DISABLED:
            case EVENT_PASSPOINT_OFF:
                transitionTo(mConfigState);
                break;
            case EVENT_DATA_DISCONNECTED:
            case EVENT_WIFI_DISCONNECTED:
                transitionTo(mDiscoveryState);
                break;
            case EVENT_SUBSCRIPTION_PROVISION_NOT_READY:
                transitionTo(mDiscoveryState);
                break;            
            case EVENT_WIFI_CONNECTED:
                Log.d(TAG, "[State]SubscriptionProvisionState enter");
                Log.d(TAG, "mOsuUrlFromWNM [" + mOsuUrlFromWNM + "]");
                updateNotificationServiceProvider(false);
                // for Sigam test
                if (mIsConnectionNoEvent) {
                    updateNotificationServiceProvider(false);
                    showToast("setHsOsuCompleted:true");
                    Log.d(TAG, "setHsOsuCompleted:true");
                    mWifiMgr.setHsOsuCompleted(true);
                    return retValue;
                } else if ("true".equals(SystemProperties.get("persist.service.manual.ut.osu"))){
                    mOsuUrlFromWNM = "http" + SystemProperties.get("persist.service.remurl");
                }
                
                if(!TextUtils.isEmpty(mOsuUrlFromWNM)) {
                    if ("true".equals(SystemProperties.get("persist.service.osuqca"))) { // format v4
                        startHttpServer(HS_OSU_LISTEN_PORT); //listen to redirection after registration
                        startSubscriptionProvision(mOsuUrlFromWNM.toLowerCase(), SoapClient.SUB_REGISTER); //convert to lower case for Qualcomm work around
                    } else {           
                        startHttpServer(HS_OSU_LISTEN_PORT);
                        startSubscriptionProvision(mOsuUrlFromWNM, SoapClient.SUB_REGISTER); //convert to lower case for Qualcomm work around
                    }
                }
                break;                
            case EVENT_REGISTER_CERT_ENROLLMENT_DONE:
                success = (Boolean)message.obj;                
                handleEventRegisterCertEnrollmentDone(success);
                break;
            case EVENT_PROVISION_DONE:
                int result = message.arg1;
                WifiTree tree = (WifiTree)message.obj;
                handleEventProvisionDone(result, tree);
                break;
            case EVENT_PROVISION_UPDATE_RESPONSE_DONE:
                success = (Boolean)message.obj;
                handleEventProvisionUpdateResponseDone(success);
                break;
            case EVENT_OSU_USER_SLCT_DONE:
                //handle user OSU result
                String bssid = (String)message.obj;

                if (bssid != null && !bssid.isEmpty()) {
                    PasspointClient client = null;                        
                    PasspointOSUStorage selectedOsuStorage = null;
                    
                    for (PasspointOSUStorage osuStorage : mPasspointOSUStorage) {
                        if (osuStorage.mBssid.equals(bssid)) {
                            selectedOsuStorage = osuStorage;
                            if (osuStorage.mOsuMethod.get(0) == 0) {
                                mUpdateMethod = PasspointManager.USING_DM;
                                client = mDmClient;
                            } else {
                                mUpdateMethod = PasspointManager.USING_SOAP;
                                client = mSoapClient;
                            }
                        }
                    }                        

                    Log.d(TAG, "EVENT_OSU_USER_SLCT_DONE:" + bssid + " mUpdateMethod:" + mUpdateMethod + " selectedOsuStorage:" + selectedOsuStorage);
                    
                    if (selectedOsuStorage != null) {
                        if("true".equals(SystemProperties.get("persist.service.aruba.osu"))){
                            mOsuUrlFromWNM = "https://passpoint.arubanetworks.com/guest/HotSpot2OnlineSignUp.php";
                            if(mIsOsuDefaultLanguage){
                                client.setOSUFriendlyName(selectedOsuStorage.mFriendlyName.get(OSU_DEFAULT_LANGUAGE));
                                client.setOSUIcon(mIconFilename);                            
                            }else{
                                Locale loc = Locale.getDefault();                                            
                                String system_lang = loc.getISO3Language();
                            
                                client.setOSUFriendlyName(selectedOsuStorage.mFriendlyName.get(system_lang));
                                client.setOSUIcon(mIconFilename);
                            }
                            String ssid = "mtkemp";
                            PasspointPolicy policy = buildPolicy(ssid,1,ssid,null ,null,PasspointPolicy.UNRESTRICTED,false);
                            mCurrentUsedPolicy = policy;
                            ConnectToPasspoint(policy);
                        } else {
                            //FOO sta_icon_req
                            //validate icon per 7.3.2.2
                            //wait for CTRL-EVENT-HS20-ICON-RECV
        
                            //if( !needVerifyIcon )
                            //Connect to selected network
                            //Should use OSUStorage.SSID
                            mOsuUrlFromWNM = selectedOsuStorage.mOsuUrl;
                            mNai = selectedOsuStorage.mNai;
                            if(mOsuUrlFromWNM.startsWith("https") || mOsuUrlFromWNM.startsWith("HTTPS")){
                                mOsuUrlFromWNM =  mOsuUrlFromWNM.replaceFirst(":443", "");
                            }                        
                            
                            if(mIsOsuDefaultLanguage){
                                client.setOSUFriendlyName(selectedOsuStorage.mFriendlyName.get(OSU_DEFAULT_LANGUAGE));
                                client.setOSUIcon(mIconFilename);
                                client.setOSUDefaultLanguage(OSU_DEFAULT_LANGUAGE);
                            }else{
                                Locale loc = Locale.getDefault();                    
                                String system_lang = loc.getISO3Language();
                                client.setOSUFriendlyName(selectedOsuStorage.mFriendlyName.get(system_lang));
                                client.setOSUIcon(mIconFilename);
                                client.setOSUDefaultLanguage(system_lang);                   
                            }
                            String ssid = selectedOsuStorage.mSSID;

                            PasspointPolicy policy = buildPolicy(ssid,1,ssid,null ,null,PasspointPolicy.UNRESTRICTED,false);
                            mCurrentUsedPolicy = policy;
                            ConnectToPasspoint(policy);
                        }
                    }else {
                        transitionTo(mDiscoveryState);
                    }
                } else {
                    transitionTo(mDiscoveryState);
                }
                break;               
            default:
                Log.d(TAG, "unknow event");
                retValue = false;
                break;
            }
            return retValue;
        }

        private void startSubscriptionProvision(String url, String requestReason) {
            PasspointClient client = null;

            Log.d(TAG, "startSubscriptionProvision:" + mUpdateMethod);
            showToast("connecting to Osu server:" + mUpdateMethod);
            if (PasspointManager.USING_SOAP.equals(mUpdateMethod)) {
                client = mSoapClient;            
            } else if (PasspointManager.USING_DM.equals(mUpdateMethod)) {
                client = mDmClient;
            } else {
                Log.d(TAG, "STOP, updateMethod is not mentioned");
                return;
            }

            try {
                URL osuURL = new URL(url);
                String fqdn = osuURL.getHost();
                client.setSpFqdn(fqdn);
                client.init();
                //enable cookie
                CookieManager cookieMan = new CookieManager(null, null);
                CookieHandler.setDefault(cookieMan);  

                //start subscription provision
                client.startSubscriptionProvision(url, requestReason);
            } catch (Exception e) {
                e.printStackTrace();
            }

        }

        private void handleEventRegisterCertEnrollmentDone(boolean success){
            //Soap used only
            PasspointClient client = mSoapClient;
        
            if("true".equals(SystemProperties.get("persist.service.soapenroltest"))){
                Message msg = mTarget.obtainMessage(PasspointManager.EVENT_PROVISION_DONE, "OK");
                msg.sendToTarget();
            }else if(success){
                client.startSubscriptionProvision(mSoapClient.getOSUServerURL(), SoapClient.CERT_ENROLL_SUCCESS);
            }else{
                showToast("Certificate Enrollment Fail !!!");
                client.startSubscriptionProvision(mSoapClient.getOSUServerURL(), SoapClient.CERT_ENROLL_FAIL);
            }
        }

        private void handleEventProvisionUpdateResponseDone(boolean success) {
            if (success) {
                reAssociate();
            } else {
                Log.d(TAG, "EVENT_PROVISION_UPDATE_RESPONSE_DONE with error");                
                showToast("EVENT_PROVISION_UPDATE_RESPONSE_DONE with error");
                disAssociate();
            }
        }

        // for SIGMA test
        private int getIndexInOsuStorage(String friendlyname) {
        
            if (mPasspointOSUStorage == null || friendlyname == null) {
                Log.d(TAG, "getSsidByFriendlyName fail");
                return -1;
            }
        
            int totalsize = mPasspointOSUStorage.size();
            String name;
            for( int i = 0; i < totalsize ; i++) {
                name = mPasspointOSUStorage.get(i).mFriendlyName.get("eng");
                if (friendlyname.equals(name)) {
                    Log.d(TAG, "getSsidByFriendlyName:" + i);
                    return i;
                }
            }
        
            return -1;
        }
    }

    class AccessState extends State {
        @Override
        public void enter() {            
            if (mCurrentUsedPolicy != null) {
                updateNotificationConnected(true, mCurrentUsedPolicy.getCredential().getWifiSPFQDN());
            }

            Log.d(TAG, "[State]AccessState enter");
            if( mIsNeedForRemediation ) {
                
                if(isSubscriptionUpdateRestricted() == false) {
                    startRemediation(mRemediationUrlFromWNM);                        
                }
                mIsNeedForRemediation = false;
            }

            if("true".equals(SystemProperties.get("persist.service.manual.rem"))){
                String rem_url = "";
                if("mup".equals(SystemProperties.get("persist.service.manual.rem.url"))){
                    rem_url = "https://passpoint.arubanetworks.com/guest/HotSpot2SRS.php?type=machine&prov_type=up";
                }else if("uup".equals(SystemProperties.get("persist.service.manual.rem.url"))){
                    rem_url = "https://passpoint.arubanetworks.com/guest/HotSpot2SRS.php?type=user&prov_type=up";
                }else if("mc".equals(SystemProperties.get("persist.service.manual.rem.url"))){
                    rem_url = "https://passpoint.arubanetworks.com/guest/HotSpot2SRS.php?type=machine&prov_type=cert";
                }else if("uc".equals(SystemProperties.get("persist.service.manual.rem.url"))){
                    rem_url = "https://passpoint.arubanetworks.com/guest/HotSpot2SRS.php?type=user&prov_type=cert";
                }else{
                    rem_url = "http" + SystemProperties.get("persist.service.manual.rem.url");
                    if(!SystemProperties.get("persist.service.manual.remurl2").isEmpty()){
                        rem_url += ("&" + SystemProperties.get("persist.service.manual.rem.url2"));
                    }
                }
                sendMessage( obtainMessage(EVENT_REMEDIATION_START, rem_url) );
            }
            updatePolicyProvisionAlarm();
            updateRemediationAlarm();
            
        }
        private boolean isIntervalValided(String interval) {
            if (interval == null) {
                return false;
            } else if (interval.toUpperCase().equals("0XFFFFFFFF") || interval.toUpperCase().equals("4294967295") || interval.isEmpty()) {
                return false;
            }
            return true;
        }

        private void updateRemediationAlarm() {
            if(mCurrentUsedPolicy != null){
                PasspointCredential currentCredential = mCurrentUsedPolicy.getCredential();
                if(currentCredential != null) {
                    String updateInterval = currentCredential.getSubscriptionUpdateInterval();
                    if(isIntervalValided(updateInterval)) {
                        Long remediationUpdateInterval = Long.decode(updateInterval) * 60;
                        Log.d(TAG, "set Remediation every " + remediationUpdateInterval + " seconds");
                        Message msg = mTarget.obtainMessage(PasspointManager.EVENT_SET_REMEDIATION_POLL_TIMER, remediationUpdateInterval);
                        msg.sendToTarget();
                    } else {
                        Log.d(TAG, "set to not to do Remediation forever");
                        Message msg = mTarget.obtainMessage(PasspointManager.EVENT_CANCEL_REMEDIATION_POLL_TIMER);
                        msg.sendToTarget();
                    }
                }
            }

        }

        private void updatePolicyProvisionAlarm() {
            if(mCurrentUsedPolicy != null){
                PasspointCredential currentCredential = mCurrentUsedPolicy.getCredential();
                if(currentCredential != null){
                    String updateInterval = currentCredential.getPolicyUpdateInterval();
                    if(isIntervalValided(updateInterval)) {
                        Long policyUpdateInterval = Long.decode(updateInterval) * 60;
                        Log.d(TAG, "set policy update every " + policyUpdateInterval + " seconds");
                        Message msg = mTarget.obtainMessage(PasspointManager.EVENT_SET_POLICY_POLL_TIMER, policyUpdateInterval);
                        msg.sendToTarget();
                    } else {
                        Log.d(TAG, "set to not to do policy update forever");
                        Message msg = mTarget.obtainMessage(PasspointManager.EVENT_CANCEL_POLICY_POLL_TIMER);
                        msg.sendToTarget();
                    }
                }
            }

        }

        @Override
        public boolean processMessage(Message message) {
            Log.d(TAG, "[State]AccessState.processMessage what=" + readableEvent(message.what));
            boolean retValue = true;
            boolean success = false;
            String reason;
            int result = -1;
            WifiTree tree = null;
            switch (message.what) {
            case EVENT_WIFI_DISABLED:
            case EVENT_PASSPOINT_OFF:
                transitionTo(mConfigState);
                break;
            case EVENT_DATA_DISCONNECTED:
            case EVENT_WIFI_DISCONNECTED:
                transitionTo(mDiscoveryState);
                break;
            case EVENT_ANQP_DONE:
                break;
            case EVENT_REGISTER_CERT_ENROLLMENT_DONE:
                success = (Boolean)message.obj;                
                handleEventRegisterCertReEnrollmentDone(success);
                break;                
            case EVENT_REMEDIATION_START:
                String url = (String)message.obj;
                handleEventRemediationStart(url);
                break;
            case EVENT_REMEDIATION_DONE:
                result = message.arg1;
                tree = (WifiTree)message.obj;
                handleEventRemediationDone(result, tree);                    
                break;
            case EVENT_REMEDIATION_UPDATE_RESPONSE_DONE:
                success = (Boolean)message.obj;
                handleEventRemediationUpdateResponseDone(success);              
                break;
            case EVENT_SIM_PROVISION_DONE:
                result = message.arg1;
                tree = (WifiTree)message.obj;
                handleEventSimProvisionDone(result, tree);
                break;   
            case EVENT_SIM_PROVISION_UPDATE_RESPONSE_DONE:
                success = (Boolean)message.obj;
                handleEventSimProvisionUpdateResponseDone(success);
                break;                
            case EVENT_POLICY_PROVISION_START:
                handleEventPolicyProvisionStart();
                break;
            case EVENT_POLICY_PROVISION_DONE:
                result = message.arg1;
                tree = (WifiTree)message.obj;
                handleEventPolicyProvisionDone(result, tree);
                break;
            case EVENT_POLICY_PROVISION_UPDATE_RESPONSE_DONE:
                success = (Boolean)message.obj;
                handleEventPolicyProvisionUpdateResponseDone(success);
                break;
            case EVENT_OSU_USER_SLCT_DONE:
                deferMessage(message);
                transitionTo(mSubscriptionProvisionState);
                break;
            default:
                Log.d(TAG, "unknow event");
                retValue = false;
                break;
            }

            return retValue;
        }

        private void startPolicyProvision() {
            Log.d(TAG, "startPolicyProvision");            
            PasspointClient client = null;
            PasspointCredential currentCredential = mCurrentUsedPolicy.getCredential();
            String updateMethod = currentCredential.getPolicyUpdateMethod();
            
            if(updateMethod == null || updateMethod.isEmpty()){
                Log.d(TAG, "Policy update method is not set in PPSMO, use mUpdateMethod");
                updateMethod = mUpdateMethod;
            }else{
                Log.d(TAG, "updateMethod in PPSMO is: " + updateMethod);            
            }

            if (PasspointManager.USING_SOAP.equals(updateMethod)) {
                client = mSoapClient;
            } else if (PasspointManager.USING_DM.equals(updateMethod)) {
                client = mDmClient;
            } else {
                Log.d(TAG, "STOP, updateMethod is not mentioned");            
                return;
            }
            
            try{
                String polUrl = currentCredential.getPolicyUpdateURI();
                URL policyUpdateUrl = new URL(polUrl);
                String fqdn = policyUpdateUrl.getHost();               
                
                if( mCurrentUsedPolicy != null ) {
                    Log.d(TAG, "connecting to policy server:" + updateMethod);
                    
                    showToast("connecting to policy server:" + updateMethod);
                    client.setSpFqdn(fqdn);
                    client.setWifiTree(mWifiTree);
                    //enable cookie
                    CookieManager cookieMan = new CookieManager(null, null);
                    CookieHandler.setDefault(cookieMan); 

                    //start policy provision
                    client.startPolicyProvision(polUrl, SoapClient.POL_UPDATE, mCurrentUsedPolicy.getCredential());
                } else {
                    Log.d(TAG, "handleEventPolicyUpdateStart mCurrentUsedPolicy=null");
                }
                
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        private void startRemediation(String url) {
            Log.d(TAG, "startRemediation");

            if( mCurrentUsedPolicy != null && mCurrentUsedPolicy.getCredential() != null ) {
                PasspointCredential currentCredential = mCurrentUsedPolicy.getCredential();
                String updateMethod = currentCredential.getUpdateMethod();
                PasspointClient client = null;
                startHttpServer(HS_REM_LISTEN_PORT);
            
                if(updateMethod == null || updateMethod.isEmpty()){
                    Log.d(TAG, "Subscription update method is not set in PPSMO, use mUpdateMethod");
                    updateMethod = mUpdateMethod;
                }else{
                    Log.d(TAG, "updateMethod in PPSMO is: " + updateMethod);            
                }

                if (PasspointManager.USING_SOAP.equals(updateMethod)) {
                    client = mSoapClient;
                } else if (PasspointManager.USING_DM.equals(updateMethod)) {
                    client = mDmClient;
                } else {
                    Log.d(TAG, "STOP, updateMethod is not mentioned");
                return;
                }
        
                Log.d(TAG, "connecting to Reme server:" + updateMethod);

                showToast("connecting to Reme server:" + updateMethod);
        
                WifiTree.CredentialInfo info = mTreeHelper.getCredentialInfo(mWifiTree, 
                mCurrentUsedPolicy.getCredential().getWifiSPFQDN(), 
                mCurrentUsedPolicy.getCredential().getCredName());
                if (info != null && info.subscriptionUpdate.URI != null) {
                    url = info.subscriptionUpdate.URI;
                }

                try {
                    URL remURL = new URL(url);
                    String fqdn = remURL.getHost();

                    client.setSpFqdn(fqdn);
                    client.setWifiTree(mWifiTree);
                    //enable cookie
                    CookieManager cookieMan = new CookieManager(null, null);
                    CookieHandler.setDefault(cookieMan); 

                    //start remediation
                    if ("SIM".equals(mCurrentUsedPolicy.getCredential().getEapMethod())) {
                        client.startRemediation(url, SoapClient.SUB_PROVISION, mCurrentUsedPolicy.getCredential());
                    } else {             
                        client.startRemediation(url, SoapClient.SUB_REMEDIATION, mCurrentUsedPolicy.getCredential());
                    }
                } catch (Exception e) {
                    Log.d(TAG, "startRemediation fail:" + e);
                }
            } else {
                Log.d(TAG, "mCurrentUsedPolicy is null");
                PasspointClient client = mSoapClient;
                try {
                    URL remURL = new URL(url);
                    String fqdn = remURL.getHost();

                    client.setSpFqdn(fqdn);
                    client.setWifiTree(mWifiTree);

                    client.startRemediation(url, SoapClient.SUB_REMEDIATION, null);   
                } catch (Exception e) {
                    e.printStackTrace();
                }
                
            }
        }

        private void handleEventRegisterCertReEnrollmentDone(boolean success){
            //Soap used only
            PasspointClient client = mSoapClient;
        
            if(success){
                client.startRemediation(mSoapClient.getREMServerURL(), SoapClient.CERT_ENROLL_SUCCESS, null);
            }else{
                Log.d(TAG, "[handleEventRegisterCertReEnrollmentDone] success == false, cert re-enroll fail");
            }
        }

        private void handleEventRemediationStart(String url) {
            if(isSubscriptionUpdateRestricted() == true) {
                Log.d(TAG, "Remediation is restriced");
                return;
            }
            Log.d(TAG, "handleEventRemediationStart:" + url);
            startRemediation(url);
        }

        private void handleEventRemediationDone(int result, WifiTree tree) {
                Log.d(TAG, "handleEventRemediationDone, reason:" + result);
        
                if (result == 0) {
                    mWifiTree = tree;
                    PasspointCredential currentCredential = mCurrentUsedPolicy.getCredential();
                    String updateMethod = currentCredential.getUpdateMethod();

                    // TODO: remove old AAA trust root
                    if (PasspointManager.USING_SOAP.equals(updateMethod)) {
                        mSoapClient.sendUpdateResponse(true, SoapClient.SUBSCRIPTION_REMEDIATION, SoapClient.WIFI_SOAP_UPDATE_RESPONSE_OK);
                    } else if (PasspointManager.USING_DM.equals(updateMethod)) {
                        sendRemediationUpdateResponseDone(true);
                    }
                } else {
                    disAssociate();
                }
                showToast("EventRemediationDone:" + result);
            }

        private void handleEventRemediationUpdateResponseDone(boolean success) {
            if (success) {
                reAssociate();
            } else {
                disAssociate();
            }
        }

        private void handleEventSimProvisionDone(int result, WifiTree tree) {
            handleEventProvisionDone(result, tree);
        }

        private void handleEventSimProvisionUpdateResponseDone(boolean success) {
            if (success) {
                reAssociate();
            } else {
                Log.d(TAG, "EVENT_SIM_PROVISION_UPDATE_RESPONSE_DONE with error");                
                showToast("EVENT_SIM_PROVISION_UPDATE_RESPONSE_DONE with error");
                disAssociate();
            }
        }

        private void handleEventPolicyProvisionStart() {
    
            if(isPolicyUpdateRestricted()) {
                return;
            }

            startPolicyProvision();
        }

        private void handleEventPolicyProvisionDone(int result, WifiTree tree) {
            Log.d(TAG, "handleEventPolicyProvisionDone:" + result);
            
            String FqdnMatchInCred = null;
            String FqdnMatchInTree = null;
            String updateMethod = null;

            PasspointCredential currentCredential = mCurrentUsedPolicy.getCredential();
            updateMethod = currentCredential.getPolicyUpdateMethod();

            if (result == 0) {
                mWifiTree = tree;
            }else{
                if (PasspointManager.USING_SOAP.equals(updateMethod)) {
                    mSoapClient.sendUpdateResponse(false, SoapClient.POLICY_UPDATE, SoapClient.WIFI_SOAP_UPDATE_RESPONSE_MO_ADD_UPDATE_FAIL);
                }

                if (PasspointManager.USING_DM.equals(updateMethod)) {
                    mWifiMgr.setPolicyUpdateResult("0");
                    sendPolicyProvisionUpdateResponseDone(false);
                }  

                showToast("EventPolicyProvisionDone fail");                
                return;
            }


            /** 
             * mWifiTree already updated by SOAP/DM client
             * inspect all PreferredRoamingPartners in mWifiTree and
             * if match current policy's credential, update the priority from Tree to credential
             */             
            Collection<WifiTree.PreferredRoamingPartnerList> list = mTreeHelper.getPreferredRoamingPartnerList(
                mTreeHelper.getCredentialInfo(mWifiTree,
                        mCurrentUsedPolicy.getCredential().getWifiSPFQDN(), 
                        mCurrentUsedPolicy.getCredential().getCredName()));

            if (list == null) {
                Log.d(TAG, "list is null");
            } else {
                for(WifiTree.PreferredRoamingPartnerList itemt : list) {
                    FqdnMatchInTree = itemt.FQDN_Match;

                    if (FqdnMatchInTree == null) {
                        Log.d(TAG, "FqdnMatchInTree is null");
                    } else {
                        for (WifiTree.PreferredRoamingPartnerList itemc : mCurrentUsedPolicy.getCredential().getPrpList()) {
                            Log.d(TAG, "itemc:" + itemc);
                            FqdnMatchInCred = itemc.FQDN_Match;
                            if(FqdnMatchInCred == null){
                                continue;
                            }
                            Log.d(TAG, "Regular compare["+( FqdnMatchInCred.split("(\\s)*,(\\s)*").equals(FqdnMatchInTree.split("(\\s)*,(\\s)*")) )
                                  +"]: prpFqdnMatch:"+FqdnMatchInCred+" FqdnMatch:"+FqdnMatchInTree);
                            Log.d(TAG, "prpFqdnMatch["+ (FqdnMatchInCred.split("(\\s)*,(\\s)*")[0]) +"]");
                            Log.d(TAG, "FqdnMatch["+ (FqdnMatchInTree.split("(\\s)*,(\\s)*")[0]) +"]");
                            //if( prpFqdnMatch.equals(soapFqdnMatch) )
                            if( (FqdnMatchInCred.split("(\\s)*,(\\s)*")[0]).equals(FqdnMatchInTree.split("(\\s)*,(\\s)*")[0]) ) {
                                Log.d(TAG, "priority:"+ itemt.Priority);
                                itemc.Priority = itemt.Priority;
                            }
                        }
                    }
                }
            }

            if (PasspointManager.USING_SOAP.equals(updateMethod)) {
                mSoapClient.sendUpdateResponse(true, SoapClient.POLICY_UPDATE, SoapClient.WIFI_SOAP_UPDATE_RESPONSE_OK);
            }

            if (PasspointManager.USING_DM.equals(updateMethod)) {
                sendPolicyProvisionUpdateResponseDone(true);
            }

            showToast("EventPolicyProvisionDone success");
        }

        private void handleEventPolicyProvisionUpdateResponseDone(boolean success) {
            if(!success){
                Log.d(TAG, "EVENT_POLICY_PROVISION_UPDATE_RESPONSE_DONE with error");
                showToast("EVENT_POLICY_PROVISION_UPDATE_RESPONSE_DONE with error");
                return;
            }
            //No clear all credential because only update PartnerList/Priority
            //No create credential from MO
            //Clear all policy
            mNetworkPolicy.clear();
            
            //Do something like ANQP done
            dumpCredentials();
            createPolicyByCredential();
            if (!mNetworkPolicy.isEmpty()) {
                // TODO: send command: STA_SET_FQDN to supplicant            
                tryConnectToPasspoint();
            }
        }

        private boolean isRestricted( int currentNetwork, String restriction) {
            /*
             * 1."RoamingPartner" then the mobile device can update its PerProviderSubscription MO,
             *    when associated to a roaming partner's HS2.0 compliant hotspot or its Home SP's
             *    HS2.0 compliant hotspot.
             * 2."Unrestricted" then the mobile device can update its PerProviderSubscription MO
             *    when connect to any WLAN connected to the public Internet.
             * 3."HomeSP" then the mobile device can only update its policy when it is connected
             *    to a hotspot operated by its Home SP.
             */
            int restrictionVal = PasspointPolicy.UNRESTRICTED;
            if( restriction.isEmpty() ) {
                Log.d(TAG, "checkRestriction: return false due to restriction empty");
                return false;
            } else if( restriction.toUpperCase().trim().indexOf( new String("HomeSP").toUpperCase().trim() ) != -1 ) {
                restrictionVal = PasspointPolicy.HOME_SP;
            } else if( restriction.toUpperCase().trim().indexOf( new String("RoamingPartner").toUpperCase().trim()  ) != -1 ) {
                restrictionVal = PasspointPolicy.ROAMING_PARTNER;
            } else if( restriction.toUpperCase().trim().indexOf( new String("Unrestricted").toUpperCase().trim()  ) != -1 ) {
                restrictionVal = PasspointPolicy.UNRESTRICTED;
            }
            Log.d(TAG, "checkRestriction: cur[" + currentNetwork + "] <= res[" + restrictionVal + "]");
            if( currentNetwork <= restrictionVal ) {
                // HOME_SP =0, ROAMING_PARTNER =1, UNRESTRICTED =2
                return false;
            } else {
                return true;
            }
        }

        private boolean isSubscriptionUpdateRestricted() {
            if(mCurrentUsedPolicy == null || "true".equals(SystemProperties.get("persist.service.skiprestriction"))) {
                //FOO Debug only!!!
                Log.d(TAG, "isSubscriptionUpdateRestricted: false");
                return false;

            }
            PasspointCredential currentCredential = mCurrentUsedPolicy.getCredential();
            String restriction = currentCredential.getSubscriptionUpdateRestriction();
            if(restriction != null){
                boolean result = isRestricted(mCurrentUsedPolicy.getRestriction(), restriction);
                Log.d(TAG, "isSubscriptionUpdateRestricted:[" + restriction + "]:" + result);
                return result;
            }

            return false;
        }

        private boolean isPolicyUpdateRestricted() {
            if(mCurrentUsedPolicy == null || "true".equals(SystemProperties.get("persist.service.skiprestriction"))) {
                Log.d(TAG, "isPolicyUpdateRestricted: false");
                return false;
            }
            PasspointCredential currentCredential = mCurrentUsedPolicy.getCredential();
            String restriction = currentCredential.getPolicyUpdateRestriction();
            boolean result = isRestricted(mCurrentUsedPolicy.getRestriction(), restriction);
            Log.d(TAG, "isPolicyUpdateRestricted:[" + restriction + "]:" + result);
            return result;
        }

    }

    private void showToast( String content) {
        Toast toast = Toast.makeText(mContext, content, Toast.LENGTH_LONG);
        toast.show();
    }
    
    private String readableEvent(int t) {

        switch (t) {

            case EVENT_PROVISION_START:
                return "EVENT_PROVISION_START";
            case EVENT_PROVISION_DONE:
                return "EVENT_PROVISION_DONE";
            case EVENT_PRE_PROVISION_DONE:
                return "EVENT_PRE_PROVISION_DONE";            
            case EVENT_SIM_PROVISION_DONE:
                return "EVENT_SIM_PROVISION_DONE";
            case EVENT_SIM_PROVISION_UPDATE_RESPONSE_DONE:
                return "EVENT_SIM_PROVISION_UPDATE_RESPONSE_DONE";
            case EVENT_PROVISION_FROM_FILE_DONE:
                return "EVENT_PROVISION_FROM_FILE_DONE";            
            case EVENT_REMEDIATION_START:
                return "EVENT_REMEDIATION_START";
            case EVENT_REMEDIATION_DONE:
                return "EVENT_REMEDIATION_DONE";
            case EVENT_POLICY_PROVISION_START:
                return "EVENT_POLICY_PROVISION_START";
            case EVENT_POLICY_PROVISION_DONE:
                return "EVENT_POLICY_PROVISION_DONE";
            case EVENT_POLICY_CHANGE:
                return "EVENT_POLICY_CHANGE";
            case EVENT_WIFI_CONNECTED:
                return "EVENT_WIFI_CONNECTED";
            case EVENT_WIFI_DISCONNECTED:
                return "EVENT_WIFI_DISCONNECTED";
            case EVENT_DATA_CONNECTED:
                return "EVENT_DATA_CONNECTED";
            case EVENT_DATA_DISCONNECTED:
                return "EVENT_DATA_DISCONNECTED";
            case EVENT_WIFI_ENABLED:
                return "EVENT_WIFI_ENABLED";
            case EVENT_WIFI_DISABLED:
                return "EVENT_WIFI_DISABLED";
            case EVENT_ANQP_DONE:
                return "EVENT_ANQP_DONE";
            case EVENT_OSU_USER_SLCT_DONE:
                return "EVENT_OSU_USER_SLCT_DONE";
            case EVENT_PROVISION_UPDATE_RESPONSE_DONE:
                return "EVENT_PROVISION_UPDATE_RESPONSE_DONE";
            case EVENT_REMEDIATION_UPDATE_RESPONSE_DONE:
                return "EVENT_REMEDIATION_UPDATE_RESPONSE_DONE";
            case EVENT_POLICY_PROVISION_UPDATE_RESPONSE_DONE:
                return "EVENT_POLICY_PROVISION_UPDATE_RESPONSE_DONE";
            case EVENT_REGISTER_CERT_ENROLLMENT_DONE:
                return "EVENT_REGISTER_CERT_ENROLLMENT_DONE";
            case EVENT_PASSPOINT_ON:
                return "EVENT_PASSPOINT_ON";
            case EVENT_PASSPOINT_OFF:
                return "EVENT_PASSPOINT_OFF";
            case EVENT_SUBSCRIPTION_PROVISION_READY:
                return "EVENT_SUBSCRIPTION_PROVISION_READY";
            case EVENT_SUBSCRIPTION_PROVISION_NOT_READY:
                return "EVENT_SUBSCRIPTION_PROVISION_NOT_READY";
            default:
                return "EVENT_INVALID printableEvent";
        }
    }
}


