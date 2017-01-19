/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.common.passpoint;

import android.os.Parcel;
import android.os.Parcelable;

public final class PasspointServiceProviderInfo implements Parcelable {
    public String   mFriendlyName;
    public String   mIconType;
    public String   mIconPath;
    public String   mSSID;
    public String   mBssid;

    public String toString() {
        String str = new String();
        str = "PasspointServiceProviderInfo mFriendlyName=[" + mFriendlyName + "] SSID[" + mSSID + "] mBssid=[" + mBssid + "] mIconType=[" + mIconType + "] mIconPath=[" + mIconPath + "]";
        return str;
    }

    public static final Parcelable.Creator<PasspointServiceProviderInfo> CREATOR = new Parcelable.Creator<PasspointServiceProviderInfo>() {
        public PasspointServiceProviderInfo createFromParcel(Parcel in) {
            PasspointServiceProviderInfo osumsg = new PasspointServiceProviderInfo();
            osumsg.readFromParcel(in);
            return osumsg;
        }
        public PasspointServiceProviderInfo[] newArray(int size) {
            return new PasspointServiceProviderInfo[size];
        }
    };

    public PasspointServiceProviderInfo() {}
    public PasspointServiceProviderInfo(String friendlyname, String ssid, String bssid, String icontype, String iconpath) {
        this.mFriendlyName      = friendlyname;
        this.mSSID              = ssid;
        this.mBssid             = bssid;
        this.mIconType          = icontype;
        this.mIconPath          = iconpath;
    }

    //@Override
    public int describeContents() {
        return 0;
    }

    //@Override
    public void writeToParcel(Parcel out, int flags) {
        out.writeString(mFriendlyName);
        out.writeString(mSSID);
        out.writeString(mBssid);
        out.writeString(mIconType);
        out.writeString(mIconPath);
    }

    //@Override
    public void readFromParcel(Parcel in) {
        mFriendlyName   = in.readString();
        mSSID           = in.readString();
        mBssid          = in.readString();
        mIconType       = in.readString();
        mIconPath       = in.readString();
    }
}
