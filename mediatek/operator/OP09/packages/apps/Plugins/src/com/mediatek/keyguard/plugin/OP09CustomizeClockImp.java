package com.mediatek.keyguard.plugin;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;

import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.util.TypedValue;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.ITelephonyRegistry;

import com.mediatek.keyguard.ext.DefaultCustomizeClock;
import com.mediatek.op09.plugin.R;

import com.mediatek.keyguard.plugin.ClockView;
import com.mediatek.telephony.TelephonyManagerEx;
import com.mediatek.xlog.Xlog;

public class OP09CustomizeClockImp extends DefaultCustomizeClock {
    private static final String TAG = "OP09CustomizeClockImp";
    private static final boolean DEBUG = true;

    /**
     * For Simulating Roaming test. CT Roaming status is not easy to test and
     * need to come to HK or TW etc. Can use adb shell to send broadcast to set
     * testing roaming condition, so add a broadcast receiver to receive the
     * forcing roaming condition. @{
     */
    private static final String ROAMING_PROPERTY = "com.mediatek.op.policy.customizeclock.property";
    private static final String ROAMING_PROPERTY_NAME = "customizeclockroaming";
    private static final int ROAMING_UPDATE = 1000;

    private DualClockPropertyBroadcastReceiver mDualClockPropertyBroadcastReceiver;
    private static boolean sPropertyRoaming = false;

    private PhoneStateListener mPhoneStateListener;
    private PhoneStateListener mPhoneStateListenerGemini;
    private TelephonyManager mTelephonyManager;

    private View mCustomizeClock;
    private View mDomesticClockView;
    private ViewGroup mKeyguardStatusArea;

    private Context mContext;

    public OP09CustomizeClockImp(Context context) {
        super(context);
        if (DEBUG) {
            Xlog.d(TAG, "OP09CustomizeClockImp(): mContext = " + mContext);
        }
        mContext = context;
    }


    public void addCustomizeClock(Context context, ViewGroup clockContainer, ViewGroup statusArea) {
        mKeyguardStatusArea = statusArea;

        mTelephonyManager = (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
        final LayoutInflater inflater = LayoutInflater.from(mContext);

        // Keyguard presentation also use the same parent view in OP09, at that time the customized clock index is not exist, 
        // customized clock is null. So save the customized clock for updating layout judgment and return null.
        mCustomizeClock = clockContainer; 
        if (clockContainer == null) {
            return;
        }
        /// remove the customize clock child view
        clockContainer.removeAllViews();

        LinearLayout.LayoutParams customizeClockParams = (LinearLayout.LayoutParams)clockContainer.getLayoutParams();
        customizeClockParams.width = LayoutParams.MATCH_PARENT;
        clockContainer.setLayoutParams(customizeClockParams);

        /// add the customize clock new view 
        View customizeClockView = inflater.inflate(R.layout.keyguard_domestic_clock, clockContainer, true);

        mCustomizeClock = customizeClockView;
        mDomesticClockView = (View) customizeClockView.findViewById(R.id.clock_view_domestic);

        /// update the customize clock
        updateCustomizeClock(isRoaming());

        initPhoneStateListener(context);

        registerPropertyBroadcast();
    }

    /**
     * Adjust the message area's layout according roaming status.
     * 
     * @param bRoaming
     *            Whether it is roaming.
     */
    private void adjustMessageArea(boolean bRoaming) {
        LinearLayout.LayoutParams linearLayoutLayoutParams = (LinearLayout.LayoutParams) mKeyguardStatusArea
                .getLayoutParams();
        if (bRoaming) {
            linearLayoutLayoutParams.width = LinearLayout.LayoutParams.WRAP_CONTENT;
        } else {
            linearLayoutLayoutParams.width = LinearLayout.LayoutParams.MATCH_PARENT;
        }
        mKeyguardStatusArea.setLayoutParams(linearLayoutLayoutParams);
    }

    /**
     * Initialize phone state listener, used to update sim's roaming status
     */
    public void initPhoneStateListener(Context context) {
        mPhoneStateListener = new PhoneStateListener() { // for SIM1 and single
            @Override
            public void onServiceStateChanged(ServiceState state) {
                if (state != null) {
                    int inServiceState = state.getRegState();
                    if (DEBUG) {
                        Xlog.d(TAG, "mPhoneStateListener--onServiceStateChanged inServiceState = " + inServiceState);
                    }
                    // if (inServiceState == ServiceState.STATE_IN_SERVICE) {
                    updateCustomizeClock(isRoaming());
                    // }
                }
            }
        };

        mPhoneStateListenerGemini = new PhoneStateListener() { // only for SIM2
            @Override
            public void onServiceStateChanged(ServiceState state) {
                if (state != null) {
                    int inServiceState = state.getRegState();
                    if (DEBUG) {
                        Xlog.d(TAG, "mPhoneStateListenerGemini--onServiceStateChanged inServiceState = " + inServiceState);
                    }
                    // if (inServiceState == ServiceState.STATE_IN_SERVICE) {
                    updateCustomizeClock(isRoaming());
                    // } else {

                    // }
                }
            }
        };

        listenPhoneEvent(PhoneStateListener.LISTEN_SERVICE_STATE);
    }

    /**
     * Reset the phone listener.
     */
    @Override
    public void reset() {
        if (DEBUG) {
            Xlog.d(TAG, "reset");
        }
        unRegisterPropertyBroadcast();
        listenPhoneEvent(PhoneStateListener.LISTEN_NONE);
    }

    /**
     * Register to listen the PhoneEvent.
     * 
     * @param phoneEvent
     *            The phone event to register.
     */
    public void listenPhoneEvent(final int phoneEvent) {
        if (DEBUG) {
            Xlog.d(TAG, "listenPhoneEvent, phoneEvent = " + phoneEvent + " mPhoneStateListener = " + mPhoneStateListener
                    + " mPhoneStateListenerGemini = " + mPhoneStateListenerGemini);
        }

        if (mPhoneStateListener == null) {
            return;
        }
        if (isGemini()) {
            if (DEBUG) {
                Xlog.d(TAG, "listenPhoneEvent, phoneEvent = " + phoneEvent + " isGemini() true");
            }
            try {
                ITelephony t = ITelephony.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICE));
                Boolean notifyNow = (t != null);
                ITelephonyRegistry tr1 = ITelephonyRegistry.Stub
                        .asInterface(ServiceManager.getService("telephony.registry"));
                tr1.listen(TAG, mPhoneStateListener.getCallback(), phoneEvent, notifyNow);
                ITelephonyRegistry tr2 = ITelephonyRegistry.Stub.asInterface(ServiceManager
                        .getService("telephony.registry2"));
                tr2.listen(TAG, mPhoneStateListenerGemini.getCallback(), phoneEvent, notifyNow);
            } catch (RemoteException e) {
                if (DEBUG) {
                    Xlog.e(TAG, "Fail to listen GEMINI state", e);
                }
            } catch (NullPointerException e) {
                if (DEBUG) {
                    Xlog.e(TAG, "The registry is null", e);
                }
            }
        } else {
            if (DEBUG) {
                Xlog.d(TAG, "listenPhoneEvent, phoneEvent = " + phoneEvent + " isGemini() false");
            }
            ((TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE))
                    .listen(mPhoneStateListener, phoneEvent);
        }
    }

    /**
     * Whether it is roaming
     * 
     * @return
     */
    private boolean isRoaming() {
        if (DEBUG) {
            Xlog.d(TAG, "isRoaming");
        }
        boolean bRoaming = false;
        if (isGemini()) {
            int masterCardSlot = getExternalModemSlot();
            if (DEBUG) {
                Xlog.d(TAG, "isRoaming, isGemini() true, masterCardSlot = " + masterCardSlot);
            }
            if (masterCardSlot != -1) {
                bRoaming = TelephonyManagerEx.getDefault().isNetworkRoaming(masterCardSlot);
            }
        } else {
            if (DEBUG) {
                Xlog.d(TAG, "isRoaming, isGemini() false");
            }
            bRoaming = mTelephonyManager.isNetworkRoaming();
        }
        if (DEBUG) {
            Xlog.d(TAG, "isRoaming, " + (bRoaming || getPropertyRoaming()));
        }
        return bRoaming || getPropertyRoaming();
    }

    /**
     * Update the customize clock's layout according to roaming status.
     * 
     * @param bRoaming
     *            Whether it is roaming.
     */
    private void updateCustomizeClock(boolean bRoaming) {
        if (DEBUG) {
            Xlog.d(TAG, "updateCustomizeClock, bRoaming = " + bRoaming);
        }
        if (mDomesticClockView != null) {
            if (DEBUG) {
                Xlog.d(TAG, "updateCustomizeClock, bRoaming = " + bRoaming + " mDomesticClockView != null ");
            }
            adjustClockLayout(bRoaming);
            adjustClockTextSize(bRoaming);
            adjustMessageArea(bRoaming);
        }
    }

    private void adjustClockLayout(boolean bRoaming) {
        TextView clockText = ((TextView) mCustomizeClock.findViewById(R.id.clock_text));
        int visibility = View.GONE;
        RelativeLayout.LayoutParams clockTextParams = (RelativeLayout.LayoutParams)clockText.getLayoutParams();
        View clockView = (View)mCustomizeClock.findViewById(R.id.clock_view);
        LinearLayout.LayoutParams customizeClockParams = (LinearLayout.LayoutParams)clockView.getLayoutParams();

        if (bRoaming) {
            visibility = View.VISIBLE;
            clockTextParams.removeRule(RelativeLayout.CENTER_IN_PARENT);
            customizeClockParams.width = LayoutParams.WRAP_CONTENT;
        } else {
            clockTextParams.addRule(RelativeLayout.CENTER_IN_PARENT);
            customizeClockParams.width = LayoutParams.MATCH_PARENT;
        }

        mDomesticClockView.setVisibility(visibility);
        clockText.setLayoutParams(clockTextParams);
        clockView.setLayoutParams(customizeClockParams);
    }

    private void adjustClockTextSize(boolean bRoaming) {
        TextView clockText = ((TextView) mCustomizeClock.findViewById(R.id.clock_text));
        TextView ampmText = ((TextView) mCustomizeClock.findViewById(R.id.am_pm));

        int kgStatusClockFontSize = R.dimen.kg_status_clock_font_size;
        int kgStatusClockAmpmFontSize = R.dimen.kg_status_clock_ampm_font_size;

        if (bRoaming) {
            kgStatusClockFontSize = R.dimen.kg_status_customize_first_clock_time_font_size;
            kgStatusClockAmpmFontSize = R.dimen.kg_status_customize_first_clock_ampm_font_size;
        }

        float clockTextSize = mContext.getResources().getDimension(kgStatusClockFontSize);
        float amPmTextSize = mContext.getResources().getDimension(kgStatusClockAmpmFontSize);

        clockText.setTextSize(TypedValue.COMPLEX_UNIT_PX, clockTextSize);
        ampmText.setTextSize(TypedValue.COMPLEX_UNIT_PX, amPmTextSize);
    }

    /**
     * Get main slot.
     * 
     * @return
     */
    public int getExternalModemSlot() {
        int slot = 0;
        slot = SystemProperties.getInt("ril.external.md", 0);
        if (DEBUG) {
            Xlog.d(TAG, "getExternalModemSlot slot=" + (slot - 1));
        }
        return slot - 1;
    }

    /**
     * Whether is gemini or not
     * 
     * @return
     */
    public static final boolean isGemini() {
        if (DEBUG) {
            Xlog.d(TAG, "isGemini = " + SystemProperties.getInt("ro.mtk_gemini_support", 0));
        }
        return SystemProperties.getInt("ro.mtk_gemini_support", 0) == 1;
    }

    /**
     * For Simulating Roaming test. Register property broadcast receiver
     */
    public void registerPropertyBroadcast() {
        if (DEBUG) {
            Xlog.d(TAG, "registerPropertyBroadcast");
        }
        mDualClockPropertyBroadcastReceiver = new DualClockPropertyBroadcastReceiver();
        IntentFilter filter = new IntentFilter(ROAMING_PROPERTY);
        mContext.registerReceiver(mDualClockPropertyBroadcastReceiver, filter);
    }

    /**
     * For Simulating Roaming test. Unregister property broadcast receiver
     */
    public void unRegisterPropertyBroadcast() {
        if (mDualClockPropertyBroadcastReceiver != null) {
            if (DEBUG) {
                Xlog.d(TAG, "unRegisterPropertyBroadcast");
            }
            mContext.unregisterReceiver(mDualClockPropertyBroadcastReceiver);
            mDualClockPropertyBroadcastReceiver = null;
        }
    }

    /**
     * For Simulating Roaming test. The class is to receive property values. Use
     * the console command"adb shell am broadcast -a com.mediatek.op.policy.customizeclock.property --ei customizeclockroaming 1"
     * to force enabling roaming, or the value after customizeclockroaming is not 1
     * to disable.
     */
    private class DualClockPropertyBroadcastReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            int roamingPropertyValue = intent.getIntExtra(ROAMING_PROPERTY_NAME, -1);
            if (DEBUG) {
                Xlog.d(TAG, "DualClockPropertyBroadcastReceiver onReceive, intent = " + intent + ROAMING_PROPERTY_NAME
                        + " = " + roamingPropertyValue);
            }
            Message msg = mHandler.obtainMessage(ROAMING_UPDATE);
            msg.what = ROAMING_UPDATE;
            msg.arg1 = roamingPropertyValue;
            mHandler.sendMessage(msg);
        }
    }

    /**
     * For Simulating Roaming test. Set the property roaming value.
     * 
     * @param bRoaming
     *            Whether it is Roaming.
     */
    private void setPropertyRoaming(boolean bRoaming) {
        sPropertyRoaming = bRoaming;
    }

    /**
     * For Simulating Roaming test. Get the property roaming value.
     * 
     * @return
     */
    private boolean getPropertyRoaming() {
        return sPropertyRoaming;
    }

    /**
     * Handler to process message sent from receiver to update UI.
     */
    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case ROAMING_UPDATE:
                if (DEBUG) {
                    Xlog.d(TAG, "mHandler handleMessage, ROAMING_UPDATE msg.arg1 = " + msg.arg1);
                }
                boolean bRoaming = (msg.arg1 == 1) ? true : false;
                setPropertyRoaming(bRoaming);
                updateCustomizeClock(bRoaming);
                break;
            default:
                break;
            }
        }
    };

    /** {@inheritDoc} */
    @Override
    public void updateClockLayout() {
        // / M: If roaming, adjust the domestic clock's baseline to align to the
        // phone settings time.
        Log.d(TAG, "updateClockLayout");

        // Keyguard presentation also use the same parent view in OP09, customize clock will be null, so return.
        if (mCustomizeClock == null) {
            return;
        }

        if (isRoaming()) {
            TextView phoneSettingsClockTimeText = ((TextView) mCustomizeClock.findViewById(R.id.clock_text));
            int phoneSettingsClockTimeOffset = phoneSettingsClockTimeText.getBottom()
                    - phoneSettingsClockTimeText.getBaseline();

            View phoneSettingClock = mCustomizeClock.findViewById(R.id.clock_view);

            if (DEBUG) {
                Xlog.d(TAG, "updateClockLayout, clockText.getBottom() = " + phoneSettingsClockTimeText.getBottom()
                        + " clockText.getBaseline() = " + phoneSettingsClockTimeText.getBaseline()
                        + " phoneSettingsClockTimeOffset = " + phoneSettingsClockTimeOffset
                        + " phoneSettingClock.getBottom() = " + phoneSettingClock.getBottom()
                        + " mDomesticClockView.getBottom() = " + mDomesticClockView.getBottom());
            }
            // / M: Only when the bottoms of the phone settings clock and the
            // domestic clock are alignment,
            // / need to adjust the the domestic clock's second line text's
            // baseline to align the phone settings clock's time text's
            // baseline.
            if (phoneSettingClock.getBottom() == mDomesticClockView.getBottom()) {
                TextView weekdayClockTextDomestic = ((TextView) mCustomizeClock.findViewById(R.id.weekday_domestic));
                int weekdayClockTextDomesticOffset = weekdayClockTextDomestic.getBottom()
                        - weekdayClockTextDomestic.getBaseline();

                // / The domestic clock view will add the offset, so phone
                // settings time text's offset(bottom - baseline) was subtract
                // and the domestic clock second line text view's offset need
                // added.
                mDomesticClockView.offsetTopAndBottom(weekdayClockTextDomesticOffset - phoneSettingsClockTimeOffset);
            }
        }
    }
}
