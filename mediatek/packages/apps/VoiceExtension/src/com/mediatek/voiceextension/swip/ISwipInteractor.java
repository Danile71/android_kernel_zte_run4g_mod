package com.mediatek.voiceextension.swip;

public interface ISwipInteractor {

    public boolean isSwipReady();

    public int createSetName(String name, int featureType);

    public int deleteSetName(String name);

    public int isSetCreated(String name, int featureType);

    public String[] getAllSets(String processName, int featureType);

    public void startRecognition(String setName, int featureType);

    public void stopRecognition(String setName, int featureType);

    public void pauseRecognition(String setName, int featureType);

    public void resumeRecognition(String setName, int featureType);

    public void setCommands(String setName, String[] commands);

    public void setCommands(String setName, byte[] data, boolean end);

    public String[] getCommands(String setName);

    public void registerCallback(int featureType, ISwipCallback callback);

}
