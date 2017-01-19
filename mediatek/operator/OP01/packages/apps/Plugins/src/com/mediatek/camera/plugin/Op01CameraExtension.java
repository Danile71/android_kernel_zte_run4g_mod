package com.mediatek.camera.plugin;

import android.content.Context;
import android.content.ContextWrapper;
import android.media.CamcorderProfile;
import android.media.MediaRecorder;

import com.mediatek.media.MediaRecorderEx;

import com.mediatek.op01.plugin.R;
import com.mediatek.xlog.Xlog;

import java.util.ArrayList;

import com.mediatek.camera.ext.DefaultCameraFeatureExt;

public class Op01CameraExtension extends DefaultCameraFeatureExt {
    private static final String TAG = "Op01CameraExtension";
    private static final boolean LOG = true;
    
    private Context mContext;
    private static final int MPEG4_CODEC = 3;
    private static final int H264_CODEC = 2;
    private static final int HEVC_CODEC = 4;
    private static final int IMAGE_DISPLAY_DURATION = 1200;

    public static final String KEY_WHITE_BALANCE = "pref_camera_whitebalance_key";
    public static final String KEY_SCENE_MODE = "pref_camera_scenemode_key";
    
    public Op01CameraExtension(Context context) {
        super(context);
        mContext = context;
    } 
    
    @Override
    public int getQuickViewDisplayDuration() {
        Xlog.i(TAG, "getQuickViewDisplayDuration(): " + IMAGE_DISPLAY_DURATION);
        return IMAGE_DISPLAY_DURATION;
    }
    
    @Override
    public void updateSettingItem(String key, ArrayList<CharSequence> entries, ArrayList<CharSequence> entryValues) {
        Xlog.i(TAG, "updateSettingItem(): key + " + key);
        if (key.equals(KEY_WHITE_BALANCE)) {
            CharSequence[] newEntries = mContext.getResources().getTextArray(
                        R.array.pref_camera_whitebalance_entries_cmcc);

            CharSequence[] newEntryValues = mContext.getResources().getTextArray(
                        R.array.pref_camera_whitebalance_entryvalues_cmcc);
            
            int size = entries.size();
            for (int i = 0; i < size; i++) {
                entries.set(i, newEntries[i]);
                entryValues.set(i, newEntryValues[i]);
            }
        } 
    }

    @Override
    public void checkCamcorderProfile(int quality, CamcorderProfile profile) {
        Xlog.i(TAG, "checkCamcorderProfile() profile.videoCodec = " + profile.videoCodec);
        if (profile.videoCodec != MPEG4_CODEC && profile.videoCodec != H264_CODEC && profile.videoCodec != HEVC_CODEC) {
            profile.videoCodec = MPEG4_CODEC;
        }
    }
    
    @Override
    public void configRecorder(MediaRecorder recorder) {
        Xlog.i(TAG, "configRecorder()");
        MediaRecorderEx.setVideoBitOffSet(recorder,1,true);
    }
}
