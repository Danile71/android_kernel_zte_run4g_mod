package com.mediatek.launcher2.plugin;

import android.util.Log;

import com.mediatek.launcher2.ext.DefaultOperatorChecker;

public class Op09OperatorChecker extends DefaultOperatorChecker {
    private static final String TAG = "Op09OperatorChecker";

    public Op09OperatorChecker() {}

    @Override
    public boolean supportEditAndHideApps() {
        Log.d(TAG, "Op09OperatorChecker supportEditAndHideApps called.");
        return true;
    }

    @Override
    public boolean supportAppListCycleSliding() {
        Log.d(TAG, "Op09OperatorChecker supportAppListCycleSliding called.");
        return true;
    }
}
