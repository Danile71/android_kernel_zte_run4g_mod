package com.mediatek.ppl;

import java.util.HashMap;

public class PplSimInfo {
        
    private final static String KEY_SIMID = "SimId";
    private final static String KEY_TITLE = "Title";
    private final static String KEY_IMAGE = "Image";
    private final static String KEY_COLOR = "Color";
    
    private HashMap<String, Object> mInfo;
    
    public PplSimInfo(String title, int image, int color) {
        mInfo = new HashMap<String, Object>();
        mInfo.put(KEY_TITLE, title);
        mInfo.put(KEY_IMAGE, image);
        mInfo.put(KEY_COLOR, color);
    }
    
    public void setSimId(int id) {
        mInfo.put(KEY_SIMID, id);
    }
    
    public int getSimId() {
        return (Integer)mInfo.get(KEY_SIMID);
    }
    
    public String getTitle() {
        return (String)mInfo.get(KEY_TITLE);
    }
    
    public int getImage() {
        return (Integer)mInfo.get(KEY_IMAGE);
    }
    
    public int getColor() {
        return (Integer)mInfo.get(KEY_COLOR);
    }
    
}
