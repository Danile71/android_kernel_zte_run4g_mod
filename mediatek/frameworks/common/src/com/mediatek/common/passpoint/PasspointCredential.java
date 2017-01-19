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
package com.mediatek.common.passpoint;

import android.os.Parcelable;
import android.os.Parcel;
import android.util.Log;

import java.util.*;


public class PasspointCredential implements Parcelable, Comparable<PasspointCredential>  {
    private final static String TAG = "PasspointCredential";
    private String mWifiSPFQDN;
    private String mCredentialName;
    private int mPpsMoId;
    private String mUpdateIdentifier;
    private String mSubscriptionUpdateMethod;
    private String mType;
    private String mInnerMethod;
    private String mCertType;
    private String mCertSha256Fingerprint;
    private String mUsername;
    private String mPasswd;
    private String mImsi;
    private String mMcc;
    private String mMnc;
    private String mCaRootCert;
    private String mRealm;
    private int mPriority; //User preferred priority; The smaller, the higher
    private boolean mUserPreferred = false;
    private String mHomeSpFqdn;
    private String mFriendlyName;
    private String mOtherhomepartnerFqdn;
    private String mClientCert;
    private String mCreationDate;
    private String mExpirationDate;

    private String mSubscriptionDMAccUsername;
    private String mSubscriptionDMAccPassword;
    private String mSubscriptionUpdateInterval;


    private String mPolicyUpdateURI;
    private String mPolicyUpdateInterval;
    private String mPolicyDMAccUsername;
    private String mPolicyDMAccPassword;
    private String mPolicyUpdateRestriction;
    private String mPolicyUpdateMethod;

    /*private Collection<WifiMOPrpList> mPrpList;
    private Collection<WifiMOHomeOIList> mHomeOIList;
    private Collection<WifiMOMinBackhaulThresholdList> mBhtList;
    private Collection<WifiMORequiredProtoPortTupleList> mRpptList;*/

    private Collection<WifiTree.PreferredRoamingPartnerList> mPreferredRoamingPartnerList;
    private Collection<WifiTree.HomeOIList> mHomeOIList;
    private Collection<WifiTree.MinBackhaulThresholdNetwork> mMinBackhaulThresholdNetwork;
    private Collection<WifiTree.RequiredProtoPortTuple> mRequiredProtoPortTuple;
    private Collection<WifiTree.SPExclusionList> mSpExclusionList;
    private String mMaxBssLoad;

    private boolean mIsMachineRemediation;

    private String mAAACertURL;
    private String mAAASha256Fingerprint;
    
    private String mSubscriptionUpdateRestriction;
    private String mSubscriptionUpdateURI;

    private int moIndex;
    private boolean mCheckAaaServerCertStatus;    
    
    public PasspointCredential() {

    }

    public PasspointCredential(String type,                               
                               String caroot,
                               String clientcert,
                               int ppsmoId,                               
                               WifiTree.SpFqdn sp,
                               WifiTree.CredentialInfo credinfo) {

        if (credinfo == null) {
            return;
        }

        mType = type;
        mCaRootCert = caroot;
        mClientCert = clientcert;

        mWifiSPFQDN = sp.nodeName;
        mUpdateIdentifier = sp.perProviderSubscription.UpdateIdentifier;

        mCredentialName = credinfo.nodeName;
        Set set = credinfo.homeSP.otherHomePartners.entrySet();
        Iterator i = set.iterator();
        if(i.hasNext()) {
            Map.Entry entry3 = (Map.Entry)i.next();
            WifiTree.OtherHomePartners ohp = (WifiTree.OtherHomePartners)entry3.getValue();
            mOtherhomepartnerFqdn = ohp.FQDN;
        }

        set = credinfo.aAAServerTrustRoot.entrySet();
        i = set.iterator();
        if(i.hasNext()) {
            Map.Entry entry3 = (Map.Entry)i.next();
            WifiTree.AAAServerTrustRoot aaa = (WifiTree.AAAServerTrustRoot)entry3.getValue();            
            mAAACertURL = aaa.CertURL;
            mAAASha256Fingerprint = aaa.CertSHA256Fingerprint;
        }            

        mCertType =                 credinfo.credential.digitalCertificate.CertificateType;
        mCertSha256Fingerprint =    credinfo.credential.digitalCertificate.CertSHA256Fingerprint;
        mUsername =                 credinfo.credential.usernamePassword.Username;
        mPasswd =                   credinfo.credential.usernamePassword.Password;
        mIsMachineRemediation =     credinfo.credential.usernamePassword.MachineManaged;
        mInnerMethod =              credinfo.credential.usernamePassword.eAPMethod.InnerMethod;
        mImsi =                     credinfo.credential.sim.IMSI;
        mCreationDate =             credinfo.credential.CreationDate;
        mExpirationDate =           credinfo.credential.ExpirationDate;
        mRealm =                    credinfo.credential.Realm;
        
        if(credinfo.credentialPriority == null){
            credinfo.credentialPriority = "128";                    
        }
        mPriority =                 Integer.parseInt(credinfo.credentialPriority);
        
        mHomeSpFqdn =               credinfo.homeSP.FQDN;

        mSubscriptionUpdateInterval = credinfo.subscriptionUpdate.UpdateInterval;
        mSubscriptionUpdateMethod =     credinfo.subscriptionUpdate.UpdateMethod;
        mSubscriptionUpdateRestriction = credinfo.subscriptionUpdate.Restriction;
        mSubscriptionUpdateURI =        credinfo.subscriptionUpdate.URI;
        mSubscriptionDMAccUsername =    credinfo.subscriptionUpdate.usernamePassword.Username;
        mSubscriptionDMAccPassword =    credinfo.subscriptionUpdate.usernamePassword.Password;

        mPolicyUpdateURI =          credinfo.policy.policyUpdate.URI;
        mPolicyUpdateInterval =     credinfo.policy.policyUpdate.UpdateInterval;
        mPolicyDMAccUsername =      credinfo.policy.policyUpdate.usernamePassword.Username;
        mPolicyDMAccPassword =      credinfo.policy.policyUpdate.usernamePassword.Password;
        mPolicyUpdateRestriction =  credinfo.policy.policyUpdate.Restriction;
        mPolicyUpdateMethod =       credinfo.policy.policyUpdate.UpdateMethod;
        mPreferredRoamingPartnerList = credinfo.policy.preferredRoamingPartnerList.values();
        mMinBackhaulThresholdNetwork = credinfo.policy.minBackhaulThreshold.values();
        mRequiredProtoPortTuple =   credinfo.policy.requiredProtoPortTuple.values();
        mMaxBssLoad =               credinfo.policy.maximumBSSLoadValue;
        mSpExclusionList =          credinfo.policy.sPExclusionList.values();

        mHomeOIList =               credinfo.homeSP.homeOIList.values();
        mFriendlyName =             credinfo.homeSP.FriendlyName;
        mCheckAaaServerCertStatus = credinfo.credential.CheckAAAServerCertStatus;
        mPpsMoId = ppsmoId;
    }

    public PasspointCredential(String type,                               
                               String caroot,
                               String clientcert,
                               String mcc,
                               String mnc,
                               int ppsmoId,                               
                               WifiTree.SpFqdn sp,
                               WifiTree.CredentialInfo credinfo) {

        if (credinfo == null) {
            return;
        }

        mType = type;
        mCaRootCert = caroot;
        mClientCert = clientcert;
        
        mWifiSPFQDN = sp.nodeName;
        mUpdateIdentifier = sp.perProviderSubscription.UpdateIdentifier;

        mCredentialName = credinfo.nodeName;        
        Set set = credinfo.homeSP.otherHomePartners.entrySet();
        Iterator i = set.iterator();
        if(i.hasNext()) {
            Map.Entry entry3 = (Map.Entry)i.next();
            WifiTree.OtherHomePartners ohp = (WifiTree.OtherHomePartners)entry3.getValue();
            mOtherhomepartnerFqdn = ohp.FQDN;
        }

        set = credinfo.aAAServerTrustRoot.entrySet();
        i = set.iterator();
        if(i.hasNext()) {
            Map.Entry entry3 = (Map.Entry)i.next();
            WifiTree.AAAServerTrustRoot aaa = (WifiTree.AAAServerTrustRoot)entry3.getValue();            
            mAAACertURL = aaa.CertURL;
            mAAASha256Fingerprint = aaa.CertSHA256Fingerprint;
        }            

        mCertType =                 credinfo.credential.digitalCertificate.CertificateType;
        mCertSha256Fingerprint =    credinfo.credential.digitalCertificate.CertSHA256Fingerprint;
        mUsername =                 credinfo.credential.usernamePassword.Username;
        mPasswd =                   credinfo.credential.usernamePassword.Password;
        mIsMachineRemediation =     credinfo.credential.usernamePassword.MachineManaged;
        mInnerMethod =              credinfo.credential.usernamePassword.eAPMethod.InnerMethod;
        mImsi =                     credinfo.credential.sim.IMSI;
        mMcc =                      mcc;
        mMnc =                      mnc;
        mCreationDate =             credinfo.credential.CreationDate;
        mExpirationDate =           credinfo.credential.ExpirationDate;
        mRealm =                    credinfo.credential.Realm;
        
        if(credinfo.credentialPriority == null){
            credinfo.credentialPriority = "128";                    
        }        
        mPriority =                 Integer.parseInt(credinfo.credentialPriority);
        
        mHomeSpFqdn =               credinfo.homeSP.FQDN;

        mSubscriptionUpdateMethod =     credinfo.subscriptionUpdate.UpdateMethod;
        mSubscriptionUpdateRestriction = credinfo.subscriptionUpdate.Restriction;
        mSubscriptionUpdateURI =        credinfo.subscriptionUpdate.URI;
        mSubscriptionDMAccUsername =    credinfo.subscriptionUpdate.usernamePassword.Username;
        mSubscriptionDMAccPassword =    credinfo.subscriptionUpdate.usernamePassword.Password;

        mPolicyUpdateURI =          credinfo.policy.policyUpdate.URI;
        mPolicyUpdateInterval =     credinfo.policy.policyUpdate.UpdateInterval;
        mPolicyDMAccUsername =      credinfo.policy.policyUpdate.usernamePassword.Username;
        mPolicyDMAccPassword =      credinfo.policy.policyUpdate.usernamePassword.Password;
        mPolicyUpdateRestriction =  credinfo.policy.policyUpdate.Restriction;
        mPolicyUpdateMethod =       credinfo.policy.policyUpdate.UpdateMethod;        
        mPreferredRoamingPartnerList = credinfo.policy.preferredRoamingPartnerList.values();
        mMinBackhaulThresholdNetwork = credinfo.policy.minBackhaulThreshold.values();
        mRequiredProtoPortTuple =   credinfo.policy.requiredProtoPortTuple.values();
        mMaxBssLoad =               credinfo.policy.maximumBSSLoadValue;
        mSpExclusionList =          credinfo.policy.sPExclusionList.values();

        mHomeOIList =               credinfo.homeSP.homeOIList.values();
        mFriendlyName =             credinfo.homeSP.FriendlyName;

        mPpsMoId = ppsmoId;
    }

    public int getSubscriptionMoIndex(){
        return moIndex;
    }
    
    public String getUpdateIdentifier() {
        return mUpdateIdentifier;
    }

    public String getUpdateMethod(){
        return mSubscriptionUpdateMethod;
    }

    public void setUpdateMethod(String method){
        mSubscriptionUpdateMethod = method;
    }
        
    public String getWifiSPFQDN() {
        return mWifiSPFQDN;
    }

    public String getCredName() {
        return mCredentialName;
    }
    
    public String getEapMethod() {
        return mType;
    }

    public String getCertType(){
        return mCertType;
    }

    public String getCertSha256Fingerprint(){
        return mCertSha256Fingerprint;
    }
    
    public String getUserName() {
        return mUsername;
    }

    public String getPassword() {
        return mPasswd;
    }

    public String getImsi() {
        return mImsi;
    }

    public String getMcc() {
        return mMcc;
    }

    public String getMnc() {
        return mMnc;
    }

    public String getCaRootCert() {
        return mCaRootCert;
    }

    public String getClientCertPath() {
        return mClientCert;
    }

    public String getRealm() {
        return mRealm;
    }

    public int getPriority() {
        if (mUserPreferred) {
            return 0;
        }
        
        return mPriority;
    }

    public String getFqdn() {
        return mHomeSpFqdn;
    }

    public String getOtherhomepartners(){
        return mOtherhomepartnerFqdn;
    }
    
    public String getSubscriptionDMAccUsername(){
        return mSubscriptionDMAccUsername;
    }

    public String getSubscriptionDMAccPassword(){
        return mSubscriptionDMAccPassword;
    }

    public String getPolicyUpdateURI(){
        return mPolicyUpdateURI;
    }

    public String getPolicyUpdateInterval(){
        return mPolicyUpdateInterval;
    }

    public String getPolicyDMAccUsername(){
        return mPolicyDMAccUsername;
    }

    public String getPolicyDMAccPassword(){
        return mPolicyDMAccPassword;
    }

    public String getPolicyUpdateRestriction(){
        return mPolicyUpdateRestriction;
    }

    public String getPolicyUpdateMethod(){
        return mPolicyUpdateMethod;
    }

    public String getCreationDate() {
        return mCreationDate;
    }

    public String getExpirationDate() {
        return mExpirationDate;
    }

    public void setExpirationDate( String expirationdate) {
        mExpirationDate = expirationdate;
    }

    /*public Collection<WifiMOPrpList> getPrpList(){
        return mPrpList;
    }

    public Collection<WifiMOHomeOIList> getHomeOIList(){
        return mHomeOIList;
    }        

    public Collection<WifiMOMinBackhaulThresholdList> getBackhaulThresholdList(){
        return mBhtList;
    }
    
    public Collection<WifiMORequiredProtoPortTupleList> getRequiredProtoPortList(){
        return mRpptList;
    }*/

    public Collection<WifiTree.PreferredRoamingPartnerList> getPrpList(){
        return mPreferredRoamingPartnerList;
    }

    public Collection<WifiTree.HomeOIList> getHomeOIList(){
        return mHomeOIList;
    }        

    public Collection<WifiTree.MinBackhaulThresholdNetwork> getBackhaulThresholdList(){
        return mMinBackhaulThresholdNetwork;
    }
    
    public Collection<WifiTree.RequiredProtoPortTuple> getRequiredProtoPortList(){
        return mRequiredProtoPortTuple;
    }

    public Collection<WifiTree.SPExclusionList> getSPExclusionList(){
        return mSpExclusionList;
    }        
    
    public boolean getIsMachineRemediation(){
        return mIsMachineRemediation;
    }

    public String getAAACertURL(){
        return mAAACertURL;
    }

    public String getAAASha256Fingerprint(){
        return mAAASha256Fingerprint;
    }

    public String getSubscriptionUpdateRestriction(){
        return mSubscriptionUpdateRestriction;
    }

    public String getSubscriptionUpdateURI(){
        return mSubscriptionUpdateURI;
    }

    public String getSubscriptionUpdateInterval() {
        return mSubscriptionUpdateInterval;
    }

    public String getFriendlyName(){
        return mFriendlyName;
    }

    public String getMaxBssLoad(){
        return mMaxBssLoad;
    }

    public boolean getUserPreference() {
        return mUserPreferred;
    }

    public boolean getCheckAaaServerCertStatus() {
        return mCheckAaaServerCertStatus;
    }

    public void setUserPreference(boolean value) {
        mUserPreferred = value;
    }

    @Override
       public boolean equals(Object obj) {
        boolean result = false;
           if (obj instanceof PasspointCredential) {
               final PasspointCredential other = (PasspointCredential) obj;
            if (this.mType.equals(other.mType)) {
                if (this.mType.equals("TTLS")) {
                    result = this.mUsername.equals(other.mUsername) &&
                             this.mPasswd.equals(other.mPasswd) &&
                             this.mRealm.equals(other.mRealm) &&
                             this.mHomeSpFqdn.equals(other.mHomeSpFqdn);
                }
                if (this.mType.equals("TLS")) {
                    result = this.mRealm.equals(other.mRealm) &&
                             this.mHomeSpFqdn.equals(other.mHomeSpFqdn);
           }
                if (this.mType.equals("SIM")) {
                    result = this.mMcc.equals(other.mMcc) &&
                             this.mMnc.equals(other.mMnc) &&
                             this.mImsi.equals(other.mImsi) &&
                             this.mHomeSpFqdn.equals(other.mHomeSpFqdn);
                }
            }
        }
        return result;
       }

    @Override
    public String toString() {
        StringBuffer sb = new StringBuffer();
        String none = "<none>";

        sb.append("ppsMO# ").append(mPpsMoId).
            //append(", WiFiSPFQDN: ").append(mWifiSPFQDN== null ? none : mWifiSPFQDN).            
            append(", UpdateIdentifier: ").append(mUpdateIdentifier == null ? none : mUpdateIdentifier).
            append(", SubscriptionUpdateMethod: ").append(mSubscriptionUpdateMethod== null ? none : mSubscriptionUpdateMethod).            
            append(", Type: ").append(mType == null ? none : mType).
            append(", Username: ").append(mUsername == null ? none : mUsername).
            append(", Passwd: ").append(mPasswd == null ? none : mPasswd).
            append(", SubDMAccUsername: ").append(mSubscriptionDMAccUsername == null ? none : mSubscriptionDMAccUsername).
            append(", SubDMAccPassword: ").append(mSubscriptionDMAccPassword == null ? none : mSubscriptionDMAccPassword).
            append(", PolDMAccUsername: ").append(mPolicyDMAccUsername == null ? none : mPolicyDMAccUsername).
            append(", PolDMAccPassword: ").append(mPolicyDMAccPassword == null ? none : mPolicyDMAccPassword).
            append(", Imsi: ").append(mImsi == null ? none : mImsi).
            append(", Mcc: ").append(mMcc == null ? none : mMcc).
            append(", Mnc: ").append(mMnc == null ? none : mMnc).
            append(", CaRootCert: ").append(mCaRootCert == null ? none : mCaRootCert).
			append(", Realm: ").append(mRealm == null ? none : mRealm).
			append(", Priority: ").append(mPriority).
			append(", Fqdn: ").append(mHomeSpFqdn == null ? none : mHomeSpFqdn).
			append(", Otherhomepartners: ").append(mOtherhomepartnerFqdn == null ? none : mOtherhomepartnerFqdn).
			append(", ExpirationDate: ").append(mExpirationDate == null ? none : mExpirationDate).			
            append(", MaxBssLoad: ").append(mMaxBssLoad == null ? none : mMaxBssLoad).
            append(", SPExclusionList: ").append(mSpExclusionList);

        if(mPreferredRoamingPartnerList != null){
            sb.append("PreferredRoamingPartnerList:");
            for(WifiTree.PreferredRoamingPartnerList prpListItem : mPreferredRoamingPartnerList){
                sb.append("[fqdnmatch:").append(prpListItem.FQDN_Match).
                    append(", priority:").append(prpListItem.Priority).
                    append(", country:").append(prpListItem.Country).append("]");
            }
        }

        if(mHomeOIList != null){
            sb.append("HomeOIList:");
            for(WifiTree.HomeOIList HomeOIListItem : mHomeOIList){
                sb.append("[HomeOI:").append(HomeOIListItem.HomeOI).
                    append(", HomeOIRequired:").append(HomeOIListItem.HomeOIRequired).
                    append("]");
            }
        }

        if(mMinBackhaulThresholdNetwork != null){
            sb.append("BackHaulThreshold:");
            for(WifiTree.MinBackhaulThresholdNetwork BhtListItem : mMinBackhaulThresholdNetwork){
                sb.append("[networkType:").append(BhtListItem.NetworkType).
                    append(", dlBandwidth:").append(BhtListItem.DLBandwidth).
                    append(", ulBandwidth:").append(BhtListItem.ULBandwidth).
                    append("]");
            }
        }

        if(mRequiredProtoPortTuple != null){
            sb.append("WifiMORequiredProtoPortTupleList:");
            for(WifiTree.RequiredProtoPortTuple RpptListItem : mRequiredProtoPortTuple){
                sb.append("[IPProtocol:").append(RpptListItem.IPProtocol).
                    append(", PortNumber:").append(RpptListItem.PortNumber).
                    append("]");
            }
        }

        return sb.toString();
    }

    /** Implement the Parcelable interface {@hide} */
    public int describeContents() {
        return 0;
    }

    /** Implement the Parcelable interface {@hide} */
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(mType);
        dest.writeString(mUsername);
        dest.writeString(mPasswd);
        dest.writeString(mImsi);
        dest.writeString(mMcc);
        dest.writeString(mMnc);
        dest.writeString(mCaRootCert);
        dest.writeString(mRealm);
        dest.writeInt(mPriority);
        dest.writeString(mHomeSpFqdn);
        dest.writeString(mOtherhomepartnerFqdn);
        dest.writeString(mClientCert);
        dest.writeString(mExpirationDate);
    }

    /** Implement the Parcelable interface {@hide} */
    public static final Creator<PasspointCredential> CREATOR =
        new Creator<PasspointCredential>() {
            public PasspointCredential createFromParcel(Parcel in) {
                PasspointCredential pc = new PasspointCredential();
                pc.mType = in.readString();
                pc.mUsername = in.readString();
                pc.mPasswd = in.readString();
                pc.mImsi = in.readString();
                pc.mMcc = in.readString();
                pc.mMnc = in.readString();
                pc.mCaRootCert = in.readString();
                pc.mRealm = in.readString();
				pc.mPriority = in.readInt();
				pc.mHomeSpFqdn = in.readString();
				pc.mOtherhomepartnerFqdn = in.readString();
                pc.mClientCert = in.readString();
                pc.mExpirationDate = in.readString();
                return pc;
            }

            public PasspointCredential[] newArray(int size) {
                return new PasspointCredential[size];
            }
        };

    public int compareTo(PasspointCredential another) {

        //The smaller the higher
        if( mPriority < another.mPriority ) {
            return -1;
        } else if( mPriority == another.mPriority ) {
            Log.d(TAG, "this.mType.compareTo(another.mType):" +this.mType.compareTo(another.mType));
            return this.mType.compareTo(another.mType);
        } else {
            return 1;
        }
    }
    @Override
    public int hashCode() {
        int hash = 208;
        if (mType != null) {
            hash += mType.hashCode();
        }
        if (mRealm != null) {
            hash += mRealm.hashCode();
        }
        if (mHomeSpFqdn != null) {
            hash += mHomeSpFqdn.hashCode();
        }
        if (mUsername != null) {
            hash += mUsername.hashCode();
        }
        if (mPasswd != null) {
            hash += mPasswd.hashCode();
        }

        return hash;
    }
}

