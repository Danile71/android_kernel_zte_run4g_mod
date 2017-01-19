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
import com.mediatek.common.ims.mo.ImsXcapInfo;

import java.io.File;

/**
 * this handler is used to process XDM nodes. Currently there are four:
 * URI: this node maps to  RCS's XCAP Root URI
 * AAUTHNAME: this node maps to RCS's XCAP Authentication
 * AAUTHSECRET: this node maps to XCAP Authentication Secret
 * AAUTHTYPE: this node maps to XCAP Authentication type
 * see doc: rcsrel2manobj2.pdf section 2.3.1 for more details.
 */
public class DmXdmNodeIoHandler implements NodeIoHandler {

    protected Context mContext;
    protected Uri mUri;

    private final static String XDM_URI = "URI";
    private final static String XDM_AUTH_NAME = "AAUTHNAME";
    private final static String XDM_AUTH_SECRET = "AAUTHSECRET";
    private final static String XDM_AUTH_TYPE = "AAUTHTYPE";

    private ImsManager mImsManager = null;

    /**
     * Constructor.
     *
     * @param ctx
     *            context
     * @param treeUri
     *            XDMMO sub node path in tree.xml
     */
    public DmXdmNodeIoHandler(Context ctx, Uri treeUri) {
        Log.i(TAG.NODE_IO_HANDLER, "DmXdmNodeIoHandler constructed");
        mContext = ctx;
        mUri = treeUri;
        mImsManager = (ImsManager) mContext.getSystemService(Context.IMS_SERVICE);
    }

    public int read(int offset, byte[] data) throws VdmException {
        if (mUri == null) {
            throw new VdmException("DmXdmNodeIoHandler read URI is null!");
        }

        String valueToRead = null;
        String uriPath = mUri.getPath();
        Log.i(TAG.NODE_IO_HANDLER, "uri: " + uriPath);
        Log.i(TAG.NODE_IO_HANDLER, "arg0: " + offset);

        int leafIndex = uriPath.lastIndexOf(File.separator);
        if (leafIndex == -1) {
            throw new VdmException("DmXdmNodeIoHandler read URI is not valid, has no '/'!");
        }
        String leafValue = uriPath.substring(leafIndex + 1);

        if (leafValue.equals(XDM_URI)) {
            valueToRead = mImsManager.readImsXcapInfoMo().getXcapRootUri();
        } else if (leafValue.equals(XDM_AUTH_NAME)) {
            valueToRead = mImsManager.readImsXcapInfoMo().getXcapAuth();
        } else if (leafValue.equals(XDM_AUTH_SECRET)) {
            valueToRead = mImsManager.readImsXcapInfoMo().getXcapAuthSecret();
        } else if (leafValue.equals(XDM_AUTH_TYPE)) {
            valueToRead = mImsManager.readImsXcapInfoMo().getXcapAuthType();
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
            Log.d(TAG.NODE_IO_HANDLER, "DmXdmNodeIoHandler write total size 0 or data null!");
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
            throw new VdmException("DmXdmNodeIoHandler URI is not valid, has no '/'!");
        }

        String leafValue = uriPath.substring(leafIndex + 1);
        ImsXcapInfo xcapInfo = mImsManager.readImsXcapInfoMo();
        if (leafValue.equals(XDM_URI)) {
            xcapInfo.setXcapRootUri(valueToWrite);
        } else if (leafValue.equals(XDM_AUTH_NAME)) {
            xcapInfo.setXcapAuth(valueToWrite);
        } else if (leafValue.equals(XDM_AUTH_SECRET)) {
            xcapInfo.setXcapAuthSecret(valueToWrite);
        } else if (leafValue.equals(XDM_AUTH_TYPE)) {
            xcapInfo.setXcapAuthType(valueToWrite);
        } else {
            Log.d(TAG.NODE_IO_HANDLER, "DmXdmNodeIoHandler ignore write.");
            return;
        }
        mImsManager.writeImsXcapInfoMo(xcapInfo);
    }
}
