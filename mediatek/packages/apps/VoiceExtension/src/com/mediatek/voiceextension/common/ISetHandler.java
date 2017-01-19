package com.mediatek.voiceextension.common;

public interface ISetHandler {

    int createSet(int pid, int uid, String keyName);

    int deleteSet(int pid, int uid, String keyName);

    int selectSet(int pid, int uid, String setName);

    int isSetCreated(int pid, int uid, String setName);

    String getSetSelected(int pid, int uid);

    String[] getAllSets(int pid, int uid);

}
