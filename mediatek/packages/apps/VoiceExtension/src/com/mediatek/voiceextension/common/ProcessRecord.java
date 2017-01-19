package com.mediatek.voiceextension.common;

import java.util.ArrayList;

import android.os.IBinder;
import android.util.ArrayMap;
import android.util.Log;

import com.mediatek.voiceextension.VoiceCommonState;

/**
 * Record the process info and the registered listeners from one application
 * 
 * @author mtk54255
 * 
 */
public class ProcessRecord implements IBinder.DeathRecipient {

    private int mPid;
    private int mUid;
    private String mProcessName;

    ListenerRecord mSwipOccupiedListener = null;
    private static final String sProcessMark = ":=";
    // private static final String sPidMark = "&";
    private static final String sFeatureMark = "_";

    // Key means feature name
    ArrayMap<String, ListenerRecord> mListeners = new ArrayMap<String, ListenerRecord>();

    public ProcessRecord(int pid, int uid, String processName) {
        mPid = pid;
        mUid = uid;
        mProcessName = processName;
    }

    public String getProcssName() {
        return mProcessName;
    }

    public int getPid() {
        return mPid;
    }

    public int getUid() {
        return mUid;
    }

    /**
     * 
     * @param name
     *            the feature name
     * @return
     */
    public ListenerRecord getListenerRecord(String name) {
        return mListeners.get(name);
    }

    /**
     * 
     * @param name
     * @param record
     */
    public void addListenerRecord(String name, ListenerRecord record) {
        mListeners.put(name, record);
    }

    public ListenerRecord createListenerRecord() {
        return new ListenerRecord();
    }

    /**
     * Switch internal listener in the same process
     * 
     * @param name
     * @return
     */
    public int switchSwipListenerRecord(ListenerRecord record) {
        int result = VoiceCommonState.SUCCESS;

        if (!mListeners.containsValue(record)) {
            result = VoiceCommonState.PROCESS_ILLEGAL;
        } else if (mSwipOccupiedListener != null
                && mSwipOccupiedListener != record) {
            result = VoiceCommonState.MIC_OCCUPIED;
        } else if (record.getSetName() == null) {
            result = VoiceCommonState.SET_NOT_SELECTED;
        } else {
            mSwipOccupiedListener = record;
        }
        return result;
    }

    public void releaseSwipListenerRecord() {
        if (mSwipOccupiedListener != null) {
            mSwipOccupiedListener.idle();
            mSwipOccupiedListener = null;

        }
    }

    public boolean isListenerOccupiedSwip(ListenerRecord record) {
        return mSwipOccupiedListener != null && mSwipOccupiedListener == record;
    }

    public ListenerRecord getSwipListenerRecord() {
        return mSwipOccupiedListener;
    }

    @Override
    public void binderDied() {
        // TODO Auto-generated method stub
        synchronized (CommonManager.getInstance()) {
            // Need to notify swip that process died if occupied swip
            if (mSwipOccupiedListener != null) {
                mSwipOccupiedListener.mCommonHandler.onProcessDiedLocked(this);
                mSwipOccupiedListener = null;
            }
            mListeners.clear();
            CommonManager.getInstance().onProcessDiedLocked(this);
        }
    }

    public String toString() {
        return "ProcessName = " + mProcessName + " Pid=" + mPid + " mUid="
                + mUid;
    }

    public static String getProcessNameFromSwipSet(String swipSetName) {

        if (swipSetName != null) {
            return swipSetName.split(sProcessMark)[0];
        }
        return null;
    }

    // public static int getProcessPidFromSwipSet(String swipSetName) {
    // if (swipSetName != null) {
    // return Integer.parseInt(swipSetName.split(sProcessMark)[0]
    // .split(sPidMark)[1]);
    // }
    // return -1;
    // }

    public static String getSetNameFromSwipSet(String swipSetName) {
        if (swipSetName != null) {
            return swipSetName.split(sFeatureMark)[1];
        }
        return null;
    }

    public static String getSetNameForSwip(String processName, int pid,
            String featureName, String setName) {
        if (processName != null && pid > 0 && featureName != null
                && setName != null)
            return processName + sProcessMark + featureName + sFeatureMark
                    + setName;
        return null;
    }

    public class RequestState {
        int mMainState;
        int mSubState;
        long mReqTime;
        Object mExtraObj1;
        Object mExtraObj2;

        RequestState(int mainState, int subState, Object obj1, Object obj2) {
            mMainState = mainState;
            mSubState = subState;
            mExtraObj1 = obj1;
            mExtraObj2 = obj2;
            mReqTime = System.currentTimeMillis();
        }

        public int getMainState() {
            return mMainState;
        }

        public int getSubState() {
            return mSubState;
        }

        public Object getExtraData1() {
            return mExtraObj1;
        }

        public Object getExtraData2() {
            return mExtraObj2;
        }
    }

    public class ListenerRecord {

        private Object mCurListener = null;
        private String mSetName;
        private RequestState mCurReqState;
        private RequestState mCurSwipState;
        private RequestState mInitState;
        private ArrayList<RequestState> mReqStateCaches = new ArrayList<RequestState>();
        private int mFeatureType;
        private String mFeatureName;
        private FeatureManager mCommonHandler;
        private boolean mWaitingSwipResponse;

        public void init(Object listener, int featureType, String featureName,
                int mainState, int subState, FeatureManager handler) {

            mCurListener = listener;
            mFeatureType = featureType;
            mFeatureName = featureName;
            mInitState = new RequestState(mainState, subState, null, null);
            mCommonHandler = handler;
            idle();
        }

        private void idle() {
            mWaitingSwipResponse = false;
            mCurReqState = mCurSwipState = mInitState;
        }

        public boolean isWaitingSwipResponse() {
            return mWaitingSwipResponse;
        }

        public void startWaitingSwipResponse() {
            if (CommonManager.DEBUG) {
                Log.d(CommonManager.TAG, "startWaitingSwipResponse");
            }
            mWaitingSwipResponse = true;
        }

        public void stopWaitingSwipResponse() {
            if (CommonManager.DEBUG) {
                Log.d(CommonManager.TAG, "stopWaitingSwipResponse");
            }
            mWaitingSwipResponse = false;
        }

        public Object getListener() {
            return mCurListener;
        }

        public int getFeatureType() {
            return mFeatureType;
        }

        public String getSetName() {
            return mSetName;
        }

        public void setSetName(String name) {
            mSetName = name;
        }

        public String getSwipSetName() {
            return getSetNameForSwip(mProcessName, mPid, mFeatureName, mSetName);
        }

        public void cacheRequestState(int mainState, int subState,
                Object extra1, Object extra2) {
            if (CommonManager.DEBUG) {
                Log.d(CommonManager.TAG, "cacheRequestState mainState:"
                        + mainState + ", subState" + subState + ", extra1:"
                        + extra1 + ", extra2:" + extra2);
            }
            mReqStateCaches.add(new RequestState(mainState, subState, extra1,
                    extra2));
        }

        public int switchToNextReqState() {
            if (CommonManager.DEBUG) {
                Log.d(CommonManager.TAG,
                        "switchToNextReqState mReqStateCaches.size():"
                                + mReqStateCaches.size());
            }
            RequestState state = mReqStateCaches.size() > 0 ? mReqStateCaches
                    .remove(0) : mInitState;

            int result = mCommonHandler.checkRequestMutexState(
                    mCurReqState.mMainState, mCurReqState.mSubState,
                    mCurSwipState.mMainState, mCurSwipState.mSubState,
                    state.mMainState, state.mSubState);

            mCurReqState = state;

            return result;
        }

        /**
         * Switch the swip state if receive the notification from native swip
         * 
         * @param notifyMainState
         * @param notifySubState
         * @param origalSubState
         * @param needCheck
         * @return
         */
        public void switchSwipState(int notifyMainState, int notifySubState) {
            mCurSwipState = new RequestState(notifyMainState, notifySubState,
                    null, null);
        }

        public RequestState getCurReqState() {
            return mCurReqState;
        }

        public RequestState getCurSwipState() {
            return mCurSwipState;
        }

        public boolean isReqStateIdle() {
            return mCurReqState == mInitState;
        }

        public boolean isSwipStateIdle() {
            return mCurSwipState == mInitState;
        }

        public ProcessRecord getProcessRecord() {
            return ProcessRecord.this;
        }
    }

}
