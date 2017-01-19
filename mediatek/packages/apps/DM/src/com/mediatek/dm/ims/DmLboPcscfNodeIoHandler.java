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

import com.mediatek.common.ims.mo.ImsLboPcscf;
import com.mediatek.dm.DmConst.TAG;
import com.mediatek.ims.ImsManager;
import com.redbend.vdm.NodeIoHandler;
import com.redbend.vdm.VdmException;

import java.io.File;

/**
 * A handler for transfer ims lbo related parameter values to session processor.
 */
public class DmLboPcscfNodeIoHandler implements NodeIoHandler {

    protected Context mContext;
    protected Uri mUri;

    private final static String[] SUB_NODE = { "1", "2", "3", "4" };
    private final static String[] LEAF_NODE = { "Address", "AddressType" };

    /**
     * Constructor.
     *
     * @param ctx
     *            context
     * @param treeUri
     *            node path of tree.xml
     */
    public DmLboPcscfNodeIoHandler(Context ctx, Uri treeUri) {
        Log.i(TAG.NODE_IO_HANDLER, "LboPcscf constructed");
        mContext = ctx;
        mUri = treeUri;
    }

    public int read(int offset, byte[] data) throws VdmException {
        if (mUri == null) {
            throw new VdmException("LboPcscf read URI is null!");
        }

        String valueToRead = null;
        String uriPath = mUri.getPath();
        Log.i(TAG.NODE_IO_HANDLER, "uri: " + uriPath);
        Log.i(TAG.NODE_IO_HANDLER, "offset: " + offset);

        String [] pathArray = uriPath.split(File.separator);
        int length = pathArray.length;
        if (length < 2) {
            throw new VdmException("LboPcscf read URI is not valid!");
        }

        String leafNode = pathArray[length-1];
        String InteriorNode = pathArray[length-2];
        ImsManager imsManager = (ImsManager) mContext.getSystemService(Context.IMS_SERVICE);
        ImsLboPcscf[] lboPcscfArray = imsManager.readImsLboPcscfMo();

        if (lboPcscfArray != null) {
            int lpLength = lboPcscfArray.length;
            Log.i(TAG.NODE_IO_HANDLER, "[read]lboPcscfArray.length: " + lpLength);
            for (int i = 0; i < lpLength; ++i) {
                if (SUB_NODE[i].equalsIgnoreCase(InteriorNode)) {
                    ImsLboPcscf lboPcscf = lboPcscfArray[i];
                    if (LEAF_NODE[0].equalsIgnoreCase(leafNode)) {
                        valueToRead = lboPcscf.getLboPcscfAddress();
                    } else if (LEAF_NODE[1].equalsIgnoreCase(leafNode)){
                        valueToRead = lboPcscf.getLboPcscfAddressType();
                    }
                    break;
                }
            }
            Log.i(TAG.NODE_IO_HANDLER, "[LboPcscf][read] result: " + valueToRead);
        } else {
            Log.e(TAG.NODE_IO_HANDLER,"[LboPcscf][read] readImsLboPcscfMo is NULL!!!");
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
            Log.d(TAG.NODE_IO_HANDLER, "LboPcscf write total size 0 or data null!");
            return;
        }

        // partial write is not allowed.
        if (offset != 0 || (offset == 0 && data.length == 0)) {
            throw new VdmException(VdmException.VdmError.TREE_EXT_NOT_PARTIAL);
        }

        String valueToWrite = new String(data);
        String uriPath = mUri.getPath();
        String [] pathArray = uriPath.split(File.separator);
        int length = pathArray.length;
        if (length < 2) {
            throw new VdmException("LboPcscf read URI is not valid!");
        }
        String leafNode = pathArray[length-1];
        String InteriorNode = pathArray[length-2];

        ImsManager imsManager = (ImsManager) mContext.getSystemService(Context.IMS_SERVICE);
        ImsLboPcscf[] lboPcscfArray = imsManager.readImsLboPcscfMo();
        if (lboPcscfArray == null){
            Log.e(TAG.NODE_IO_HANDLER,"[LboPcscf][write] readImsLboPcscfMo is NULL!!!");
            return;
        }

        ImsLboPcscf lboPcscf = null;
        int index = 0;
        int lpLength = lboPcscfArray.length;
        Log.i(TAG.NODE_IO_HANDLER, "[write]lboPcscfArray.length: " + lpLength);
        for (; index < lpLength; ++index) {
            if (SUB_NODE[index].equalsIgnoreCase(InteriorNode)) {
                if (LEAF_NODE[0].equalsIgnoreCase(leafNode)) {
                    lboPcscf = new ImsLboPcscf(valueToWrite, lboPcscfArray[index].getLboPcscfAddressType());
                } else if (LEAF_NODE[1].equalsIgnoreCase(leafNode)){
                    lboPcscf = new ImsLboPcscf(lboPcscfArray[index].getLboPcscfAddress(), valueToWrite);
                }
                break;
            }
        }
        if (lboPcscf != null) {
            Log.i(TAG.NODE_IO_HANDLER, "write LboPcscf " + index);
            lboPcscfArray[index] = lboPcscf;
            imsManager.writeImsLboPcscfMo(lboPcscfArray);
        }
    }
}
