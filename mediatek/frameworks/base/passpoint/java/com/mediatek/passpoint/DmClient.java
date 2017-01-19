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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;
import android.util.Base64;

import com.mediatek.common.passpoint.*;

import java.io.IOException;
import java.io.File;
import java.util.*;

public class DmClient implements PasspointClient {
    private static final String TAG = "DmClient";
    private Handler mTarget;
    private Context mContext;
    private PasspointCertificate mPasspointCertificate;
    private WifiTree mDmTree;
    private WifiTreeHelper mHelper = new WifiTreeHelper();
    private PasspointCredential mCred;
    private String mSPFQDN;
    private String mOSUFriendlyName;
    private String mOSULanguage;
    private String mIconFileName;
    private String mIconHash;

    public DmClient(Context context, Handler handler) {
        mContext = context;
        mTarget = handler;

        final IntentFilter stateFilter = new IntentFilter();
        stateFilter.addAction("com.mediatek.dm.remediation.end");
        stateFilter.addAction("com.mediatek.dm.policy.end");
        stateFilter.addAction("com.mediatek.dm.provision.end");
        stateFilter.addAction("com.mediatek.dm.certificateenrollment");
        stateFilter.addAction("com.mediatek.dm.certificatereenrollment");
        stateFilter.addAction("com.mediatek.dm.getaaaservertrustroot");
        stateFilter.addAction("com.mediatek.dm.sim.provision.end");
        mContext.registerReceiver(mStateReceiver, stateFilter);
        mPasspointCertificate = PasspointCertificate.getInstance(null);
    }

    public void startSubscriptionProvision(String serverUrl, String requestReason) {
        Log.d(TAG, ">>> startSubscriptionProvisioning(" + serverUrl + ")");
        Intent intent = new Intent("com.mediatek.dm.provision.start");
        intent.putExtra("type", PasspointManager.SUBSCRIPTION_PROVISIONING);
        intent.putExtra("serverurl", serverUrl);
        intent.putExtra("redirecturl", "http://127.0.0.1:54685/index.htm");
        mContext.sendBroadcast(intent);
    }

    public void startRemediation(String serverUrl, String requestReason, PasspointCredential cred) {
        Log.d(TAG, ">>> startRemediation(" + serverUrl + ")");
        if (cred != null) mCred = cred;
        Intent intent = new Intent("com.mediatek.dm.remediation.start");
        String imsi = cred.getImsi();
        String eapType = cred.getEapMethod();

        intent.putExtra("serverurl", serverUrl);
        intent.putExtra("redirecturl", "http://127.0.0.1:54686/index.htm");
        intent.putExtra("spnodename", cred.getWifiSPFQDN());
        intent.putExtra("credentialnodename", cred.getCredName());

        if(!TextUtils.isEmpty(eapType)) {
            if (eapType.equals("TTLS")) {
                intent.putExtra("type", PasspointManager.REMEDIATION_USERNAME_PASSWORD);
            } else if ("TLS".equals(eapType)) {
                Log.d(TAG, "Remediation with certificate");
                intent.putExtra("type", PasspointManager.REMEDIATION_CERTIFICATE);
            } else if (eapType.equals("SIM")) {
                Log.d(TAG, "Remediation with sim");
                intent.putExtra("type", PasspointManager.REMEDIATION_SIM);
                intent.putExtra("imsi", imsi);
            }
        }

        mContext.sendBroadcast(intent);
    }

    public void startPolicyProvision(String serverUrl, String requestReason, PasspointCredential cred) {
        Log.d(TAG, ">>> startPolicyProvision(" + serverUrl + ")");
        if (cred != null) mCred = cred;
        Intent intent = new Intent("com.mediatek.dm.policy.start");

        String eapType = cred.getEapMethod();
        intent.putExtra("serverurl", serverUrl);
        intent.putExtra("spnodename", cred.getWifiSPFQDN());
        intent.putExtra("credentialnodename", cred.getCredName());

        if(!TextUtils.isEmpty(eapType)) {
            if ("TTLS".equals(eapType)) {
                intent.putExtra("type", PasspointManager.POLICY_PROVISIONING_USERNAME_PASSWORD);
            } else if ("TLS".equals(eapType)) {
                Log.d(TAG, "policy update with certificate");
                intent.putExtra("type", PasspointManager.POLICY_PROVISIONING_CERTIFICATE);
            } else if (eapType.equals("SIM")) {
                Log.d(TAG, "policy update with sim");
                intent.putExtra("type", PasspointManager.POLICY_PROVISIONING_SIM);
            }
        }

        mContext.sendBroadcast(intent);
    }

    public void init() {
        /** no need */
    }

    public void setWifiTree(WifiTree tree) {
        mDmTree = tree;
    }

    private BroadcastReceiver mStateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "<<< onReceive:" + action);

            if (action.equals("com.mediatek.dm.provision.end")) {
                int result = intent.getIntExtra("result", -1);
                int response = -1;

                if (result == 0) {
                    Bundle b = intent.getExtras();
                    mDmTree = b.getParcelable("wifitree");
                    WifiTree.SpFqdn sp = mHelper.getSp(mDmTree, mSPFQDN);
                    Log.d(TAG, "subscription provision installing aaa trust root for:" + mSPFQDN);
                    try {
                        Collection<WifiTree.CredentialInfo> credlist = mHelper.getCredentialInfo(sp);

                        for (WifiTree.CredentialInfo cred : credlist) {
                            Collection<WifiTree.AAAServerTrustRoot> aaaList = mHelper.getAaaServerTrustRoot(cred);

                            /** install aaa trust root certificate */
                            for (WifiTree.AAAServerTrustRoot aaa : aaaList) {
                                String aaacerturl = aaa.CertURL;
                                String aaacertsha256fingerprint = aaa.CertSHA256Fingerprint;

                                if (aaacerturl != null && aaacertsha256fingerprint != null) {
                                    /** certificate case */
                                    Log.d(TAG, "Re-getaaaservertrustroot url:" + aaacerturl + ",aaacertsha256fingerprint:" + aaacertsha256fingerprint);
                                    if (mPasspointCertificate.installServerTrustRoot(aaacerturl, aaacertsha256fingerprint, PasspointCertificate.AAA_ROOT, true)) {
                                        Log.d(TAG, "subscription provision install aaa trust root ok");
                                    } else {
                                        Log.d(TAG, "subscription provision install aaa trust root fail ,aaacertsha256fingerprint:" + aaacertsha256fingerprint);
                                        throw new RuntimeException("install aaa server trust root fail");
                                    }
                                }
                            }

                            /** install policyUpdate trust root certificate */
                            if (mPasspointCertificate.installServerTrustRoot(cred.policy.policyUpdate.trustRoot.CertURL,
                                    cred.policy.policyUpdate.trustRoot.CertSHA256Fingerprint,
                            PasspointCertificate.SUBSCRIPTION_ROOT, false)) {
                                throw new RuntimeException("install policy trust root fail");
                            }
                            /** install subscriptionUpdate trust root certificate */
                            if (mPasspointCertificate.installServerTrustRoot(cred.subscriptionUpdate.trustRoot.CertURL,
                                    cred.subscriptionUpdate.trustRoot.CertSHA256Fingerprint,
                            PasspointCertificate.POLICY_ROOT, false)) {
                                throw new RuntimeException("install policy trust root fail");
                            }
                        }
                        response = 0;
                    } catch (Exception e) {
                        Log.d(TAG, "subscription provision fail:" + e);
                    }
                } else {
                    Log.d(TAG, "subscription provision fail from dm");
                }

                Message msg = mTarget.obtainMessage(PasspointManager.EVENT_PROVISION_DONE, response, 0, mDmTree);
                msg.sendToTarget();

            } else if (action.equals("com.mediatek.dm.remediation.end")) {
                int result = intent.getIntExtra("result", -1);

                if (result == 0) {
                    Log.d(TAG, "subscription remediation ok");
                    Bundle b = intent.getExtras();
                    mDmTree = b.getParcelable("wifitree");

                    Message msg = mTarget.obtainMessage(PasspointManager.EVENT_REMEDIATION_DONE, 0, 0 ,mDmTree);
                    msg.sendToTarget();
                } else {
                    Log.d(TAG, "subscription remediation fail from dm");
                }

            } else if (action.equals("com.mediatek.dm.policy.end")) {
                int result = intent.getIntExtra("result", -1);

                if (result == 0) {
                    Log.d(TAG, "policy provision ok");
                    Bundle b = intent.getExtras();
                    mDmTree = b.getParcelable("wifitree");
                } else {
                    Log.d(TAG, "policy provision fail from dm");
                }
                Message msg = mTarget.obtainMessage(PasspointManager.EVENT_POLICY_PROVISION_DONE, result, 0, mDmTree);
                msg.sendToTarget();
            } else if (action.equals("com.mediatek.dm.certificateenrollment")) {//enrollment
                String url = intent.getStringExtra("enrollurl");
                String un = intent.getStringExtra("enrollusername");
                String base64edpw = intent.getStringExtra("enrollpassword");
                String pw = decodeBase64(base64edpw);
                Log.d(TAG, "Enrollment url:" + url + ",un:" + un + ",pw:" + pw);
                startCertificateEnroll(url, PasspointCertificate.ENROLL, un, pw);

            } else if (action.equals("com.mediatek.dm.certificatereenrollment")) { //re-enrollment
                String url = intent.getStringExtra("reenrollurl");
                String un = intent.getStringExtra("reenrollusername");
                String base64edpw = intent.getStringExtra("reenrollpassword");
                String pw = decodeBase64(base64edpw);
                Log.d(TAG, "Re-enrollment url:" + url + ",un:" + un + ",pw:" + pw);
                startCertificateEnroll(url, PasspointCertificate.REENROLL, un, pw);

            } else if (action.equals("com.mediatek.dm.sim.provision.end")) {
                int result = intent.getIntExtra("result", -1);
                if (result == 0) {
                    Log.d(TAG, "sim provision ok");
                    Bundle b = intent.getExtras();
                    mDmTree = b.getParcelable("wifitree");

                    Message msg = mTarget.obtainMessage(PasspointManager.EVENT_SIM_PROVISION_DONE, 0, 0, mDmTree);
                    msg.sendToTarget();
                } else {
                    Log.d(TAG, "sim provision fail");
                }
            }
        };
    };

    private String decodeBase64(String s) {
        try {
            byte[] decoded = Base64.decode(s, Base64.DEFAULT);
            return new String(decoded);
        } catch (Exception e) {
            Log.d(TAG, "decode err:" + e);
        }
        return null;
    }

    /* ------------------
     * get functions
     * ------------------
     */

    public Vector<String> getOtherHomePartner() {
        return null;
    }

    /* ------------------
     * set functions
     * ------------------
     */

    public void setSpFqdn(String fqdn) {
        mSPFQDN = fqdn;
    }

    public void setRemediationHttpDigest() {
        /** no need */
    }

    public void setOSUFriendlyName(String fn) {
        mOSUFriendlyName = fn;
    }

    public void setOSUDefaultLanguage(String lang) {
        mOSULanguage = lang;
    }

    public void setOSUIcon(String iconFilename) {
        try {
            mIconFileName = iconFilename;
            mIconHash = mPasspointCertificate.computeHash(FileOperationUtil.Read(new File("/data/misc/wifi/icon/"+mIconFileName)), "SHA-256");//ext: image/xxx
            Log.d(TAG,"[getOSUIconHash]:"+mIconFileName+":"+mIconHash);
        } catch(Exception e) {
            e.printStackTrace();
        }
    }

    private void startCertificateEnroll(String uri, String operation, String enrollDigestUsername, String enrollDigestPassword) {
        //uri = "https://10.123.103.1:9446/.well-known/est";
        Log.d(TAG, "startCertificateEnroll(" + operation + "):" + uri);
        WifiManager wifiManager = (WifiManager) mContext.getSystemService(Context.WIFI_SERVICE);
        boolean enrolled;
        Message msg = null;
        if (wifiManager != null) {
            WifiInfo wifiInfo = wifiManager.getConnectionInfo();
            if (wifiInfo != null) {
                Log.d(TAG, "wifiInfo MAC = " + wifiInfo.getMacAddress());
                mPasspointCertificate.setMacAddress(wifiInfo.getMacAddress());
            }
            mPasspointCertificate.setImeiOrMeid(getDeviceId());
            if(operation.equals(mPasspointCertificate.ENROLL)) {
                enrolled = mPasspointCertificate.connectESTServer(uri, operation, enrollDigestUsername, enrollDigestPassword, null);
            } else {
                String credentialCertSHA256Fingerprint = mCred.getCertSha256Fingerprint();

                String subjectDN = mPasspointCertificate.getSubjectX500PrincipalFromPKCS12Keystore(credentialCertSHA256Fingerprint);
                enrolled = mPasspointCertificate.connectESTServer(uri, operation, enrollDigestUsername, enrollDigestPassword, subjectDN);
            }
            Log.d(TAG, "Certificate Enrolled :" + enrolled);
        } else {
            Log.d(TAG, "Wifi service not exist, OSU stops");
            return;
        }

        if (operation.equals(PasspointCertificate.ENROLL)) {
            Intent intent = new Intent("com.mediatek.dm.certificateenrollment.done");
            intent.putExtra("result", enrolled? 0:-1);
            mContext.sendBroadcast(intent);
        } else if (operation.equals(PasspointCertificate.REENROLL)) {
            Intent intent = new Intent("com.mediatek.dm.certificatereenrollment.done");
            intent.putExtra("result", enrolled? 0:-1);
            mContext.sendBroadcast(intent);
        }
    }

    private String getDeviceId() {
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
}
