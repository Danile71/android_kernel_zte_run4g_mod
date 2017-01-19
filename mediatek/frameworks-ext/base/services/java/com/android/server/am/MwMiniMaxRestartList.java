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
 * MediaTek Inc. (C) 2014. All rights reserved.
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

package com.android.server.am;

import java.util.ArrayList;

public class MwMiniMaxRestartList {
    //For app whick need restart during Restore and Max
    private final ArrayList<String> mMiniMaxRestartApps = new ArrayList<String>();

    public MwMiniMaxRestartList() {
        mMiniMaxRestartApps.add("com.teamchaos.dragonacademy");
        mMiniMaxRestartApps.add("sina.mobile.tianqitong");
        mMiniMaxRestartApps.add("com.gameloft.android.ANMP.GloftCPHM");
        mMiniMaxRestartApps.add("jp.gree.android.pf.greeapp58078a");
        mMiniMaxRestartApps.add("com.google.android.youtube");
        mMiniMaxRestartApps.add("com.android.chrome");
        mMiniMaxRestartApps.add("com.google.android.gm");  
        mMiniMaxRestartApps.add("com.android.contacts");
        mMiniMaxRestartApps.add("com.tencent.mtt");
        mMiniMaxRestartApps.add("com.android.calendar");
        mMiniMaxRestartApps.add("com.storm.smart");
        mMiniMaxRestartApps.add("com.google.android.apps.maps");
        mMiniMaxRestartApps.add("com.igg.castleclash");
        mMiniMaxRestartApps.add("com.sega.sonicdash");
        mMiniMaxRestartApps.add("com.tencent.feiji");
        mMiniMaxRestartApps.add("com.tencent.clover");
        mMiniMaxRestartApps.add("com.qiyi.video");
        mMiniMaxRestartApps.add("com.yhx.air.other");
        mMiniMaxRestartApps.add("com.magmamobile.game.Fireman");
        mMiniMaxRestartApps.add("com.venan.boh");
        mMiniMaxRestartApps.add("com.zsy.lovefishing");
        mMiniMaxRestartApps.add("com.ppgames.songguess");
        mMiniMaxRestartApps.add("com.umonistudio.tile");
        mMiniMaxRestartApps.add("com.qqgame.hlddz");
        mMiniMaxRestartApps.add("com.carrot.carrotfantasy");
        mMiniMaxRestartApps.add("com.sdu.didi.psnger");
        //TODO,Can add app packagename into this list, 
        //when Max/Restore this app,it will restart it              
    }
    
    //Call by MwMonitorActivity, when doing Restore and Max,query the write list to restart apps
    public boolean matchMiniMaxRestartAppList(String packageName) {
        boolean ret = false;
        for (String needRestartAPP : mMiniMaxRestartApps){
             if (packageName.equals(needRestartAPP)){
                 ret = true;
                 break;                
             }
        }
        return ret;
    } 
        
}

