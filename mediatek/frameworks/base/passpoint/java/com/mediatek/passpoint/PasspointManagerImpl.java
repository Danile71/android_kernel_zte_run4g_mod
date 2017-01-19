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

import android.content.Context;
import android.os.IBinder;
import android.os.ServiceManager;
import android.os.RemoteException;
import android.util.Log;

import com.mediatek.common.passpoint.*;
import java.util.List;

public class PasspointManagerImpl implements PasspointManager {

    private final static String TAG = "PasspointService";
    private IPasspointManager mService;
    private Context mContext;

    public PasspointManagerImpl(Context context) {
        mContext = context;
        IBinder b = ServiceManager.getService(Context.MTK_PASSPOINT_SERVICE);
        mService = IPasspointManager.Stub.asInterface(b);
        if (mService == null) {
            Log.e(TAG, "mService is still null..");
        }
    }

    public void addCredential(PasspointCredential cp) {
        if (mService == null) {
            return;
        }
        try {
            mService.addCredential(cp);
        } catch (RemoteException e) {}

    }

    public boolean addCredentialbyValue(String type, String username, String passwd, String imsi, String mnc, String mcc, String root_ca, String realm, int preferred, String fqdn, String client_ca) {
        if (mService == null) {
            return false;
        }
        try {
            return mService.addCredentialbyValue(type, username, passwd, imsi, mnc, mcc, root_ca, realm, preferred, fqdn, client_ca );
        } catch (RemoteException e) {
            return false;
        }
    }

    public void delCredential(PasspointCredential cp) {
        if (mService == null) {
            return;
        }
        try {
            mService.delCredential(cp);
        } catch (RemoteException e) {}
    }

    public void delCredentialbyindex(int index) {
        if (mService == null) {
            return;
        }

        try {
            mService.delCredentialbyindex(index);
        } catch (RemoteException e) {}
    }

    public void delAllCredential() {
        if (mService == null) {
            return;
        }
        try {
            mService.delAllCredential();
        } catch (RemoteException e) {}
    }

    public String dumpCredential() {
        if (null == mService) {
            return null;
        }
        try {
            return mService.dumpCredential();
        } catch (RemoteException e) {
            return null;
        }
    }

    public void installPpsMo(int index) {
        if (mService == null) {
            return;
        }
        try {
            mService.installPpsMo(index);
        } catch (RemoteException e) {}
    }

    public void installRootCA() {
        if (mService == null) {
            return;
        }
        try {
            mService.installRootCA();
        } catch (RemoteException e) {}
    }

    public void installServerCA() {
        if (mService == null) {
            return;
        }
        try {
            mService.installServerCA();
        } catch (RemoteException e) {}
    }    
    
    public void Set8211W(int index) {
        if (mService == null) {
            return;
        }
        try {
            mService.Set8211W(index);
        } catch (RemoteException e) {}
    }

    public void clearPolicies() {
        if (null == mService) {
            return;
        }
        try {
            mService.clearPolicies();
        } catch (RemoteException e) {}
    }

    public void sendregisterPollAlarm(long second) {
        if (mService == null) {
            return;
        }

        try {
            mService.sendregisterPollAlarm(second);
        } catch (RemoteException e) {}
    }

    public void setSpExLis(String exSpList) {
        if (mService == null) {
            return;
        }

        try {
            mService.setSpExLis(exSpList);
        } catch (RemoteException e) {}
    }

    public void setOsuServiceProvider(String bssid) {
        if (mService == null) {
            return;
        }

        try {
            mService.setOsuServiceProvider(bssid);
        } catch (RemoteException e) {}
    }

    public List<PasspointServiceProviderInfo> getServiceProviderInfoList() {
        if (mService == null) {
            return null;
        }

        try {
            return mService.getServiceProviderInfoList();
        } catch (RemoteException e) {
            return null;
        }
    }

    public void setPreferCredential(String credname, String wifispfqdn) {
        if (mService == null) {
            return;
        }

        try {
            mService.setPreferCredential(credname, wifispfqdn);
        } catch (RemoteException e) {}
    }

    public List<PasspointCredentialInfo> getCredentialInfoList() {
        if (mService == null) {
            return null;
        }

        try {
            return mService.getCredentialInfoList();
        } catch (RemoteException e) {
            return null;
        }
    }

    public void notifySubscriptionProvisionStatus(boolean isReady) {
        if (mService == null) {
            return;
        }

        try {
            mService.notifySubscriptionProvisionStatus(isReady);
        } catch (RemoteException e) {}
    }

}

