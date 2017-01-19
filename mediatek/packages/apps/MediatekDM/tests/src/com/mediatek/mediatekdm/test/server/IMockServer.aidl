package com.mediatek.mediatekdm.test.server;

interface IMockServer {
    String getHost();
    int getPort();
    String getId();
    void start();
    void stop();
    String getParameter(String key);
    String setParameter(String key, String value);
    int getResult();
}
