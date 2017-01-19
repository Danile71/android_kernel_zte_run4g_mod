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

package com.mediatek.videoplayer;

import android.app.Activity;
import android.content.Context;
import android.content.res.Resources;
import com.mediatek.drm.OmaDrmClient;
import com.mediatek.drm.OmaDrmUiUtils;
import com.mediatek.drm.OmaDrmStore;
import android.graphics.Bitmap;

/**
 * The class is defined for easily use DrmManagerClient.
 */
public final class DrmManager {
    private static final String TAG = "DrmManager";
    private OmaDrmClient mDrmManagerClient = null;
    private static DrmManager sInstance = new DrmManager();

    /**
     * Constructor for DrmManager.
     */
    private DrmManager() {
    }

    /**
     * Initial the DrmManagerClient.
     * 
     * @param context The context to use.
     */
    public void init(Context context) {
        if (MtkUtils.isSupportDrm()) {
            if (mDrmManagerClient == null) {
                mDrmManagerClient = new OmaDrmClient(context);
            }
        }
    }

    /**
     * Get a DrmManager Object. init() must be called before using it.
     * 
     * @return a instance of DrmManager.
     */
    public static DrmManager getInstance() {
        return sInstance;
    }

    /**
     * This method check weather the rights-protected content has valid right to transfer.
     * 
     * @param path path to the rights-protected content.
     * @return false for having right to transfer, true for not having the right.
     */
    public boolean isRightsStatus(String path) {
        if (mDrmManagerClient != null && MtkUtils.isSupportDrm()) {
            boolean ret = mDrmManagerClient.checkRightsStatus(path, OmaDrmStore.Action.TRANSFER) 
            != OmaDrmStore.RightsStatus.RIGHTS_VALID;
            MtkLog.d(TAG, "isRightsStatus() path:" + path + ", ret:" + ret); 
            return ret;
        } else {
            return false;
        }
    }

    /**
     * This method release drm manager client
     */
    public void release() {
        if (mDrmManagerClient != null) {
            mDrmManagerClient.release();
            mDrmManagerClient = null;
            MtkLog.d(TAG,"release drm manager client.");
        }
    }
}
