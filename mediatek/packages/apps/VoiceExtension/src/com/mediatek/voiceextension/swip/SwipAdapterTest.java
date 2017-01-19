package com.mediatek.voiceextension.swip;

import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.List;

import android.util.ArrayMap;
import android.util.Log;

import com.mediatek.voiceextension.VoiceCommandResult;
import com.mediatek.voiceextension.VoiceCommonState;

public class SwipAdapterTest implements ISwipInteractor {
    ArrayMap<Integer, ISwipCallback> mCallbacks = new ArrayMap<Integer, ISwipCallback>();
    private static final String TAG = "SwipAdapterTest";
    private List<String> mCommandSetsList = new ArrayList<String>();
    private List<String> mCommandsList = new ArrayList<String>();

    @Override
    public boolean isSwipReady() {
        // TODO Auto-generated method stub
        return true;
    }

    @Override
    public int createSetName(String name, int featureType) {
        // TODO Auto-generated method stub
        mCommandSetsList.clear();
        mCommandSetsList.add(name);
        Log.d(TAG, "createSetName name:" + name + ", featureType:"
                + featureType);
        return VoiceCommandResult.SUCCESS;
    }

    @Override
    public int deleteSetName(String name) {
        // TODO Auto-generated method stub
        mCommandSetsList.remove(name);
        Log.d(TAG, "deleteSetName name:" + name);
        return VoiceCommandResult.SUCCESS;
    }

    @Override
    public int isSetCreated(String name, int featureType) {
        // TODO Auto-generated method stub
        Log.d(TAG, "isSetCreated name:" + name + ", featureType:" + featureType);
        return VoiceCommandResult.SUCCESS;
    }

    @Override
    public String[] getAllSets(String processName, int featureType) {
        // TODO Auto-generated method stub
        Log.d(TAG,
                "getAllSets processName:" + processName + ", featureType"
                        + featureType + ", mCommandSetsList:"
                        + mCommandSetsList.toString());
        return mCommandSetsList.toArray(new String[mCommandSetsList.size()]);
    }

    @Override
    public void startRecognition(String setName, int featureType) {
        // TODO Auto-generated method stub
        postEventFromNative(setName, featureType,
                VoiceCommonState.API_COMMAND_START_RECOGNITION,
                VoiceCommandResult.SUCCESS, 0, null);

        int commandId = 0;
        String commandString = mCommandsList.get(commandId);
        Log.d(TAG, "startRecognition setName:" + setName + ", featureType:"
                + featureType + "commandId:" + commandId + ", commandString:"
                + commandString);
        postEventFromNative(setName, featureType,
                VoiceCommonState.API_COMMAND_RECOGNIZE_RESULT,
                VoiceCommandResult.SUCCESS, commandId, commandString);
    }

    @Override
    public void stopRecognition(String setName, int featureType) {
        // TODO Auto-generated method stub
        postEventFromNative(setName, featureType,
                VoiceCommonState.API_COMMAND_STOP_RECOGNITION,
                VoiceCommandResult.SUCCESS, 0, null);
    }

    @Override
    public void pauseRecognition(String setName, int featureType) {
        // TODO Auto-generated method stub
        postEventFromNative(setName, featureType,
                VoiceCommonState.API_COMMAND_PAUSE_RECOGNITION,
                VoiceCommandResult.SUCCESS, 0, null);
    }

    @Override
    public void resumeRecognition(String setName, int featureType) {
        // TODO Auto-generated method stub
        postEventFromNative(setName, featureType,
                VoiceCommonState.API_COMMAND_RESUME_RECOGNITION,
                VoiceCommandResult.SUCCESS, 0, null);

        int commandId = 0;
        String commandString = mCommandsList.get(commandId);

        Log.d(TAG, "resumeRecognition setName:" + setName + ", featureType:"
                + featureType + "commandId:" + commandId + ", commandString:"
                + commandString);
        postEventFromNative(setName, featureType,
                VoiceCommonState.API_COMMAND_RECOGNIZE_RESULT,
                VoiceCommandResult.SUCCESS, commandId, commandString);
    }

    @Override
    public void setCommands(String setName, String[] commands) {
        // TODO Auto-generated method stub
        mCommandsList.clear();
        for (int i = 0; i < commands.length; i++) {
            mCommandsList.add(commands[i]);
        }
        Log.d(TAG, "setCommands setName:" + setName + ", commands:"
                + mCommandsList.toString());
        postEventFromNative(setName, VoiceCommonState.VIE_FEATURE_COMMAND,
                VoiceCommonState.API_COMMAND_SET_COMMANDS,
                VoiceCommandResult.SUCCESS, 0, null);
    }

    @Override
    public void setCommands(String setName, byte[] data, boolean end) {
        // TODO Auto-generated method stub
        mCommandsList.clear();
        try {
            String commands = new String(data ,"gbk");
            mCommandsList.add(commands);
        } catch (UnsupportedEncodingException e) {
            Log.d(TAG,
                    "setCommands data exception :" + e.getMessage());
        }

        Log.d(TAG,
                "setCommands setName:" + setName + ", data:"
                        + mCommandsList.toString());
        postEventFromNative(setName, VoiceCommonState.VIE_FEATURE_COMMAND,
                VoiceCommonState.API_COMMAND_SET_COMMANDS,
                VoiceCommandResult.SUCCESS, 0, null);
    }

    @Override
    public String[] getCommands(String setName) {
        // TODO Auto-generated method stub
        Log.d(TAG, "getCommands setName:" + setName + ", mCommandsList:"
                + mCommandsList.toString());
        return mCommandsList.toArray(new String[mCommandsList.size()]);
    }

    @Override
    public void registerCallback(int featureType, ISwipCallback callback) {
        // TODO Auto-generated method stub
        // mCallbacks.append(featureType, callback);
        mCallbacks.put(featureType, callback);
    }

    private void postEventFromNative(String setName, int featureType,
            int apiType, int msg1, int msg2, Object extraMsg) {
        (mCallbacks.get(featureType)).onSwipMessageNotify(setName, apiType,
                msg1, msg2, extraMsg);

    }

}
