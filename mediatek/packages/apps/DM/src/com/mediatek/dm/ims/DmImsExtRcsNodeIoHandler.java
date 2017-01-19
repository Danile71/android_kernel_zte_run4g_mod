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

package com.mediatek.dm.ims;

import android.content.Context;
import android.net.Uri;
import android.text.TextUtils;
import android.util.Log;

import com.mediatek.dm.DmConst.TAG;
import com.redbend.vdm.NodeIoHandler;
import com.redbend.vdm.VdmException;

import com.mediatek.ims.ImsManager;
import com.mediatek.ims.ImsConstants;
import com.mediatek.common.ims.mo.ImsAuthInfo;

import java.io.File;

/**
 * this handler is used to process RCS's nodes, currently include four:
 * AuthType
 * Realm
 * UserName
 * UserPwd
 * see doc: rcsrel2manobj2.pdf section 2.6.3 for more details.
 */
public class DmImsExtRcsNodeIoHandler implements NodeIoHandler {

    protected Context mContext;
    protected Uri mUri;
    private ImsManager mImsManager = null;

    private final static String RCS_AUTH_TYPE = "AuthType";
    private final static String RCS_REALM = "Realm";
    private final static String RCS_USER_NAME = "UserName";
    private final static String RCS_USER_PWD = "UserPwd";

    /**
     * Constructor.
     *
     * @param ctx
     *            context
     * @param treeUri
     *            node path of tree.xml
     */
    public DmImsExtRcsNodeIoHandler(Context ctx, Uri treeUri) {
        Log.i(TAG.NODE_IO_HANDLER, "DmImsExtRcsNodeIoHandler constructed");
        mContext = ctx;
        mUri = treeUri;
        mImsManager = (ImsManager) mContext.getSystemService(Context.IMS_SERVICE);
    }

    public int read(int offset, byte[] data) throws VdmException {
        if (mUri == null) {
            throw new VdmException("DmImsExtRcsNodeIoHandler read URI is null!");
        }

        String valueToRead = null;
        String uriPath = mUri.getPath();
        Log.i(TAG.NODE_IO_HANDLER, "uri: " + uriPath);
        Log.i(TAG.NODE_IO_HANDLER, "arg0: " + offset);

        int leafIndex = uriPath.lastIndexOf(File.separator);
        if (leafIndex == -1) {
            throw new VdmException("DmImsExtRcsNodeIoHandler URI is not valid, has no '/'!");
        }

        String leafValue = uriPath.substring(leafIndex + 1);
        if (leafValue.equals(RCS_AUTH_TYPE)) {
            valueToRead = mImsManager.readImsAuthInfoMo().getAuthType();
        } else if (leafValue.equals(RCS_REALM)) {
            valueToRead = mImsManager.readImsAuthInfoMo().getRelam();
        } else if (leafValue.equals(RCS_USER_NAME)) {
            valueToRead = mImsManager.readImsAuthInfoMo().getUserName();
        } else if (leafValue.equals(RCS_USER_PWD)) {
            valueToRead = mImsManager.readImsAuthInfoMo().getUserPwd();
        }

        if (TextUtils.isEmpty(valueToRead)) {
            return 0;
        } else {
            byte[] temp = valueToRead.getBytes();
            if (data == null) {
                return temp.length;
            }
            int numberRead = 0;
            for (; numberRead < data.length - offset; numberRead++) {
                if (numberRead < temp.length) {
                    data[numberRead] = temp[offset + numberRead];
                } else {
                    break;
                }
            }
            return numberRead;
        }
    }

    public void write(int offset, byte[] data, int totalSize) throws VdmException {
        Log.i(TAG.NODE_IO_HANDLER, "uri: " + mUri.getPath());
        Log.i(TAG.NODE_IO_HANDLER, "data: " + new String(data));
        Log.i(TAG.NODE_IO_HANDLER, "offset: " + offset);
        Log.i(TAG.NODE_IO_HANDLER, "total size: " + totalSize);

        if (totalSize == 0 || data == null) {
            Log.d(TAG.NODE_IO_HANDLER, "DmImsExtRcsNodeIoHandler write total size 0 or data null!");
            return;
        }

        // partial write is not allowed.
        if (offset != 0 || (offset == 0 && data.length == 0)) {
            throw new VdmException(VdmException.VdmError.TREE_EXT_NOT_PARTIAL);
        }

        String valueToWrite = new String(data);
        String uriPath = mUri.toString();
        int leafIndex = uriPath.lastIndexOf("/");
        if (leafIndex == -1) {
            throw new VdmException("DmImsExtRcsNodeIoHandler URI is not valid, has no '/'!");
        }

        String leafValue = uriPath.substring(leafIndex + 1);
        ImsAuthInfo authInfo = mImsManager.readImsAuthInfoMo();
        if (leafValue.equals(RCS_AUTH_TYPE)) {
            authInfo.setAuthType(valueToWrite);
        } else if (leafValue.equals(RCS_REALM)) {
            authInfo.setRelam(valueToWrite);
        } else if (leafValue.equals(RCS_USER_NAME)) {
            authInfo.setUserName(valueToWrite);
        } else if (leafValue.equals(RCS_USER_PWD)) {
            authInfo.setUserPwd(valueToWrite);
        } else {
            Log.d(TAG.NODE_IO_HANDLER, "DmImsExtRcsNodeIoHandler ignore write.");
            return;
        }

        mImsManager.writeImsAuthInfoMo(authInfo);
    }
}
