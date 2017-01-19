/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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

package com.mediatek.rcse.plugin.message;

import java.util.ArrayList;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import android.content.Context;
import android.content.Intent;
import android.os.Message;
import android.widget.AbsListView;

import com.mediatek.mms.ipmessage.ChatManager;
import com.mediatek.mms.ipmessage.IpMessageConsts;
import com.mediatek.mms.ipmessage.IpMessageConsts.ContactStatus;
import com.mediatek.rcse.activities.InvitationDialog;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.interfaces.ChatController;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage.Status;
import com.mediatek.rcse.service.RcsNotification;
import com.mediatek.rcse.service.binder.ThreadTranslater;

import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.utils.PhoneUtils;

/**
 * Provide chat management related interface
 */
public class IpMessageChatManger extends ChatManager {
    private static final String TAG = "IpMessageChatManger";
    private static final Map<String, Integer> VISIBLE_ITEM_MAP = new ConcurrentHashMap<String, Integer>(); 

    public IpMessageChatManger(Context context) {
        super(context);
    }

    @Override
    public void sendChatMode(String number, int status) {
        Logger.d(TAG, "sendChatMode() the number is " + number + " status is " + status);
        boolean isEmpty = true;
        if(PluginUtils.getMessagingMode() == 0)
        {
        	if (number.startsWith(IpMessageConsts.JOYN_START)) {
        		number = number.substring(4);
        	}
        }
        if (status == ContactStatus.STOP_TYPING) {
            isEmpty = true;
        } else if (status == ContactStatus.TYPING) {
            isEmpty = false;
        }
        Message controllerMessage = PluginController.obtainMessage(
                ChatController.EVENT_TEXT_CHANGED, PhoneUtils.formatNumberToInternational(number),
                isEmpty);
        controllerMessage.sendToTarget();
    }

    @Override
    public void enterChatMode(String number) {
        Logger.d(TAG, "enterChatMode() entry, number " + number);
        PluginUtils.reloadingMessages = false;
        if(PluginUtils.getMessagingMode() == 0)
        {
        	if (number.startsWith(IpMessageConsts.JOYN_START)) {
        		number = number.substring(4);
        	}
        }
        Message controllerMessage = PluginController.obtainMessage(
                ChatController.EVENT_SHOW_WINDOW, PhoneUtils
                        .formatNumberToInternational(number));
        controllerMessage.sendToTarget();
        if ((PluginUtils.getMessagingMode() == 1) && (PluginUtils.translateThreadId(number)==1)) {
			Logger.v(TAG, "enterChatMode(), open Window = " + number);
			Intent intent = new Intent(InvitationDialog.ACTION);
			intent.putExtra(RcsNotification.CONTACT, number);
			intent.putExtra(InvitationDialog.KEY_STRATEGY,
					InvitationDialog.STRATEGY_IPMES_SEND_BY_SMS);
			intent.putExtra("showDialog",true);
			intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK
					| Intent.FLAG_ACTIVITY_NEW_TASK);
			AndroidFactory.getApplicationContext().startActivity(intent);
		}
        super.enterChatMode(number);
    }

    @Override
    public void exportChat(long threadId) {
        PluginUtils.initiateExportChat(threadId);
    }

    @Override
    public void onScroll(AbsListView view , int firstVisibleItem, int visibleItemCount,
    					int totalItemCount, long threadId)
    {
    	String tag = ThreadTranslater.translateThreadId(threadId);
    	Logger.d(TAG, "reloadRcseOnScroll() entry, number " + tag + "firstVisibleItem : " + firstVisibleItem
    			+ "visibleItemCount : " + visibleItemCount + "totalItemCount : " + totalItemCount);
    	ArrayList<Integer>messageList = PluginUtils.O2OMap.get(tag);
    	if(messageList == null)
    		return;
    	int size = messageList.size();
    	if(VISIBLE_ITEM_MAP.containsKey(tag) && firstVisibleItem == VISIBLE_ITEM_MAP.get(tag))
        {	            	
    		Logger.d(TAG, "reloadRcseOnScroll() duplicate, firstVisibleItem  " + firstVisibleItem);
    		return;
    	}
    	if(messageList !=null &&  size>10 && firstVisibleItem >0 && firstVisibleItem%10 == 0)
        {	
    		VISIBLE_ITEM_MAP.put(tag, firstVisibleItem);    			
    		ArrayList<Integer> trimMessageList = new ArrayList<Integer>();
        	trimMessageList.addAll(messageList.subList(0, 10));
            Logger.i(TAG, "reloadRcseOnScroll() O2Otag is  " + tag + " trimMessageList: " + trimMessageList);
            PluginController.obtainMessage(ChatController.EVENT_RELOAD_MESSAGE,
            		trimMessageList).sendToTarget();
            //messageList = (ArrayList<Integer>) messageList.subList(30, messageList.size()-1);
            messageList.removeAll(trimMessageList);
            PluginUtils.O2OMap.put(tag, messageList);
            Logger.i(TAG, "New Message List List " + tag + " messageList: " + messageList);
        
        }
    	else if(messageList !=null && size > 0 && size <= 10)
        {
    		ArrayList<Integer> trimMessageList = new ArrayList<Integer>();
        	trimMessageList.addAll(messageList.subList(0, size -1));
         	Logger.i(TAG, "reloadRcseOnScroll O2Otag is " + tag  + "size is " + size + "trimmessageList: " + trimMessageList);
            PluginController.obtainMessage(ChatController.EVENT_RELOAD_MESSAGE,
            		trimMessageList).sendToTarget();
            messageList.clear();
            VISIBLE_ITEM_MAP.remove(tag);
            PluginUtils.O2OMap.put(tag, messageList);  
        }    
    }

    @Override
    public void exitFromChatMode(String number) {
        Logger.d(TAG, "exitFromChatMode() entry, number " + number);
        if(PluginUtils.getMessagingMode() == 0)
        {
        	if (number.startsWith(IpMessageConsts.JOYN_START)) {
        		number = number.substring(4);
        	}
        }
        Message controllerMessage = PluginController.obtainMessage(
                ChatController.EVENT_HIDE_WINDOW, PhoneUtils
                        .formatNumberToInternational(number));
        controllerMessage.sendToTarget();
        super.exitFromChatMode(number);
    }
}
