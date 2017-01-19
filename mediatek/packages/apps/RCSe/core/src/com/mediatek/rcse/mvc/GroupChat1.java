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

package com.mediatek.rcse.mvc;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.sql.Blob;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.EmptyStackException;
import java.util.HashMap;
import java.util.List;
import java.util.Stack;
import java.util.TimerTask;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.HashSet;
import java.util.Set;
import java.util.Iterator;

import org.gsma.joyn.JoynServiceException;
import org.gsma.joyn.JoynServiceNotAvailableException;
import org.gsma.joyn.capability.Capabilities;
import org.gsma.joyn.capability.CapabilitiesListener;
import org.gsma.joyn.capability.CapabilityService;
import org.gsma.joyn.chat.Chat;
import org.gsma.joyn.chat.ChatListener;
import org.gsma.joyn.chat.ChatLog;
import org.gsma.joyn.chat.ChatMessage;
import org.gsma.joyn.chat.ChatService;
import org.gsma.joyn.chat.ChatServiceConfiguration;
import org.gsma.joyn.chat.GeolocMessage;
import org.gsma.joyn.chat.GroupChat;
import org.gsma.joyn.chat.GroupChatListener;
import org.gsma.joyn.chat.IChat;
import org.gsma.joyn.chat.IChatListener;
import org.gsma.joyn.chat.IChatService;
import org.gsma.joyn.chat.IGroupChat;
import org.gsma.joyn.chat.IGroupChatListener;
import org.gsma.joyn.chat.INewChatListener;
import org.gsma.joyn.chat.NewChatListener;
import org.gsma.joyn.ft.FileTransfer;
import org.gsma.joyn.ft.FileTransferService;
import org.gsma.joyn.ft.FileTransferIntent;
import org.gsma.joyn.ft.FileTransferListener;
import org.gsma.joyn.ft.FileTransferServiceConfiguration;
import org.gsma.joyn.ft.IFileTransfer;
import org.gsma.joyn.ft.IFileTransferListener;
import org.gsma.joyn.ft.INewFileTransferListener;
import org.gsma.joyn.ft.NewFileTransferListener;




import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
import android.widget.Toast;

import com.mediatek.rcse.interfaces.ChatModel.IChat1;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
//import com.mediatek.rcse.api.CapabilityApi;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.api.RegistrationApi;
import com.mediatek.rcse.fragments.ChatFragment;
import com.mediatek.rcse.interfaces.ChatModel.IChatMessage;
import com.mediatek.rcse.interfaces.ChatView;
import com.mediatek.rcse.interfaces.ChatView.IChatEventInformation.Information;
import com.mediatek.rcse.interfaces.ChatView.IChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IGroupChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IOne2OneChatWindow;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage.Status;
import com.mediatek.rcse.mvc.ModelImpl.ChatEventStruct;
import com.mediatek.rcse.mvc.ModelImpl.ChatImpl;
import com.mediatek.rcse.mvc.ModelImpl.ChatMessageReceived;
import com.mediatek.rcse.mvc.ModelImpl.ChatMessageSent;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.mvc.ModelImpl.SentFileTransfer;
import com.mediatek.rcse.mvc.ModelImpl.ChatImpl.ReceiveFileTransfer;


import com.mediatek.rcse.api.INetworkConnectivity;
import com.mediatek.rcse.api.NetworkConnectivityApi;


import com.mediatek.rcse.provider.RichMessagingDataProvider;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.RcsNotification;
import com.mediatek.rcse.service.Utils;
import com.orangelabs.rcs.R;
import com.orangelabs.rcs.core.ims.service.im.chat.ChatUtils;
import com.orangelabs.rcs.core.ims.service.im.chat.GroupChatInfo;
import com.orangelabs.rcs.core.ims.service.im.chat.event.User;
import com.orangelabs.rcs.core.ims.service.im.chat.imdn.ImdnDocument;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.messaging.ChatData;
import com.orangelabs.rcs.provider.messaging.RichMessagingHistory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.utils.PhoneUtils;         	








/**
 * The implement of group chat. It provides session management, session event
 * handling, and message sending.
 */
public class GroupChat1 extends ChatImpl {
    public static final String TAG = "M0CF GroupChat1";
    private ArrayList<ChatMessage> mReceivedMessagesDelayed = new ArrayList<ChatMessage>();
    private ArrayList<IChatMessage> mAllMessages = new ArrayList<IChatMessage>();
    private GroupChatParticipants mCurrentParticipants = new GroupChatParticipants();
    public static final int TIME_OUT = RcsSettings.getInstance().getRingingPeriod() * 1000 + 5000;
    private boolean mIsRegistered = false;
    private NetworkConnectivityListener mNetworkListener = null;
    private Stack<GroupChatImplInfo> mGroupChatImplStack = new Stack<GroupChatImplInfo>();
    private HashMap<String, Integer> mMessagesToSendDelayed = new HashMap<String, Integer>();
    private Handler mUiHandler = new GroupChatHandler(Looper.getMainLooper());
    private final Context mContext;
    private ChatService mMessagingApi;
    private CapabilityService mCapabilityApi;
    //private final EventsLogApi mEventsLogApi;  //TODo
    private RegistrationApi mRegistrationApi; 
	private NetworkConnectivityApi mNetworkConnectivityApi;
    private String mChatId = null;
    private String mChatTitle = "";
    private boolean mInvite = false;
	private FileTransferService  fileService = null;
	FileTransferDeliveryListener deliveryListener = null;
	private String inviteContact = null;
   
    public boolean ismInvite() {
		return mInvite;
	}

	public void setmInvite(boolean mInvite) {
		this.mInvite = mInvite;
	}

    /**
     * M: managing extra local chat participants that are 
     * not present in the invitation for sending them invite request.@{
     */
    private ArrayList<Participant> mExtraLocalparticipants = null;
    private boolean isResendGrpInvite = false;
    /**
	 * @}
	 */ 
    

    public void setmChatId(String mChatId) {
        this.mChatId = mChatId;
    }
    
    /**
     * Generate an sent file transfer instance in a specific chat window
     * 
     * @param filePath The path of the file to be sent
     * @return A sent file transfer instance in a specific chat window
     */
    public SentFileTransfer generateSentFileTransfer(IChat1 chat,String filePath, Object fileTransferTag) {

    	
    	List<String> participants = new ArrayList<String>();
    	for(ParticipantInfo p :mCurrentParticipants.mParticipants)
    	{
    		participants.add(p.getContact());
    	}
    		
    	 return new SentFileTransfer(mTag, chat, (IGroupChatWindow) mChatWindow, filePath, participants,
                 fileTransferTag);
    }

	public GroupChat getGroupChatImpl(){
		Logger.v(TAG, "GroupChat1 getGroupChat1");
		try {
            GroupChatImplInfo sessionInfo = mGroupChatImplStack.peek();
            GroupChat currentSession = sessionInfo.getGroupchatImpl();
            return currentSession;
        } catch (EmptyStackException e) {
            Logger.e(TAG, "sendMessage() EmptyStackException");
            e.printStackTrace();
        }
		return null;
	}
    
    
    public void addGroupSubject(String title, int sendInvite1)
    {    
        mChatTitle = title;
    	((IGroupChatWindow)mChatWindow).addgroupSubject (title);
    	if (mMessagingApi == null) {
            mMessagingApi = ApiManager.getInstance().getChatApi();
        }
        if (mMessagingApi == null) {
            Logger.e(TAG, "sendMessage(), mMessagingApi is null");
            return;
        }
    	sendInvite(getGroupSubject(), 0);
    };
    
    public void addGroupSubjectFromInvite(String title)
    {    
        mChatTitle = title;
        ((IGroupChatWindow)mChatWindow).addgroupSubject (title);        
    }

	public  String getGroupSubject()
    {
       return mChatTitle;
    }

    /**
     * Handler to deal with some common function.
     */
    private class GroupChatHandler extends Handler {
        /**
         * Update send button.
         */
        public static final int UPDATE_SEND_BUTTON = 1;
        /**
         * No IM capability.
         */
        public static final int IM_FAIL_TOAST = 2;

        private GroupChatHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case UPDATE_SEND_BUTTON:
                    GroupChatImplStatus status = GroupChatImplStatus.UNKNOWN;
                    try {
                        GroupChatImplInfo info = mGroupChatImplStack.peek();
                        status = info.getGroupChatImplStatus();
                    } catch (EmptyStackException e) {
                        e.printStackTrace();
                    }
                    Logger.d(TAG, "handleMessage(), status: " + status);
                    if (!mIsRegistered) {
                        ((IGroupChatWindow) mChatWindow)
                                .updateChatStatus(Utils.GROUP_STATUS_UNAVIALABLE);
                    } else if (status == GroupChatImplStatus.REJOINING
                            || status == GroupChatImplStatus.AUTO_REJOIN) {
                        ((IGroupChatWindow) mChatWindow)
                                .updateChatStatus(Utils.GROUP_STATUS_REJOINING);
                    } else if (status == GroupChatImplStatus.RESTARTING) {
                        ((IGroupChatWindow) mChatWindow)
                                .updateChatStatus(Utils.GROUP_STATUS_RESTARTING);
                    } else if (status == GroupChatImplStatus.TERMINATED) {
                        ((IGroupChatWindow) mChatWindow)
                                .updateChatStatus(Utils.GROUP_STATUS_TERMINATED);
                    } else {
                        ((IGroupChatWindow) mChatWindow)
                                .updateChatStatus(Utils.GROUP_STATUS_CANSENDMSG);
                    }
                    break;
                case IM_FAIL_TOAST:
                    Toast.makeText(mContext,
                            mContext.getString(R.string.im_capabillity_failed, msg.obj),
                            Toast.LENGTH_LONG).show();
                    break;
                default:
                    break;
            }
        }
    }

    /**
     * Session info.
     */
    public static class GroupChatImplInfo {
        private GroupChat mGroupChatImpl;
        private GroupChatImplStatus mGroupChatImplStatus = GroupChatImplStatus.UNKNOWN;

        public GroupChatImplInfo(GroupChat groupImpl, GroupChatImplStatus status) {
            mGroupChatImpl = groupImpl;
            mGroupChatImplStatus = status;
        }

        /**
         * Set chat session.
         * 
         * @param session The chat session.
         */
        public void setSession(GroupChat groupImpl) {
            mGroupChatImpl = groupImpl;
        }

        /**
         * Get chat session.
         * 
         * @return Chat session.
         */
        public GroupChat getGroupchatImpl() {
            return mGroupChatImpl;
        }

        /**
         * Set session status.
         * 
         * @param status Session status.
         */
        public void setSessionStatus(GroupChatImplStatus status) {
            mGroupChatImplStatus = status;
        }

        /**
         * Get session status.
         * 
         * @return Session status.
         */
        public GroupChatImplStatus getGroupChatImplStatus() {
            return mGroupChatImplStatus;
        }

        /**
         * To string.
         */
        @Override
        public String toString() {
            return "session= " + mGroupChatImpl + " mGroupChatImplStatus= " + mGroupChatImplStatus;
        }
    }

    public class GroupChatParticipants {

        private class TimerOutTimer extends TimerTask {
            ParticipantInfo mTimerParticipantInfo = null;

            TimerOutTimer(ParticipantInfo info) {
                mTimerParticipantInfo = info;
            }

            @Override
            public void run() {
                if (mTimerParticipantInfo != null
                        && mTimerParticipantInfo.mState.equals(User.STATE_PENDING)) {
                    ((IGroupChatWindow) mChatWindow).updateParticipants(mParticipants);
                }
                cancel();
            }
        }

        private CopyOnWriteArrayList<ParticipantInfo> mParticipants = new CopyOnWriteArrayList<ParticipantInfo>();
        private CopyOnWriteArrayList<TimerOutTimer> mTimerOutTimers = new CopyOnWriteArrayList<TimerOutTimer>();

        private TimerOutTimer findTimerOutTimer(ParticipantInfo info) {
            for (TimerOutTimer timer : mTimerOutTimers) {
                if (timer.mTimerParticipantInfo.mParticipant.equals(info.mParticipant)) {
                    return timer;
                }
            }
            return null;
        }

        private void initicalGroupChatParticipant(List<Participant> participants) {
            if (null != participants && 0 != participants.size()) {
                for (Participant participant : participants) {
                    mParticipants.add(new ParticipantInfo(participant, User.STATE_PENDING));
                }
            }
        }

        private void addAll(List<Participant> toAdd) {
            if (null != toAdd && 0 != toAdd.size()) {
                for (Participant participant : toAdd) {
                    ParticipantInfo info = new ParticipantInfo(participant, User.STATE_PENDING);
                    ModelImpl.TIMER.schedule(new TimerOutTimer(info), TIME_OUT);
                    mParticipants.add(info);
                }
            }
            ((IGroupChatWindow) mChatWindow).updateParticipants(mParticipants);
        }

        /**
         * Convert participant info to participant
         * 
         * @return group chat participant list
         */
        public ArrayList<Participant> convertToParticipants() {
            ArrayList<Participant> participants = new ArrayList<Participant>();
            for (ParticipantInfo info : mParticipants) {
                participants.add(info.mParticipant);
            }
            return participants;
        }

        /**
         * Convert participant info to number Return group chat participant
         * number list
         */
        public ArrayList<String> convertParticipantsToNumbers() {
            ArrayList<String> participants = new ArrayList<String>();
            for (ParticipantInfo info : mParticipants) {
                participants.add(info.mParticipant.getContact());
            }
            return participants;
        }

        private void add(Participant participant, String state) {
            ParticipantInfo info = new ParticipantInfo(participant, state);
            mParticipants.add(info);
        }

        private void remove(ParticipantInfo participantInfo) {
            mParticipants.remove(participantInfo);
        }

        /**
         * Update model data
         * 
         * @param contact Remote contacts
         * @param contactDisplayname Remote contacts display name
         * @param state Remote user state
         * @param toBeInvited Participants to be invited.
         * @return True if contact is to be invited.
         */
        public boolean updateParticipants(String contact, String contactDisplayname, String state,
                List<String> toBeInvited) {
            Logger.v(TAG, "updateParticipants(), contact: " + contact + " contactDisplayname: "
                    + contactDisplayname + " state: " + state);
            boolean result = false;
            boolean isNewAdded = true;
            String displayName = null;
            String loacalDisplayName = ContactsListManager.getInstance()
                    .getDisplayNameByPhoneNumber(contact);
            if (loacalDisplayName != null) {
                displayName = loacalDisplayName;
            } else if (contactDisplayname != null) {
                displayName = contactDisplayname;
            } else {
                displayName = contact;
            }
            Participant participantToBeUpdated = new Participant(contact, displayName);
            ParticipantInfo infoToBeUpdated = new ParticipantInfo(participantToBeUpdated, state);
            int size = mParticipants.size();
            for (int index = 0; index < size; index++) {
                ParticipantInfo info = mParticipants.get(index);
                Participant participant = info.mParticipant;
                if (null != participant) {
                    String remoteContact = participant.getContact();
                    if (null != remoteContact && remoteContact.equals(contact)) {
                        info.mState = state;
                        participantToBeUpdated = participant;
                        infoToBeUpdated = info;
                        isNewAdded = false;
                        timerUnSchedule(info);
                        break;
                    }
                }
            }
            if (toBeInvited.contains(contact)) {
            	boolean isAlreadyAdded = false;
            	
            	
            		for(int j =0 ; j<mParticipants.size();j++)
            		{
	            		if(contact.equalsIgnoreCase(mParticipants.get(j).mParticipant.getContact()))
	            		{
	            			isAlreadyAdded = true;
	                        Logger.v(TAG, "ALready Added, contact: " + contact );
	                        break;

	            		}
            		}
            	
            	
            	
                if(!isAlreadyAdded)
                {
                
                isNewAdded = true;
                result = true;
            }
            }
            if (User.STATE_DECLINED.equals(state)) {
                remove(infoToBeUpdated);
                ((IGroupChatWindow) mChatWindow).setIsComposing(false, participantToBeUpdated);
            } else if (User.STATE_DISCONNECTED.equals(state)) {
                // remove(infoToBeUpdated);
                ((IGroupChatWindow) mChatWindow).setIsComposing(false, participantToBeUpdated);
            } else if (User.STATE_DEPARTED.equals(state)) {
                remove(infoToBeUpdated);
                ((IGroupChatWindow) mChatWindow).setIsComposing(false, participantToBeUpdated);
                if (!isNewAdded) {
                    ChatEventStruct chatEventStruct = new ChatEventStruct(Information.LEFT,
                            contact, new Date());
                    ((IGroupChatWindow) mChatWindow).addChatEventInformation(chatEventStruct);
                }
            } else if (User.STATE_CONNECTED.equals(state)) {
                if (isNewAdded) {
                    add(participantToBeUpdated, state);
                    ChatEventStruct chatEventStruct = new ChatEventStruct(Information.JOIN,
                            contact, new Date());
                    ((IGroupChatWindow) mChatWindow).addChatEventInformation(chatEventStruct);
                }
            }
            ((IGroupChatWindow) mChatWindow).updateParticipants(mParticipants);
            return result;
        }

        /**
         * Rest All connected participants' statuses to be disconnected.
         */
        public void resetAllStatus() {
            Logger.v(TAG, "resetAllStatus()");
            for (ParticipantInfo info : mParticipants) {
                if (User.STATE_CONNECTED.equalsIgnoreCase(info.mState)) {
                    info.mState = User.STATE_DISCONNECTED;
                }
            }
            ((IGroupChatWindow) mChatWindow).updateParticipants(mParticipants);
        }

        /**
         * Timer schedule
         * 
         * @param ParticipantInfo
         */
        public void timerSchedule(ParticipantInfo info) {
            Logger.d(TAG, "timerSchedule(): info = " + info);
            if (info != null) {
                TimerOutTimer timerTask = findTimerOutTimer(info);
                Logger.v(TAG, "timerSchedule(): timerTask = " + timerTask);
                if (info.mState.equals(User.STATE_PENDING)) {
                    if (timerTask == null) {
                        timerTask = new TimerOutTimer(info);
                        mTimerOutTimers.add(timerTask);
                        ModelImpl.TIMER.schedule(timerTask, TIME_OUT);
                    }
                }
            }
        }

        /**
         * Timer stop schedule
         * 
         * @param ParticipantInfo
         */
        public void timerUnSchedule(ParticipantInfo info) {
            Logger.d(TAG, "timerUnSchedule(): info = " + info);
            if (info != null) {
                TimerOutTimer timerTask = findTimerOutTimer(info);
                if (timerTask != null) {
                    timerTask.cancel();
                    mTimerOutTimers.remove(timerTask);
                }
            }
        }
    }

    /**
     * Get GroupChat Participant list
     * 
     * @return ParticipantInfo
     */
    public List<ParticipantInfo> getParticipantInfos() {
        return mCurrentParticipants.mParticipants;
    }

	
    public GroupChat1(ModelImpl modelImpl, IGroupChatWindow chatWindow,
            List<Participant> participants, Object tag) {
        modelImpl.super(tag);
        Logger.v(TAG, "GroupChat1 constructor entry");
        mCurrentParticipants.initicalGroupChatParticipant(participants);
        mChatWindow = chatWindow;
        mContext = ApiManager.getInstance().getContext();
        //mEventsLogApi = new EventsLogApi(mContext);  //TODo check this
        mCapabilityApi = ApiManager.getInstance().getCapabilityApi();
        mMessagingApi = ApiManager.getInstance().getChatApi();
        mRegistrationApi = ApiManager.getInstance().getRegistrationApi();
        mNetworkConnectivityApi = ApiManager.getInstance().getNetworkConnectivityApi(); 
		mFileTransferController = new FileTransferController();
		try{
			if(fileService != null){
					if(ApiManager.getInstance() != null){
					fileService = ApiManager.getInstance().getFileTransferApi();
					if(fileService != null){
						deliveryListener =  new FileTransferDeliveryListener();
						fileService.addNewFileTransferListener(deliveryListener);
					}
				}
			}
		}
		catch(JoynServiceException e){
			Logger.d(TAG, "MOCFF GroupChat1 handleSendFileTransferInvitation() exception");
		}
        Logger.v(TAG, "GroupChat1 constructor exit");
    }

 
    protected void reloadFileTransfer(final FileStruct fileStruct, final int transferDirection, final int messageStatus) {
        Runnable worker = new Runnable() {
            @Override
            public void run() {
                Logger.d(TAG, "M0CFF reloadFileTransfer()->run() entry, file transfer tag: " + fileStruct.mFileTransferTag
                        + "file transfer path: " + fileStruct.mFilePath + " , transferType: " + transferDirection
                        + ", messageStatus: " + messageStatus);
                ChatView.IFileTransfer fileTransfer = null;
                if (FileTransfer.Direction.INCOMING == transferDirection) { //TODo check this
                    if(messageStatus == FileTransfer.State.DISPLAYED) {
                        fileTransfer = ((IGroupChatWindow) mChatWindow).addReceivedFileTransfer(fileStruct, false,true);
                    } else {
                        fileTransfer = ((IGroupChatWindow) mChatWindow).addReceivedFileTransfer(fileStruct, false,false);
                    }
                    if (fileTransfer != null) {
                        if (fileStruct.mFilePath == null) {
                            if((mReceiveFileTransferManager != null) &&(messageStatus == 0 || messageStatus == 1)) //TODo check this
                            {   
                                if(mReceiveFileTransferManager.findFileTransferByTag(fileStruct.mFileTransferTag) == null) 
                                {
                                	fileTransfer.setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.FAILED);
                                }
                            	else
                                {
                                Logger.d(TAG,
                                        "M0CFF reloadFileTransfer(), file path is null, set status Transferring!");                           
                                mReceiveFileTransferManager.findFileTransferByTag(fileStruct.mFileTransferTag).mFileTransfer = fileTransfer;                               
                                if(fileTransferStatus == com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.TRANSFERING)
                                fileTransfer
                                .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.TRANSFERING);
                                else if(fileTransferStatus == com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.WAITING)
                                    fileTransfer
                                    .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.WAITING);
                                else if(fileTransferStatus == com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.REJECTED)
                                    fileTransfer
                                    .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.REJECTED);
                               }
                            }
                            else
                            {
                            Logger.d(TAG,
                                    "reloadFileTransfer(), file path is null, set status failed!");
                            fileTransfer
                                    .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.FAILED);
                            }
                        } else {
                            Logger.d(TAG, "reloadFileTransfer(), set status finished!");
                            fileTransfer
                                    .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.FINISHED);
                        }
                    } else {
                        if(mReceiveFileTransferManager!=null)
                        {
                            fileTransfer = mReceiveFileTransferManager.findFileTransferByTag(fileStruct.mFileTransferTag).mFileTransfer;
                            fileTransfer
                            .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.WAITING);
                        Logger.e(TAG, "reloadFileTransfer(), fileTransfer is null!");
                    }
                    }
                } else if (FileTransfer.Direction.OUTGOING == transferDirection) {  //TODo check this
                    fileTransfer = ((IGroupChatWindow) mChatWindow).addSentFileTransfer(fileStruct);
                    Logger.d(TAG, "reloadFileTransfer(), messageStatus = " + messageStatus);
                    if (fileTransfer != null && ( messageStatus == 4 || messageStatus == 7 || messageStatus == 8)) { //TODo check this
                        fileTransfer.setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.FINISHED);
                    }
                    /* We can have a single condition which sets Fail in all cases other then terminated */
                    else 
                        fileTransfer.setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.FAILED);
                }
            }
        };
        mWorkerHandler.post(worker);
    }


	/**
	 * File transfer session event listener
	 */
	private class FileTransferDeliveryListener extends NewFileTransferListener {
		private static final String TAG = "M0CFF FileTransferDeliveryListener";

		/**
		 * Callback called when the file transfer is started
		 */
		public	void onNewFileTransfer(String transferId){
			Logger.d(TAG, "onNewFileTransfer() transfer id is  " + transferId);
		}	

		/**
		 * Callback called when the file transfer is delivered
		 */
		public	void onReportFileDelivered(String transferId){
			Logger.d(TAG, "onReportFileDelivered() transfer id is  " + transferId);
		}	

		/**
		 * Callback called when the file transfer is delivered
		 */
		public	void onReportFileDisplayed(String transferId){
			Logger.w(TAG, "onReportFileDisplayed() transfer id is  " + transferId);
		}	


		/**
		* Callback called when the file transfer is displayed
		 */
		public	void onFileDisplayedReport(String transferId,String contact){
			Logger.w(TAG, "onReportFileDisplayed() transfer id is  " + transferId + "Contact: " + contact);			
   			com.mediatek.rcse.fragments.GroupChatFragment.SentFileTransfer msg = (com.mediatek.rcse.fragments.GroupChatFragment.SentFileTransfer) mChatWindow.getSentChatMessage(transferId);
	        if (msg != null) {
	            msg.setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.FINISHED,contact); //TODo Contact info required from stack
	        }
		}	

		/**
		 * Callback called when the file transfer is delivered
		 */
		public	void onFileDeliveredReport(String transferId,String contact){
			Logger.w(TAG, "onReportFileDelivered() transfer id is  " + transferId + "Contact: " + contact);		
			com.mediatek.rcse.fragments.GroupChatFragment.SentFileTransfer msg = (com.mediatek.rcse.fragments.GroupChatFragment.SentFileTransfer) mChatWindow.getSentChatMessage(transferId);
	       if (msg != null) {
	            msg.setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.FINISHED,contact); //TODo Contact info required from stack
	        }
		}	


		/**
		* Callback called when the file transfer is displayed
		 */
		public	void onReportFileDisplayed(String transferId,String contact){
			Logger.w(TAG, "onReportFileDisplayed() transfer id is  " + transferId + "Contact: " + contact);
		}	
	}

    @Override
    public void setChatWindow(IChatWindow chatWindow) {
        super.setChatWindow(chatWindow);
        Logger.d(TAG, "setChatWindow() entry");
        init();
        Logger.d(TAG, "setChatWindow() exit");
    }

    /**
     * Change session status.
     * 
     * @param info The old info.
     * @param status The new status.
     */
    private void changeGroupChatImplStatus(GroupChatImplInfo info, GroupChatImplStatus status) {
        Logger.d(TAG, "changeGroupChatImplStatus() entry, info: " + info + " status: " + status);
        int location = mGroupChatImplStack.indexOf(info);
        GroupChatImplStatus oldStatus = info.getGroupChatImplStatus();
        Logger.d(TAG, "changeGroupChatImplStatus() oldStatus: " + oldStatus);
        if (oldStatus == GroupChatImplStatus.TERMINATED) {
            return;
        }
        if (-1 != location) {
            mGroupChatImplStack.get(location).setSessionStatus(status);
        }
        String participants = "";
        List<ParticipantInfo> ListParticipant = mCurrentParticipants.mParticipants;

        for(ParticipantInfo currentPartc : ListParticipant){
        participants += currentPartc.mParticipant.getContact() + ";"; 

        }        
        GroupChat groupChatImpl = info.getGroupchatImpl();        
        try {
            if(info.mGroupChatImplStatus == GroupChatImplStatus.TERMINATED )
            //RichMessaging.getInstance().addChatSessionTerminationOnQuitGroup(mChatId, groupChatImpl.getSessionID(), participants); //TODo check this

        Logger.d(TAG, "changeGroupChatImplStatus() exit, location: " + location);
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
            Logger.d(TAG, "changeGroupChatImplStatus() exception " + location);
        }
    }

    private void init() {
        Logger.d(TAG, "init() entry, mNetworkListener: " + mNetworkListener);
        if (mNetworkListener == null) { 
            if (mRegistrationApi == null) {
                mRegistrationApi = ApiManager.getInstance().getRegistrationApi();
            }
            if (mNetworkConnectivityApi == null) {
                mNetworkConnectivityApi = ApiManager.getInstance().getNetworkConnectivityApi();
            }
            try {
                if (mNetworkConnectivityApi != null) {
                    mNetworkListener = new NetworkConnectivityListener();
                    mNetworkConnectivityApi.addNetworkConnectivityListener(mNetworkListener);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
            if (mRegistrationApi != null) {
                mIsRegistered = mRegistrationApi.isRegistered();
            }
            Message msg = Message.obtain();
            msg.what = GroupChatHandler.UPDATE_SEND_BUTTON;
            mUiHandler.sendMessage(msg);
        }
        Logger.d(TAG, "init() exit, mNetworkConnectivityApi: " + mNetworkConnectivityApi
                + " mRegistrationApi: " + mRegistrationApi + " mIsRegistered: " + mIsRegistered);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Logger.d(TAG, "onDestroy() entry");
        quitGroup();
        mAllMessages.clear();
        mGroupChatImplStack.clear();
        mReceivedMessagesDelayed.clear();
        clearRestoredMessages();
		if(fileService != null){
			Logger.d(TAG, "MOCFF onDestroy() removeNewFileTransferListener");
			try {
				fileService.removeNewFileTransferListener(deliveryListener);
			} catch (Exception e) {
		        e.printStackTrace();
		    }
			
			deliveryListener = null;
			fileService = null;
		}
        if (mNetworkConnectivityApi != null) {
            try {
				Logger.w(TAG, "onDestroy() remove file listener");
                mNetworkConnectivityApi.removeNetworkConnectivityListener(mNetworkListener);
                mNetworkListener = null;
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        Logger.d(TAG, "onDestroy() exit, mNetworkConnectivityApi: " + mNetworkConnectivityApi);
    }

    protected void onQuit() {
        Logger.d(TAG, "onQuit() entry");
        quitGroup();
		if (mNetworkConnectivityApi != null) {
            try {
                mNetworkConnectivityApi.removeNetworkConnectivityListener(mNetworkListener);
                mNetworkListener = null;
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        Logger.d(TAG, "onQuit() exit, mNetworkConnectivityApi: " + mNetworkConnectivityApi);
        Logger.d(TAG, "onQuit() exit");
    }

    protected void clearExtraMessageGroup() {
    	Logger.d(TAG, "clearExtraMessageGroup() entry");
    	((IGroupChatWindow) mChatWindow).clearExtraMessage();
    }

    protected void initiateExportChat() {
        Logger.d(TAG, "initiateExportChat() entry");
        String contact = "";
        Cursor cursor = null;
        String chatId = getChatId();
        String subject = getGroupSubject();
        String messageList = "GroupChat subject " + subject + "\n";
        try {
            
            cursor = RichMessagingDataProvider.getInstance().getAllMessageforGroupChat(chatId);
            Logger.d(TAG, "initiateExportChat() cursor count = " + cursor.getCount());
			if (cursor != null && cursor.moveToFirst()) {
				do {
					byte[] messageBlob = cursor.getBlob(cursor
							.getColumnIndex(ChatLog.Message.BODY));
					String message = new String(messageBlob);
					contact = cursor.getString(cursor
							.getColumnIndex(ChatLog.Message.CONTACT_NUMBER));
					String displayName = ContactsListManager.getInstance().getDisplayNameByPhoneNumber(contact);
					Logger.d(TAG, "initiateExportChat() contact is " + contact
							+ " message is " + message);
                    messageList = messageList + displayName+ "(" + contact + "):" + message + "\n";
				} while (cursor.moveToNext());
            } else {
                Logger.d(TAG, "initiateExportChat() empty cursor");              
            }
        } finally {
            if (null != cursor) {
                cursor.close();
            }
        }
		if (cursor != null && !messageList.equals("")) {
			String fileName = Environment.getExternalStorageDirectory()
					.getPath() + "/" + "Joyn" + "/" + subject + "_chat" + ".txt";
			Logger.d(TAG, "initiateExportChat() starting to create file with filename " + fileName);
			File chatContentFile = null;
			try {
				chatContentFile = new File(fileName);
				chatContentFile.createNewFile();
				FileOutputStream fOut = new FileOutputStream(chatContentFile);
				OutputStreamWriter myOutWriter = new OutputStreamWriter(fOut);
				myOutWriter.append(messageList);
				myOutWriter.close();
				fOut.close();
			} catch (FileNotFoundException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			if (chatContentFile != null) {
				Uri uri = Uri.fromFile(chatContentFile);
				Intent shareIntent = new Intent();
				shareIntent.setAction(Intent.ACTION_SEND);
				shareIntent.putExtra(Intent.EXTRA_STREAM, uri);
				shareIntent.setType("text/plain");
				shareIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
				AndroidFactory.getApplicationContext().startActivity(
						shareIntent);
			}
		}
        Logger.d(TAG, "initiateExportChat() exit");
    }

    private void quitGroup() {
        Logger.d(TAG, "quitGroup() entry, mGroupChatImplStack size: " + mGroupChatImplStack.size());
        GroupChatImplStatus status = GroupChatImplStatus.TERMINATED;
        try {
            for (GroupChatImplInfo info : mGroupChatImplStack) {
                changeGroupChatImplStatus(info, status);
                GroupChat mGroupChatImpl = info.getGroupchatImpl();
                if(mGroupChatImpl!=null)
                mGroupChatImpl.quitConversation();
            }
        } catch (EmptyStackException e) {
            Logger.e(TAG, "quitGroup() EmptyStackException");
            e.printStackTrace();
        } catch (JoynServiceException e) {
            Logger.e(TAG, "quitGroup() JoynServiceException");
            e.printStackTrace();
        }
        Logger.d(TAG, "quitGroup() exit");
    }

    @Override
    protected void onResume() {
        super.onResume();
        Logger.d(TAG, "onResume() entry, mRegistrationApi: " + mRegistrationApi);
        if (mRegistrationApi == null) {
            mRegistrationApi = ApiManager.getInstance().getRegistrationApi();
        }
        if (mRegistrationApi != null) {
            mIsRegistered = mRegistrationApi.isRegistered();
        }
        Message msg = Message.obtain();
        msg.what = GroupChatHandler.UPDATE_SEND_BUTTON;
        mUiHandler.sendMessage(msg);
        RcsNotification.getInstance().cancelReceiveMessageNotification(mTag);
        Logger.d(TAG, "onResume() exit, mIsRegistered: " + mIsRegistered);
    }

    /**
     * Get all the IM chat history
     * 
     * @return All messages
     */
    public ArrayList<IChatMessage> getAllMessages() {
        return mAllMessages;
    }

    /**
     * Clear the group chat history of this associated participant.
     * 
     * @return True if success, else False.
     */
    public boolean clearGroupHistory() {
        Logger.d(TAG, "clearGroupHistory() entry, mGroupChatImplStack size: " + mGroupChatImplStack.size());
        boolean success = false;
        //delete the group caht for this chatID
          if(RichMessagingDataProvider.getInstance() == null)
          {
          	RichMessagingDataProvider.createInstance(mContext);
          }
        try {
            for (GroupChatImplInfo info : mGroupChatImplStack) {
                GroupChat groupChatImpl = info.getGroupchatImpl();
                // this function is called in worker thread, so it's safe to
                // do I/O operation.
                if(groupChatImpl!=null){
               		 RichMessagingDataProvider.getInstance().deleteImSessionEntry(groupChatImpl.getChatId()); 
                }
            }
            RichMessagingDataProvider.getInstance().deleteGroupChat(mChatId);
			RichMessagingDataProvider.getInstance().deleteGroupFt(mChatId);
            success = true;
        } catch (EmptyStackException e) {
            Logger.e(TAG, "clearGroupHistory() EmptyStackException"); 
            e.printStackTrace();
        } 
		catch (JoynServiceException e) {
            Logger.e(TAG, "clearGroupHistory() EmptyStackException"); 
            e.printStackTrace();
        } 
        clearChatWindowAndList();
        Logger.d(TAG, "clearGroupHistory() exit, success: " + success);
        return success;
    }

    public void clearGroupHistoryMemory() {
        Logger.d(TAG, "clearGroupHistoryMemory() entry");
        clearChatWindowAndList();
    }

    /**
     * Get chat id of group chat.
     * 
     * @return Chat id.
     */
    public String getChatId() {
        return mChatId;
    }

    @Override
    public void sendMessage(String content, int messageTag) {
        Logger.d(TAG, "sendMessage() entry, content: " + content + " messageTag: " + messageTag);
        if (mMessagingApi == null) {
            mMessagingApi = ApiManager.getInstance().getChatApi();
        }
        if (mMessagingApi == null) {
            Logger.e(TAG, "sendMessage(), mMessagingApi is null");
            return;
        }
        GroupChat currentSession = null;
        GroupChatImplStatus status = GroupChatImplStatus.UNKNOWN;
        try {
            GroupChatImplInfo sessionInfo = mGroupChatImplStack.peek();
            currentSession = sessionInfo.getGroupchatImpl();
            status = sessionInfo.getGroupChatImplStatus();
            
        } catch (EmptyStackException e) {
            Logger.e(TAG, "sendMessage() EmptyStackException");
            e.printStackTrace();
        }
        Logger.d(TAG, "sendMessage() currentSession: " + currentSession + " status: " + status
                + " mChatId: " + mChatId);
        switch (status) {
            case UNKNOWN:
                sendInvite(content , messageTag);
                break;
            case MANULLY_REJOIN:
                rejoinGroup(content, mChatId, messageTag);
                break;
            case MANULLY_RESTART:
                restartGroup(content, mChatId, messageTag);
                break;
            case ACTIVE:
                sendMsrpMessage(currentSession, content, messageTag);
                break;
            case INVITING:
            default:
                restoreMessages(content, messageTag);
                break;
        }
        Logger.d(TAG, "sendMessage() exit, content: " + content);
    }

    private void restoreMessages(final String message, final int messageTag) {
            Logger.d(TAG, "restoreMessages() message: " + message + " messageTag: "
                        + messageTag);
        mWorkerHandler.post(new Runnable() {
            @Override
            public void run() {
                if (message != null) {                   
                    ChatMessage msg = new ChatMessage("-1", Utils.DEFAULT_REMOTE, message, new Date(), true,RcsSettings.getInstance().getJoynUserAlias());                  
                    synchronized(this)
                    {
                        // TODO This is a hack because above operation is database, need to find better approach
                        try {
                                          Thread.sleep(500);
                                    } catch (InterruptedException e) {
                                          
                                          e.printStackTrace();
                                    }
                    }
                              
                              mChatWindow.addSentMessage(msg, messageTag);
                    mMessagesToSendDelayed.put(message, messageTag);
                    //mAllMessages.add(new ChatMessageSent(msg));
                }
            }
        });
    }


    private void clearRestoredMessages() {
        mWorkerHandler.post(new Runnable() {
            @Override
            public void run() {
                mMessagesToSendDelayed.clear();
            }
        });
    }

    private void sendInvite(String subject , int messageTag) {
        Logger.d(TAG, "sendInvite() entry, content: " + subject + " messageTag: " + messageTag);
        if (mCapabilityApi == null) {
            mCapabilityApi = ApiManager.getInstance().getCapabilityApi();
        }
        if (mCapabilityApi == null) {
            Logger.e(TAG, "sendInvite() mCapabilityApi is null");
            return;
        }
        ChatMessage message = null;
        Set<String> pList = convertParticipantInfosToContactsSet(mCurrentParticipants.mParticipants);
        ArrayList<ParticipantInfo> nonImList = new ArrayList<ParticipantInfo>();
        ArrayList<ParticipantInfo> participantInfos = new ArrayList<ParticipantInfo>(
                mCurrentParticipants.mParticipants);
        int size = participantInfos.size();
        for (int i = 0; i < size; i++) {
            ParticipantInfo participant = participantInfos.get(i);
            String contact = participant.getContact();
			Capabilities remoteCapablities = null;
			try{
            	 remoteCapablities = mCapabilityApi.getContactCapabilities(contact);
				 mCapabilityApi.requestContactCapabilities(contact);
			}
			catch(JoynServiceException e){
				Logger.d(TAG, "sendInvite() getContactCapabilities JoynServiceException");
			}
            if (remoteCapablities != null) {
                boolean imSupported = remoteCapablities.isImSessionSupported();
                Logger.d(TAG, "sendInvite() contact: " + contact + " imSupported: " + imSupported);
                if (!imSupported) {
                    //mCurrentParticipants.timerUnSchedule(participant);
                    //nonImList.add(participant);
                    //pList.remove(contact);
                }
            } else {
                Logger.e(TAG, "sendInvite() remoteCapabilites is null contact: " + contact);
            }
        }
        int nonImsize = nonImList.size();
        Logger.d(TAG, "sendInvite() nonImsize: " + nonImsize);
        if (nonImsize > 0) {
            mCurrentParticipants.mParticipants.removeAll(nonImList);
            final String displayname = ChatFragment.getParticipantsName(nonImList
                    .toArray(new ParticipantInfo[1]));
            ((IGroupChatWindow) mChatWindow).updateParticipants(mCurrentParticipants.mParticipants);
            Message msg = Message.obtain();
            msg.what = GroupChatHandler.IM_FAIL_TOAST;
            msg.obj = displayname;
            mUiHandler.sendMessage(msg);
        }
        int inviterSize = pList.size();
        Logger.d(TAG, "sendInvite() inviterSize: " + inviterSize);
        if (inviterSize > 0) {
            try {
				GroupChatModelListener listener = new GroupChatModelListener(pList);
                GroupChat groupChatImpl = mMessagingApi.initiateGroupChat(pList,subject,listener);   
				GroupChatImplInfo sessionInfo = new GroupChatImplInfo(groupChatImpl, GroupChatImplStatus.INVITING);
				listener.setGroupChatImplInfo(sessionInfo);
                // caused by different processes.
                Logger.d(TAG, "sendInvite() groupChatImpl: " + groupChatImpl);
                if (groupChatImpl != null) {
                    mCurrentGroupChatImpl.set(groupChatImpl);
                    mChatId = groupChatImpl.getChatId();
                   // message = groupChatImpl.getFirstMessage();
                    Message msg = Message.obtain();
                    msg.what = GroupChatHandler.UPDATE_SEND_BUTTON;
                    mUiHandler.sendMessage(msg);
                    
                    Logger.w(TAG, "MOCFF sendInvite0() addNewFileTransferListener");
                    
                    if(ApiManager.getInstance() != null){
    					fileService = ApiManager.getInstance().getFileTransferApi();
    					if(fileService != null){
    						Logger.w(TAG, "MOCFF sendInvite1() addNewFileTransferListener");
    						deliveryListener =  new FileTransferDeliveryListener();
    						fileService.addNewFileTransferListener(deliveryListener);
    					}
    				}
                }
            } catch (JoynServiceException e) {
                e.printStackTrace();
            }finally{
            	//TODo call API to mark messages as failed.
            }
        }
        for (ParticipantInfo info : mCurrentParticipants.mParticipants) {
            if (info.getState().equals(User.STATE_PENDING)) {
                mCurrentParticipants.timerSchedule(info);
            }
        }
        mComposingManager.messageWasSent();
        if (null != message) {
            mChatWindow.addSentMessage(message, messageTag);
        } else {
            Logger.e(TAG, "sendInvite() message is null");
        }
        mAllMessages.add(new ChatMessageSent(message));
        Logger.d(TAG, "sendInvite() exit, subject: " + subject + " mChatId: " + mChatId);
    }

    private boolean rejoinGroup(String content, String rejoinId, int messageTag) {
        Logger.d(TAG, "rejoinGroup() entry, content: " + content + " rejoinId: " + rejoinId);
        boolean success = false;
        if (rejoinId == null) {
            restoreMessages(content, messageTag);
            return success;
        }
        try {
            GroupChat mGroupChatImpl = mMessagingApi.rejoinGroupChat(rejoinId);
            // caused by different processes.
            Logger.d(TAG, "rejoinGroup() mGroupChatImpl: " + mGroupChatImpl);
            if (mGroupChatImpl != null) {
			GroupChatModelListener listener = new GroupChatModelListener(mGroupChatImpl.getParticipants());
			Logger.e(TAG, "rejoinGroup() Add event listener ");
			mGroupChatImpl.addEventListener(listener); 
                GroupChatImplInfo sessionInfo = new GroupChatImplInfo(mGroupChatImpl, GroupChatImplStatus.REJOINING);
				listener.setGroupChatImplInfo(sessionInfo);
                success = true;
            }
			Logger.d(TAG, "rejoinGroup() mGroupChatImpl is not null ");
        } catch (JoynServiceException e) {
            e.printStackTrace();
        }
        restoreMessages(content, messageTag);
        Message msg = Message.obtain();
        msg.what = GroupChatHandler.UPDATE_SEND_BUTTON;
        mUiHandler.sendMessage(msg);
        Logger.d(TAG, "rejoinGroup() exit, success: " + success);
        return success;
    }

    private boolean restartGroup(String content, String restartId) {
        return restartGroup(content, restartId, -1);
    }

    private boolean restartGroup(String content, String restartId, int messageTag) {
        Logger.d(TAG, "restartGroup() entry, content: " + content + " restartId: " + restartId);
        boolean success = false;
        if (restartId == null) {
            restoreMessages(content, messageTag);
            return success;
        }
        try {
            GroupChat mGroupChatImpl = mMessagingApi.restartGroupChat(restartId);
            // caused by different processes.
            Logger.d(TAG, "restartGroup() session: " + mGroupChatImpl);
            if (mGroupChatImpl != null) {
				GroupChatModelListener listener = new GroupChatModelListener(mGroupChatImpl.getParticipants());
				mGroupChatImpl.addEventListener(listener); 
                GroupChatImplInfo sessionInfo = new GroupChatImplInfo(mGroupChatImpl, GroupChatImplStatus.RESTARTING);
                //GroupChatModelListener listener = new GroupChatModelListener(mGroupChatImpl.getParticipants());
				listener.setGroupChatImplInfo(sessionInfo);
                success = true;
            }
        } catch (JoynServiceException e) {
            e.printStackTrace();
        }
        restoreMessages(content, messageTag);
        Message msg = Message.obtain();
        msg.what = GroupChatHandler.UPDATE_SEND_BUTTON;
        mUiHandler.sendMessage(msg);
        Logger.d(TAG, "restartGroup() exit, success: " + success);
        return success;
    }

    private void sendMsrpMessage(GroupChat mGroupChatImpl, String content, int messageTag) {
        Logger.d(TAG, "sendMsrpMessage() entry, content: " + content + " messageTag: " + messageTag
                + " chatSession: " + mGroupChatImpl);
        if (mGroupChatImpl == null) {
            restoreMessages(content, messageTag);
            return;
        }
        try {
            String messageId = mGroupChatImpl.sendMessage(content);
            ChatMessage message = new ChatMessage(messageId, mGroupChatImpl.getRemoteContact(), content,new Date(), true,RcsSettings.getInstance().getJoynUserAlias());
            mComposingManager.messageWasSent();
            if (null != message) {
                mChatWindow.addSentMessage(message, messageTag);
            }
            mAllMessages.add(new ChatMessageSent(message));
        } catch (JoynServiceException e) {
            e.printStackTrace();
        }
        Logger.d(TAG, "sendMsrpMessage() exit, content: " + content + " messageTag: " + messageTag);
    }

    private List<Participant> convertContactsToParticipants(List<String> list) {
        List<Participant> participants = new ArrayList<Participant>();
        int size = list.size();
        for (int i = 0; i < size; i++) {
            String contact = list.get(i);
            String displayName = ContactsListManager.getInstance().getDisplayNameByPhoneNumber(
                    contact);
            participants.add(new Participant(contact, displayName));
        }
        return participants;
    }

    private List<String> convertParticipantInfosToContacts(List<ParticipantInfo> participantInfos) {
        List<String> pList = new ArrayList<String>();
        int size = participantInfos.size();
        for (int i = 0; i < size; i++) {
            pList.add(participantInfos.get(i).getContact());
        }
        return pList;
    }

	private List<String> convertSetToList(Set<String> participantInfos) {
		// create an iterator
       Iterator iterator = participantInfos.iterator(); 
	   List<String> pList = new ArrayList<String>();
      
      // check values
        while (iterator.hasNext()){
            pList.add(iterator.next().toString());
        }
        return pList;
    }

	private Set<String> convertParticipantInfosToContactsSet(List<ParticipantInfo> participantInfos) {
        Set<String> pList = new HashSet<String>();
        int size = participantInfos.size();
        for (int i = 0; i < size; i++) {
            pList.add(participantInfos.get(i).getContact());
        }
        return pList;
    }

	private Set<String> convertParticipantsToContactsSet(List<Participant> participants) {
        Set<String> pList = new HashSet<String>();
        int size = participants.size();
        for (int i = 0; i < size; i++) {
            pList.add(participants.get(i).getContact());
        }
        return pList;
    }

    private List<String> convertParticipantsToContacts(List<Participant> participants) {
        List<String> pList = new ArrayList<String>();
        int size = participants.size();
        for (int i = 0; i < size; i++) {
            pList.add(participants.get(i).getContact());
        }
        return pList;
    }

    /**
     * Provided for controller to call.
     * 
     * @param participants The participants that should be updated to group
     *            chat.
     * @return True if it is updated successfully, otherwise false;
     */
    public boolean addParticipants(List<Participant> toAdd) {
        Logger.d(TAG, "addParticipants entry");
        if (toAdd.size() > 0) {
            int index = 0;
            int size = mCurrentParticipants.mParticipants.size();
            for (; index < size; index++) {
                Participant participant = mCurrentParticipants.mParticipants.get(index).mParticipant;
                if (toAdd.contains(participant)) {
                    toAdd.remove(participant);
                    index -= 1;
                    size = mCurrentParticipants.mParticipants.size();
                }
            }
            ArrayList<Participant> iMCapabilityFailedList = new ArrayList<Participant>();
            CapabilityService capabilityApi = ApiManager.getInstance().getCapabilityApi();
            if (capabilityApi == null) {
                Logger.w(TAG, "addParticipants() capabilityApi is null!");
            } else {
                for (Participant participant : toAdd) {
                    String contactNum = participant.getContact();
					Capabilities remoteCapability = null;
					try{
                    	 remoteCapability = capabilityApi.getContactCapabilities(contactNum);
						 capabilityApi.requestContactCapabilities(contactNum);
					}
					catch(JoynServiceException e){
						Logger.d(TAG, "sendInvite() getContactCapabilities JoynServiceException");
					}					
                    Logger.d(TAG, "addParticipants() remoteCapability: " + remoteCapability);
                    if (remoteCapability != null) {
                        if (remoteCapability.isImSessionSupported()) {
                            Logger.d(TAG, "addParticipants the contactNum : " + contactNum
                                    + "supports the Im session");
                        } else {
                            iMCapabilityFailedList.add(participant);
                        }
                    }
                }
            }
            if (iMCapabilityFailedList.size() > 0) {
                final String displayname = ChatFragment.getParticipantsName(iMCapabilityFailedList
                        .toArray(new Participant[1]));
                Message msg = Message.obtain();
                msg.what = GroupChatHandler.IM_FAIL_TOAST;
                msg.obj = displayname;
                mUiHandler.sendMessage(msg);
                toAdd.removeAll(iMCapabilityFailedList);
            }
            Logger.d(
                    TAG,
                    "addParticipants() the toadd list is " + toAdd + " and size is  "
                            + toAdd.size());
            mCurrentParticipants.addAll(toAdd);
            GroupChat currentChatImpl = mCurrentGroupChatImpl.get();
            if (currentChatImpl == null) {
                Logger.d(TAG, "addParticipants mCurrentSession is null");
                return false;
            }
            try {
                currentChatImpl.addParticipants(convertParticipantsToContactsSet(toAdd));
            } catch (JoynServiceException e) {
                Logger.d(TAG, "addParticipants fail");
                e.printStackTrace();
                return false;
            }
            Logger.d(TAG, "addParticipants exit with true");
            return true;
        } else {
            Logger.d(TAG, "addParticipants() no participant to add");
            return false;
        }
    }

/**
     * Network connectivity listener.
     */
    public class NetworkConnectivityListener extends INetworkConnectivity.Stub {
        private boolean mShouldAutoRejoin = false;

  
        public void prepareToDisconnect() throws RemoteException {
            Logger.d(TAG, "prepareToDisconnect() entry, mIsRegistered: " + mIsRegistered);
            if (!mIsRegistered) {
                return;
            }
            mIsRegistered = false;
            try {
                GroupChatImplInfo mGroupChatImplInfo = mGroupChatImplStack.peek();
                GroupChat mGroupChatImpl = mGroupChatImplInfo.getGroupchatImpl();
                //check if the GroupChat is not null
                if(mGroupChatImpl != null)
                {
	                try {
						int state = mGroupChatImpl.getState();
						Logger.d(TAG, "prepareToDisconnect() state: " + state);
						if (state == GroupChat.State.STARTED) {
						    mShouldAutoRejoin = true;
						    changeGroupChatImplStatus(mGroupChatImplInfo, GroupChatImplStatus.AUTO_REJOIN);
						} else {
						    mShouldAutoRejoin = false;
						}
					} catch (JoynServiceException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
                }
                 else{
                  Logger.d(TAG, "prepareToDisconnect() : session = null");
                }
                
            } catch (EmptyStackException e) {
                Logger.e(TAG, "prepareToDisconnect() EmptyStackException");
                e.printStackTrace();
            }
            Message msg = Message.obtain();
            msg.what = GroupChatHandler.UPDATE_SEND_BUTTON;
            mUiHandler.sendMessage(msg);
            Logger.d(TAG, "prepareToDisconnect() exit, mShouldAutoRejoin: " + mShouldAutoRejoin);
        }


        public void connect() throws RemoteException {
            Logger.d(TAG, "connect() entry, mShouldAutoRejoin: " + mShouldAutoRejoin + " mChatId: "
                    + mChatId + " mIsRegistered: " + mIsRegistered);
          //  if (mIsRegistered) {
          //      return;
           // }
			if (mRegistrationApi == null) {
            	mRegistrationApi = ApiManager.getInstance().getRegistrationApi();
	        }
	        if (mRegistrationApi != null) {
	            mIsRegistered = mRegistrationApi.isRegistered();
            }
           // mIsRegistered = true;
            if (mMessagingApi == null) {
                mMessagingApi = ApiManager.getInstance().getChatApi();
            }
            if (mMessagingApi == null) {
                Logger.e(TAG, "connect() mMessagingApi is null");
                return;
            }
            if (mShouldAutoRejoin) {               
				 try {
				 	// this function is called in worker thread, so it's safe to
                    // do overload operation.
	            GroupChat mGroupChatImpl = mMessagingApi.rejoinGroupChat(mChatId);
	            Logger.d(TAG, "rejoinGroup() mGroupChatImpl: " + mGroupChatImpl);
	            if (mGroupChatImpl != null) {
	                GroupChatImplInfo mGroupChatImplInfo = new GroupChatImplInfo(mGroupChatImpl, GroupChatImplStatus.REJOINING);
	                GroupChatModelListener listener = new GroupChatModelListener(mGroupChatImpl.getParticipants());
					listener.setGroupChatImplInfo(mGroupChatImplInfo);
	                mGroupChatImpl.addEventListener(listener); 
	            }
		        } catch (JoynServiceException e) {
		            e.printStackTrace();
		        }
                mShouldAutoRejoin = false;
            }
            Message msg = Message.obtain();
            msg.what = GroupChatHandler.UPDATE_SEND_BUTTON;
            mUiHandler.sendMessage(msg);
            Logger.d(TAG, "connect() exit");
        }
    }


    private void sendRestoredMessages(GroupChat mGroupChatImpl) {
        Logger.d(TAG, "sendRestoredMessages() mMessagesToSendDelayed size: "
                + mMessagesToSendDelayed.size());
        for (String msg : mMessagesToSendDelayed.keySet()) {
             sendMsrpMessage(mGroupChatImpl, msg ,mMessagesToSendDelayed.get(msg));               
        }
    }

    private void handleReceiveMessage(final ChatMessage msg, final GroupChatParticipants groupChatParticipants) {
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... params) {
                Context context = null;
                if (ApiManager.getInstance() != null) {
                    context = ApiManager.getInstance().getContext();
                }
                if (msg.isDisplayedReportRequested()) {
                    if (!mIsInBackground) {
                        /*
                         * Mark the received message as displayed when the chat
                         * window is not in background
                         */
                        markMessageAsDisplayed(msg);
                    } else {
                        /*
                         * Save the message and will mark it as displayed when
                         * the chat screen resumes
                         */
                        GroupChat1.this.addUnreadMessage(msg);
                        /*
                         * Showing notification of a new incoming message when
                         * the chat window is in background
                         */
                        RcsNotification.getInstance().onReceiveMessageInBackground(context, mTag, msg,
                                groupChatParticipants.convertToParticipants(), 0);
                    }
                } else {
                    if (!mIsInBackground) {
                        /*
                         * Mark the received message as read if the chat window
                         * is not in background
                         */
                        markMessageAsRead(msg);
                    } else {
                        /*
                         * Save the message and will mark it as read when the
                         * activity resumes
                         */
                        GroupChat1.this.addUnreadMessage(msg);
                        /*
                         * Showing notification of a new incoming message when
                         * the chat window is in background
                         */
                        RcsNotification.getInstance().onReceiveMessageInBackground(context, mTag, msg,
                                groupChatParticipants.convertToParticipants(), 0);
                    }
                }
                mChatWindow.addReceivedMessage(msg, !mIsInBackground);
                mAllMessages.add(new ChatMessageReceived(msg));
                
                /**
                 * M: managing extra local chat participants that are 
                 * not present in the invitation for sending them invite request.@{
                 */
                if(isResendGrpInvite){
                	Logger.d(TAG, "handleReceiveMessage: adding mExtraLocalparticipants in chat and sending refer request");
                	isResendGrpInvite = false;
                	//add the extra local particpants and send them refer request
                	
                	//we add only if its not there
             /*   	for ( Participant p : mExtraLocalparticipants)
                	{
	                	if(mCurrentParticipants.mParticipants.contains(p))
	                	{
	                		//remove it
	                		mExtraLocalparticipants.remove(p);
	                	}
                	}*/
            		addParticipants(mExtraLocalparticipants);	
                }
                /**
            	 * @}
            	 */ 
                return null;
            }
        }.execute();
    }

    private class GroupChatModelListener extends GroupChatListener {

        public static final String TAG = "M0CF GroupChatModelListener";
        private GroupChatImplStatus mStatus = GroupChatImplStatus.UNKNOWN;
        protected GroupChat mGroupChatImpl = null;
        private GroupChatImplInfo mGroupChatImplInfo = null;
        private GroupChatParticipants mGroupChatParticipants = new GroupChatParticipants();
        private ArrayList<String> mParticipantsToBeInvited = new ArrayList<String>();

        protected GroupChatModelListener(Set<String> pSet) {
            //mGroupChatImpl = info.getGroupchatImpl();
            //mGroupChatImplInfo = info;
            //mGroupChatImplStack.push(mGroupChatImplInfo);
            List<String> pList = new ArrayList<String>();
            if(pSet != null){
				pList = convertSetToList(pSet);
            }
            if (pList == null) {
                return;
            }
                mGroupChatParticipants
                        .initicalGroupChatParticipant(convertContactsToParticipants(pList));
            List<String> currentList = convertParticipantInfosToContacts(mCurrentParticipants.mParticipants);
            for (String contact : currentList) {
                if (!pList.contains(contact)) {
                    mParticipantsToBeInvited.add(contact);
                    Logger.d(TAG, "GroupChatModelListener mParticipantsToBeInvited contact: " + contact);
                }
            }
            mCurrentParticipants = mGroupChatParticipants;
            // TODO auto-invite OTHER participants.
        }

		public void setGroupChatImplInfo(GroupChatImplInfo info){
			Logger.d(TAG, "setGroupChatImplInfo " + info.getGroupchatImpl());
			mGroupChatImpl = info.getGroupchatImpl();
            mGroupChatImplInfo = info;
            mGroupChatImplStack.push(mGroupChatImplInfo);
            
		}

        private void onDestroy() {
            Logger.d(TAG, "onDestroy entry, mGroupChatImpl: " + mGroupChatImpl);
            mCurrentParticipants.resetAllStatus();
            if (null != mGroupChatImpl) {
                try {
                    Logger.d(TAG, "onDestroy() mGroupChatImpl" + mGroupChatImpl);
                    mGroupChatImpl.removeEventListener(this);
                } catch (JoynServiceException e) {
                    Logger.e(TAG, "onDestroy() JoynServiceException" + e);
                    e.printStackTrace();
                }
            }
        }

		/**
	 * Callback called when the session is well established and messages
	 * may be exchanged with the group of participants
	 */
	public  void onSessionStarted(){	
        Logger.d(TAG, "onSessionStarted entry, mStatus: " + mStatus);
        mCurrentGroupChatImpl.set(mGroupChatImpl);
        mStatus = GroupChatImplStatus.ACTIVE;
        changeGroupChatImplStatus(mGroupChatImplInfo, mStatus);
        ((IGroupChatWindow) mChatWindow).setIsComposing(false, null);
        ((IGroupChatWindow) mChatWindow).setmChatID(mChatId);
        mWorkerHandler.post(new Runnable() {
            @Override
            public void run() {
                sendRestoredMessages(mGroupChatImpl);
                clearRestoredMessages();
            }
        });
        Message msg = Message.obtain();
        msg.what = GroupChatHandler.UPDATE_SEND_BUTTON;
        mUiHandler.sendMessage(msg);
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... params) {
                Logger.d(TAG, "mReceivedMessagesDelayed = " + mReceivedMessagesDelayed);
                if (mReceivedMessagesDelayed != null) {
                    int size = mReceivedMessagesDelayed.size();
                    for (int i = 0; i < size; i++) {
                        markMessageAsDisplayed(mReceivedMessagesDelayed.get(i));
                    }
                    mReceivedMessagesDelayed.clear();
                }
                return null;
            }
        }.execute();
        
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... params) {
            	Logger.d(TAG, "handleSessionStarted entry startGroupFileStransfer");
       			ModelImpl instance = (ModelImpl) ModelImpl.getInstance();
    			if(instance != null){
    				instance.startGroupFileStransfer();
    			}
    			Logger.d(TAG, "handleSessionStarted exit startGroupFileStransfer");
    			return null;
            }
        }.execute();
	}
	
	/**
	 * Callback called when the session has been aborted or terminated
	 */
	public  void onSessionAborted(){
	  Logger.d(TAG, "onSessionAborted() entry, mStatus: " + mStatus);
            ((IGroupChatWindow) mChatWindow).setIsComposing(false, null);
            int location = mGroupChatImplStack.indexOf(mGroupChatImplInfo);
            if (location != -1) {
                mStatus = mGroupChatImplStack.get(location).getGroupChatImplStatus();
            }
            if (mStatus != GroupChatImplStatus.AUTO_REJOIN) {
                mStatus = GroupChatImplStatus.MANULLY_REJOIN;
            }
            changeGroupChatImplStatus(mGroupChatImplInfo, mStatus);
            Message msg = Message.obtain();
            msg.what = GroupChatHandler.UPDATE_SEND_BUTTON;
            mUiHandler.sendMessage(msg);
            Logger.d(TAG, "handleSessionAborted() exit, mStatus: " + mStatus);
            onDestroy();
	}

	/**
	 * Callback called when the session has failed
	 * 
	 * @param error Error
	 * @see GroupChat.Error
	 */
	public  void onSessionError(int error){
	 Logger.d(TAG, "onSessionError() entry, error: " + error + " mStatus: " + mStatus);
        ((IGroupChatWindow) mChatWindow).setIsComposing(false, null);
        int location = mGroupChatImplStack.indexOf(mGroupChatImplInfo);
        if (location != -1) {
            mStatus = mGroupChatImplStack.get(location).getGroupChatImplStatus();
        }
        if (mGroupChatImplStack.size() == 1 && mStatus == GroupChatImplStatus.INVITING) {
            mStatus = GroupChatImplStatus.UNKNOWN;
        } else if (mStatus == GroupChatImplStatus.RESTARTING) {
            
        	/**
	         * M: added to restart a new grp chat in case of error  or timeout @(
	         * */
            //mStatus = SessionStatus.TERMINATED;
             Logger.d(TAG, "set mStatus = GroupChatImplStatus.MANULLY_RESTART");
        	 mStatus = GroupChatImplStatus.MANULLY_RESTART;
        	/**
	         * @}
	         */
        	
        } else if (mStatus == GroupChatImplStatus.REJOINING) {
            mStatus = GroupChatImplStatus.MANULLY_RESTART;
            restartGroup(null, mChatId);
        } else {
            mStatus = GroupChatImplStatus.MANULLY_REJOIN;
        }
        changeGroupChatImplStatus(mGroupChatImplInfo, mStatus);
        Message msg = Message.obtain();
        msg.what = GroupChatHandler.UPDATE_SEND_BUTTON;
        mUiHandler.sendMessage(msg);
        onDestroy();
        Logger.v(TAG, "onSessionError() exit, mStatus: " + mStatus);
	}
	
	/**
	 * Callback called when a new message has been received
	 * 
	 * @param message New chat message
	 * @see ChatMessage
	 */
	public  void onNewMessage(ChatMessage message){
		Logger.d(TAG, "ABCG onNewMessage() entry DISPLAY: " + message.getDisplayName());
        GroupChat1.this.handleReceiveMessage(message, mGroupChatParticipants);
        Logger.d(TAG, "onNewMessage() exit");
	}

	/**
	 * Callback called when a new geoloc has been received
	 * 
	 * @param message Geoloc message
	 * @see GeolocMessage
	 */
	public  void onNewGeoloc(GeolocMessage message){
	}
	
	/**
	 * Callback called when a message has been delivered to the remote
	 * 
	 * @param msgId Message ID
	 */
	public  void onReportMessageDeliveredContact(String msgId,String contact){
	  Logger.d(TAG, "ABCG onReportMessageDelivered() entry, msgId: " + msgId + "contact: " + contact);
        ISentChatMessage msg = (ISentChatMessage) mChatWindow.getSentChatMessage(msgId);
        if (msg != null) {
            msg.updateStatus(Status.DELIVERED,contact); //TODo Contact info required from stack
        }
	}

	/**
	 * Callback called when a message has been displayed by the remote
	 * 
	 * @param msgId Message ID
	 */
	public  void onReportMessageDisplayedContact(String msgId,String contact){
	 	Logger.d(TAG, "ABCG onReportMessageDisplayed() entry, msgId: " + msgId + "contact: " + contact);
	    ISentChatMessage msg = (ISentChatMessage) mChatWindow.getSentChatMessage(msgId);
	    if (msg != null) {
	        msg.updateStatus(Status.DISPLAYED,contact); //TODo Contact info required from stack
	    }
	}
	
	/**
	 * Callback called when a message has failed to be delivered to the remote
	 * 
	 * @param msgId Message ID
	 */
	public  void onReportMessageFailedContact(String msgId,String contact){
		Logger.d(TAG, "ABCG onReportMessageFailed() entry, msgId: " + msgId + "contact: " + contact);
	    ISentChatMessage msg = (ISentChatMessage) mChatWindow.getSentChatMessage(msgId);
	    if (msg != null) {
	        msg.updateStatus(Status.FAILED,contact); //TODo Contact info required from stack
	    }
	}
	
	/**
	 * Callback called when a message has been delivered to the remote
	 * 
	 * @param msgId Message ID
	 */
	public  void onReportMessageDelivered(String msgId){
	  Logger.d(TAG, "onReportMessageDelivered() entry, msgId: " + msgId);
	}

	/**
	 * Callback called when a message has been displayed by the remote
	 * 
	 * @param msgId Message ID
	 */
	public  void onReportMessageDisplayed(String msgId){
	 	Logger.d(TAG, "onReportMessageDisplayed() entry, msgId: " + msgId);
	}
	
	/**
	 * Callback called when a message has failed to be delivered to the remote
	 * 
	 * @param msgId Message ID
	 */
	public  void onReportMessageFailed(String msgId){
		Logger.d(TAG, "onReportMessageFailed() entry, msgId: " + msgId);
	}
	
	/**
	 * Callback called when an Is-composing event has been received. If the
	 * remote is typing a message the status is set to true, else it is false.
	 * 
	 * @param contact Contact
	 * @param status Is-composing status
	 */
	public  void onComposingEvent(String contact, boolean status){
        Logger.d(TAG, "onComposingEvent() contact: " + contact + " status: " + status);
        int size = mGroupChatParticipants.mParticipants.size();
        String contactNumber = PhoneUtils.extractNumberFromUri(contact);
        for (int index = 0; index < size; index++) {
            final Participant participant = mGroupChatParticipants.mParticipants.get(index).mParticipant;
            Logger.d(TAG, "onComposingEvent() participant: " + participant);
            if (null != participant) {
                String remoteContact = participant.getContact();
                if (null != remoteContact && remoteContact.equals(contactNumber)) {
                    ((IGroupChatWindow) mChatWindow).setIsComposing(status, participant);
                }
            }
        }
   }

	/**
	 * Callback called when a new participant has joined the group chat
	 *  
	 * @param contact Contact
	 * @param contactDisplayname Contact displayname
	 */
	public  void onParticipantJoined(String contact, String contactDisplayname){
	     boolean result = mGroupChatParticipants.updateParticipants(contact, contactDisplayname,
                    User.STATE_CONNECTED, mParticipantsToBeInvited);
        if (result) {
            mParticipantsToBeInvited.remove(contact);
        }
        Logger.d(TAG, "onParticipantJoined() result: " + result);
	}
	
	/**
	 * Callback called when a participant has left voluntary the group chat
	 *  
	 * @param contact Contact
	 */
	public  void onParticipantLeft(String contact){
	    boolean result = mGroupChatParticipants.updateParticipants(contact, null,
                    User.STATE_DEPARTED, mParticipantsToBeInvited);
        if (result) {
            mParticipantsToBeInvited.remove(contact);
        }
        Logger.d(TAG, "onParticipantLeft() result: " + result);
	}

	/**
	 * Callback called when a participant is disconnected from the group chat
	 * 
	 * @param contact Contact
	 */
	public  void onParticipantDisconnected(String contact){
	     boolean result = mGroupChatParticipants.updateParticipants(contact, null,
                    User.STATE_DISCONNECTED, mParticipantsToBeInvited);
        if (result) {
            mParticipantsToBeInvited.remove(contact);
        }
        Logger.d(TAG, "onParticipantDisconnected() result: " + result);
	}
   };

    /**
     * Post a request to chat participants for transferring file .
     * 
     * @param file The file to be transferred.
     * @return True if it's successful to post a request, otherwise return
     *         false.
     */
    public boolean requestTransferFile(File file) {
        Logger.v(TAG, "M0CFF GroupChatImpl requestTransferFile entry");
        Logger.v(TAG, "M0CFF GroupChatImpl requestTransferFile exit");
        return false;
    }

    /**
     * Accept or reject to transfer file.
     * 
     * @param accept true if accepting to transfer file, else reject.
     * @return True if the operation is successfully handled, else return false.
     */
    public boolean acceptTransferFile(boolean accept) {
        Logger.v(TAG, "M0CFF GroupChatImpl acceptTransferFile entry");
        Logger.v(TAG, "M0CFF GroupChatImpl acceptTransferFile exit");
        return false;
    }

	/**
     * Handle a file transfer invitation
     */
    public void addReceiveFileTransfer(FileTransfer fileTransferObject, boolean isAutoAccept,boolean isGroup) {
    	Logger.v(TAG, "M0CFF addReceiveFileTransfer isAutoAccept" + isAutoAccept+" isGroup:"+isGroup);
        mReceiveFileTransferManager.addReceiveFileTransfer(fileTransferObject, isAutoAccept,isGroup);
		if (mIsInBackground) {
			GroupChat1.this.addUnreadFt(fileTransferObject);
		}
    }

    @Override
    protected void checkCapabilities() {
        if (mCapabilityApi == null) {
            mCapabilityApi = ApiManager.getInstance().getCapabilityApi();
        }
        Logger.d(TAG, "M0CFF checkCapabilities() mCapabilityApi: " + mCapabilityApi);
        if (mCapabilityApi == null) {
            return;
        }      
        ArrayList<ParticipantInfo> participants = new ArrayList<ParticipantInfo>(
                mCurrentParticipants.mParticipants);
        ArrayList<ParticipantInfo> uncapableParticipants = new ArrayList<ParticipantInfo>();
        for (ParticipantInfo participantInfo : participants) {
            Participant participant = participantInfo.getParticipant();
            String contact = participant.getContact();
			Capabilities capability = null;
			try{
            	 capability = mCapabilityApi.getContactCapabilities(contact);
				 mCapabilityApi.requestContactCapabilities(contact);
			}
			catch(JoynServiceException e){
				Logger.d(TAG, "M0CFF sendInvite() getContactCapabilities JoynServiceException");
			}
            if (capability == null || !capability.isImSessionSupported()) {
                Logger.w(TAG, "M0CFF checkCapabilities() The capabilities of " + contact + " is "
                        + capability);
                uncapableParticipants.add(participantInfo);
            }
        }
        Logger.w(TAG, "M0CFF checkCapabilities() uncapableParticipants: " + uncapableParticipants);
        mCurrentParticipants.mParticipants.removeAll(uncapableParticipants);
    }

    @Override
    protected void queryCapabilities() {
        checkCapabilities();
    }

	protected void checkAllGroupCapability() {
        Logger.i(TAG, "checkAllGroupCapability() entry: mFileTransferController = "
                + mFileTransferController + ", mParticipant = " + mParticipant);
        final RegistrationApi registrationApi = ApiManager.getInstance().getRegistrationApi();
        if (mFileTransferController != null) {
            if (registrationApi != null && registrationApi.isRegistered()) {
                Logger.d(TAG, "checkAllGroupCapability() already registered");
                mFileTransferController.setRegistrationStatus(true);
                final CapabilityService capabilityApi = ApiManager.getInstance().getCapabilityApi();
                Logger.v(TAG, "checkAllGroupCapability() capabilityApi = " + capabilityApi);
                if (capabilityApi != null) {
					try{
                    Capabilities myCapablities = capabilityApi.getMyCapabilities();
                    if (null != inviteContact) {
                        final String contact = inviteContact;
						Capabilities remoteCapablities = null;
						try{
                        	 remoteCapablities = capabilityApi.getContactCapabilities(contact);
							 AsyncTask.execute(new Runnable() {
			                	@Override
				               	public void run() {
					               	try{
										capabilityApi.requestContactCapabilities(contact);
					               	}
									catch(JoynServiceException e){
										Logger.i(TAG, "checkAllGroupCapability() requestContactCapabilities JoynServiceException");
									}	
				                }
			            	});
						}
						catch(JoynServiceException e){
							Logger.d(TAG, "checkAllGroupCapability() getContactCapabilities JoynServiceException");
						}						
                        Logger.v(TAG, "checkAllGroupCapability() myCapablities = "
                                + myCapablities + ",remoteCapablities = "
                                + remoteCapablities);
                        if (myCapablities != null) {
                            mFileTransferController.setLocalGroupFtCapability(false);
                            if (myCapablities.isFileTransferHttpSupported()) {
                                Logger.d(TAG,"checkAllGroupCapability() my capability support filetransfer");
                                mFileTransferController.setLocalGroupFtCapability(true);
                                if (remoteCapablities != null) {
                                    mFileTransferController.setRemoteGroupFtCapability(false);
                                    if (remoteCapablities.isFileTransferHttpSupported()) {
                                        Logger.i(TAG,"checkAllGroupCapability() participant support filetransfer ");
                                        mFileTransferController.setRemoteGroupFtCapability(true);
                                    }
                                }
                            }
                        }
                    }
				 }
				catch(JoynServiceException e){
					}
                }
            } else {
                mFileTransferController.setRegistrationStatus(false);
            }
        }
        //if (mFileTransferController != null) {
        //    mFileTransferController.controlFileTransferIconStatus();
        //}
    }

    public void handleReInvitation(GroupChat mGroupChatImpl, String participantList) {
        Logger.v(TAG, "group chat handleInvitation entry, chatSession: " + mGroupChatImpl);
        if (mGroupChatImpl == null) {
            return;
        }
        try {
            mGroupChatImpl.acceptInvitation();
            
            /**
             * M: managing extra local chat participants that are 
             * not present in the invitation for sending them invite request.@{
             */   	
            String[] particpnts  = {"default"};
            if(participantList != null){
            	particpnts = participantList.split(";");
            }
            List<String>ParticipantArray = Arrays.asList(particpnts);
           
            List<String>localParticipantArray = new ArrayList<String>();
            ArrayList<Participant> localParticipantList = mCurrentParticipants.convertToParticipants();
            for(Participant p :localParticipantList){
            	localParticipantArray.add(p.getContact());
            }
           
            if(ParticipantArray.size()>0){
            	//check if current invitation contains all the participants present locally
            	if(!ParticipantArray.containsAll(localParticipantArray)){
            		
            		
            		isResendGrpInvite = true; //flag to send the group invite to extra local particpants of the chat
            		
            		localParticipantArray.removeAll(ParticipantArray);
            		Logger.v(TAG, "handleReInvitation : ExtraLocalparticipants exists: " + localParticipantArray.toString());
            		
            		mExtraLocalparticipants = new ArrayList<Participant>();
            		for(String extraParticipant: localParticipantArray){
            			Participant p = new Participant(extraParticipant,extraParticipant);
            			mExtraLocalparticipants.add(p); //add the extra participants
            		}

            	}
            }
            /**
    		 * @}
    		 */   
            
            GroupChatImplInfo sessionInfo = new GroupChatImplInfo(mGroupChatImpl, GroupChatImplStatus.INVITING);
            GroupChatModelListener listener = new GroupChatModelListener(mGroupChatImpl.getParticipants());
			listener.setGroupChatImplInfo(sessionInfo);
            mChatId = mGroupChatImpl.getChatId();
            mGroupChatImpl.addEventListener(listener);           
                   
        } catch (JoynServiceException e) {
            Logger.d(TAG, "handleInvitation() acceptSession or addSessionListener fail");
            e.printStackTrace();
        }       
    }

	public void handleInvitation(Chat o2oChatImpl, ArrayList<IChatMessage> messages, boolean isAutoAccept) {
	}

    @Override
    public void handleInvitation(GroupChat mGroupChatImpl, ArrayList<IChatMessage> messages, boolean isAutoAccept) {
        Logger.w(TAG, "group chat handleInvitation entry, chatSession: " + mGroupChatImpl);
        if (mGroupChatImpl == null) {
            return;
        }
        try {
			Logger.w(TAG, "handleInvitation() acceptSession12 ");
            mGroupChatImpl.acceptInvitation();
            GroupChatImplInfo sessionInfo = new GroupChatImplInfo(mGroupChatImpl, GroupChatImplStatus.INVITING);
            GroupChatModelListener listener = new GroupChatModelListener(mGroupChatImpl.getParticipants());
			listener.setGroupChatImplInfo(sessionInfo);
            mChatId = mGroupChatImpl.getChatId();
            mGroupChatImpl.addEventListener(listener);
        } catch (JoynServiceException e) {
            Logger.i(TAG, "handleInvitation() acceptSession or addSessionListener fail");
            e.printStackTrace();
        }

		Logger.w(TAG, "MOCFF handleInvitation() addNewFileTransferListener fileService :" + fileService);
		Logger.w(TAG, "MOCFF handleInvitation() addNewFileTransferListener deliveryListener :" + deliveryListener);

		try{
			//if(fileService != null){
					if(ApiManager.getInstance() != null){
					fileService = ApiManager.getInstance().getFileTransferApi();
					if(fileService != null){
							Logger.w(TAG, "MOCFF handleInvitation() addNewFileTransferListener");
						deliveryListener =  new FileTransferDeliveryListener();
						fileService.addNewFileTransferListener(deliveryListener);
					}
				}
			//}
		}
		catch(JoynServiceException e){
			Logger.i(TAG, "MOCFF handleSendFileTransferInvitation() exception");
		}
		
        Logger.w(TAG, "Group chat handleInvitation messages is null");
            return;

    }

	public void setInviteContact(String contact){
		Logger.i(TAG, "setInviteContact : " + contact);
		inviteContact = contact;
		checkAllGroupCapability();
	}

    @Override
    public void loadChatMessages(int count) {
        // TODO
    }

	public void reloadSessionStack(int mGroupChatImplStatus) {
		GroupChat reloadedGroupChatImpl = null;
		GroupChatImplInfo reloadedGroupChatImplInfo;
		Logger.w(TAG, "reloadSessionStack()");
//		if (sessionStatus == 1 ){
//		    Logger.w(TAG, "reloadSessionStack() Terminated ");
//		    reloadedSessionInfo = new GroupChatImplInfo(reloadedSession, GroupChatImplStatus.TERMINATED);
//		}
//		else {
            reloadedGroupChatImplInfo = new GroupChatImplInfo(reloadedGroupChatImpl, GroupChatImplStatus.UNKNOWN);
		//}	
		mGroupChatImplStack.push(reloadedGroupChatImplInfo);

	}

	public void reloadStackApi(String chatId) {
		try {
			GroupChat mGroupChatImpl = mMessagingApi.getGroupChat(chatId);
			reloadSessionStack(mGroupChatImpl.getState());
		} catch (JoynServiceException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

    @Override
    protected void reloadMessage(final ChatMessage message, final int messageDirection,
            final int status, final int messageTag, final String chatId) {
	if(ismInvite() == false)
	{
        Logger.w(TAG, "reloadMessage() entry chatId  " + chatId);
		String groupChatSubject = "";
		Cursor cursor = null;
		try {
			ContentResolver cr = AndroidFactory.getApplicationContext()
            .getContentResolver();
			cursor = cr.query(ChatLog.GroupChat.CONTENT_URI, 
	    			new String[] {
					    ChatLog.GroupChat.SUBJECT			
	    			},
	    			"(" + ChatLog.GroupChat.CHAT_ID + "='" + chatId + "')", 
					null, 
					ChatLog.GroupChat.TIMESTAMP + " DESC");
	    	
	    	if (cursor.moveToFirst()) {
	    		groupChatSubject = cursor.getString(0); 
	        	
	    	}
	    	
	    	final String newSubject = groupChatSubject;
		
		
			//GroupChat mGroupChatImpl = mMessagingApi.getGroupChat( chatId);
			
			((IGroupChatWindow)mChatWindow).addgroupSubject(groupChatSubject);
		((IGroupChatWindow)mChatWindow).setmChatID(chatId);
		
        Logger.d(TAG, "reloadMessage : groupChatSubject outer :-> "+ groupChatSubject +"Chat Id =" + chatId);
        
        		Logger.d(TAG, "reloadMessage()->run() entry, message id: " + message.getId()
                        + "message text: " + message.getMessage() + " , messageType: "
                        + messageDirection + " status: " + status);
                if(mChatWindow != null)
                {                   
                  Logger.d(TAG, "reloadMessage : groupChatSubject :-> "+ newSubject +"Chat Id =" + chatId);
                	               	   
                if (ChatLog.Message.Direction.INCOMING == messageDirection && !(chatId.equals(message.getContact()))) {
                    Logger.d(TAG, "reloadMessage() the mchatwindow is " + mChatWindow);
					if(status == ChatLog.Message.Status.Content.READ){
                    mChatWindow.addReceivedMessage(message, true);
					} else {
					    Logger.d(TAG, "mReceivedAfterReloadMessage group Id: " + message.getId());
					    mReceivedAfterReloadMessage.add(message.getId());
					    mChatWindow.addReceivedMessage(message, false);
					}
                    mAllMessages.add(new ChatMessageReceived(message));
                } 
                else if (ChatLog.Message.Direction.OUTGOING == messageDirection && !(chatId.equals(message.getContact()))) {
                    mChatWindow.addSentMessage(message, -1);
                    mAllMessages.add(new ChatMessageSent(message));
                } 
                else if (ChatLog.Message.Direction.INCOMING== messageDirection && !(chatId.equals(message.getContact()))) { //TODo check this
                    mChatWindow.addSentMessage(message, -1);
                    mAllMessages.add(new ChatMessageSent(message));
                } else {
                    Logger.e(TAG,
                            "reloadMessage() the messageType is not incoming or outgoing message "
                                    + messageDirection);
                }
            }
        
     /*   Runnable worker = new Runnable() {
            @Override
            public void run() {
                Logger.d(TAG, "reloadMessage()->run() entry, message id: " + message.getId()
                        + "message text: " + message.getMessage() + " , messageType: "
                        + messageDirection);
                if(mChatWindow != null)
                {                   
                  Logger.d(TAG, "reloadMessage : groupChatSubject :-> "+ newSubject +"Chat Id =" + chatId);
                	               	   
                if (ChatLog.Message.Direction.INCOMING == messageDirection && !(chatId.equals(message.getContact()))) {
                    Logger.d(TAG, "reloadMessage() the mchatwindow is " + mChatWindow);
                    mChatWindow.addReceivedMessage(message, true);
                    mAllMessages.add(new ChatMessageReceived(message));
                } 
                else if (ChatLog.Message.Direction.OUTGOING == messageDirection && !(chatId.equals(message.getContact()))) {
                    mChatWindow.addSentMessage(message, -1);
                    mAllMessages.add(new ChatMessageSent(message));
                } 
                else if (ChatLog.Message.Direction.INCOMING== messageDirection && !(chatId.equals(message.getContact()))) { //TODo check this
                    mChatWindow.addSentMessage(message, -1);
                    mAllMessages.add(new ChatMessageSent(message));
                } else {
                    Logger.e(TAG,
                            "reloadMessage() the messageType is not incoming or outgoing message "
                                    + messageDirection);
                }
            }
            }
        };
        mWorkerHandler.post(worker);*/
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} 
		finally{
	    	cursor.close();
		}
    }
  }

    @Override
    public void onCapabilitiesReceived(String contact, Capabilities capabilities) {
        // TODO Auto-generated method stub
        Logger.i(TAG, "M0CFF onCapabilitiesReceived() lmn: " + contact + "HTTP Support: " + capabilities.isFileTransferHttpSupported());
    	if(mFileTransferController != null)
    	{
    		if(inviteContact != null && inviteContact.equals(contact)){
				Logger.i(TAG, "M0CFF onCapabilitiesReceived() lmn: same contact");
    	mFileTransferController.setRemoteGroupFtCapability(capabilities.isFileTransferHttpSupported());
				mFileTransferController.controlGroupFileTransferIconStatus();
    		}
    	}
    }

    @Override
    public void onStatusChanged(boolean status) {
        Logger.d(TAG, "M0CFF onStatusChanged() status: " + status);
        mIsRegistered = status;
        if (status) {
            // to make sure that init() is called when registered.
            init();
        }
        Message msg = Message.obtain();
        msg.what = GroupChatHandler.UPDATE_SEND_BUTTON;
        mUiHandler.sendMessage(msg);
    }

    /**
     * Helper function for test case.
     * 
     * @return Group chat participants.
     */
    public GroupChatParticipants getGroupChatParticipants() {
        return mCurrentParticipants;
    }

    /**
     * Session status.
     */
    public static enum GroupChatImplStatus {
        /**
         * Unknown, initial status.
         */
        UNKNOWN,
        /**
         * Inviting, wait to accept.
         */
        INVITING,
        /**
         * Session is established.
         */
        ACTIVE,
        /**
         * Session rejoin is on-going.
         */
        REJOINING,
        /**
         * Session should be auto-rejoined.
         */
        AUTO_REJOIN,
        /**
         * Session should be manually rejoined.
         */
        MANULLY_REJOIN,
        /**
         * Session should be restarted.
         */
        MANULLY_RESTART,
        /**
         * Session restarting is on-going.
         */
        RESTARTING,
        /**
         * No need to try to rejoin.
         */
        TERMINATED
    }
   
    
   
    
    
    
    private static ISentChatMessage.Status formatStatus(String s) {
        Logger.d(TAG, "formatStatus entry with status: " + s);
        ISentChatMessage.Status status = ISentChatMessage.Status.SENDING;
        if (s == null) {
            return status;
        }
        if (s.equals(ImdnDocument.DELIVERY_STATUS_DELIVERED)) {
            status = ISentChatMessage.Status.DELIVERED;
        } else if (s.equals(ImdnDocument.DELIVERY_STATUS_DISPLAYED)) {
            status = ISentChatMessage.Status.DISPLAYED;
        } else {
            status = ISentChatMessage.Status.FAILED;
        }
        Logger.d(TAG, "formatStatus entry exit");
        return status;
    }
    /**
     * This method is normally to handle the received delivery notifications via
     * SIP
     * 
     * @param messageId The message id of the delivery notification
     * @param status The type of the delivery notification
     */
    public void onMessageDelivered(final String messageId, final String status, final long timeStamp) {
        Runnable worker = new Runnable() {
            @Override
            public void run() {
                mSentMessageManager.onMessageDelivered(messageId, formatStatus(status), timeStamp);
            }
        };
        Thread currentThread = Thread.currentThread();
        if (currentThread.equals(mWorkerThread)) {
            Logger.v(TAG, "onMessageDelivered() run on worker thread");
            worker.run();
        } else {
            Logger.v(TAG, "onMessageDelivered() post to worker thread");
            mWorkerHandler.post(worker);
        }
    }

	/**
     * Handle accept a file transfer invitation
     */
    public void handleAcceptFileTransfer(Object fileTransferTag) {
        ReceiveFileTransfer receiveFileTransfer =
                mReceiveFileTransferManager.findFileTransferByTag(fileTransferTag);
        Logger.d(TAG,
                "M0CFF handleAcceptFileTransfer  group entry(): receiveFileTransfer = "
                        + receiveFileTransfer + ", fileTransferTag = "
                        + fileTransferTag);
        if (null != receiveFileTransfer) {
            receiveFileTransfer.acceptFileTransferInvitation();
        }
    }

	/**
     * Handle pause a file transfer 
     */
    public void handlePauseReceiveFileTransfer(Object fileTransferTag) {
        ReceiveFileTransfer receiveFileTransfer =
                mReceiveFileTransferManager.findFileTransferByTag(fileTransferTag);
        Logger.d(TAG,
                "M0CFF handlePauseReceiveFileTransfer group entry(): receiveFileTransfer = "
                        + receiveFileTransfer + ", fileTransferTag = "
                        + fileTransferTag);
        if (null != receiveFileTransfer) {
			 Logger.d(TAG,"handlePauseReceiveFileTransfer group 1");
            receiveFileTransfer.onPauseReceiveTransfer();
        }
    }

	/**
     * Handle resume a file transfer 
     */
    public void handleResumeReceiveFileTransfer(Object fileTransferTag) {
        ReceiveFileTransfer receiveFileTransfer =
                mReceiveFileTransferManager.findFileTransferByTag(fileTransferTag);
        Logger.d(TAG,
                "M0CFF handleResumeReceiveFileTransfer group entry(): receiveFileTransfer = "
                        + receiveFileTransfer + ", fileTransferTag = "
                        + fileTransferTag);
        if (null != receiveFileTransfer) {
			Logger.d(TAG,"handleResumeReceiveFileTransfer group 1");
            receiveFileTransfer.onResumeReceiveTransfer();
        }
    }

}

