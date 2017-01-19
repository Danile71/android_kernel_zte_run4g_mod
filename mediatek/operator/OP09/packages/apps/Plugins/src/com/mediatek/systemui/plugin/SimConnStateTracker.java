package com.mediatek.systemui.plugin;

import android.content.Context;
import android.content.Intent;
import android.graphics.drawable.AnimationDrawable;
import android.net.ConnectivityManager;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.provider.Settings;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.widget.FrameLayout;
import android.widget.ImageView;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.PhoneConstants;
import com.android.systemui.statusbar.phone.QuickSettingsTileView;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.telephony.ITelephonyEx;
import com.mediatek.op09.plugin.R;
import com.mediatek.systemui.ext.IconIdWrapper;
import com.mediatek.systemui.statusbar.util.SIMHelper;
import com.mediatek.systemui.statusbar.util.StateTracker;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.xlog.Xlog;

/**
 * M: Subclass of StateTracker for Sim Connect state.
 */
public final class SimConnStateTracker extends StateTracker {
    private static final String TAG = "SimConnStateTracker";
    private static final int EVENT_DETACH_TIME_OUT = 2000;
    private static final int EVENT_ATTACH_TIME_OUT = 2001;
    private static final int DETACH_TIME_OUT_LENGTH = 10000;
    private static final int ATTACH_TIME_OUT_LENGTH = 30000;

    private boolean mIsAirlineMode = false;
    private boolean mHasSim = false;
    public boolean mIsMmsOngoing = false;
    private int mCurrentState = SIMConnState.NO_SIM;
    private Context mContext;
    private ImageView mSimConnSwitchIngGifView;
    private FrameLayout mSimConnLayout;
    private QuickSettingsTileView mSimConnTileView;
    private ImageView mSimConnIcon;

    private interface SIMConnState {
        /// M: Add for op09, SIM1_E_D means SIM1 open, the other sim is enable, data disable. @ {
        int SIM1_E_D = 0;
        int SIM1_E_E = 1;
        int SIM1_D_D = 2;
        int SIM1_D_E = 3;
        int SIM2_E_D = 4;
        int SIM2_E_E = 5;
        int SIM2_D_D = 6;
        int SIM2_D_E = 7;
        int NO_SIM = 8;
        /// @}
        /// M: Add for op09, SIM1_E_F means SIM1 open, the other sim is enable, sim1 radio off. @ {
        int SIM1_E_F = 9;
        int SIM1_D_F = 10;
        int SIM2_E_F = 11;
        int SIM2_D_F = 12;
        /// @}
    };

    /**
     * M: Used to check weather this device is wifi only.
     */
    private boolean isWifiOnlyDevice() {
      ConnectivityManager cm = (ConnectivityManager) mContext.getSystemService(mContext.CONNECTIVITY_SERVICE);
      return  !(cm.isNetworkSupported(ConnectivityManager.TYPE_MOBILE));
    }

    /// M: Add for op09 SIM Conn.
    private Handler mDataTimerHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            long simId;
            switch (msg.what) {
            case EVENT_DETACH_TIME_OUT:
                simId = Settings.System.getLong(mContext.getContentResolver(),
                        Settings.System.GPRS_CONNECTION_SIM_SETTING,
                        Settings.System.DEFAULT_SIM_NOT_SET);
                Xlog.d(TAG, "detach time out......simId is " + simId);
                /// M: only apply if NOT wifi-only device @{
                if (!isWifiOnlyDevice()) {
                    refresh();
                }
                /// M: }@
                break;
            case EVENT_ATTACH_TIME_OUT:
                simId = Settings.System.getLong(mContext.getContentResolver(),
                        Settings.System.GPRS_CONNECTION_SIM_SETTING,
                        Settings.System.DEFAULT_SIM_NOT_SET);
                Xlog.d(TAG, "attach time out......simId is " + simId);
                /// M: only apply if NOT wifi-only device @{
                if (!isWifiOnlyDevice()) {
                    refresh();
                }
                /// M: }@
                break;
            default:
                break;
            }
        }
    };

    /// M: Constant for current sim mode
    private static final int ALL_RADIO_OFF = 0;

    public SimConnStateTracker(Context context, QuickSettingsTileView tile) {
        mContext = context;
        mSimConnTileView = tile;
        mSimConnLayout = (FrameLayout) mSimConnTileView.findViewById(R.id.layout);
        mSimConnIcon = (ImageView) mSimConnTileView.findViewById(R.id.image);
        (mSimConnTileView.findViewById(R.id.text)).setVisibility(View.GONE);
    }

    private boolean isAllRadioOff() {
        boolean isAllRadioOff = mIsAirlineMode
                || (Settings.System.getInt(mContext.getContentResolver(),
                        Settings.System.DUAL_SIM_MODE_SETTING, -1) == ALL_RADIO_OFF);
        Xlog.d(TAG, "isAllRadioOff=" + isAllRadioOff);
        return isAllRadioOff;
    }

    private boolean getSIMEnableState(int slotId) {
        return !mIsAirlineMode
                && SIMHelper.isSimInserted(slotId);
    }

    private boolean isSimInsertedWithUnAvaliable(int slotId) {
        return SIMHelper.isSimInserted(slotId)
                && (isRadioOff(slotId) || SIMHelper
                        .getSimIndicatorStateGemini(slotId) == PhoneConstants.SIM_INDICATOR_LOCKED);
    }

    private boolean isRadioOff(int slotId) {
        boolean isRadioOff = true;
        try {
            if (FeatureOption.MTK_GEMINI_SUPPORT) {
                ITelephonyEx iTelephony = SIMHelper.getITelephonyEx();
                if (iTelephony != null) {
                    isRadioOff = !iTelephony.isRadioOn(slotId);
                }
            } else {
                ITelephony iTelephony = SIMHelper.getITelephony();
                if (iTelephony != null) {
                    isRadioOff = !iTelephony.isRadioOn();
                }
            }
        } catch (RemoteException e) {
            Xlog.e(TAG, "MobileStateTracker: isRadioOff() mTelephony exception");
        }
        Xlog.d(TAG, "MobileStateTracker: isRadioOff() is " + isRadioOff + ", slotId=" + slotId);
        return isAllRadioOff() || isRadioOff;
    }

    public void refresh() {
        onActualStateChange(mContext, null);
        setCurrentState(mContext, getActualState(mContext));
        setIsUserSwitching(false);
        mSimConnTileView.setEnabled(isClickable());
        mSimConnIcon.setVisibility(View.VISIBLE);
        stopSimFrameAnim();
        setImageViewResources(mContext);
    }

    public void setHasSim(boolean enable) {
        mHasSim = enable;
    }

    public void setAirlineMode(boolean enable) {
        Xlog.d(TAG, "Sim Connect setAirlineMode called, enabled is: " + enable);
        mIsAirlineMode = enable;
    }

    public void setIsMmsOngoing(boolean enable) {
        mIsMmsOngoing = enable;
    }

    public void setIsUserSwitching(boolean enable) {
        mIsUserSwitching = enable;
    }

    public boolean getIsUserSwitching() {
        return mIsUserSwitching;
    }

    public boolean isClickable() {
        Xlog.d(TAG, "isClickable(), Sim mobile mHasSim is " + mHasSim + ", mIsAirlineMode is "
                + mIsAirlineMode + ", mIsMmsOngoing is " + mIsMmsOngoing
                + ", mIsUserSwitching is " + mIsUserSwitching);
        return (mHasSim && !isAllRadioOff() && !mIsMmsOngoing && super.isClickable());
    }

    @Override
    public int getActualState(Context context) {
        boolean sim1Enable = getSIMEnableState(PhoneConstants.GEMINI_SIM_1);
        boolean sim2Enable = getSIMEnableState(PhoneConstants.GEMINI_SIM_2);
        Xlog.d(TAG, "SimConnStateTracker getActualState sim1Enable = " + sim1Enable + " sim2Enable = " + sim2Enable);
        return ((mHasSim) && (getEnabledResource().getIconId() != -1) && (sim1Enable || sim2Enable)) ? STATE_ENABLED
                : STATE_DISABLED;
    }

    private void enterNextState(int state) {
        Xlog.d(TAG, "enterNextState state is " + state);
        switch (state) {
        case SIMConnState.NO_SIM:
        case SIMConnState.SIM1_D_D:
        case SIMConnState.SIM1_D_E:
        case SIMConnState.SIM2_D_D:
        case SIMConnState.SIM2_D_E:
        case SIMConnState.SIM1_D_F:
        case SIMConnState.SIM2_D_F:
            Xlog.d(TAG, "No Sim or one Sim do nothing!");
            break;
        case SIMConnState.SIM1_E_D:
            Xlog.d(TAG, "Try to switch from Sim1 to Sim2! mCurrentState is " + mCurrentState);
            mCurrentState = SIMConnState.SIM2_E_D;
            switchDataDefaultSIM(PhoneConstants.GEMINI_SIM_2);
            break;
        case SIMConnState.SIM1_E_E:
            Xlog.d(TAG, "Try to switch from Sim1 to Sim2! mCurrentState is " + mCurrentState);
            mCurrentState = SIMConnState.SIM2_E_E;
            switchDataDefaultSIM(PhoneConstants.GEMINI_SIM_2);
            break;
        case SIMConnState.SIM2_E_D:
            Xlog.d(TAG, "Try to switch from Sim2 to Sim1! mCurrentState is " + mCurrentState);
            mCurrentState = SIMConnState.SIM1_E_D;
            switchDataDefaultSIM(PhoneConstants.GEMINI_SIM_1);
            break;
        case SIMConnState.SIM2_E_E:
            Xlog.d(TAG, "Try to switch from Sim2 to Sim1! mCurrentState is " + mCurrentState);
            mCurrentState = SIMConnState.SIM1_E_E;
            switchDataDefaultSIM(PhoneConstants.GEMINI_SIM_1);
            break;
        case SIMConnState.SIM1_E_F:
            Xlog.d(TAG, "Try to switch from Sim1 to Sim2! mCurrentState is " + mCurrentState);
            switchDataDefaultSIM(PhoneConstants.GEMINI_SIM_2);
            break;
        case SIMConnState.SIM2_E_F:
            Xlog.d(TAG, "Try to switch from Sim2 to Sim1! mCurrentState is "
                    + mCurrentState);
            switchDataDefaultSIM(PhoneConstants.GEMINI_SIM_1);
            break;
        default:
            break;
        }
    }

    private void switchDataDefaultSIM(int slotId) {
        /// M: only apply if NOT wifi-only device @{
        if (!isWifiOnlyDevice()) {
        /// M: }@
            setIsUserSwitching(true);
        /// M: only apply if NOT wifi-only device @{
        }
        /// M: }@

        ConnectivityManager cm = (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        boolean cmConn = false;
        if (cm != null) {
            cmConn = cm.getMobileDataEnabled();
        }

        if (cmConn) {
            mDataTimerHandler.sendEmptyMessageDelayed(EVENT_ATTACH_TIME_OUT, ATTACH_TIME_OUT_LENGTH);
            /// M: only apply if NOT wifi-only device @{
            if (!isWifiOnlyDevice()) {
                /// M: }@
                mSimConnIcon.setVisibility(View.GONE);
                int resId = getInterMedateResource();
                if (resId != -1) {
                    getSwitchingGifView().setImageResource(resId);
                    getSwitchingGifView().setVisibility(View.VISIBLE);
                }
                mSimConnTileView.setEnabled(false);
                /// M: only apply if NOT wifi-only device @{
            }
            /// M: }@
            AnimationDrawable frameDrawable = (AnimationDrawable) getSwitchingGifView().getDrawable();
            if (frameDrawable != null && !frameDrawable.isRunning()) {
                frameDrawable.start();
            }
            long simId = SIMHelper.getSIMIdBySlot(mContext, slotId);
            Intent intent = new Intent();
            intent.putExtra(PhoneConstants.MULTI_SIM_ID_KEY, simId);
            intent.setAction(Intent.ACTION_DATA_DEFAULT_SIM_CHANGED);
            mContext.sendBroadcast(intent);
            Xlog.d(TAG, "switchDataDefaultSIM() sendBroadcast slotId = "
                    + slotId + ", simId = " + simId);
        } else {
            Settings.System.putInt(mContext.getContentResolver(), Settings.System.GPRS_CONNECTION_SETTING, slotId + 1);
            Long changeSimId = SIMHelper.getSIMIdBySlot(mContext, slotId);
            Settings.System.putLong(mContext.getContentResolver(), Settings.System.GPRS_CONNECTION_SIM_SETTING, changeSimId);
            Xlog.d(TAG,
                    "switchDataDefaultSIM() just change Provider, slotId = "
                            + slotId + ", changeSimId = " + changeSimId);
        }
    }

    @Override
    public void toggleState(Context context) {
        enterNextState(mCurrentState);
    }

    @Override
    public void onActualStateChange(Context context, Intent intent) {
        boolean sim1Enable = !isRadioOff(PhoneConstants.GEMINI_SIM_1)
                && SIMHelper.getSimIndicatorStateGemini(PhoneConstants.GEMINI_SIM_1) != PhoneConstants.SIM_INDICATOR_LOCKED
                && getSIMEnableState(PhoneConstants.GEMINI_SIM_1);
        boolean sim2Enable = !isRadioOff(PhoneConstants.GEMINI_SIM_2)
                && SIMHelper.getSimIndicatorStateGemini(PhoneConstants.GEMINI_SIM_2) != PhoneConstants.SIM_INDICATOR_LOCKED
                && getSIMEnableState(PhoneConstants.GEMINI_SIM_2);
        boolean sim1Conn = false;
        boolean sim2Conn = false;
        // If insert sim card first time, mobile data service do the related action for open  data connection.
        final int simSetting = Settings.System.getInt(
                mContext.getContentResolver(),
                Settings.System.GPRS_CONNECTION_SETTING,
                Settings.System.GPRS_CONNECTION_SETTING_DEFAULT);
        int slotId = simSetting - 1;
        Xlog.e(TAG, "SimConnStateTracker slotId = " + slotId);
        if (slotId == PhoneConstants.GEMINI_SIM_1) {
            sim1Conn = true;
        } else {
            sim2Conn = true;
        }
        Xlog.e(TAG, "SimConnStateTracker onActualStateChange sim1Enable = " + sim1Enable
                + ", sim2Enable = " + sim2Enable);
        if (sim1Enable || sim2Enable) {
            ConnectivityManager cm = (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
            boolean cmConn = false;
            if (cm != null) {
                cmConn = cm.getMobileDataEnabled();
            }
            if (slotId < 0
                    && simSetting != Settings.System.GPRS_CONNECTION_SETTING_DEFAULT) {
                Xlog.e(TAG, "SimConnStateTracker slotId <= 0, selected slotId is "
                                + slotId);
                if (SIMHelper.isSimInserted(PhoneConstants.GEMINI_SIM_1)
                        && !isRadioOff(PhoneConstants.GEMINI_SIM_1)) {
                    Xlog.e(TAG, "SimConnStateTracker Open Sim1 DataConn...");
                    switchDataDefaultSIM(PhoneConstants.GEMINI_SIM_1);
                } else if (SIMHelper.isSimInserted(PhoneConstants.GEMINI_SIM_2)
                        && !isRadioOff(PhoneConstants.GEMINI_SIM_2)) {
                    Xlog.e(TAG, "SimConnStateTracker Open Sim2 DataConn...");
                    switchDataDefaultSIM(PhoneConstants.GEMINI_SIM_2);
                }
                return;
            }
            Xlog.e(TAG, "SimConnStateTracker onActualStateChange, cm = " + cm
                    + " cmConn = " + cmConn + ", sim1Enable = " + sim1Enable
                    + ", sim2Enable = " + sim2Enable + ", sim1Conn = "
                    + sim1Conn + ", sim2Conn = " + sim2Conn + ", slotId = "
                    + slotId);
            if (cmConn) {
                if (sim1Enable && sim2Enable) {
                    if (sim1Conn) {
                        mCurrentState = SIMConnState.SIM1_E_E;
                    } else {
                        mCurrentState = SIMConnState.SIM2_E_E;
                    }
                } else if (!sim1Enable && sim2Enable) {
                    if (isSimInsertedWithUnAvaliable(PhoneConstants.GEMINI_SIM_1)
                            && sim1Conn) {
                        mCurrentState = SIMConnState.SIM1_E_F;
                    } else {
                        mCurrentState = SIMConnState.SIM2_D_E;
                    }
                } else if (sim1Enable && !sim2Enable) {
                    if (isSimInsertedWithUnAvaliable(PhoneConstants.GEMINI_SIM_2)
                            && sim2Conn) {
                        mCurrentState = SIMConnState.SIM2_E_F;
                    } else {
                        mCurrentState = SIMConnState.SIM1_D_E;
                    }
                }
            } else {
                if (sim1Enable && sim2Enable) {
                    if (sim1Conn) {
                        mCurrentState = SIMConnState.SIM1_E_D;
                    } else {
                        mCurrentState = SIMConnState.SIM2_E_D;
                    }
                } else if (!sim1Enable && sim2Enable) {
                    if (isSimInsertedWithUnAvaliable(PhoneConstants.GEMINI_SIM_1)
                            && sim1Conn) {
                        mCurrentState = SIMConnState.SIM1_E_F;
                    } else {
                        mCurrentState = SIMConnState.SIM2_D_D;
                    }
                } else if (sim1Enable && !sim2Enable) {
                    if (isSimInsertedWithUnAvaliable(PhoneConstants.GEMINI_SIM_2)
                            && sim2Conn) {
                        mCurrentState = SIMConnState.SIM2_E_F;
                    } else {
                        mCurrentState = SIMConnState.SIM1_D_D;
                    }
                }
            }
        } else {
            if (isSimInsertedWithUnAvaliable(PhoneConstants.GEMINI_SIM_1)
                    && sim1Conn) {
                mCurrentState = SIMConnState.SIM1_D_F;
            } else if (isSimInsertedWithUnAvaliable(PhoneConstants.GEMINI_SIM_2)
                    && sim2Conn) {
                mCurrentState = SIMConnState.SIM2_D_F;
            } else {
                mCurrentState = SIMConnState.NO_SIM;
            }
        }
        mIsUserSwitching = false;
        setCurrentState(context, mobileStateToFiveState());
    }

    private int mobileStateToFiveState() {
        int simSlotId = Settings.System.getInt(mContext.getContentResolver(),
                Settings.System.GPRS_CONNECTION_SETTING, -1) - 1;
        ConnectivityManager cm = (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        boolean dataState = false;
        if (cm != null) {
            dataState = cm.getMobileDataEnabled();
        }
        Xlog.d(TAG, "SimConnStateToFiveState simSlotId is : " + simSlotId + " cmConn is : " + dataState);
        int currentState;
        if (dataState) {
            mDataTimerHandler.removeMessages(EVENT_ATTACH_TIME_OUT);
            SimInfoManager.SimInfoRecord simInfo = SIMHelper.getSIMInfoBySlot(mContext, simSlotId);
            if (simInfo == null) {
                Xlog.e(TAG, "SimConnStateTracker mobileStateToFiveState error for simInfo, slotId is "
                        + simSlotId);
                return STATE_UNKNOWN;
            }
            currentState = STATE_ENABLED;
        } else {
            mDataTimerHandler.removeMessages(EVENT_DETACH_TIME_OUT);
            currentState = STATE_DISABLED;
        }
        return currentState;
    }

    @Override
    public void requestStateChange(final Context context,
            final boolean desiredState) {
        Xlog.e(TAG, "SimConnStateTracker requestStateChange do nothing.");
    }

    public IconIdWrapper getDisabledResource() {
        IconIdWrapper dataIcon = new IconIdWrapper();
        dataIcon.setResources(mContext.getResources());
        dataIcon.setIconId(R.drawable.ic_qs_mobile_all_disable_disable);
        return dataIcon;
    }

    public IconIdWrapper getEnabledResource() {
        int simSlotId = Settings.System.getInt(mContext.getContentResolver(),
                Settings.System.GPRS_CONNECTION_SETTING, -1) - 1;
        int iconId = -1;
        if (simSlotId < 0) {
            Xlog.e(TAG, "SimConnStateTracker getEnabledResource error, selected simSlotId is " + simSlotId);
        } else {
            iconId = TelephonyIcons.IC_SIM_CONNECT_INDICATOR[mCurrentState];
        }
        IconIdWrapper dataIcon = new IconIdWrapper();
        dataIcon.setResources(mContext.getResources());
        dataIcon.setIconId(iconId);
        return dataIcon;
    }

    public ImageView getImageButtonView() {
        return mSimConnIcon;
    }

    public View getTileView() {
        return mSimConnTileView;
    }

    @Override
    public int getInterMedateResource() {
        return R.drawable.ic_qs_stat_sys_mobile_switch_anim;
    }

    @Override
    public ImageView getSwitchingGifView() {
        if (mSimConnSwitchIngGifView == null) {
            ViewGroup.LayoutParams layoutParam = new ViewGroup.LayoutParams(
                    LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT);
            mSimConnSwitchIngGifView = new ImageView(mContext);
            mSimConnLayout.addView(mSimConnSwitchIngGifView, 0);
            mSimConnSwitchIngGifView.setVisibility(View.GONE);
        }
        return mSimConnSwitchIngGifView;
    }

    @Override
    public ImageView getIndicatorView() {
        return null;
    }

    private void stopSimFrameAnim() {
        ImageView animView = getSwitchingGifView();
        if (animView != null) {
            animView.setVisibility(View.GONE);
            AnimationDrawable frameDrawable = (AnimationDrawable) animView.getDrawable();
            if (frameDrawable != null && frameDrawable.isRunning()) {
                frameDrawable.stop();
            }
        }
    }
}
