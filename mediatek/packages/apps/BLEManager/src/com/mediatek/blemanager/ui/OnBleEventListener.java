package com.mediatek.blemanager.ui;

/**
 * Interface definition for a callback to be invoked when a BleStageView is clicked.
 */

public interface OnBleEventListener {
    void onClick(int locationIndex);
    void onFocus(int locationIndex);
}
