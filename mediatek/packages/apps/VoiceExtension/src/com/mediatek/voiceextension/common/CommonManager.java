package com.mediatek.voiceextension.common;

import java.util.Iterator;
import java.util.List;
import java.util.Map.Entry;

import android.app.ActivityManagerNative;
import android.app.IActivityManager;
import android.app.ActivityManager.RunningAppProcessInfo;
import android.os.IInterface;
import android.os.RemoteException;
import android.util.ArrayMap;
import android.util.Log;

import com.mediatek.voiceextension.VoiceCommonState;
import com.mediatek.voiceextension.common.ProcessRecord.ListenerRecord;
import com.mediatek.voiceextension.swip.ISwipInteractor;
import com.mediatek.voiceextension.swip.SwipAdapter;
import com.mediatek.voiceextension.swip.SwipAdapterTest;

public class CommonManager {

    public static final String TAG = "VieManagerService";
    public static boolean DEBUG = true;

    private static CommonManager sMgrSelf;

    private final IActivityManager mActivityManager;

    private final ISwipInteractor mSwip;

    ArrayMap<Integer, ProcessRecord> mProcessRecords = new ArrayMap<Integer, ProcessRecord>();

    public static final int STATE_IDLE = 1;
    public static final int STATE_FEATURE_SET = (STATE_IDLE << 1) & 0xFF;
    public static final int STATE_FEATURE_INTERNAL = (STATE_IDLE << 2) & 0xFF;
    public static final int STATE_FEATURE_SWIP = (STATE_IDLE << 3) & 0xFF;

    public static final int SUBSTATE_IDLE = 1;

    /**
     * Used to record the current process who occupied the native swip module
     */
    private ProcessRecord mSwipOccupiedProcess = null;

    private CommonManager() {

        mActivityManager = ActivityManagerNative.getDefault();
        if (mActivityManager == null) {
            Log.e(TAG, "What's wrong with system server?");
        }
        mSwip = new SwipAdapter();
        // mSwip = new SwipAdapterTest();

    }

    public static CommonManager getInstance() {

        if (sMgrSelf == null) {
            synchronized (CommonManager.class) {
                if (null == sMgrSelf) {
                    sMgrSelf = new CommonManager();
                }
            }
        }
        return sMgrSelf;
    }

    /**
     * Get running process information from AMS and get the same pid , uid
     * process
     * 
     * @param pid
     * @param uid
     * @return
     */
    private ProcessRecord createProcessRecordLocked(int pid, int uid) {

        ProcessRecord record = mProcessRecords.get(pid);
        if (record == null) {
            try {
                List<RunningAppProcessInfo> procInfos = mActivityManager
                        .getRunningAppProcesses();
                if (procInfos != null && procInfos.size() > 0) {
                    for (RunningAppProcessInfo info : procInfos) {
                        if (info.pid == pid && info.uid == uid) {
                            record = new ProcessRecord(info.pid, info.uid,
                                    info.processName);
                            mProcessRecords.put(pid, record);
                            break;
                        }
                    }
                }
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                Log.e(TAG,
                        "IPC Exception while get process info from AMS "
                                + e.toString());
            }
        }

        return record;
    }

    /**
     * Get the process record with the same pid & uid
     * 
     * @param pid
     * @param uid
     * @return
     */
    public ProcessRecord getProcessRecordLocked(int pid, int uid) {
        ProcessRecord record = mProcessRecords.get(pid);

        if (record != null && record.getUid() != uid) {
            record = null;
            // mProcessRecords.remove(pid);
        }
        return record;
    }

    public ProcessRecord getProcessRecordLocked(String processName) {
        ProcessRecord record = null;

        if (mSwipOccupiedProcess != null
                && mSwipOccupiedProcess.getProcssName().equals(processName)) {
            record = mSwipOccupiedProcess;
        } else {
            Iterator<Entry<Integer, ProcessRecord>> iterator = mProcessRecords
                    .entrySet().iterator();
            while (iterator.hasNext()) {
                record = iterator.next().getValue();
                if (!record.getProcssName().equals(processName)) {
                    record = null;
                    continue;
                }
                break;
            }
        }
        return record;
    }

    public boolean isProcessExist(ProcessRecord record) {
        return mProcessRecords.containsValue(record);
    }

    public void removeProcessRecordLocked(int pid, int uid) {
        ProcessRecord record = mProcessRecords.get(pid);

        if (record != null && record.getUid() == uid) {
            mProcessRecords.remove(pid);
        }
    }

    public ISwipInteractor getSwipInteractionLocked() {
        return mSwip;
    }

    public int selectSwipOccupiedRecordLocked(int pid, int uid) {
        int result = VoiceCommonState.SUCCESS;

        return result;
    }

    public ProcessRecord getOccupiedSwipProcessLocked() {

        return mSwipOccupiedProcess;
    }

    public int switchSwipOccupiedProcessLocked(ProcessRecord processRecord,
            int featureType) {

        int result = VoiceCommonState.SUCCESS;

        ListenerRecord listenerRecord = processRecord == null ? null
                : processRecord.getListenerRecord(getFeatureName(featureType));

        if (processRecord == null || listenerRecord == null) {
            result = VoiceCommonState.PROCESS_ILLEGAL;
        } else {
            if (mSwipOccupiedProcess != null
                    && processRecord != mSwipOccupiedProcess) {
                result = VoiceCommonState.MIC_OCCUPIED;
            } else {
                // (1)Switch process record
                mSwipOccupiedProcess = processRecord;
                // Means switch success , current mSwipOccupiedProcess can
                // access swip
                // (2)Switch listener record
                result = processRecord.switchSwipListenerRecord(listenerRecord);
            }
        }
        return result;
    }

    public void releaseSwipOccupiedProcessLocked() {
        if (mSwipOccupiedProcess != null) {
            mSwipOccupiedProcess.releaseSwipListenerRecord();
            mSwipOccupiedProcess = null;
        }
    }

    /**
     * 
     * @param pid
     * @param uid
     * @param listener
     * @param featureType
     * @return
     */
    public int registerListenerLocked(int pid, int uid, Object listener,
            int featureType, int commonState, int featureState,
            FeatureManager handler) {

        int result = VoiceCommonState.SUCCESS;

        ProcessRecord processRecord = createProcessRecordLocked(pid, uid);
        String featureName = getFeatureName(featureType);
        if (processRecord == null) {
            result = VoiceCommonState.PROCESS_ILLEGAL;
        } else {

            ListenerRecord record = processRecord
                    .getListenerRecord(featureName);

            if (record == null) {
                record = processRecord.createListenerRecord();
                try {
                    ((IInterface) listener).asBinder().linkToDeath(
                            processRecord, 0);
                    processRecord.addListenerRecord(featureName, record);
                } catch (RemoteException ex) {
                    result = VoiceCommonState.PROCESS_ILLEGAL;
                }
            } else {
                // This case only can be happened while service didn't
                // receive
                // the died notification.
                // We need to notify native if possible
                if (CommonManager.DEBUG) {
                    Log.d(TAG,
                            "Register listener old pid="
                                    + processRecord.getPid() + " old uid="
                                    + processRecord.getUid()
                                    + " old processName="
                                    + processRecord.getProcssName());
                }
            }
            if (result == VoiceCommonState.SUCCESS) {
                record.init(listener, featureType, featureName, commonState,
                        featureState, handler);
            }

        }

        return result;
    }

    public int createSetLocked(int pid, int uid, String setName, int featureType) {
        // TODO Auto-generated method stub

        int result = VoiceCommonState.SUCCESS;

        ProcessRecord processRecord = getProcessRecordLocked(pid, uid);
        ListenerRecord listenerRecord = processRecord == null ? null
                : processRecord.getListenerRecord(getFeatureName(featureType));

        if (listenerRecord == null) {
            // Check whether is illegal process from third party application
            // Maybe third application connect service without using
            // VoiceCommandManager
            result = VoiceCommonState.PROCESS_ILLEGAL;
        } else {

            if (setName.equals(listenerRecord.getSetName())) {
                // The setName already in used ,so don't need to ask swip
                // creating the set
                result = VoiceCommonState.SET_ALREADY_EXIST;
            } else {
                // Ask Swip whether the setName is created
                String swipSet = ProcessRecord.getSetNameForSwip(
                        processRecord.getProcssName(), processRecord.getPid(),
                        getFeatureName(featureType), setName);
                result = mSwip.createSetName(swipSet, featureType);
            }
        }

        return result;
    }

    public int isSetCreatedLocked(int pid, int uid, String setName,
            int featureType) {

        int result = VoiceCommonState.SUCCESS;

        ProcessRecord processRecord = getProcessRecordLocked(pid, uid);
        ListenerRecord listenerRecord = processRecord == null ? null
                : processRecord.getListenerRecord(getFeatureName(featureType));

        if (processRecord == null || listenerRecord == null) {
            // Check whether is illegal process from third party application
            // Maybe third application connect service without using
            // VoiceCommandManager
            result = VoiceCommonState.PROCESS_ILLEGAL;
        } else {
            // Ask swip to check whether the set is already created
            if (!setName.equals(listenerRecord.getSetName())) {
                // First check whether the set is already selected
                String swipSet = ProcessRecord.getSetNameForSwip(
                        processRecord.getProcssName(), processRecord.getPid(),
                        getFeatureName(featureType), setName);
                result = mSwip.isSetCreated(swipSet, featureType);
            }
        }

        return result;
    }

    public int deleteSetLocked(int pid, int uid, String setName, int featureType) {

        int result = VoiceCommonState.SUCCESS;

        ProcessRecord processRecord = getProcessRecordLocked(pid, uid);
        ListenerRecord listenerRecord = processRecord == null ? null
                : processRecord.getListenerRecord(getFeatureName(featureType));

        if (processRecord == null || listenerRecord == null) {
            // Check whether is illegal process from third party application
            // Maybe third application connect service without using
            // VoiceCommandManager
            result = VoiceCommonState.PROCESS_ILLEGAL;
        } else {

            if (setName.equals(listenerRecord.getSetName())) {
                if (listenerRecord == processRecord.getSwipListenerRecord()) {
                    // The setName already in used ,so we need to check state
                    // Swip is using the set , so we can't delete the set
                    result = VoiceCommonState.SET_OCCUPIED;
                } else {
                    listenerRecord.setSetName(null);
                }
            }
            // Ask swip to delete the set
            // listenerRecord.selectSet(null);
            if (result == VoiceCommonState.SUCCESS) {
                String swipSet = ProcessRecord.getSetNameForSwip(
                        processRecord.getProcssName(), processRecord.getPid(),
                        getFeatureName(featureType), setName);
                result = mSwip.deleteSetName(swipSet);
            }
        }
        return result;
    }

    public String getSetSelected(int pid, int uid, int featureType) {
        String setName = null;

        ProcessRecord processRecord = getProcessRecordLocked(pid, uid);
        ListenerRecord listenerRecord = processRecord == null ? null
                : processRecord.getListenerRecord(getFeatureName(featureType));

        if (listenerRecord != null) {
            // Check whether is illegal process from third party application
            // Maybe third application connect service without using
            // VoiceCommandManager
            setName = listenerRecord.getSetName();
        }

        return setName;
    }

    public int selectSetLocked(int pid, int uid, String setName, int featureType) {

        int result = VoiceCommonState.SUCCESS;

        if (setName == null) {

            result = VoiceCommonState.SET_ILLEGAL;
            Log.e(TAG, "select Set fail, set name =" + setName);

        } else {

            ProcessRecord processRecord = getProcessRecordLocked(pid, uid);
            ListenerRecord listenerRecord = processRecord == null ? null
                    : processRecord
                            .getListenerRecord(getFeatureName(featureType));
            if (processRecord == null || listenerRecord == null) {
                // Check whether is illegal process from third party application
                // Maybe third application connect service without using
                // VoiceCommandManager
                result = VoiceCommonState.PROCESS_ILLEGAL;
            } else if (setName.equals(listenerRecord.getSetName())) {
                result = VoiceCommonState.SET_SELECTED;
            } else if (processRecord.isListenerOccupiedSwip(listenerRecord)) {
                // Current listenerRecord occupy the swip , can't switch set
                result = VoiceCommonState.MIC_OCCUPIED;
            } else {
                // Ask swip that is this set created
                String swipSet = ProcessRecord.getSetNameForSwip(
                        processRecord.getProcssName(), processRecord.getPid(),
                        getFeatureName(featureType), setName);
                result = mSwip.isSetCreated(swipSet, featureType);
                if (result == VoiceCommonState.SET_ALREADY_EXIST) {
                    listenerRecord.setSetName(setName);
                    // result = processRecord
                    // .switchSwipListenerRecord(listenerRecord);
                    result = VoiceCommonState.SUCCESS;
                }
            }
        }
        return result;
    }

    public String[] getAllSetsLocked(int pid, int uid, int featureType) {

        String[] sets = null;
        ProcessRecord processRecord = getProcessRecordLocked(pid, uid);
        ListenerRecord listenerRecord = processRecord == null ? null
                : processRecord.getListenerRecord(getFeatureName(featureType));
        if (listenerRecord != null) {
            // Ask swip to get the sets of this process
            sets = mSwip.getAllSets(processRecord.getProcssName(), featureType);
        }
        return sets;
    }

    public void onProcessDiedLocked(ProcessRecord record) {

        if (mSwipOccupiedProcess == record) {
            mSwipOccupiedProcess = null;
        }
        removeProcessRecordLocked(record.getPid(), record.getUid());

    }

    private String getFeatureName(int featureType) {
        String featureName = null;
        switch (featureType) {
        case VoiceCommonState.VIE_FEATURE_COMMAND:
            featureName = VoiceCommonState.VIE_FEATURE_COMMAND_NAME;
            break;
        case VoiceCommonState.VIE_FEATURE_SEARCH:
            featureName = VoiceCommonState.VIE_FEATURE_SEARCH_NAME;
            break;
        case VoiceCommonState.VIE_FEATURE_PASSPHRASE:
            featureName = VoiceCommonState.VIE_FEATURE_PASSPHRASE_NAME;
            break;
        }
        return featureName;
    }

}