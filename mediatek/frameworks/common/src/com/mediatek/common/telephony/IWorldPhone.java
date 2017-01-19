/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
package com.mediatek.common.telephony;

/**
 * The World Phone network selection logic.  
 * 
 */
public interface IWorldPhone {
    public static final String LOG_TAG = "PHONE";

    public static final int POLICY_OM       = 0;
    public static final int POLICY_OP01     = 1;
    public static final int POLICY_OM_LTE   = 100;
    public static final int POLICY_OP01_LTE = 101;

    static final int SELECTION_MODE_MANUAL = 0;
    static final int SELECTION_MODE_AUTO   = 1;

    static final int EVENT_GSM_PLMN_CHANGED_1 = 0;
    static final int EVENT_GSM_PLMN_CHANGED_2 = 1;
    static final int EVENT_GSM_SUSPENDED_1    = 10;
    static final int EVENT_GSM_SUSPENDED_2    = 11;
    static final int EVENT_RADIO_ON_1         = 30;
    static final int EVENT_RADIO_ON_2         = 31;
    static final int EVENT_SET_RAT_GSM_ONLY   = 40;
    static final int EVENT_SET_RAT_WCDMA_PREF = 50;
    static final int EVENT_SET_RAT_3_4G_PREF  = 60;
    static final int EVENT_STORE_MODEM_TYPE   = 70;
    static final int EVENT_QUERY_MODEM_TYPE   = 80;
    static final int EVENT_GET_PLMN_PREFER_LIST_1 = 90;
    static final int EVENT_GET_PLMN_PREFER_LIST_2 = 91;
    static final int EVENT_INVALID_SIM_NOTIFY_1   = 100;
    static final int EVENT_INVALID_SIM_NOTIFY_2   = 101;

    static final int EVENT_REG_PLMN_CHANGED_1 = EVENT_GSM_PLMN_CHANGED_1;
    static final int EVENT_REG_PLMN_CHANGED_2 = EVENT_GSM_PLMN_CHANGED_2;
    static final int EVENT_REG_SUSPENDED_1    = EVENT_GSM_SUSPENDED_1;
    static final int EVENT_REG_SUSPENDED_2    = EVENT_GSM_SUSPENDED_2;
    static final int EVENT_REG_PLMN_CHANGED_SGLTE_1 = 1000;
    static final int EVENT_REG_PLMN_CHANGED_SGLTE_2 = 1001;
    static final int EVENT_REG_SUSPENDED_SGLTE_1    = 1010;
    static final int EVENT_REG_SUSPENDED_SGLTE_2    = 1011;
    static final int EVENT_RADIO_ON_SGLTE_1         = 1020;
    static final int EVENT_RADIO_ON_SGLTE_2         = 1021;

    static final int SET_RAT_TO_2G   = 0;
    static final int SET_RAT_TO_AUTO = 1;

    static final int DEFAULT_3G_SLOT     = 0;
    static final int NO_3G_CAPABILITY    = -1;
    static final int AUTO_SELECT_DISABLE = -98;
    static final int UNKNOWN_3G_SLOT     = -99;

    static final int sUnknownUser = 0;
    static final int sType1User   = 1;
    static final int sType2User   = 2;
    static final int sType3User   = 3;

    static final int REGION_UNKNOWN  = 0;
    static final int REGION_DOMESTIC = 1;
    static final int REGION_FOREIGN  = 2;

    static final int CAMP_ON_NOT_DENIED                     = 0;
    static final int DENY_CAMP_ON_REASON_UNKNOWN            = 1;
    static final int DENY_CAMP_ON_REASON_NEED_SWITCH_TO_FDD = 2;
    static final int DENY_CAMP_ON_REASON_NEED_SWITCH_TO_TDD = 3;
    static final int DENY_CAMP_ON_REASON_DOMESTIC_WCDMA     = 4;

    static final int ICC_CARD_TYPE_UNKNOWN = 0;
    static final int ICC_CARD_TYPE_SIM     = 1;
    static final int ICC_CARD_TYPE_USIM    = 2;

    static final String NO_OP = "OM";
    static final String ACTION_SHUTDOWN_IPO = "android.intent.action.ACTION_SHUTDOWN_IPO";
    static final String ACTION_ADB_SWITCH_MODEM = "android.intent.action.ACTION_ADB_SWITCH_MODEM";

    public void setNetworkSelectionMode(int mode, int modemType);
    public int onNetworkModeChanged(int currentNwMode, int newNwMode);
    public void disposeWorldPhone();
}
