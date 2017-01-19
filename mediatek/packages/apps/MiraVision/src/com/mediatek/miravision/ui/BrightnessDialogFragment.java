/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.miravision.ui;

import android.app.Dialog;
import android.app.DialogFragment;
import android.content.Context;
import android.content.res.Resources;
import android.database.ContentObserver;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.IPowerManager;
import android.os.PowerManager;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.UserHandle;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.SeekBar;

import com.mediatek.miravision.setting.MiraVisionJni;
import com.mediatek.miravision.setting.MiraVisionJni.Range;
import com.mediatek.miravision.utils.CurrentUserTracker;

/** A dialog that provides controls for adjusting the screen brightness. */
public class BrightnessDialogFragment extends DialogFragment implements
        SeekBar.OnSeekBarChangeListener {

    private static final String TAG = "MiraVision/BrightnessDialogFragment";

    private final int mBrightnessDialogLongTimeout;
    private final int mBrightnessDialogShortTimeout;

    private final Handler mHandler = new Handler();

    private SeekBar mSeekBar;
    private int mMinimumBacklight;
    private int mMaximumBacklight;

    private IPowerManager mPower;
    private CurrentUserTracker mUserTracker;
    private Context mContext;
    private int mDialogId;
    private Dialog mDialog;
    private boolean mTracking;

    private final ContentObserver mBrightnessObserver = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange) {
            onChange(selfChange, null);
        }

        @Override
        public void onChange(boolean selfChange, Uri uri) {
            Log.d(TAG, "BrightnessObserver, selfChange = " + selfChange + ", uri = " + uri);
            if (selfChange) {
                return;
            }
            updateSeekbar();
            onBrightnessLevelChanged();
        }
    };

    private final Runnable mDismissDialogRunnable = new Runnable() {
        public void run() {
            if (getDialog() != null && getDialog().isShowing()) {
                dismiss();
            }
        };
    };

    public BrightnessDialogFragment(Context ctx, int dialogId) {
        super();
        Resources r = ctx.getResources();
        mDialogId = dialogId;
        Log.d(TAG, "mDialogId = " + mDialogId);
        mBrightnessDialogLongTimeout = r
                .getInteger(R.integer.quick_settings_brightness_dialog_long_timeout);
        mBrightnessDialogShortTimeout = r
                .getInteger(R.integer.quick_settings_brightness_dialog_short_timeout);
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        Log.d(TAG, "onCreateDialog()");
        mDialog = new Dialog(getActivity());
        mDialog.setContentView(createDialogView());
        return mDialog;
    }

    private View createDialogView() {

        Window window = mDialog.getWindow();
        window.setType(WindowManager.LayoutParams.TYPE_VOLUME_OVERLAY);
        window.getAttributes().privateFlags |= WindowManager.LayoutParams.PRIVATE_FLAG_SHOW_FOR_ALL_USERS;
        window.clearFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
        window.requestFeature(Window.FEATURE_NO_TITLE);
        final LayoutInflater layoutInflater = (LayoutInflater) getActivity().getSystemService(
                Context.LAYOUT_INFLATER_SERVICE);
        View view = layoutInflater.inflate(R.layout.brightness_dialog, null);

        mContext = getActivity();
        mDialog.setCanceledOnTouchOutside(true);
        mSeekBar = (SeekBar) view.findViewById(R.id.control_seekbar);
        mSeekBar.setOnSeekBarChangeListener(this);
        return view;
    }

    @Override
    public void onResume() {
        super.onResume();
        Log.d(TAG, "onResume()");
        initBrightnessController();
        dismissBrightnessDialog(mBrightnessDialogLongTimeout);
    }

    @Override
    public void onPause() {
        Log.d(TAG, "onPause()");
        unregisterCallbacks();
        removeAllBrightnessDialogCallbacks();
        super.onPause();
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy()");
        mDialog = null;
        super.onDestroy();
    }

    private void initBrightnessController() {
        mUserTracker = new CurrentUserTracker(mContext) {
            @Override
            public void onUserSwitched(int newUserId) {
                updateSeekbar();
            }
        };
        if (mDialogId == AalSettingsFragment.DIALOG_ID_BRIGHTNESS) {
            mContext.getContentResolver().registerContentObserver(
                    Settings.System.getUriFor(Settings.System.SCREEN_BRIGHTNESS), false,
                    mBrightnessObserver, UserHandle.USER_ALL);
            PowerManager pm = (PowerManager) mContext.getSystemService(Context.POWER_SERVICE);
            mMinimumBacklight = pm.getMinimumScreenBrightnessSetting();
            mMaximumBacklight = pm.getMaximumScreenBrightnessSetting();
            mPower = IPowerManager.Stub.asInterface(ServiceManager.getService("power"));
        } else if (mDialogId == AalSettingsFragment.DIALOG_ID_SENSITIVITY) {
            mContext.getContentResolver().registerContentObserver(
                    Settings.System.getUriFor(Settings.System.SCREEN_BRIGHTNESS_OF_AUTO_MODE),
                    false, mBrightnessObserver, UserHandle.USER_ALL);
            Range sensityRange = MiraVisionJni.getUserBrightnessRange();
            mMinimumBacklight = sensityRange.min;
            mMaximumBacklight = sensityRange.max;
        }

        // Update the slider and mode before attaching the listener so we don't
        // receive the
        // onChanged notifications for the initial values.
        updateSeekbar();
    }

    /** Unregister all call backs, both to and from the controller */
    private void unregisterCallbacks() {
        mContext.getContentResolver().unregisterContentObserver(mBrightnessObserver);
        mUserTracker.stopTracking();
    }

    private void removeAllBrightnessDialogCallbacks() {
        mHandler.removeCallbacks(mDismissDialogRunnable);
    }

    /** on seekbar change */
    private void onChanged(int value) {
        final int val = value + mMinimumBacklight;
        Log.d(TAG, "onChanged, value = " + val + ", mDialogId = " + mDialogId);
        setBrightness(val);

        final String item = mDialogId == 0 ? Settings.System.SCREEN_BRIGHTNESS
                : Settings.System.SCREEN_BRIGHTNESS_OF_AUTO_MODE;
        Log.d(TAG, item + " onChanged");
        if (!mTracking) {
            AsyncTask.execute(new Runnable() {
                public void run() {
                    Settings.System.putIntForUser(mContext.getContentResolver(), item, val,
                            UserHandle.USER_CURRENT);
                }
            });
        }
        onBrightnessLevelChanged();
    }

    /** Fetch the brightness from the system settings and update the seekbar */
    private void updateSeekbar() {
        int value = 0;
        try {
            if (mDialogId == AalSettingsFragment.DIALOG_ID_BRIGHTNESS) {
                value = Settings.System.getIntForUser(mContext.getContentResolver(),
                        Settings.System.SCREEN_BRIGHTNESS, UserHandle.USER_CURRENT);
            } else if (mDialogId == AalSettingsFragment.DIALOG_ID_SENSITIVITY) {
                value = Settings.System.getIntForUser(mContext.getContentResolver(),
                        Settings.System.SCREEN_BRIGHTNESS_OF_AUTO_MODE, MiraVisionJni
                                .getUserBrightnessRange().defaultValue, UserHandle.USER_CURRENT);
            }
        } catch (SettingNotFoundException ex) {
            value = mMaximumBacklight;
        }
        Log.d(TAG, "updateSeekbar, value = " + value + ", mDialogId = " + mDialogId);
        mSeekBar.setMax(mMaximumBacklight - mMinimumBacklight);
        mSeekBar.setProgress(value - mMinimumBacklight);
    }

    private void setBrightness(int brightness) {
        if (mDialogId == AalSettingsFragment.DIALOG_ID_BRIGHTNESS) {
            try {
                mPower.setTemporaryScreenBrightnessSettingOverride(brightness);
            } catch (RemoteException ex) {
            }
        } else if (mDialogId == AalSettingsFragment.DIALOG_ID_SENSITIVITY) {
            MiraVisionJni.setUserBrightness(brightness);
        }
    }

    private void onBrightnessLevelChanged() {
        dismissBrightnessDialog(mBrightnessDialogShortTimeout);
    }

    private void dismissBrightnessDialog(int timeout) {
        removeAllBrightnessDialogCallbacks();
        mHandler.postDelayed(mDismissDialogRunnable, timeout);
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        if (fromUser) {
            onChanged(progress);
        }
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
        mTracking = true;
        onChanged(seekBar.getProgress());
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        mTracking = false;
        onChanged(seekBar.getProgress());
    }

}
