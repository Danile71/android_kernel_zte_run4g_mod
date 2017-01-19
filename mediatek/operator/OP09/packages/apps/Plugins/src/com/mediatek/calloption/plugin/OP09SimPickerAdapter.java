package com.mediatek.calloption.plugin;

import android.content.Context;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.provider.Settings;
import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.RadioButton;
import android.widget.TextView;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.op09.plugin.R;
import com.mediatek.calloption.SimPickerAdapter;
import com.mediatek.calloption.SimPickerAdapter.ViewHolder;
import com.mediatek.telephony.SimInfoManager;

public class OP09SimPickerAdapter extends SimPickerAdapter {

    private static final String TAG = "OP09SimPickerAdapter";

    public static class NoSimItemInfo {
        public int mPhoneType;
        public int mSlot;

        public NoSimItemInfo(int phoneType, int slot) {
            mPhoneType = phoneType;
            mSlot = slot;
        }
    }

    public static final int ITEM_TYPE_NO_SIM = 4;
    private static final int VIEW_TYPE_COUNT = 5;

    private static final int SIM_BACKGROUND_COLOR_INDEX_ORANGE = 0;
    private static final int SIM_BACKGROUND_COLOR_INDEX_BLUE = 1;

    private Context mPluginContext;

    public OP09SimPickerAdapter(Context pluginContext, Context hostContext, long suggestedSimId, boolean isMultiSim) {
        super(hostContext, suggestedSimId, isMultiSim);
        mPluginContext = pluginContext;
    }

    @Override
    public int getViewTypeCount() {
        return VIEW_TYPE_COUNT;
    }

    public Object getItem(int position) {
        log("getItem(), position = " + position);
        ItemHolder itemHolder = mItems.get(position);
        if (itemHolder.mType == ITEM_TYPE_NO_SIM) {
            return Integer.valueOf(((NoSimItemInfo)itemHolder.mData).mSlot);
        } else {
            return super.getItem(position);
        }
    }

    public View getView(int position, View convertView, ViewGroup parent) {
        log("getView(), position = " + position + ", convertView = " + convertView + ", parent = " + parent);
        View view = super.getView(position, convertView, parent);

        ViewHolder holder = (ViewHolder)view.getTag();
        if (ITEM_TYPE_NO_SIM == getItemViewType(position)) {
            NoSimItemInfo noSimItemInfo = (NoSimItemInfo)mItems.get(position).mData;
            if (PhoneConstants.PHONE_TYPE_GSM == noSimItemInfo.mPhoneType) { // GSM network
                holder.mDisplayName.setText(mPluginContext.getResources().getString(R.string.gsm_network));
                holder.mPhoneNumber.setText(mPluginContext.getResources().getString(R.string.no_sim_card));
            } else if (PhoneConstants.PHONE_TYPE_CDMA == noSimItemInfo.mPhoneType) { // CDMA network
                holder.mDisplayName.setText(mPluginContext.getResources().getString(R.string.cdma_network));
                holder.mPhoneNumber.setText(mPluginContext.getResources().getString(R.string.no_sim_card_cdma));
            } else {
                log("phone is neither GSM nor CDMA for no insert sim card slot");
            }
            if (noSimItemInfo.mSlot >= 0) {
                if (PhoneConstants.GEMINI_SIM_1 == noSimItemInfo.mSlot) {
                    holder.mSimIcon.setBackgroundResource(
                            SimInfoManager.SimBackgroundDarkRes[SIM_BACKGROUND_COLOR_INDEX_BLUE]);
                } else if (PhoneConstants.GEMINI_SIM_2 == noSimItemInfo.mSlot) {
                    holder.mSimIcon.setBackgroundResource(
                            SimInfoManager.SimBackgroundDarkRes[SIM_BACKGROUND_COLOR_INDEX_ORANGE]);
                } else {
                    log("slot is neither GEMINI_SIM_1 or GEMINI_SIM_2");
                }
            }
            holder.mSuggested.setVisibility(View.GONE);
        }

        return view;
    }

    protected View createView(ViewHolder holder, final int viewType) {
        log("createView(), holder = " + holder + ", viewType = " + viewType);
        LayoutInflater inflater = LayoutInflater.from(mPluginContext);
        View view = null;
        if (SimPickerAdapter.ITEM_TYPE_SIM == viewType || ITEM_TYPE_NO_SIM == viewType) {
            view = inflater.inflate(R.layout.sim_picker_item, null);
            holder.mSimSignal = (TextView)view.findViewById(R.id.simSignal);
            holder.mSimStatus = (ImageView)view.findViewById(R.id.simStatus);
            holder.mShortPhoneNumber = (TextView)view.findViewById(R.id.shortPhoneNumber);
            holder.mDisplayName = (TextView)view.findViewById(R.id.displayName);
            holder.mPhoneNumber = (TextView)view.findViewById(R.id.phoneNumber);
            holder.mSimIcon = view.findViewById(R.id.simIcon);
            holder.mSuggested = (ImageView)view.findViewById(R.id.suggested);
        }
        return view;
    }

    private static void log(String msg) {
        Log.d(TAG, msg);
    }
}
