package com.mediatek.calloption.plugin;

import android.app.Activity;
import android.content.Intent;
import android.content.Context;
import android.os.Bundle;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.RadioButton;

import com.android.phone.Constants;
import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.PhoneConstants;

import com.mediatek.common.telephony.ITelephonyEx;
import com.mediatek.op09.plugin.R;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;

import java.util.ArrayList;
import java.util.List;

public class ESurfingGuideSelectActivity extends Activity
	implements View.OnClickListener {

    private static final String TAG = "ESurfingGuideSelectActivity";

    private static final int NUMBER_LENGTH_MINUS = 4;
    private static final int SIM_BACKGROUND_COLOR_INDEX_ORANGE = 1;
    private static final int SIM_BACKGROUND_COLOR_INDEX_BLUE = 0;
    private static final String PLUGIN_PACKAGE = "com.mediatek.op09.plugin";
    private static final String DIALING_GUIDE_CLASS =
        "com.mediatek.calloption.plugin.ESurfingDialingGuideActivity";

    private Context mContext;
    private Intent mIntent;

    public class ViewHolder {
        public View      mSimIcon;
        public ImageView mSimStatus;
        public TextView  mSimSignal;
        public TextView  mShortPhoneNumber;
        public TextView  mDisplayName;
        public TextView  mPhoneNumber;
        public ImageView  mSuggested;
        public TextView  mText;
        public ImageView mInternetIcon;
        public RadioButton mRadioButton;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        mContext = this;

        List<SimInfoRecord> insertedSimInfoList = null;
        SimInfoRecord simInfo = null;
        int slotId = -1;
        insertedSimInfoList = SimInfoManager.getInsertedSimInfoList(mContext);
        if (insertedSimInfoList == null || insertedSimInfoList.size() < 1) {
            log(" mInsertedSimInfoList is null");
            this.finish();
            return;
        }
        String slot1Mcc = OP09CallOptionUtils.getCountryBySlotId(PhoneConstants.GEMINI_SIM_1);
        String slot2Mcc = OP09CallOptionUtils.getCountryBySlotId(PhoneConstants.GEMINI_SIM_2);
        log(" onCreate is slot1Mcc : " + slot1Mcc + " slot2Mcc : " + slot2Mcc);
        if (1 == insertedSimInfoList.size()) {
            simInfo = insertedSimInfoList.get(0);
        }
        if (null != simInfo) {
            slotId = simInfo.mSimSlotId;
        }
        if (2 == insertedSimInfoList.size() &&
                      (null != slot1Mcc && null != slot2Mcc && slot1Mcc.equals(slot2Mcc))) {
            slotId = PhoneConstants.GEMINI_SIM_1;
        }
        if (-1 != slotId) {
            startEsurfingGuideBySlotId(slotId);
            this.finish();
            return;
        }
        if (2 == insertedSimInfoList.size() &&
                      (null != slot1Mcc && null != slot2Mcc && !slot1Mcc.equals(slot2Mcc))) {
            setContentView(R.layout.esurfing_guide_select);
            View simItem1 = findViewById(R.id.sim_item1);
            View simItem2 = findViewById(R.id.sim_item2);
            Button cancel = (Button)findViewById(R.id.button);
            ViewHolder viewHolder1 = createViewHolder(simItem1);
            ViewHolder viewHolder2 = createViewHolder(simItem2);
            if (simItem1 != null) {
                simItem1.setOnClickListener(this);
            }
            if (simItem2 != null) {
                simItem2.setOnClickListener(this);
            }
            if (cancel != null) {
                cancel.setOnClickListener(this);
            }

            initItemViewHolder(viewHolder1, getSimInfoBySlot(PhoneConstants.GEMINI_SIM_1));
            initItemViewHolder(viewHolder2, getSimInfoBySlot(PhoneConstants.GEMINI_SIM_2));
        } else {
            this.finish();
            return;
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        log("onResume");

    }

    private SimInfoRecord getSimInfoBySlot(int slot) {
        List<SimInfoRecord> insertedSimInfoList = null;
        SimInfoRecord simInfo = null;
        insertedSimInfoList = SimInfoManager.getInsertedSimInfoList(mContext);

        if (insertedSimInfoList == null) {
            log("mSimInfoList OR mInsertedSimInfoList is null");
            return null;
        }

        for (int i = 0; i < insertedSimInfoList.size(); i++) {
            simInfo = insertedSimInfoList.get(i);
            if (simInfo.mSimSlotId == slot) {
                return simInfo;
            }
        }

        return null;
    }

    public void initItemViewHolder(ViewHolder holder, SimInfoRecord simInfo) {
        if (null == holder || null == simInfo) {
            return;
        }
        log("initItemView(), simInfo.mNumber = " + simInfo.mNumber);
        holder.mDisplayName.setText(simInfo.mDisplayName);
        holder.mSimIcon.setBackgroundResource(SimInfoManager.SimBackgroundDarkRes[simInfo.mColor]);

        String shortNumber = "";
        if (!TextUtils.isEmpty(simInfo.mNumber)) {
            switch (simInfo.mDispalyNumberFormat) {
                case SimInfoManager.DISPLAY_NUMBER_FIRST:
                    if (simInfo.mNumber.length() <= NUMBER_LENGTH_MINUS) {
                        shortNumber = simInfo.mNumber;
                    } else {
                        shortNumber = simInfo.mNumber.substring(0, NUMBER_LENGTH_MINUS);
                    }
                    break;
                case SimInfoManager.DISPLAY_NUMBER_LAST:
                    if (simInfo.mNumber.length() <= NUMBER_LENGTH_MINUS) {
                        shortNumber = simInfo.mNumber;
                    } else {
                        shortNumber = simInfo.mNumber.substring(simInfo.mNumber.length()
                                - NUMBER_LENGTH_MINUS, simInfo.mNumber.length());
                    }
                    break;
                case 0://SimInfoManager.DISPLAY_NUMBER_NONE:
                    shortNumber = "";
                    break;
                default:
                    break;
            }
            holder.mPhoneNumber.setText(simInfo.mNumber);
            holder.mPhoneNumber.setVisibility(View.VISIBLE);
        } else {
            holder.mPhoneNumber.setVisibility(View.GONE);
        }
        holder.mShortPhoneNumber.setText(shortNumber);
        holder.mSimSignal.setVisibility(View.INVISIBLE);
        holder.mSimStatus.setImageResource(getSimStatusIcon(simInfo.mSimSlotId));
        holder.mSuggested.setVisibility(View.GONE);
        if (PhoneConstants.GEMINI_SIM_1 == simInfo.mSimSlotId) {
            holder.mSimIcon.setBackgroundResource(
                    SimInfoManager.SimBackgroundDarkRes[SIM_BACKGROUND_COLOR_INDEX_ORANGE]);
        } else if (PhoneConstants.GEMINI_SIM_2 == simInfo.mSimSlotId) {
            holder.mSimIcon.setBackgroundResource(
                    SimInfoManager.SimBackgroundDarkRes[SIM_BACKGROUND_COLOR_INDEX_BLUE]);
        } else {
            log("slot is neither GEMINI_SIM_1 or GEMINI_SIM_2");
        }
    }

    protected ViewHolder createViewHolder(View view) {
        log("createView(), holder = " + view);
        ViewHolder holder =  new ViewHolder();
        holder.mSimSignal = (TextView)view.findViewById(R.id.simSignal);
        holder.mSimStatus = (ImageView)view.findViewById(R.id.simStatus);
        holder.mShortPhoneNumber = (TextView)view.findViewById(R.id.shortPhoneNumber);
        holder.mDisplayName = (TextView)view.findViewById(R.id.displayName);
        holder.mPhoneNumber = (TextView)view.findViewById(R.id.phoneNumber);
        holder.mSimIcon = view.findViewById(R.id.simIcon);
        holder.mSuggested = (ImageView)view.findViewById(R.id.suggested);

        return holder;
    }

    protected int getSimStatusIcon(int slot) {
        ITelephonyEx iTel = ITelephonyEx.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICEEX));
        int state = PhoneConstants.SIM_INDICATOR_UNKNOWN;
        if (iTel == null) {
            return 0;
        }
        try {
            state = iTel.getSimIndicatorState(slot); 		 
        } catch (RemoteException ex) {
            log("getSimIndicatorState fail!!");
        }

        int resourceId = 0;
        switch (state) {
            case com.android.internal.telephony.PhoneConstants.SIM_INDICATOR_LOCKED:
                resourceId = com.mediatek.internal.R.drawable.sim_locked;
                break;
            case com.android.internal.telephony.PhoneConstants.SIM_INDICATOR_RADIOOFF:
                resourceId = com.mediatek.internal.R.drawable.sim_radio_off;
                break;
            case com.android.internal.telephony.PhoneConstants.SIM_INDICATOR_ROAMING:
                resourceId = com.mediatek.internal.R.drawable.sim_roaming;
                break;
            case com.android.internal.telephony.PhoneConstants.SIM_INDICATOR_SEARCHING:
                resourceId = com.mediatek.internal.R.drawable.sim_searching;
                break;
            case com.android.internal.telephony.PhoneConstants.SIM_INDICATOR_INVALID:
                resourceId = com.mediatek.internal.R.drawable.sim_invalid;
                break;
            case com.android.internal.telephony.PhoneConstants.SIM_INDICATOR_CONNECTED:
                resourceId = com.mediatek.internal.R.drawable.sim_connected;
                break;
            case com.android.internal.telephony.PhoneConstants.SIM_INDICATOR_ROAMINGCONNECTED:
                resourceId = com.mediatek.internal.R.drawable.sim_roaming_connected;
                break;
            default:
                break;
        }
        return resourceId;
    }

    @Override
    public void onStop() {
        super.onStop();
        log("onStop");
        this.finish();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        log("onDestroy");
    }

    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.sim_item1: {
                log("sim_item1");
                startEsurfingGuideBySlotId(PhoneConstants.GEMINI_SIM_1);
                return;
            }
            case R.id.sim_item2: {
                log("sim_item2");
                startEsurfingGuideBySlotId(PhoneConstants.GEMINI_SIM_2);
                return;
            }
            case R.id.button: {
                log("cancel button");
                this.finish();
                return;
            }
            default: {
                return;
            }
        }
    }

    private void startEsurfingGuideBySlotId(int slotId) {
        log("startEsurfingGuideBySlotId " + slotId);
        Intent intent = new Intent(Intent.ACTION_MAIN, null);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                    | Intent.FLAG_ACTIVITY_NO_ANIMATION);
        intent.setClassName(PLUGIN_PACKAGE, DIALING_GUIDE_CLASS);
        intent.putExtra(Constants.EXTRA_SLOT_ID, slotId);
        this.startActivity(intent);
    }

    private static void log(String msg) {
        Log.d(TAG, msg);
    }
 }

