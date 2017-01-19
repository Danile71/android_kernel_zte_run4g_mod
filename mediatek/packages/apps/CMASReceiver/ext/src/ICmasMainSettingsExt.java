
package com.mediatek.cmas.ext;

import android.preference.PreferenceActivity;

public interface ICmasMainSettingsExt {

    public float getAlertVolume(int msgId);
    public boolean getAlertVibration(int msgId);
    public boolean setAlertVolumeVibrate(int msgId, boolean currentValue);
    public void addAlertSoundVolumeAndVibration(PreferenceActivity prefActivity);
}
