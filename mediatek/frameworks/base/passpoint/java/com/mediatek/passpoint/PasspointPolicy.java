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

import android.os.Parcelable;
import android.os.Parcel;
import com.mediatek.common.passpoint.PasspointCredential;

import android.util.Log;

import com.android.internal.util.Objects;

public class PasspointPolicy implements Parcelable, Comparable<PasspointPolicy> {
    public static final long NONE = -1;
    private final static String TAG = "PasspointPolicy";
    public static final int HOME_SP = 0;
    public static final int ROAMING_PARTNER = 1;
    public static final int UNRESTRICTED = 2;


    private String mName;
    private int mSubscriptionPriority;
    private int mRoamingPriority;
    private String mBssid;
    private String mSsid;
    private PasspointCredential mCredential;
    private int mRestriction;// Permitted values are "HomeSP", "RoamingPartner", or "Unrestricted"
    private boolean mIsHomeSp;//(HOME_SP & mIsHomeSp ==true) meas Home_SP, (HOME_SP & mIsHomeSp == false) means OtherHomePartner

    public PasspointPolicy(String name, int priority, String ssid,
                           String bssid, PasspointCredential pc,
                           int restriction, boolean ishomesp) {
        mName = name;
        mSubscriptionPriority = priority;
        //PerProviderSubscription/<X+>/Policy/PreferredRoamingPartnerList/<X+>/Priority
        mRoamingPriority = 128; //default priority value of 128
        mSsid = ssid;
        mCredential = pc;
        mBssid = bssid;
        mRestriction = restriction;
        mIsHomeSp = ishomesp;
    }

    public PasspointPolicy(Parcel in) {
        mName = in.readString();
        mSubscriptionPriority = in.readInt();
        mRoamingPriority = in.readInt();
        mSsid = in.readString();
        mCredential = in.readParcelable(null);
        mRestriction =in.readInt();
        mIsHomeSp = in.readInt() == 1 ? true:false;
    }

    public String getSsid() {
        return mSsid;
    }

    public void setBssid(String bssid) {
        mBssid = bssid;
    }

    public String getBssid() {
        return mBssid;
    }

    public void setRestriction(int r) {
        mRestriction = r;
    }

    public int getRestriction() {
        return mRestriction;
    }

    public void setHomeSp(boolean b) {
        mIsHomeSp = b;
    }

    public boolean getHomeSp() {
        return mIsHomeSp;
    }

    public void setCredential(PasspointCredential newCredential) {
        mCredential = newCredential;
    }

    public PasspointCredential getCredential() {
        return mCredential;
    }

    public void setSubscriptionPriority( int priority) {
        mSubscriptionPriority = priority;
    }

    public void setRoamingPriority( int priority) {
        mRoamingPriority = priority;
    }

    public int getSubscriptionPriority() {
        return mSubscriptionPriority;
    }

    public int getRoamingPriority() {
        return mRoamingPriority;
    }

    /** {@inheritDoc} */
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(mName);
        dest.writeInt(mSubscriptionPriority);
        dest.writeInt(mRoamingPriority);
        dest.writeString(mSsid);
        dest.writeParcelable(mCredential, flags);
        dest.writeInt(mRestriction);
        dest.writeInt((mIsHomeSp == true ? 1:0));
    }

    /** {@inheritDoc} */
    public int compareTo(PasspointPolicy another) {
        Log.d(TAG, "this:" + this);
        Log.d(TAG, "another:" + another);

        if (another == null) {
            return -1;
        } else if(this.mIsHomeSp == true && another.getHomeSp() == false){
            //home sp priority is higher then roaming
            Log.d(TAG, "compare HomeSP  first, this is HomeSP, another isn't");             
            return -1;
        } else if((this.mIsHomeSp == true && another.getHomeSp() == true)){
            Log.d(TAG, "both HomeSP");
            //if both home sp, compare subscription priority
            if(this.mSubscriptionPriority < another.getSubscriptionPriority()) {
                Log.d(TAG, "this priority is higher");        
                return -1;
            } else if(this.mSubscriptionPriority == another.getSubscriptionPriority()){
                Log.d(TAG, "both priorities equal");     
                //if priority still the same, compare name(ssid)
                if (this.mName.compareTo(another.mName) != 0) {
                    Log.d(TAG, "compare mName return:" + this.mName.compareTo(another.mName));
                    return this.mName.compareTo(another.mName);
                }
                /**
                 *if name still the same, compare credential
                 *the device may has two more credentials(TLS,SIM..etc)
                 *it can associate to one AP(same ssid). so we should compare by credential
                 */
                if (this.mCredential != null && another.mCredential != null) {
                    if (this.mCredential.compareTo(another.mCredential) != 0) {
                        Log.d(TAG, "compare mCredential return:" + this.mName.compareTo(another.mName));
                        return this.mCredential.compareTo(another.mCredential);
                    }
                }
            } else {
                return 1;
            }
        } else if ((this.mIsHomeSp == false && another.getHomeSp() == false)) {
            Log.d(TAG, "both RoamingSp");
            //if both roaming sp, compare roaming priority(preferredRoamingPartnerList/<X+>/priority)
            if(this.mRoamingPriority < another.getRoamingPriority()) {
                Log.d(TAG, "this priority is higher");        
                return -1;
            } else if(this.mRoamingPriority == another.getRoamingPriority()){//priority equals, compare name
                Log.d(TAG, "both priorities equal");
                //if priority still the same, compare name(ssid)
                if (this.mName.compareTo(another.mName) != 0) {
                    Log.d(TAG, "compare mName return:" + this.mName.compareTo(another.mName));
                    return this.mName.compareTo(another.mName);
                }
                //if name still the same, compare credential
                if (this.mCredential != null && another.mCredential != null) {
                    if (this.mCredential.compareTo(another.mCredential) != 0) {
                        Log.d(TAG, "compare mCredential return:" + this.mCredential.compareTo(another.mCredential));
                        return this.mCredential.compareTo(another.mCredential);
                    }
                }
            } else {
                return 1;
            }
        }

        Log.d(TAG, "both policies equal");
        return 0;
    }

    @Override
    public int hashCode() {
        int hash = 209;
        if (mName != null) {
            hash += mName.hashCode();
        }
        if (mCredential != null) {
            hash += mCredential.hashCode();
        }
        Log.d(TAG, "hashCode=" +hash);
        return hash;
    }

    @Override
    public boolean equals(Object obj) {
        if (obj instanceof PasspointPolicy && obj != null) {
            final PasspointPolicy other = (PasspointPolicy) obj;
            //Passpoint R2 mtk04049 modified-->
            Log.d(TAG,"equals");
            //Log.d(TAG,"mName="+mName+" mSsid="+mSsid+" mSubscriptionPriority="+mSubscriptionPriority);
            //Log.d(TAG,"other.mName="+other.mName+" other.mSsid="+other.mSsid+" other.mSubscriptionPriority="+other.mSubscriptionPriority);
            boolean isSameSsid = false;
            boolean isSameEapType = false;

            isSameEapType = ( this.mCredential == null || other.mCredential == null )?
                false : this.mCredential.equals(other.mCredential);
            isSameSsid = ( mSsid == null || other.mSsid == null )? false:mSsid.equals(other.mSsid);
            return  this.mName.equals(other.mName)
                    && this.mSubscriptionPriority == other.mSubscriptionPriority
                    && this.mRoamingPriority == other.mRoamingPriority
                    && isSameSsid
                    && isSameEapType;
            /*
            Log.d(TAG,"mName="+mName+" mPriority="+mPriority+" mSsid="+mSsid);
            Log.d(TAG,"other.mName="+other.mName+" other.mPriority="+other.mPriority+" other.mSsid="+other.mSsid);
            return  mName.equals(other.mName)
                    && mPriority == other.mPriority
                    && mSsid.equals(other.mSsid);
                    //&& Objects.equal(mCredential, other.mCredential);
            */
            //Passpoint R2 mtk04049 modified-->
        }
        return false;
    }

    @Override
    public String toString() {
        return "PasspointPolicy: name=" + mName + " SubscriptionPriority=" + mSubscriptionPriority +
               " mRoamingPriority" + mRoamingPriority +
               " ssid=" + mSsid + " restriction=" + mRestriction +
               " ishomesp=" + mIsHomeSp + " Credential=" + mCredential;
    }

    public static final Creator<PasspointPolicy> CREATOR = new Creator<PasspointPolicy>() {
        public PasspointPolicy createFromParcel(Parcel in) {
            return new PasspointPolicy(in);
        }

        public PasspointPolicy[] newArray(int size) {
            return new PasspointPolicy[size];
        }
    };

    /** Implement the Parcelable interface {@hide} */
    public int describeContents() {
        return 0;
    }

}

