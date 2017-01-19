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

package com.mediatek.phone.plugin;


import android.app.Dialog;
import android.content.Context;
import android.os.Message;
import android.os.Handler;
import com.android.internal.telephony.MmiCode;

import com.mediatek.phone.ext.MmiCodeExtension;
import com.mediatek.xlog.Xlog;

import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;
import android.text.Editable;
import android.text.TextWatcher;
import android.widget.EditText;
import android.widget.TextView;
import android.content.DialogInterface;


public class MmiCodeOP03Extension extends MmiCodeExtension {
    private static final String LOG_TAG = "MmiCodeOP03";
    private final Context mContext;
    private static final long TIME_DELAY = 300000;
    private static final int USSD_DIALOG_REQUEST = 1;
  
   public MmiCodeOP03Extension(Context context) {
        mContext = context;
    }

	public void onMmiCodeStarted(Context context,MmiCode mmiCode, Message buttonCallbackMessage,Dialog progressDailog){
		Handler mHandler = buttonCallbackMessage.getTarget();
		mHandler.removeMessages(buttonCallbackMessage.what);
		mHandler.sendMessageDelayed(buttonCallbackMessage, TIME_DELAY);
		Xlog.d(LOG_TAG, "ussd dismisses in 5 minutes if network does not dismiss it");
	}
	
    public boolean onUssdAlertActivityResume(AlertActivity ussdActivity, EditText inputText, int ussdType, AlertController Alert){
        final AlertController ALert_button = Alert; 
        if (ussdType == USSD_DIALOG_REQUEST) {
            ALert_button.getButton(DialogInterface.BUTTON_POSITIVE).setEnabled(true);
        	  Xlog.d(LOG_TAG, "ussd send button enabled even when no text entered");      
    	 }
    return true;
}
	
    public boolean onUssdUserInitiated(AlertActivity ussdActivity, boolean userInit) {
        Xlog.d(LOG_TAG, "if userInit is true, ussd user initiated else network");    
        return userInit;
    }	
}
