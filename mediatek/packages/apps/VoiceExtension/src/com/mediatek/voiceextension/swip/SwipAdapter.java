package com.mediatek.voiceextension.swip;

import java.lang.ref.WeakReference;

import android.util.ArrayMap;

import com.mediatek.voiceextension.VoiceCommonState;
import com.mediatek.voiceextension.cfg.ConfigurationManager;

public class SwipAdapter implements ISwipInteractor {

    // Native will set this memory address to check whether the call back
    // exist
    private int mNativeContext = 0;
    private static int mInitResult = 0;
    /**
     * @Integer indicate feature type
     * @ISwipCallback indicate managers which need to be notified
     */
    ArrayMap<Integer, ISwipCallback> mCallbacks = new ArrayMap<Integer, ISwipCallback>();

    public SwipAdapter() {
        native_setup(new WeakReference<SwipAdapter>(this));
        mInitResult = setViePath(ConfigurationManager.getInstance().getModelPath(),
                                           ConfigurationManager.getInstance().getDatabasePath());
    }

    static {
        System.loadLibrary("vie_jni");
        try {
             native_init();
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    @Override
    public void registerCallback(int featureType, ISwipCallback callback) {

        mCallbacks.put(featureType, callback);

    }

    /**
     * | Native init when load library
     * 
     * @param modelPath
     * @param databasePath
     */
    private static final native void native_init();

    /**
     * Need to setup the pointer of swipadapter for native call back
     * 
     * @param jniSwipAdapter_this
     * @throws RuntimeException
     */
    private final native void native_setup(Object jniSwipAdapter_this)
            throws RuntimeException;

    /**
     * When error happen , native need to release some occupied object if
     * possible
     */
    private final native void release();

    private native int setViePath(String modelPath, String databasePath);

    private native int createVieSetName(String keyName, int featureType);

    private native int deleteVieSetName(String keyName);

    private native int isVieSetCreated(String keyName, int featureType);

    private native String[] getVieSets(String processName, int featureType);

    private native void startVieRecognition(String keyName, int featureType);

    private native void stopVieRecognition(String keyName, int featureType);

    private native void pauseVieRecognition(String keyName, int featureType);

    private native void resumeVieRecognition(String keyName, int featureType);

    private native void setupVieCommandsByString(String keyName,
            String[] commands);

    private native void setupVieCommandsByFile(String keyName, byte[] fileData,
            boolean end);

    private native String[] getVieCommands(String keyName);

    /**
     * Called from native code when an interesting event happens. This method
     * just uses the EventHandler system to post the event back to the main app
     * thread. We use a weak reference to the original SwipAdapter object so
     * that the native code is safe from the object disappearing from underneath
     * it. (This is the cookie passed to native_setup().)
     * 
     * @param jniSwipAdapter_ref
     * @param keyName
     * @param feature
     * @param apiType
     * @param msg1
     * @param msg2
     * @param extraMsg
     */
    private static void postEventFromNative(Object jniSwipAdapter_ref,
            String setName, int featureType, int apiType, int msg1, int msg2,
            Object extraMsg) {

        SwipAdapter adapter = (SwipAdapter) ((WeakReference<?>) jniSwipAdapter_ref)
                .get();

        String[] recogStrings = null;
        if (extraMsg != null) {
            String[] tempArray = (String[]) extraMsg;
            if (tempArray != null) {
                int length = tempArray.length;
                recogStrings = new String[length];
                System.arraycopy(tempArray, 0, recogStrings, 0, length);
            }
        }

        (adapter.mCallbacks.get(featureType)).onSwipMessageNotify(setName,
                apiType, msg1, msg2, recogStrings);

    }

    @Override
    public boolean isSwipReady() {
        return (mInitResult == VoiceCommonState.SUCCESS)
                && (mNativeContext > 0);
    }

    @Override
    public int createSetName(String name, int featureType) {
        // TODO Auto-generated method stub
        return createVieSetName(name, featureType);
    }

    @Override
    public int deleteSetName(String name) {
        // TODO Auto-generated method stub
        return deleteVieSetName(name);
    }

    @Override
    public String[] getAllSets(String processName, int featureType) {
        // TODO Auto-generated method stub
        return getVieSets(processName, featureType);
    }

    @Override
    public String[] getCommands(String setName) {
        // TODO Auto-generated method stub
        return getVieCommands(setName);
    }

    @Override
    public int isSetCreated(String name, int featureType) {
        // TODO Auto-generated method stub
        return isVieSetCreated(name, featureType);
    }

    @Override
    public void startRecognition(String setName, int featureType) {
        // TODO Auto-generated method stub
        startVieRecognition(setName, featureType);
    }

    @Override
    public void stopRecognition(String setName, int featureType) {
        // TODO Auto-generated method stub
        stopVieRecognition(setName, featureType);
    }

    @Override
    public void pauseRecognition(String setName, int featureType) {
        // TODO Auto-generated method stub
        pauseVieRecognition(setName, featureType);
    }

    @Override
    public void resumeRecognition(String setName, int featureType) {
        // TODO Auto-generated method stub
        resumeVieRecognition(setName, featureType);
    }

    @Override
    public void setCommands(String setName, String[] commands) {
        // TODO Auto-generated method stub
        setupVieCommandsByString(setName, commands);
    }

    @Override
    public void setCommands(String setName, byte[] data, boolean end) {
        // TODO Auto-generated method stub
        setupVieCommandsByFile(setName, data, end);
    }

}