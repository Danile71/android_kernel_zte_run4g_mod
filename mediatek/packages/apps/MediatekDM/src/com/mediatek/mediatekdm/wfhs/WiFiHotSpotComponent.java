package com.mediatek.mediatekdm.wfhs;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.util.Set;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;

import com.mediatek.common.passpoint.WifiTree;
import com.mediatek.mediatekdm.DmApplication;
import com.mediatek.mediatekdm.DmConfig;
import com.mediatek.mediatekdm.DmConst;
import com.mediatek.mediatekdm.DmOperation;
import com.mediatek.mediatekdm.DmOperationManager;
import com.mediatek.mediatekdm.DmService;
import com.mediatek.mediatekdm.IDmComponent;
import com.mediatek.mediatekdm.PlatformManager;
import com.mediatek.mediatekdm.SessionHandler;
import com.mediatek.mediatekdm.DmConst.IntentAction;
import com.mediatek.mediatekdm.DmConst.TAG;
import com.mediatek.mediatekdm.DmOperation.KEY;
import com.mediatek.mediatekdm.IDmComponent.DispatchResult;
import com.mediatek.mediatekdm.mdm.MdmEngine;
import com.mediatek.mediatekdm.mdm.MdmException;
import com.mediatek.mediatekdm.mdm.MdmTree;
import com.mediatek.mediatekdm.mdm.PLHttpConnection;
import com.mediatek.mediatekdm.mdm.SessionInitiator;
import com.mediatek.mediatekdm.mdm.SessionStateObserver.SessionState;
import com.mediatek.mediatekdm.mdm.SessionStateObserver.SessionType;
import com.mediatek.mediatekdm.util.Path;
import com.mediatek.mediatekdm.util.Utilities;
import com.mediatek.mediatekdm.operator.cmcc.IoNodeHandlerRegister;
import com.mediatek.mediatekdm.wfhs.SubscriptionSessionInitiator;
import com.mediatek.mediatekdm.wfhs.MdmWifi;
import com.mediatek.xlog.Xlog;

public class WiFiHotSpotComponent implements IDmComponent {

    private MdmWifi mMdmWifi;
    private WifiTreeManager mWifiTreeManager; 
    private Context mContext;
    static final String NAME = "WIFIHS2.0";

    static final class IntentExtraKey {
        public static final String OPERATION_KEY_ACTION = "action";
        public static final String OPERATION_KEY_TYPE = "type";
    }
    @Override
    public boolean acceptOperation(SessionInitiator initiator, DmOperation operation) {
        // TODO Auto-generated method stub
        return false;
    }

    @Override
    public void attach(DmService service) {
        // TODO Auto-generated method stub
        Log.i(TAG.WFHS, "Create wifi hotspot 2.0."); 
        mContext = service;
        mMdmWifi = new MdmWifi(service);
        mWifiTreeManager = new WifiTreeManager(service, mMdmWifi);
        mMdmWifi.onInit();

    }

    @Override
    public void detach(DmService service) {
        // TODO Auto-generated method stub
        mContext = null;
        mMdmWifi = null;
        mWifiTreeManager = null;
    }

    public MdmWifi getMdmWifi() {
    	return mMdmWifi;
    }

    @Override
    public DispatchResult dispatchBroadcast(Context context, Intent intent) {
        // TODO Auto-generated method stub
            String action = intent.getAction();
            boolean result = false;
            result = isWifiHS20Action(action);
            Log.i(TAG.WFHS, "isWifiHS20Action :" + result);
            if (result) {
                //start service to handle this action
                Log.i(TAG.WFHS, "receive [WiFi Hotspot 2.0] action : " + action);
                Intent serviceIntent = new Intent(intent);
                serviceIntent.setClass(context, DmService.class);
                context.startService(serviceIntent);
                return DispatchResult.ACCEPT;
            } else {
                return DispatchResult.IGNORE;
            }
    }

    @Override
    public DispatchResult dispatchCommand(Intent intent) {
        // TODO Auto-generated method stub
        String action = intent.getAction();
        DispatchResult result = DispatchResult.IGNORE;
        Log.i(TAG.WFHS, "dispatchCommand intent Action :" + action);
        if (action == null) {
            return DispatchResult.IGNORE;
        } else if (isWifiHS20DMAction(action)) {
            Log.i(TAG.WFHS, "DM command +++++++");
            result = handleWiFiDMIntent(intent);
            Log.i(TAG.WFHS, "DM command -------");
        } else if (isWifiHS20SOAPAction(action)) {
            Log.i(TAG.WFHS, "SOAP command +++++++");
            result = handleSOAPIntent(intent);
            Log.i(TAG.WFHS, "SOAP command -------");
        } else if (isWifiHS20GetTreeAction(action)) {
            Log.i(TAG.WFHS, "DM tree xml->class command +++++++");
            result = handleGetTreeIntent(intent);
            Log.i(TAG.WFHS, "DM tree xml->class command -------");
        }
        return result;
    }

    @Override
    public void dispatchMmiProgressUpdate(DmOperation operation, int current, int total) {
        // TODO Auto-generated method stub

    }

    @Override
    public DispatchResult dispatchOperationAction(OperationAction action, DmOperation operation) {
        // TODO Auto-generated method stub
        String type = operation.getProperty(KEY.TYPE);
        switch (action) {
            case NEW:
                if (KEY.WFHS_TAG.equals(type)) {
                    Intent intent = Dmoperation2Intent(operation);
                    Log.i(TAG.WFHS, "dispatchOperation intent Action :" + action);
                    String intentAction = intent.getAction();
                    
                    if (isWifiHS20DMAction(intentAction)) {
                        return ExecuteWIFIDMCommand(intent);
                    } else if (isWifiHS20SOAPAction(intentAction)) {
                        return ExecuteSOAPCommand(intent);
                    } else if (isWifiHS20GetTreeAction(intentAction)) {
                        return ExecuteGetTreeCommand(intent);
                    }
                    return DispatchResult.IGNORE;
                }    
            default:
                return DispatchResult.IGNORE;
        }
    }

    @Override
    public SessionHandler dispatchSessionStateChange(SessionType type, SessionState state, int lastError,
            SessionInitiator initiator, DmOperation operation) {
        // TODO Auto-generated method stub
        if (operation.getProperty(KEY.TYPE).contains(KEY.WFHS_TAG)) {
            return mWifiTreeManager;
        } else {
            return null;
        }
    }

    @Override
    public boolean forceSilentMode() {
        // TODO Auto-generated method stub
        return false;
    }

    @Override
    public IBinder getBinder(Intent intent) {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public String getDlPackageFilename() {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public String getDlResumeFilename() {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public String getName() {
        // TODO Auto-generated method stub
        return NAME;
    }

    @Override
    public void kickoff(Context context) {
        // TODO Auto-generated method stub

    }

    @Override
    public void configureDmTree(MdmTree tree) {
        // TODO Auto-generated method stub
        boolean result = false;
        try {
          //Init OSU package1  data to server
            result = isWifiPackage1Ready(Path.DM_OSU_PACKAGE1_FILE, MdmWifi.DATA.CDATA_OSU);
            Log.i(TAG.WFHS, "Init OSU package1 result: " + result);
            //Init policy update package1  data to server
            result = isWifiPackage1Ready(Path.DM_POLICY_PACKAGE1_FILE, MdmWifi.DATA.CDATA_POLICY_UPDATE);
            Log.i(TAG.WFHS, "Init PolicyUpdate package1 result: " + result);
            //Init Remediation package1 data to server
            result = isWifiPackage1Ready(Path.DM_REM_PACKAGE1_FILE, MdmWifi.DATA.CDATA_REMEDIATION);
            Log.i(TAG.WFHS, "Init Remediation package1 result: " + result);
        } catch (Exception e) {
            // TODO: handle exception
            Log.e(TAG.WFHS, e.getMessage());
        }
        
    }

    @Override
    public DispatchResult validateWapPushMessage(Intent intent) {
        return DispatchResult.IGNORE;
    }

    @Override
    public boolean checkPrerequisites() {
        return true;
    }
    private boolean isWifiHS20Action(String action) {
        boolean result = false;
        if (action == null) {
            Log.i(TAG.WFHS, "receive  action is null");
            return result;
        }
        if (action.equals(MdmWifi.SessionAction.PROVISION_START)) {
            Log.i(TAG.WFHS, "receive [WiFi Hotspot 2.0] OSU start intent."); 
            result = true;
        } else if (action.equals(MdmWifi.SessionAction.REMEDIATION_START)) {
            Log.i(TAG.WFHS, "receive [WiFi Hotspot 2.0] remediation start intent."); 
            result = true;
        } else if (action.equals(MdmWifi.SessionAction.POLICY_UPDATE_START)) {
            Log.i(TAG.WFHS, "receive [WiFi Hotspot 2.0] policy update intent."); 
            result = true;
        } else if (action.equals(MdmWifi.SessionAction.LAUNCH_BROWSER_RESPONSE)) {
            Log.i(TAG.WFHS, "receive [WiFi Hotspot 2.0] launch browser intent."); 
            result = true;
        } else if (action.equals(MdmWifi.SessionAction.LAUNCH_BROWSER_RESPONSE_REM)) {
            Log.i(TAG.WFHS, "receive [WiFi Hotspot 2.0] launch browser remediation intent."); 
            result = true;
        } else if (action.equals(MdmWifi.SessionAction.ENROLLMENT_RESPONSE)) {
            Log.i(TAG.WFHS, "receive [WiFi Hotspot 2.0] enrollment response intent."); 
            result = true;
        } else if (action.equals(MdmWifi.SessionAction.REENROLLMENT_RESPONSE)) {
            Log.i(TAG.WFHS, "receive [WiFi Hotspot 2.0] re-enrollment response intent."); 
            result = true;
        } else if (action.equals(MdmWifi.SessionAction.NEGOTIATE_RESPONSE)) {
            Log.i(TAG.WFHS, "receive [WiFi Hotspot 2.0] negotiate response intent."); 
            result = true;
        } else if (action.equals(MdmWifi.SessionAction.SOAP_PROVISION_START)) {
            Log.i(TAG.WFHS, "receive [WiFi Hotspot 2.0] SOAP PROVISION intent."); 
            result = true;
        } else if (action.equals(MdmWifi.SessionAction.SOAP_REMEDIATION_START)) {
            Log.i(TAG.WFHS, "receive [WiFi Hotspot 2.0] SOAP REMEDIATION intent."); 
            result = true;
        } else if (action.equals(MdmWifi.SessionAction.SOAP_POLICY_UPDATE_START)) {
            Log.i(TAG.WFHS, "receive [WiFi Hotspot 2.0] SOAP POLICY_UPDATE intent."); 
            result = true;
        } else if (action.equals(MdmWifi.SessionAction.SOAP_SIM_PROVISION_START)) {
            Log.i(TAG.WFHS, "receive [WiFi Hotspot 2.0] SOAP SIM_PROVISION intent."); 
            result = true;
        } else if (action.equals(MdmWifi.SessionAction.GET_DM_TREE_START)) {
            Log.i(TAG.WFHS, "receive [WiFi Hotspot 2.0] GET DM tree intent."); 
            result = true;
        }
        return result;
    }
    /**
     * Trigger Subscription Provision session to start wifi hotspot 2.0 subscription provisioning
     * @param int sessionType - the session type
     */    
    public void triggerSubscriptionProvisionSession(Intent intent) {
        int sessionId = intent.getIntExtra("type", 0);            
        try {
            mMdmWifi.initOSUSession(intent);
            triggerSubscriptionProvisionSession(sessionId, new SubscriptionSessionInitiator(sessionId));
            Log.i(TAG.WFHS, "Provision session triggered."); 
        } catch (MdmException e) {
            Log.e(TAG.WFHS, "Exception accur when trigger Subscription Provision session.");
            e.printStackTrace();
        }
    }

    /**
     * Trigger Subscription Remediation session to start wifi hotspot 2.0 remediation
     * @param int sessionType - the session type
     */ 
    public void triggerSubscriptionRemediationSession(Intent intent) {
        int sessionId = 2;  
        try {
            mMdmWifi.initRemediationSession(intent);
            triggerSubscriptionRemediationSession(sessionId, new SubscriptionSessionInitiator(sessionId));
            Log.i(TAG.WFHS, "Remediation session triggered."); 
        } catch (MdmException e) {
            Log.e(TAG.WFHS, "Exception accur when trigger Subscription Remediation session.");
            e.printStackTrace();
        }
    }
    
    /**
     * Trigger Subscription Policy Update session to update wifi hotspot 2.0 policy
     * @param int sessionType - the session type
     */ 
    public void triggerSubscriptionPolicyUpdateSession(Intent intent) {
        int sessionId = intent.getIntExtra("type", 30); 
        try {
            mMdmWifi.initPolicyUpdateSession(intent);
            triggerSubscriptionPolicyUpdateSession(sessionId, new SubscriptionSessionInitiator(sessionId));
            Log.i(TAG.WFHS, "Policy Update session triggered."); 
        } catch (MdmException e) {
            Log.e(TAG.WFHS, "Exception accur when trigger Policy Update session.");
            e.printStackTrace();
        }
        Log.i(TAG.WFHS, "Policy Update session triggered."); 
    }
    
    private  DispatchResult handleGetTreeIntent(Intent intent) {
        String action = intent.getAction();
        Log.i(TAG.WFHS, "WiFiDMIntent Action is:" + action);
        DmOperation operation = createDmOperation(intent, action);
        DmOperationManager.getInstance().enqueue(operation, true);
        return DispatchResult.ACCEPT_AND_TRIGGER;
    }
    
    private  DispatchResult handleWiFiDMIntent(Intent intent) {
        String action = intent.getAction();
        Log.i(TAG.WFHS, "WiFiDMIntent Action is:" + action);
        
        if (action.equals(MdmWifi.SessionAction.PROVISION_START)||
                action.equals(MdmWifi.SessionAction.REMEDIATION_START)||
                action.equals(MdmWifi.SessionAction.POLICY_UPDATE_START)) {
            DmOperation operation = createDmOperation(intent, action);
            DmOperationManager.getInstance().enqueue(operation, true);
            return DispatchResult.ACCEPT_AND_TRIGGER;
        } else if (action.equals(MdmWifi.SessionAction.LAUNCH_BROWSER_RESPONSE)
                || action.equals(MdmWifi.SessionAction.LAUNCH_BROWSER_RESPONSE_REM)) {
            Log.i(TAG.WFHS, "start subscription mo creation");
            String response = intent.getStringExtra("responsedata");
            mMdmWifi.notifyExecute(response);
            return DispatchResult.ACCEPT;
        } else if (action.equals(MdmWifi.SessionAction.ENROLLMENT_RESPONSE)) {
            Log.i(TAG.WFHS, "enrollment success");
            mMdmWifi.notifyExecute(null);
            return DispatchResult.ACCEPT;
        } else if (action.equals(MdmWifi.SessionAction.REENROLLMENT_RESPONSE)) {
            Log.i(TAG.WFHS, "re-enrollment success");
            mMdmWifi.notifyExecute(null);
            return DispatchResult.ACCEPT;
        } else if (action.equals(MdmWifi.SessionAction.NEGOTIATE_RESPONSE)) {
            Log.i(TAG.WFHS, "negotiate ");
            mMdmWifi.notifyExecute(null);
            return DispatchResult.ACCEPT;
        } 
        return DispatchResult.IGNORE;
    }
    
    private  DispatchResult handleSOAPIntent(Intent intent) {
        Log.i(TAG.WFHS, "SOAP command ++++");
        String action = intent.getAction();
        DmOperation operation = createDmOperation(intent, action);
        DmOperationManager.getInstance().enqueue(operation, true);
        return DispatchResult.ACCEPT_AND_TRIGGER;
    }
    
    private DispatchResult ExecuteWIFIDMCommand (Intent intent) {
        String intentAction = intent.getAction();
        if (intentAction == null) {
            return DispatchResult.IGNORE;
        } else if (intentAction.equals(MdmWifi.SessionAction.PROVISION_START)) {        
            Log.i(TAG.WFHS, "start subscription provision");
            triggerSubscriptionProvisionSession(intent);
            return DispatchResult.ACCEPT;
        } else if (intentAction.equals(MdmWifi.SessionAction.REMEDIATION_START)) {
            Log.i(TAG.WFHS, "start subscription remediation");
            triggerSubscriptionRemediationSession(intent);
            return DispatchResult.ACCEPT;
        } else if (intentAction.equals(MdmWifi.SessionAction.POLICY_UPDATE_START)) {
            Log.i(TAG.WFHS, "start subscription policy update");
            triggerSubscriptionPolicyUpdateSession(intent);
            return DispatchResult.ACCEPT;
        }
        return DispatchResult.IGNORE;
    }
    
    private DispatchResult ExecuteSOAPCommand (Intent intent) {
        final String CMD_ADD_SUBTREE  =    "addsubtree";
        final String CMD_REPLACE_SUBTREE = "replacesubtree";
        final String CMD_DELETE_SUBTREE = "deletesubtree";
        final String IS_NEED_WIFI_TREE = "isneedwifitree";
        boolean result = false;
        String startAction = null;
        String endAction = null;
        String cmd = null;
        Intent responseSoapIntent = new Intent();
        startAction = intent.getAction();
        endAction = getObtainSoapEndAction(startAction);
        Log.i(TAG.WFHS, "soap intent start action is :" + startAction);
        Log.i(TAG.WFHS, "soap intent end action is :" + endAction);
        cmd = intent.getStringExtra("cmd");
        Log.i(TAG.WFHS, "soap intent cmd  is :" + cmd);
        String targetUri = intent.getStringExtra("targeturi");
        Log.i(TAG.WFHS, "soap intent  targeturi is :" + targetUri);
        String subtreeData = intent.getStringExtra("data");
        // Log.i(TAG.WFHS, "soap intent  subtreeData is :" + subtreeData);
        //Execute soap command
        if (null == cmd) {
            result = false;
            Log.i(TAG.WFHS, "get soap intent cmd  is null:");
        } else if(cmd.equalsIgnoreCase(CMD_ADD_SUBTREE)) {
            Log.i(TAG.WFHS, "ADD Subtree command");
            if (subtreeData != null) {
                result = mMdmWifi.addSubTree(targetUri, subtreeData.getBytes());
            } else {
                result = false;
                Log.i(TAG.WFHS, "soap cmd subtreeData is null:");
            }
            boolean isNeedWifitree = intent.getBooleanExtra(IS_NEED_WIFI_TREE, false);
            Log.i(TAG.WFHS, "soap cmd isNeedWifitree: " + isNeedWifitree);
            WifiTree wifiTree = null;
            if(isNeedWifitree) {
                wifiTree = mMdmWifi.getDMTreeToSOAPTree();
                mMdmWifi.dumpTreeInfo(wifiTree);

            }
            responseSoapIntent.putExtra("wifitree", wifiTree);
        } else if(cmd.equalsIgnoreCase(CMD_REPLACE_SUBTREE)) {
            Log.i(TAG.WFHS, "Replace Subtree command");
            if (subtreeData != null) {
                result = mMdmWifi.replaceSubTree(targetUri, subtreeData.getBytes());
            } else {
                result = false;
                Log.i(TAG.WFHS, "soap cmd subtreeData is null:");
            }
            boolean isNeedWifitree = intent.getBooleanExtra(IS_NEED_WIFI_TREE, false);
            Log.i(TAG.WFHS, "soap cmd isNeedWifitree: " + isNeedWifitree);
            WifiTree wifiTree = null;
            if(isNeedWifitree) {
                wifiTree = mMdmWifi.getDMTreeToSOAPTree();
                mMdmWifi.dumpTreeInfo(wifiTree);

            }
            responseSoapIntent.putExtra("wifitree", wifiTree);
        } else if(cmd.equalsIgnoreCase(CMD_DELETE_SUBTREE)) {
            Log.i(TAG.WFHS, "delete Subtree command");
            if (targetUri != null) {
                result = mMdmWifi.deleteNode(targetUri);
            } else {
                result = false;
                Log.i(TAG.WFHS, "soap cmd subtreeData is null:");
            }
        } else {
            Log.i(TAG.WFHS, "Unknown soap command");
        }
        Log.i(TAG.WFHS, "The result of execute soap command is :" + result);
        
        //send result to soap
        responseSoapIntent.setAction(getObtainSoapEndAction(startAction));
        responseSoapIntent.putExtra("cmd", cmd);
        if (result) {
            responseSoapIntent.putExtra("result", 0);
        } else {
            responseSoapIntent.putExtra("result", -1);
        }
        
        mContext.sendBroadcast(responseSoapIntent);
        DmOperationManager mOperationManager = DmOperationManager.getInstance();
        mOperationManager.finishCurrent();
        Log.i(TAG.WFHS, "SOAP command ----");
        return DispatchResult.ACCEPT;

    }
    
    private DispatchResult ExecuteGetTreeCommand (Intent intent) {
        WifiTree wifitree = mMdmWifi.getDMTreeToSOAPTree();
        int result = -1;
        if (wifitree != null) {
            result = 0;
            mMdmWifi.dumpTreeInfo(wifitree);
        }
        Intent dmTreeIntent = new Intent();
        dmTreeIntent.setAction(MdmWifi.SessionAction.GET_DM_TREE_END);
        dmTreeIntent.putExtra("wifitree", wifitree);
        dmTreeIntent.putExtra("result", result);
        mContext.sendBroadcast(dmTreeIntent);
        DmOperationManager mOperationManager = DmOperationManager.getInstance();
        mOperationManager.finishCurrent();
        return DispatchResult.ACCEPT;
    }
    public boolean isWifiHS20DMAction(String action) {
        boolean result = false;
        if (action == null) {
            Log.i(TAG.WFHS, "receive  action is null");
            return result;
        }
        if (action.equals(MdmWifi.SessionAction.PROVISION_START)) {
            Log.i(TAG.WFHS, "receive [WiFi Hotspot 2.0] OSU start intent."); 
            result = true;
        } else if (action.equals(MdmWifi.SessionAction.REMEDIATION_START)) {
            Log.i(TAG.WFHS, "receive [WiFi Hotspot 2.0] remediation start intent."); 
            result = true;
        } else if (action.equals(MdmWifi.SessionAction.POLICY_UPDATE_START)) {
            Log.i(TAG.WFHS, "receive [WiFi Hotspot 2.0] policy update intent."); 
            result = true;
        } else if (action.equals(MdmWifi.SessionAction.LAUNCH_BROWSER_RESPONSE)) {
            Log.i(TAG.WFHS, "receive [WiFi Hotspot 2.0] launch browser intent."); 
            result = true;
        } else if (action.equals(MdmWifi.SessionAction.LAUNCH_BROWSER_RESPONSE_REM)) {
            Log.i(TAG.WFHS, "receive [WiFi Hotspot 2.0] launch browser remediation intent."); 
            result = true;
        } else if (action.equals(MdmWifi.SessionAction.ENROLLMENT_RESPONSE)) {
            Log.i(TAG.WFHS, "receive [WiFi Hotspot 2.0] enrollment response intent."); 
            result = true;
        } else if (action.equals(MdmWifi.SessionAction.REENROLLMENT_RESPONSE)) {
            Log.i(TAG.WFHS, "receive [WiFi Hotspot 2.0] re-enrollment response intent."); 
            result = true;
        } else if (action.equals(MdmWifi.SessionAction.NEGOTIATE_RESPONSE)) {
            Log.i(TAG.WFHS, "receive [WiFi Hotspot 2.0] negotiate response intent."); 
            result = true;
        }
        return result;
    }

    public boolean isWifiHS20SOAPAction(String action) {
        boolean result = false;
        if (action.equals(MdmWifi.SessionAction.SOAP_PROVISION_START)) {
            Log.i(TAG.WFHS, "receive [WiFi Hotspot 2.0] SOAP PROVISION intent."); 
            result = true;
        } else if (action.equals(MdmWifi.SessionAction.SOAP_REMEDIATION_START)) {
            Log.i(TAG.WFHS, "receive [WiFi Hotspot 2.0] SOAP REMEDIATION intent."); 
            result = true;
        } else if (action.equals(MdmWifi.SessionAction.SOAP_POLICY_UPDATE_START)) {
            Log.i(TAG.WFHS, "receive [WiFi Hotspot 2.0] SOAP POLICY_UPDATE intent."); 
            result = true;
        } else if (action.equals(MdmWifi.SessionAction.SOAP_SIM_PROVISION_START)) {
            Log.i(TAG.WFHS, "receive [WiFi Hotspot 2.0] SOAP SIM_PROVISION intent."); 
            result = true;
        }
         return result;
    }
    
    public boolean isWifiHS20GetTreeAction(String action) {
        boolean result = false;
        if (action.equals(MdmWifi.SessionAction.GET_DM_TREE_START)) {
            Log.i(TAG.WFHS, "receive [WiFi Hotspot 2.0] GET DM tree intent."); 
            result = true;
        }
        return result;
    }

    
    
    /** 
     * Get soap start action obtain end action
     */
    public String getObtainSoapEndAction(String startAction) {
        if (null == startAction) {
            return "false";
        } else if(MdmWifi.SessionAction.SOAP_PROVISION_START.equals(startAction)) {
            return MdmWifi.SessionAction.SOAP_PROVISION_END;
        } else if(MdmWifi.SessionAction.SOAP_REMEDIATION_START.equals(startAction)) {
            return MdmWifi.SessionAction.SOAP_REMEDIATION_END;
        } else if(MdmWifi.SessionAction.SOAP_POLICY_UPDATE_START.equals(startAction)) {
            return MdmWifi.SessionAction.SOAP_POLICY_UPDATE_END;
        } else if(MdmWifi.SessionAction.SOAP_SIM_PROVISION_START.equals(startAction)) {
            return MdmWifi.SessionAction.SOAP_SIM_PROVISION_END;
        } else if(MdmWifi.SessionAction.GET_DM_TREE_START.equals(startAction)) {
            return MdmWifi.SessionAction.GET_DM_TREE_END;
        } else {
            return "false";
        }
    }
    
    public Boolean isWifiPackage1Ready(String fileName, String data) throws Exception{
        Boolean ret = false;
        FileOutputStream out=null;
        Log.i(TAG.WFHS,"Init package1 file name:" + fileName);
        Log.i(TAG.WFHS,"Init package1 data:" + data);
        try {
            File package1 = new File(Path.getPathInData(mContext, fileName));        
            File dataFilesDir = mContext.getFilesDir();
            if(!package1.exists()) {
                if(!dataFilesDir.exists()){
                    Log.e(TAG.WFHS, "there is no /files dir in dm folder");
                    if(dataFilesDir.mkdir()){
                        // dirty
                        Utilities.openPermission(dataFilesDir.getAbsolutePath());
                    } else {
                         throw new Error("Failed to create folder in data folder.");
                    }
                }
                out=new FileOutputStream(package1);
                out.write(data.getBytes());
                out.close();
            } else {
                Log.i(TAG.WFHS,"package1 xml is exists :" + fileName);
            }
        } catch(Exception e) {
            Log.e(TAG.WFHS, e.getMessage());
        } 
        ret = true;
        return ret;
    }

    public void triggerSubscriptionProvisionSession(int sessionType, SessionInitiator initiator) throws MdmException {
        Log.e(TAG.WFHS,"[ENGINE] +triggerSubscriptionProvisionSession()");
        String CDATA_OSU = getFileString(Path.getPathInData(mContext, Path.DM_OSU_PACKAGE1_FILE));
        MdmEngine.getInstance().triggerGenericAlertSession(
                "Wi-Fi",                                   //correlator
                MdmWifi.Format.XML,                             //format
                MdmWifi.AlertType.SUBSCRIPTION_REGISTRATION,    //alert type
                null,                                           //mark
                MdmWifi.Source.PROVIOSN,                        //source
                null,                                           //target
                CDATA_OSU,                             //data
                MdmWifi.ACCOUNT,                                //account
                initiator);
        Log.e(TAG.WFHS,"[ENGINE] -triggerSubscriptionProvisionSession()");
    }

    public void triggerSubscriptionRemediationSession(int sessionType, SessionInitiator initiator) throws MdmException {
        Log.e(TAG.WFHS,"[ENGINE] +triggerSubscriptionRemediationSession()");
        String CDATA_REMEDIATION = getFileString(Path.getPathInData(mContext, Path.DM_REM_PACKAGE1_FILE));
        if (mMdmWifi.getSessionId() == MdmWifi.SessionID.REMEDIATION_UPDATE_SIM_OMA) {
            CDATA_REMEDIATION = MdmWifi.sSimRemPackage1;
            MdmEngine.getInstance().triggerGenericAlertSession(
                    "Wi-Fi",                                   //correlator
                    MdmWifi.Format.XML,                             //format
                    MdmWifi.AlertType.SUBSCRIPTION_REMEDIATION_SIM,     //alert type
                    null,                                           //mark
                    MdmWifi.Source.REMEDIATION,                     //source
                    null,                                           //target
                    CDATA_REMEDIATION,                             //data
                    MdmWifi.ACCOUNT,                                //account
                    initiator);
        } else {
            MdmEngine.getInstance().triggerGenericAlertSession(
                    "Wi-Fi",                                   //correlator
                    MdmWifi.Format.XML,                             //format
                    MdmWifi.AlertType.SUBSCRIPTION_REMEDIATION,     //alert type
                    null,                                           //mark
                    MdmWifi.Source.REMEDIATION,                     //source
                    null,                                           //target
                    CDATA_REMEDIATION,                             //data
                    MdmWifi.ACCOUNT,                                //account
                    initiator);
        }

        Log.e(TAG.WFHS,"[ENGINE] -triggerSubscriptionRemediationSession()");
    }
    
    public void triggerSubscriptionPolicyUpdateSession(int sessionType, SessionInitiator initiator) throws MdmException {
        Log.e(TAG.WFHS,"[ENGINE] +triggerSubscriptionPolicyUpdateSession()");
        String CDATA_POLICY_UPDATE = getFileString(Path.getPathInData(mContext, Path.DM_POLICY_PACKAGE1_FILE));
        MdmEngine.getInstance().triggerGenericAlertSession(
                "Wi-Fi",                                   //correlator
                MdmWifi.Format.XML,                             //format
                MdmWifi.AlertType.POLICY_UPDATE_FOR_NDS,        //alert type
                null,                                           //mark
                MdmWifi.Source.POLICY_UPDATE,                   //source
                null,                                           //target
                CDATA_POLICY_UPDATE,                             //data
                MdmWifi.ACCOUNT,                                //account
                initiator);
        Log.e(TAG.WFHS,"[ENGINE] -triggerSubscriptionPolicyUpdateSession()");
    }

    public String getFileString(String fileName) {
        String str = "";
        File file = new File(fileName); 
        try {
            if(file.exists()) {
                FileInputStream in = new FileInputStream(file);
                int size = in.available();
                byte[] buffer = new byte[size];
                in.read(buffer);
                in.close();
                str = new String(buffer);
                Log.e(TAG.WFHS, "The file text is" + str);
            } else {
                Log.e(TAG.WFHS, "The file  is not exist," + fileName);
                return str;
            }
        } catch(Exception e) {
            Log.e(TAG.WFHS, e.getMessage());
        } 
        return str;
    }

    public DmOperation createDmOperation (Intent intent, String action) {
        
        DmOperation operation = new DmOperation();
        operation.setProperty(KEY.TYPE, KEY.WFHS_TAG);
        operation.setProperty(IntentExtraKey.OPERATION_KEY_ACTION, action);
        Bundle bundle = intent.getExtras();
        //bundle.keySet();
        for (String key : bundle.keySet()) {
            String value = null;
            if (IntentExtraKey.OPERATION_KEY_TYPE.equals(key)) {
                int type = intent.getIntExtra(IntentExtraKey.OPERATION_KEY_TYPE, 0);
                value = Integer.toOctalString(type);
            } else {
                value = intent.getStringExtra(key);
            }
            operation.setProperty(key, value);
            Log.i(TAG.WFHS, "WiFiDM Intent Key : value is: "+ key + ":" + value);
        }
        return operation;
    }
    
    public Intent Dmoperation2Intent (DmOperation operation) {
        Intent intent = new Intent();
        String intentAction = operation.getProperty(IntentExtraKey.OPERATION_KEY_ACTION);
        Log.i(TAG.WFHS, "dispatchOperationAction intent Action :" + intentAction);
        intent.setAction(intentAction);
        Set<Object> keys = operation.keySet();
        for(Object key : keys) {
            String keyString = (String)key;
            String value = null;
            value = operation.getProperty(keyString);
            Log.i(TAG.WFHS, "WiFiDM operation Key : value is: "+ key + ":" + value);
            if (IntentExtraKey.OPERATION_KEY_TYPE.equals(key)) {
                intent.putExtra(keyString, Integer.parseInt(value));
            } else {
                intent.putExtra(keyString, value);
            }
        }
        return intent;
    }
    
	@Override
	public PLHttpConnection getHttpConnection(DmOperation operation) {
		if (operation.containsKey(DmOperation.KEY.WFHS_TAG)) {
			return new WFHSHttpConnection(mMdmWifi);
		}
		return null;
	}
}
