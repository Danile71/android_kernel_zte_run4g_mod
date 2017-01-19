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

import com.mediatek.common.ims.mo.ImsPhoneCtx;
import com.mediatek.dm.DmConst.TAG;
import com.mediatek.ims.ImsManager;
import com.redbend.vdm.NodeIoHandler;
import com.redbend.vdm.VdmException;

import java.io.File;

/**
 * A handler for transfer ims phonecontext related parameter values to session processor.
 */
public class DmPhoneCtxNodeIoHandler implements NodeIoHandler {

    protected Context mContext;
    protected Uri mUri;

    private final static String[] SUB_NODE = { "1", "2", "3", "4" };
    private final static String[] LEAF_NODE = { "PhoneContext", "Public_user_identity"};
    /* phone context's user identitys can be more than one.
     * if only need one, config the node name as Public_user_identity in tree.xml
     * if need more than one, use the following name, support 4 at most.
     */
    private final static String[] LEAF_NODE_MULTI_USER_ID = {
             "Public_user_ID1",  "Public_user_ID2", "Public_user_ID3", "Public_user_ID4" };

    /**
     * Constructor.
     *
     * @param ctx
     *            context
     * @param treeUri
     *            node path of tree.xml
     */
    public DmPhoneCtxNodeIoHandler(Context ctx, Uri treeUri) {
        Log.i(TAG.NODE_IO_HANDLER, "DmPhoneCtxNodeIoHandler constructed");
        mContext = ctx;
        mUri = treeUri;
    }

    public int read(int offset, byte[] data) throws VdmException {
        if (mUri == null) {
            throw new VdmException("PhoneCtx read URI is null!");
        }

        String valueToRead = null;
        String uriPath = mUri.getPath();
        Log.i(TAG.NODE_IO_HANDLER, "uri: " + uriPath);
        Log.i(TAG.NODE_IO_HANDLER, "offset: " + offset);

        String[] pathArray = uriPath.split(File.separator);
        int length = pathArray.length;
        if (length < 2) {
            throw new VdmException("PhoneCtx read URI is not valid!");
        }
        String leafNode = pathArray[length - 1];
        String InteriorNode = pathArray[length - 2];
        ImsManager imsManager = (ImsManager) mContext.getSystemService(Context.IMS_SERVICE);
        ImsPhoneCtx[] phoneCtxArray = imsManager.readImsPhoneCtxMo();
        if (phoneCtxArray != null) {
            int pcLength = phoneCtxArray.length;
            Log.i(TAG.NODE_IO_HANDLER, "[read]pcLength: " + pcLength);
            for (int i = 0; i < pcLength; ++i) {
                if (SUB_NODE[i].equalsIgnoreCase(InteriorNode)) {
                    ImsPhoneCtx phoneCtx = phoneCtxArray[i];
                    if (LEAF_NODE[0].equalsIgnoreCase(leafNode)) {
                        valueToRead = phoneCtx.getPhoneCtx();
                    } else if (LEAF_NODE[1].equalsIgnoreCase(leafNode)) {
                        valueToRead = phoneCtx.getPhoneCtxIpuis()[0];
                    } else {
                        for (int j = 0; j < 4; ++j) {
                            if (LEAF_NODE_MULTI_USER_ID[j].equalsIgnoreCase(leafNode)) {
                                valueToRead = phoneCtx.getPhoneCtxIpuis()[j];
                                break;
                            }
                        }
                    }
                    break;
                }
            }
            Log.i(TAG.NODE_IO_HANDLER, "[PhoneCtx][read] result: " + valueToRead);
        } else {
            Log.e(TAG.NODE_IO_HANDLER,"[PhoneCtx][read] readImsPhoneCtxMo is NULL!!!");
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
        String uriPath = mUri.getPath();
        String[] pathArray = uriPath.split(File.separator);
        int length = pathArray.length;
        if (length < 2) {
            throw new VdmException("PhoneCtx read URI is not valid!");
        }
        String leafNode = pathArray[length - 1];
        String InteriorNode = pathArray[length - 2];

        ImsManager imsManager = (ImsManager) mContext.getSystemService(Context.IMS_SERVICE);
        ImsPhoneCtx[] phoneCtxArray = imsManager.readImsPhoneCtxMo();
        if (phoneCtxArray == null){
            Log.e(TAG.NODE_IO_HANDLER,"[PhoneCtx][write] readImsPhoneCtxMo is NULL!!!");
            return;
        }

        ImsPhoneCtx phoneCtx = null;
        int index = 0;
        int pcLength = phoneCtxArray.length;
        Log.i(TAG.NODE_IO_HANDLER, "[write]pcLength: " + pcLength);
        for (; index < pcLength; ++index) {
            if (SUB_NODE[index].equalsIgnoreCase(InteriorNode)) {
                if (LEAF_NODE[0].equalsIgnoreCase(leafNode)) {
                    phoneCtx = new ImsPhoneCtx(valueToWrite,
                            phoneCtxArray[index].getPhoneCtxIpuis());
                } else if (LEAF_NODE[1].equalsIgnoreCase(leafNode)) {
                    String [] userID = phoneCtxArray[index].getPhoneCtxIpuis();
                    userID[0] = valueToWrite;
                    Log.i(TAG.NODE_IO_HANDLER, "write userID 0");
                    phoneCtx = new ImsPhoneCtx(phoneCtxArray[index].getPhoneCtx(), userID);
                } else {
                    String [] userID = phoneCtxArray[index].getPhoneCtxIpuis();
                    int userIdLength = userID.length;
                    for (int j = 0; j < userIdLength; ++j) {
                        if (LEAF_NODE_MULTI_USER_ID[j].equalsIgnoreCase(leafNode)) {
                            userID[j] = valueToWrite;
                            Log.i(TAG.NODE_IO_HANDLER, "write userID " + j);
                            phoneCtx = new ImsPhoneCtx(phoneCtxArray[index].getPhoneCtx(), userID);
                            break;
                        }
                    }
                }
                break;
            }
        }
        if (phoneCtx != null) {
            Log.i(TAG.NODE_IO_HANDLER, "write PhoneCtx " + index);
            phoneCtxArray[index] = phoneCtx;
            imsManager.writeImsImsPhoneCtxMo(phoneCtxArray);
        }
    }
}
