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

import java.io.File;
import java.io.IOException;
import java.io.ByteArrayOutputStream;
import java.io.ByteArrayInputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;

/**
 * A handler for transfer ims level 1 related parameter values to session processor.
 */
public class DmImsNodeIoHandler implements NodeIoHandler {

    protected Context mContext;
    protected Uri mUri;
    private ImsManager mImsManager = null;

    private final static String PCSCF_ADRESS = "P-CSCF_Address";
    private final static String TIMER_ONE = "Timer_T1";
    private final static String TIMER_TWO = "Timer_T2";
    private final static String TIMER_FOUR = "Timer_T4";
    private final static String PRIVATE_USER_ID = "Private_user_identity"; // this is a read only node.
    private final static String DOMAIN_NAME = "Home_network_domain_name"; // this is a read only node.
    private final static String RESOURCE_ALLOCATION_MODE = "Resource_Allocation_Mode";
    private final static String VOICE_E_UTRAN = "Voice_Domain_Preference_E_UTRAN";
    private final static String SMS_IP_INDICATION = "SMS_Over_IP_Networks_Indication";
    private final static String KEEP_ALIVE_ENABLED = "Keep_Alive_Enabled";
    private final static String VOICE_UTRAN = "Voice_Domain_Preference_UTRAN";
    private final static String IMS_VOICE_TERMINATION = "Mobility_Management_IMS_Voice_Termination";
    private final static String REG_RETRY_BASE_TIME = "RegRetryBaseTime";
    private final static String REG_RETRY_MAX_TIME = "RegRetryMaxTime";

    /**
     * Constructor.
     *
     * @param ctx
     *            context
     * @param treeUri
     *            node path of tree.xml
     */
    public DmImsNodeIoHandler(Context ctx, Uri treeUri) {
        Log.i(TAG.NODE_IO_HANDLER, "DmImsNodeIoHandler constructed");
        mContext = ctx;
        mUri = treeUri;
        mImsManager = (ImsManager) mContext.getSystemService(Context.IMS_SERVICE);
    }

    public byte[] toByteArray(Object obj) {
        if (obj != null) {
            if (obj instanceof Boolean) {
                Boolean value = (Boolean)obj;
                return value.toString().getBytes();
            } else if (obj instanceof Integer) {
                Integer value = (Integer)obj;
                return Integer.toString(value).getBytes();
            }
        }
        // normally should not come here, just avoid warnning.
        Log.d(TAG.NODE_IO_HANDLER, "obj is null!");
        return new byte[0];
    }

    public String byteToString(byte[] data) {
        String str = new String(data);
        return str;
    }

    public int byteToInt(byte[] data) {
        String str = new String(data);
        int value = 0;
        try {
            value = Integer.parseInt(str);
        } catch (NumberFormatException e) {
            Log.d(TAG.NODE_IO_HANDLER, "get int from byte[] fail!!");
        }
        return value;
    }

    public boolean byteToBoolean(byte[] data) {
        String str = new String(data);
        Boolean value = Boolean.parseBoolean(str);
        return value;
    }

    public int read(int offset, byte[] data) throws VdmException {
        if (mUri == null) {
            throw new VdmException("IMS read URI is null!");
        }

        Object valueToRead = null;
        String uriPath = mUri.getPath();
        Log.i(TAG.NODE_IO_HANDLER, "uri: " + uriPath);
        Log.i(TAG.NODE_IO_HANDLER, "offset: " + offset);

        int leafIndex = uriPath.lastIndexOf(File.separator);
        if (leafIndex == -1) {
            throw new VdmException("IMS read URI is invalid, has no '/'!");
        }

        String leafValue = uriPath.substring(leafIndex + 1);

        if (leafValue.equals(PCSCF_ADRESS)) {
            valueToRead = mImsManager.readImsMoString(ImsConstants.IMS_MO_PCSCF);
        } else if (leafValue.equals(PRIVATE_USER_ID)) {
            valueToRead = mImsManager.readImsMoString(ImsConstants.IMS_MO_IMPI);
        } else if (leafValue.equals(DOMAIN_NAME)) {
            valueToRead = mImsManager.readImsMoString(ImsConstants.IMS_MO_DOMAIN);
        } else if (leafValue.equals(TIMER_ONE)) {
            valueToRead = mImsManager.readImsMoInt(ImsConstants.IMS_MO_TIMER_1);
        } else if (leafValue.equals(TIMER_TWO)) {
            valueToRead = mImsManager.readImsMoInt(ImsConstants.IMS_MO_TIMER_2);
        } else if (leafValue.equals(TIMER_FOUR)) {
            valueToRead = mImsManager.readImsMoInt(ImsConstants.IMS_MO_TIMER_4);
        } else if (leafValue.equals(RESOURCE_ALLOCATION_MODE)) {
            valueToRead = mImsManager.readImsMoBool(ImsConstants.IMS_MO_RESOURCE);
        } else if (leafValue.equals(VOICE_E_UTRAN)) {
            valueToRead = mImsManager.readImsMoInt(ImsConstants.IMS_MO_VOICE_E);
        } else if (leafValue.equals(SMS_IP_INDICATION)) {
            valueToRead = mImsManager.readImsMoBool(ImsConstants.IMS_MO_SMS);
        } else if (leafValue.equals(KEEP_ALIVE_ENABLED)) {
            valueToRead = mImsManager.readImsMoBool(ImsConstants.IMS_MO_KEEPALIVE);
        } else if (leafValue.equals(VOICE_UTRAN)) {
            valueToRead = mImsManager.readImsMoInt(ImsConstants.IMS_MO_VOICE_U);
        } else if (leafValue.equals(IMS_VOICE_TERMINATION)) {
            valueToRead = mImsManager.readImsMoBool(ImsConstants.IMS_MO_MOBILITY);
        } else if (leafValue.equals(REG_RETRY_BASE_TIME)) {
            valueToRead = mImsManager.readImsMoInt(ImsConstants.IMS_MO_REG_BASE);
        } else if (leafValue.equals(REG_RETRY_MAX_TIME)) {
            valueToRead = mImsManager.readImsMoInt(ImsConstants.IMS_MO_REG_MAX);
        }

        byte[] temp = null;

        if (valueToRead instanceof String) {
            String str = (String)valueToRead;
            if (TextUtils.isEmpty(str)) {
                return 0;
            } else {
                temp = str.getBytes();
            }
        } else if (valueToRead instanceof Integer) {
            int result = (Integer)valueToRead;
            temp = toByteArray(result);
        } else if (valueToRead instanceof Boolean) {
            Boolean result = (Boolean)valueToRead;
            temp = toByteArray(result);
        }

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

    public void write(int offset, byte[] data, int totalSize) throws VdmException {
        Log.i(TAG.NODE_IO_HANDLER, "uri: " + mUri.getPath());
        Log.i(TAG.NODE_IO_HANDLER, "data: " + new String(data));
        Log.i(TAG.NODE_IO_HANDLER, "offset: " + offset);
        Log.i(TAG.NODE_IO_HANDLER, "total size: " + totalSize);

        if (totalSize == 0 || data == null) {
            Log.d(TAG.NODE_IO_HANDLER, "IMS write total size 0 or data null!");
            return;
        }

        // partial write is not allowed.
        if (offset != 0 || (offset == 0 && data.length == 0)) {
            throw new VdmException(VdmException.VdmError.TREE_EXT_NOT_PARTIAL);
        }

        String uriPath = mUri.toString();
        int leafIndex = uriPath.lastIndexOf("/");
        if (leafIndex == -1) {
            throw new VdmException("IMS MO read URI is not valid, has no '/'!");
        }

        String leafValue = uriPath.substring(leafIndex + 1);

        if (leafValue.equals(PCSCF_ADRESS)) {
            mImsManager.writeImsMoString(ImsConstants.IMS_MO_PCSCF, byteToString(data));
        } else if (leafValue.equals(PRIVATE_USER_ID)) {
            // this is a read only node
            throw new VdmException(VdmException.VdmError.MAY_TREE_NOT_REPLACE);
            //mImsManager.writeImsMoString(ImsConstants.IMS_MO_IMPI, byteToString(data));
        }  else if (leafValue.equals(DOMAIN_NAME)) {
            // this is a read only node
            throw new VdmException(VdmException.VdmError.MAY_TREE_NOT_REPLACE);
            //mImsManager.writeImsMoString(ImsConstants.IMS_MO_DOMAIN, byteToString(data));
        } else if (leafValue.equals(TIMER_ONE)) {
            mImsManager.writeImsMoInt(ImsConstants.IMS_MO_TIMER_1, byteToInt(data));
        } else if (leafValue.equals(TIMER_TWO)) {
            mImsManager.writeImsMoInt(ImsConstants.IMS_MO_TIMER_2, byteToInt(data));
        } else if (leafValue.equals(TIMER_FOUR)) {
            mImsManager.writeImsMoInt(ImsConstants.IMS_MO_TIMER_4, byteToInt(data));
        } else if (leafValue.equals(RESOURCE_ALLOCATION_MODE)) {
            mImsManager.writeImsMoBool(ImsConstants.IMS_MO_RESOURCE, byteToBoolean(data));
        } else if (leafValue.equals(VOICE_E_UTRAN)) {
            mImsManager.writeImsMoInt(ImsConstants.IMS_MO_VOICE_E, byteToInt(data));
        } else if (leafValue.equals(SMS_IP_INDICATION)) {
            mImsManager.writeImsMoBool(ImsConstants.IMS_MO_SMS, byteToBoolean(data));
        } else if (leafValue.equals(KEEP_ALIVE_ENABLED)) {
            mImsManager.writeImsMoBool(ImsConstants.IMS_MO_KEEPALIVE, byteToBoolean(data));
        } else if (leafValue.equals(VOICE_UTRAN)) {
            mImsManager.writeImsMoInt(ImsConstants.IMS_MO_VOICE_U, byteToInt(data));
        } else if (leafValue.equals(IMS_VOICE_TERMINATION)) {
            mImsManager.writeImsMoBool(ImsConstants.IMS_MO_MOBILITY, byteToBoolean(data));
        } else if (leafValue.equals(REG_RETRY_BASE_TIME)) {
            mImsManager.writeImsMoInt(ImsConstants.IMS_MO_REG_BASE, byteToInt(data));
        } else if (leafValue.equals(REG_RETRY_MAX_TIME)) {
            mImsManager.writeImsMoInt(ImsConstants.IMS_MO_REG_MAX, byteToInt(data));
        }
    }
}
