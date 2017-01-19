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

import java.util.List;

public interface PasspointManager {
    public static final int EVENT_POLICY_PROVISION_DONE                 = 3;
    public static final int EVENT_REMEDIATION_DONE                      = 4;
    public static final int EVENT_SET_POLICY_POLL_TIMER                 = 5;
    public static final int EVENT_PROVISION_DONE                        = 6;
    public static final int EVENT_CANCEL_POLICY_POLL_TIMER              = 7;
    public static final int EVENT_SET_REMEDIATION_POLL_TIMER            = 8;
    public static final int EVENT_CANCEL_REMEDIATION_POLL_TIMER         = 9;
    public static final int EVENT_PROVISION_UPDATE_RESPONSE_DONE        = 10;
    public static final int EVENT_REMEDIATION_UPDATE_RESPONSE_DONE      = 11;
    public static final int EVENT_POLICY_PROVISION_UPDATE_RESPONSE_DONE = 12;
    public static final int EVENT_REGISTER_CERT_ENROLLMENT_DONE         = 13;
    public static final int EVENT_PROVISION_FROM_FILE_DONE              = 14;
    public static final int EVENT_SIM_PROVISION_DONE                    = 15;    
    public static final int EVENT_SIM_PROVISION_UPDATE_RESPONSE_DONE    = 16;
    public static final int EVENT_PRE_PROVISION_DONE                    = 17;
    public static final int EVENT_UPDATE_NOTIFICATION_SPINFO            = 20;
    public static final int EVENT_UPDATE_NOTIFICATION_CONNEDTED         = 21;
    public static final int EVENT_UPDATE_NOTIFICATION_PROVISION_FAIL    = 22;

    public static final String HTTP_PASSPOINT_BROWSER_RESPONSE = "com.mediatek.passpoint.browser.response";
    public static final String AP_OSU_LIST = "android.net.wifi.passpoint.AP_OSU_LIST";
    public static final String USING_SOAP = "SPP-ClientInitiated";
    public static final String USING_DM = "OMA-DM-ClientInitiated";

    /** for DM @{ */
    public static final String DM_SUBSCRIPTION_PROVISION = "android.net.wifi.passpoint.DM_SUBSCRIPTION_PROVISION";
    public static final String DM_SUBSCRIPTION_REMEDIATION = "android.net.wifi.passpoint.DM_SUBSCRIPTION_REMEDIATION";
    public static final String DM_POLICY_PROVISION = "android.net.wifi.passpoint.DM_POLICY_PROVISION";
    public static final String ACTION_PASSPOINT_R2_REFRESH_SPINFO = "com.mediatek.passpoint.REFRESH_SPINFO";
    public static final String ACTION_PASSPOINT_R2_NOSHOW_SPINFO = "com.mediatek.passpoint.NOSHOW_SPINFO";
    
    public static final int SUBSCRIPTION_PROVISIONING                   = 10;
    public static final int SUBSCRIPTION_PROVISIONING_USERNAME_PASSWORD = 11;
    public static final int SUBSCRIPTION_PROVISIONING_CERTIFICATE       = 12;
    
    public static final int REMEDIATION_USERNAME_PASSWORD               = 21;
    public static final int REMEDIATION_CERTIFICATE                     = 23;
    public static final int REMEDIATION_SIM                             = 27;

    public static final int POLICY_PROVISIONING_USERNAME_PASSWORD       = 31;
    public static final int POLICY_PROVISIONING_CERTIFICATE             = 32;
    public static final int POLICY_PROVISIONING_SIM                     = 33;
    /** @} */

    public void addCredential(PasspointCredential cp);

    public boolean addCredentialbyValue(String type, String username, String passwd, String imsi, String mnc, String mcc, String root_ca, String realm, int preferred, String fqdn, String client_ca);

    public void delCredential(PasspointCredential cp);

    public void delCredentialbyindex(int index);

    public void delAllCredential();

    public String dumpCredential();

    public void installPpsMo(int index);

    public void installRootCA();

    public void installServerCA();
    
    public void Set8211W(int index);

    public void clearPolicies();

    public void sendregisterPollAlarm(long second);

    public void setSpExLis(String exSpList);

    public void setOsuServiceProvider(String bssid);

    public List<PasspointServiceProviderInfo> getServiceProviderInfoList();

    public List<PasspointCredentialInfo> getCredentialInfoList();

    public void setPreferCredential(String credname, String wifispfqdn);

    public void notifySubscriptionProvisionStatus(boolean isReady);
}

