package com.mediatek.mediatekdm.operator.cmcc;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.IBinder;
import android.os.RemoteException;
import android.provider.Settings.System;
import android.util.Log;

import com.mediatek.common.dm.DmAgent;
import com.mediatek.mediatekdm.CollectSetPermissionControl;
import com.mediatek.mediatekdm.CollectSetPermissionDialog;
import com.mediatek.mediatekdm.DmApplication;
import com.mediatek.mediatekdm.DmConfig;
import com.mediatek.mediatekdm.DmConst;
import com.mediatek.mediatekdm.DmConst.IntentAction;
import com.mediatek.mediatekdm.DmConst.TAG;
import com.mediatek.mediatekdm.DmFeatureSwitch;
import com.mediatek.mediatekdm.DmOperation;
import com.mediatek.mediatekdm.DmService;
import com.mediatek.mediatekdm.IDmComponent;
import com.mediatek.mediatekdm.PlatformManager;
import com.mediatek.mediatekdm.SessionHandler;
import com.mediatek.mediatekdm.mdm.MdmTree;
import com.mediatek.mediatekdm.mdm.PLHttpConnection;
import com.mediatek.mediatekdm.mdm.SessionInitiator;
import com.mediatek.mediatekdm.mdm.SessionStateObserver.SessionState;
import com.mediatek.mediatekdm.mdm.SessionStateObserver.SessionType;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;

import java.util.List;

public class CMCCComponent implements IDmComponent {
    static final String NAME = "CMCC";

    private DmService mService = null;

    @Override
    public boolean acceptOperation(SessionInitiator initiator, DmOperation operation) {
        return false;
    }

    @Override
    public void attach(DmService service) {
        mService = service;
    }

    @Override
    public void detach(DmService service) {
        mService = null;
    }

    @Override
    public DispatchResult dispatchBroadcast(Context context, Intent intent) {
        String intentAction = intent.getAction();
        Log.i(TAG.CMCCCOMPONENT, "Received intentAction: " + intentAction);
        if (intentAction == null) {
            return DispatchResult.IGNORE;
        }
        if (DmConst.IntentAction.DM_BOOT_COMPLETED.equals(intentAction)) {
            sendStickyBroadcastForAutoBoot(context);
        }
        // Filter CMCC WAP push message ,and ignore other operator WAP push message
        if (intentAction.equalsIgnoreCase(IntentAction.DM_WAP_PUSH)) {
            Log.i(TAG.CMCCCOMPONENT, "WAP push message received");
            String type = intent.getType();
            Log.i(TAG.CMCCCOMPONENT, "WAP push message type : " + type);
            if (type != null && type.equals(DmConst.IntentType.DM_NIA)) {
                byte nia[] = intent.getByteArrayExtra("data");
                if (nia != null) {
                    int length = nia[DmConst.DM_NIA.DM_INDEX_LENGTH_SERVER_ID];
                    String serverIdNIA = new String(nia, DmConst.DM_NIA.DM_INDEX_CONTENT_SERVER_ID, length);
                    Log.i(TAG.CMCCCOMPONENT, "WAP push message NIA ServerID is : " + serverIdNIA);
                        if (serverIdNIA.equals(DmConst.DM_NIA_SERVER.CMCC_DMACC_SERVER_ID)) {
                            //For CMCC wap push message,If the device is not registered 
                            //or the message is not from registered SIM, ignore this message
                            //Context context = DmApplication.getInstance();
                            Intent isAutoBootintent = null;
                            isAutoBootintent = registerReceiverForAutoBoot(context);
                            if(isAutoBootintent == null) {
                                Log.i(TAG.CMCCCOMPONENT, "not receive BOOT_COMPLETED broadcast");
                                if (!enableAutoBoot(context)) {
                                    return DispatchResult.ACCEPT;
                                }
                            } else {
                                Log.i(TAG.CMCCCOMPONENT, "received BOOT_COMPLETED broadcast");
                                boolean enableAutoBoot = isAutoBootintent.getBooleanExtra(DmConst.ExtraKey.IS_AUTO_BOOT,false);
                                Log.i(TAG.CMCCCOMPONENT, "boot isAutoBoot is:" + enableAutoBoot);
                                if(!enableAutoBoot) {
                                    return DispatchResult.ACCEPT;
                                }
                            }
                            int simSlot = getRegisteredSimId(context);
                            int receivedSimId = intent.getIntExtra("simId", -1);
                            if (receivedSimId != simSlot) {
                                Log.i(TAG.CMCCCOMPONENT, "the device is not registered");
                                return DispatchResult.IGNORE;
                            }
                            if (isNeedNotify()) {
                                //if user not set notify flag 
                                startCollectSetPermissionConfirmDialog(context,intent);
                                return DispatchResult.ACCEPT;
                            } else {
                                if(isNeedAgree()) {
                                    Intent serviceIntent = new Intent(intent);
                                    serviceIntent.setClass(context, DmService.class);
                                    serviceIntent.setAction(IntentAction.DM_WAP_PUSH);
                                    serviceIntent.putExtra(DmConst.DM_NIA_SERVER.IS_CMCC_DMACC_SERVER, true);
                                    context.startService(serviceIntent);
                                    return DispatchResult.ACCEPT;
                                } else {
                                    return DispatchResult.ACCEPT;
                                }
                            }
                        } else {
                            //ignore other operator NIA message by server id segment
                            return DispatchResult.IGNORE;
                        }
                }
            } else {
                // ignore other wap push message type
                return DispatchResult.IGNORE;
            }
        } else if (intentAction.equalsIgnoreCase(IntentAction.DM_COLLECT_SET_DIALOG_END)) {
            boolean isNeedAgree = intent.getBooleanExtra(DmConst.ExtraKey.IS_NEED_AGREE, true);
            boolean isNeedNotify = intent.getBooleanExtra(DmConst.ExtraKey.IS_NEED_NOTIFY, true);
            Log.d(TAG.CMCCCOMPONENT, "User isNeedAgree " + isNeedAgree);
            Log.d(TAG.CMCCCOMPONENT, "User isNeedNotify " + isNeedNotify);
            if (!isNeedNotify) {
                CollectSetPermissionControl.getInstance().writeKeyValueToFile(isNeedNotify, isNeedAgree);
            }
            if (!isNeedAgree) {
                //if user do not  agree this wap push task,return
                return DispatchResult.ACCEPT;
            } else {
                //user agree this task,Set intent action as wap push message, forward it to dm service
                Log.w(TAG.CMCCCOMPONENT, "Wap push message user confirmed. Forward it to service.");
                Intent serviceIntent = new Intent(intent);
                serviceIntent.setClass(context, DmService.class);
                serviceIntent.setAction(IntentAction.DM_WAP_PUSH);
                serviceIntent.putExtra(DmConst.DM_NIA_SERVER.IS_CMCC_DMACC_SERVER, true);
                context.startService(serviceIntent);
                return DispatchResult.ACCEPT;
            }
        } else if (intentAction.equalsIgnoreCase(IntentAction.DM_SMSREG_MESSAGE_NEW)) {
            CollectSetPermissionControl.getInstance().resetKeyValue();
            return DispatchResult.ACCEPT;
        } else {
            return DispatchResult.IGNORE;
        }
        return DispatchResult.IGNORE;
    }

    @Override
    public DispatchResult dispatchCommand(Intent intent) {
        return DispatchResult.IGNORE;
    }

    @Override
    public void dispatchMmiProgressUpdate(DmOperation operation, int current, int total) {
    }

    @Override
    public DispatchResult dispatchOperationAction(OperationAction action, DmOperation operation) {
        return DispatchResult.IGNORE;
    }

    @Override
    public SessionHandler dispatchSessionStateChange(
            SessionType type,
            SessionState state,
            int lastError,
            SessionInitiator initiator,
            DmOperation operation) {
        return null;
    }

    @Override
    public boolean forceSilentMode() {
        return false;
    }

    @Override
    public IBinder getBinder(Intent intent) {
        return null;
    }

    @Override
    public String getDlPackageFilename() {
        return null;
    }

    @Override
    public String getDlResumeFilename() {
        return null;
    }

    @Override
    public String getName() {
        return NAME;
    }

    @Override
    public void kickoff(Context context) {
    }

    @Override
    public void configureDmTree(MdmTree tree) {
        String mccmnc = null;
        Context context = DmApplication.getInstance();
        if (DmConfig.getInstance().useSmsReg()) {
            int simSlot = getRegisteredSimId(context);
            if (simSlot != -1) {
                mccmnc = PlatformManager.getInstance().getSimOperator(simSlot);
                Log.w(TAG.CONTROLLER, "reg sim MccMnc: " + mccmnc);
                IoNodeHandlerRegister dr = new IoNodeHandlerRegister(context, mccmnc);
                dr.registerSettingsNodeIoHandler();
            }
        }

    }

    /**
     * getRegisteredSimId
     * 
     * @param context
     * @return: -1 -- no sim registered 0 -- Device support one sim and is registered other -- registered sim ID.
     */
    public static int getRegisteredSimId(Context context) {
        String registerIMSI = null;

        try {
            DmAgent agent = PlatformManager.getInstance().getDmAgent();
            if (agent == null) {
                Log.e(TAG.COMMON, "get dm_agent_binder failed.");
                return -1;
            }
            byte[] imsiData = agent.readImsi();
            if (imsiData == null) {
                Log.e(TAG.COMMON, "get imsi failed.");
                return -1;
            }
            registerIMSI = new String(imsiData);
        } catch (RemoteException e) {
            Log.e(TAG.COMMON, "get registered IMSI failed", e);
        }

        if (registerIMSI == null) {
            Log.e(TAG.COMMON, "get registered IMSI failed");
            return -1;
        }

        Log.i(TAG.COMMON, "registered imsi=" + registerIMSI);

        List<SimInfoRecord> simList = PlatformManager.getInstance().getInsertedSimInfoList(context);
        for (SimInfoRecord sim : simList) {
            String imsi = PlatformManager.getInstance().getSubscriberId(sim.mSimSlotId);
            Log.d(TAG.COMMON, "Sim" + sim.mSimSlotId + ":imsi=" + imsi);
            if (imsi != null && imsi.equals(registerIMSI)) {
                Log.d(TAG.COMMON, "registered SIM card is SIM" + sim.mSimSlotId);
                return sim.mSimSlotId;
            }
        }

        Log.d(TAG.COMMON, "getRegisteredSimId error!");
        return -1;
    }

    @Override
    public DispatchResult validateWapPushMessage(Intent intent) {
        if (!DmConfig.getInstance().useSmsReg()) {
            return DispatchResult.ACCEPT;
        }

        // Check whether the NIA came from the registered SIM card
        int registeredSimId = getRegisteredSimId(mService);
        if (DmFeatureSwitch.MTK_GEMINI_SUPPORT) {
            int receivedSimId = intent.getIntExtra("simId", -1);
            if (receivedSimId != registeredSimId) {
                Log.w(TAG.SERVICE, "NIA is not from the registered card. Do nothing.");
                Log.d(TAG.SERVICE, "receivedSimId = " + receivedSimId + ", registeredSimId = " + registeredSimId);
                return DispatchResult.ABORT;
            } else {
                return DispatchResult.ACCEPT;
            }
        } else {
            return DispatchResult.ACCEPT;
        }
    }

    @Override
    public boolean checkPrerequisites() {
        if (!DmConfig.getInstance().useSmsReg()) {
            return true;
        }

        return !(getRegisteredSimId(mService) == -1);
    }

    private boolean isNeedNotify() {
        boolean isNeedNotify = CollectSetPermissionControl.getInstance().readKeyValueFromFile(DmConst.ExtraKey.IS_NEED_NOTIFY);
        Log.i(TAG.RECEIVER, "read file isNeedNotify flag is " + isNeedNotify);
        return isNeedNotify;
    }

    private boolean isNeedAgree() {
        boolean isNeedAgree = CollectSetPermissionControl.getInstance().readKeyValueFromFile(DmConst.ExtraKey.IS_NEED_AGREE);
        Log.i(TAG.RECEIVER, "read file isNeedAgree flag is " + isNeedAgree);
        return isNeedAgree;
    }

    public void startCollectSetPermissionConfirmDialog(Context context, Intent intent) {
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.setClass(context, CollectSetPermissionDialog.class);
        context.startActivity(intent);
    }

	@Override
	public PLHttpConnection getHttpConnection(DmOperation operation) {
		// TODO Auto-generated method stub
		return null;
	}

    public void sendStickyBroadcastForAutoBoot(Context context) {
        Intent intent = new Intent();
        intent.setAction(DmConst.IntentAction.DM_AUTO_BOOT_FLAG);
        boolean isAutoBoot = enableAutoBoot(context);
        intent.putExtra(DmConst.ExtraKey.IS_AUTO_BOOT, isAutoBoot);
        context.sendStickyBroadcast(intent);
    }

    public boolean enableAutoBoot(Context context) {
        //DEF_DM_BOOT_START_ENABLE_VALUE = 1;
        int value = System.getInt(context.getContentResolver(), System.DM_BOOT_START_ENABLE_KEY, 1);
        Log.w(TAG.RECEIVER, "MDM AutoBoot switch is  " + value);
        if (value == 1) {
            return true;
        } else {
            return false;
        }
    }

    public Intent registerReceiverForAutoBoot(Context context) {
        return context.getApplicationContext().registerReceiver(new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                // TODO Auto-generated method stub
            }
        }, new IntentFilter(DmConst.IntentAction.DM_AUTO_BOOT_FLAG));
    }
}
