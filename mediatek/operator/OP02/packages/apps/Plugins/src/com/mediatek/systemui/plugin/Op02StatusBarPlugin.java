package com.mediatek.systemui.plugin;

import android.content.Context;
import android.content.res.Resources;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.view.Gravity;
import android.view.View;
import android.widget.TextView;

import com.android.internal.telephony.ITelephony;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.telephony.ITelephonyEx;
import com.mediatek.op02.plugin.R;
import com.mediatek.systemui.ext.BehaviorSet;
import com.mediatek.systemui.ext.DataType;
import com.mediatek.systemui.ext.DefaultStatusBarPlugin;
import com.mediatek.systemui.ext.IconIdWrapper;
import com.mediatek.systemui.ext.NetworkType;
import com.mediatek.xlog.Xlog;

import java.util.List;

/**
 * M: OP02 implementation of Plug-in definition of Status bar.
 */
public class Op02StatusBarPlugin extends DefaultStatusBarPlugin {
    private static final String TAG = "Op02StatusBarPlugin";

    public Op02StatusBarPlugin(Context context) {
        super(context);
    }

    public void customizeSignalStrengthNullIconGemini(IconIdWrapper icon, int slotId) {
        icon.setResources(this.getResources());
        int iconId = -1;
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            switch (slotId) {
            case 0:
                iconId = R.drawable.stat_sys_gemini_signal_null_sim1;
                break;
            case 1:
                iconId = R.drawable.stat_sys_gemini_signal_null_sim2;
                break;
            case 2:
                iconId = R.drawable.stat_sys_gemini_signal_null_sim3;
                break;
            case 3:
                iconId = R.drawable.stat_sys_gemini_signal_null_sim4;
                break;
            default:
                iconId = -1;
                break;
            }
        } else {
            iconId = R.drawable.stat_sys_gemini_signal_null;
        }
        icon.setIconId(iconId);
    }

    public int[] getDataTypeIconListGemini(boolean roaming, DataType dataType) {
        int[] iconList = null;
        if (roaming) {
            iconList = TelephonyIcons.DATA_ROAM[dataType.getTypeId()];
        }
        return iconList;
    }


    public void customizeSignalIndicatorIconGemini(IconIdWrapper icon, int slotId) {
        icon.setResources(this.getResources());
        int iconId = -1;
        switch (slotId) {
        case 0:
            iconId = R.drawable.stat_sys_gemini_signal_indicator_sim1;
            break;
        case 1:
            iconId = R.drawable.stat_sys_gemini_signal_indicator_sim2;
            break;
        case 2:
            iconId = R.drawable.stat_sys_gemini_signal_indicator_sim3;
            break;
        case 3:
            iconId = R.drawable.stat_sys_gemini_signal_indicator_sim4;
            break;
        default:
            iconId = -1;
            break;
        }
        icon.setIconId(iconId);
    }

    @Override
    public BehaviorSet customizeBehaviorSet() {
        return BehaviorSet.OP02_BS;
    }

    @Override
    public void customizeDataActivityIcon(IconIdWrapper icon, int simColor, int dataActivity) {
        Xlog.d(TAG, "customizeDataTypeIconListGemini, dataActivity = " + dataActivity
                + " simColor = " + simColor);

        if (simColor >= 0 && simColor < TelephonyIcons.DATA_ACTIVITY.length) {
            icon.setResources(this.getResources());
            icon.setIconId(TelephonyIcons.DATA_ACTIVITY[simColor][dataActivity]);
        }
    }

    @Override
    public void customizeDataTypeIconListGemini(IconIdWrapper[] iconList, boolean roaming,
            DataType dataType) {
        Xlog.d(TAG, "customizeDataTypeIconListGemini, dataType = " + dataType
                + " roaming = " + roaming);

        if (dataType != DataType.Type_3G_PLUS) {
            int[] resIconList = TelephonyIcons.DATA[dataType.getTypeId()];
            for (int i = 0; i < iconList.length; i++) {
                iconList[i].setResources(this.getResources());
                iconList[i].setIconId(resIconList[i]);
            }
        }
    }

    public void customizeDataNetworkTypeIconGemini(IconIdWrapper icon, NetworkType networkType, int simColorId) {
        icon.setResources(this.getResources());

        Xlog.d(TAG, "customizeDataNetworkTypeIconGemini, networkType = " + networkType + " simColorId = " + simColorId);
        switch(networkType) {
        case Type_G:
        case Type_E:
            icon.setIconId(TelephonyIcons.NETWORK_TYPE_G[simColorId]);
            break;
        case Type_3G:
            icon.setIconId(TelephonyIcons.NETWORK_TYPE_3G[simColorId]);
            break;
        case Type_4G:
            icon.setIconId(TelephonyIcons.NETWORK_TYPE_4G[simColorId]);
            break;
        default:
              break;
        }
    }

    public boolean customizeMobileGroupVisible(boolean isSimInserted) {
        return true;
    }

    public void customizeCarrierText(List<? extends TextView> carrierList,
            List<View> dividerList, int[] insertedSim,
            boolean[] carrierShowDefault) {
        ITelephony telephony = ITelephony.Stub.asInterface(ServiceManager
                .getService(Context.TELEPHONY_SERVICE));

        ITelephonyEx telephonyEx = ITelephonyEx.Stub.asInterface(ServiceManager
                .getService(Context.TELEPHONY_SERVICEEX));
        int numOfSIM = 0;
        int hideDivider = 0;
        TextView carrierLeft = null;
        TextView carrierRight = null;
        ServiceState sStateLeft = new ServiceState();
        ServiceState sStateRight = new ServiceState();
        boolean carrierShowDefaultLeft = false;
        boolean carrierShowDefaultRight = false;
        int simStateLeft = 0;
        int simStateRight = 0;

        if (telephony != null && telephonyEx != null) {
            try {
                for (int i = 0; i < carrierList.size(); i++) {
                    if (insertedSim[i] == 1) {
                        numOfSIM++;
                        if (numOfSIM == 1) {
                            carrierLeft = carrierList.get(i);
                            sStateLeft = ServiceState.newFromBundle(telephonyEx
                                    .getServiceState(i));
                            carrierShowDefaultLeft = carrierShowDefault[i];
                            simStateLeft = telephony.getSimState(i);
                        } else if (numOfSIM == 2) {
                            hideDivider = i;
                            carrierRight = carrierList.get(i);
                            sStateRight = ServiceState.newFromBundle(telephonyEx
                                    .getServiceState(i));
                            carrierShowDefaultRight = carrierShowDefault[i];
                            simStateRight = telephony.getSimState(i);
                        }
                    }
                }
            } catch (RemoteException e) {
                Xlog.d(TAG, "customrizePlmnInNotification occur exceprion", e);
            }

            Xlog.d(TAG, "isSimLocked(simStateLeft, simStateRight) = "
                    + isSimLocked(simStateLeft, simStateRight));

            // sim1 or sim2 not locked
            if (numOfSIM == 2 && carrierLeft != null && carrierRight != null
                    && !isSimLocked(simStateLeft, simStateRight)) {
                Xlog.d(TAG,
                        "sStateLeft.getState() = " + sStateLeft.getState()
                                + ", sStateRight.getState() = "
                                + sStateRight.getState() + ", carrierLeft = "
                                + carrierLeft.getText().toString()
                                + ", carrierRight = "
                                + carrierRight.getText().toString()
                                + ", sStateLeft.isEmergencyOnly() = "
                                + sStateLeft.isEmergencyOnly()
                                + ", sStateRight.isEmergencyOnly() = "
                                + sStateRight.isEmergencyOnly()
                                + ", carrierShowDefaultLeft = "
                                + carrierShowDefaultLeft
                                + ", carrierShowDefaultRight = "
                                + carrierShowDefaultRight + ", simStateLeft = "
                                + simStateLeft + ", simStateRight = "
                                + simStateRight);
                // nw send sim1 "no service"
                if (
                ((sStateLeft.getState() != ServiceState.STATE_IN_SERVICE) && (!sStateLeft
                        .isEmergencyOnly()))
                // nw send sim2 "no service"
                        || ((sStateRight.getState() != ServiceState.STATE_IN_SERVICE) && (!sStateRight
                                .isEmergencyOnly()))
                        // nw send no invalid plmn, systemui show
                        // default "no service"
                        || carrierShowDefaultLeft || carrierShowDefaultRight) {
                    Xlog.d(TAG, "dimiss the sim1 info");
                    carrierRight.setVisibility(View.GONE);
                    carrierLeft.setGravity(Gravity.CENTER);
                    dividerList.get(hideDivider - 1).setVisibility(
                            View.INVISIBLE);
                }
            }
        }
    }

    private boolean isSimLocked(int sim1State, int sim2State) {
        return TelephonyManager.SIM_STATE_PIN_REQUIRED == sim1State
                || TelephonyManager.SIM_STATE_PUK_REQUIRED == sim1State
                || TelephonyManager.SIM_STATE_PIN_REQUIRED == sim2State
                || TelephonyManager.SIM_STATE_PUK_REQUIRED == sim2State;
    }

    public void customizeCarrierTextBySimInserted(TextView carrierTextView, boolean simInserted, String networkName) {
        String originalCarrierText = carrierTextView != null ? carrierTextView.getText().toString() : null;
        Xlog.d(TAG, "customizeCarrierTextBySimInserted, carrierTextView = " + originalCarrierText + " simInserted = "
                + simInserted + " networkName = " + networkName);
        String noSimStr = Resources.getSystem().getText(com.android.internal.R.string.lockscreen_missing_sim_message_short)
                .toString();
        if (carrierTextView != null) {
            if (simInserted) {
                carrierTextView.setText(networkName);
            } else {
                carrierTextView.setText(noSimStr);
            }
        }
    }
}
