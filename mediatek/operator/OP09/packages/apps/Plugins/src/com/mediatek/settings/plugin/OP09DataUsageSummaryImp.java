package com.mediatek.settings.plugin;

import android.app.Activity;
import android.content.Context;
import android.database.ContentObserver;
import android.provider.Settings;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TabHost.TabSpec;
import android.widget.TabWidget;
import android.widget.TextView;

import com.android.internal.telephony.PhoneConstants;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.op09.plugin.R;
import com.mediatek.settings.ext.DefaultDataUsageSummaryExt;

public class OP09DataUsageSummaryImp extends DefaultDataUsageSummaryExt {

    private static final String TAG = "DataUsageSummaryImp";
    private static final String TAB_SIM_1 = "sim1";
    private static final String TAB_SIM_2 = "sim2";
    private static final String TAB_SIM_3 = "sim3";
    private static final String TAB_SIM_1_INDEX = "1";
    private static final String TAB_SIM_2_INDEX = "2";

    private Context mContext;

    public OP09DataUsageSummaryImp(Context context) {
        super(context);
        mContext = context;
    }

    /**
     * Customize the summary of mobile data. Used in OverViewTabAdapter.java
     */
    @Override
    public void customizeMobileDataSummary(View container, View titleView,
            int slotId) {
        if (container instanceof RelativeLayout) {
            RelativeLayout viewContainer = (RelativeLayout) container;
            ImageView simIndicator = new ImageView(mContext);
            simIndicator.setId(R.id.data_usage_mobile_item_sim_indicator_view);
            RelativeLayout.LayoutParams simIndicatorLayoutParams = new RelativeLayout.LayoutParams(
                    RelativeLayout.LayoutParams.WRAP_CONTENT,
                    RelativeLayout.LayoutParams.WRAP_CONTENT);
            simIndicatorLayoutParams.addRule(RelativeLayout.CENTER_VERTICAL);
            simIndicatorLayoutParams.addRule(RelativeLayout.ALIGN_PARENT_START);
            int marginRight = (int) mContext.getResources().getDimension(
                    R.dimen.data_usage_summary_sim_indicator_margin_right);
            simIndicatorLayoutParams.rightMargin = marginRight;
            simIndicator.setLayoutParams(simIndicatorLayoutParams);
            int paddingLeft = (int) mContext.getResources().getDimension(
                    R.dimen.data_usage_summary_sim_indicator_padding_left);
            simIndicator.setPadding(paddingLeft, 0, 0, 0);
            simIndicator
                    .setImageResource(slotId == PhoneConstants.GEMINI_SIM_1 ? R.drawable.ic_datausage_sim_1
                            : R.drawable.ic_datausage_sim_2);
            viewContainer.addView(simIndicator, 0);
            RelativeLayout.LayoutParams titleLayoutParams = (RelativeLayout.LayoutParams) titleView
                    .getLayoutParams();
            titleLayoutParams.removeRule(RelativeLayout.ALIGN_PARENT_START);
            titleLayoutParams.addRule(RelativeLayout.END_OF,
                    R.id.data_usage_mobile_item_sim_indicator_view);
        }
    }

    /**
     * Customize the tabspec. Change title, backgournd resource etc. It will be
     * called when rebuild all tabs. Add String 1 or 2 in TAB_SIM1 or TAB_SIM2
     */
    @Override
    public TabSpec customizeTabInfo(Activity activity, String tag, TabSpec tab,
            TabWidget tabWidget, String title) {
        if (tag.equals(TAB_SIM_1) || tag.equals(TAB_SIM_2)) {
            LayoutInflater inflater = (LayoutInflater) mContext
                    .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            RelativeLayout layout = (RelativeLayout) inflater
                    .inflate(R.layout.data_usage_summary_tab_indicator,
                            tabWidget, false);
            TextView simTitle = (TextView) layout
                    .findViewById(android.R.id.title);
            simTitle.setText(title);
            TextView numberIndicator = (TextView) layout
                    .findViewById(R.id.simnumber);
            String indicatorText = null;
            if (tag.equals(TAB_SIM_1)) {
                indicatorText = TAB_SIM_1_INDEX;
            } else if (tag.equals(TAB_SIM_2)) {
                indicatorText = TAB_SIM_2_INDEX;
            }
            numberIndicator.setText(indicatorText);
            tab.setIndicator(layout);
        }
        return tab;
    }

    @Override
    public void customizeDataConnectionObserver(Activity activity,
            ContentObserver mDataConnectionObserver) {
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            activity.getContentResolver().registerContentObserver(
                    Settings.Secure.getUriFor(Settings.Global.MOBILE_DATA),false, mDataConnectionObserver);
        }
    }

    @Override
    public void customizeUnregisterDataConnectionObserver(Activity activity,
            ContentObserver mDataConnectionObserver) {
        activity.getContentResolver().unregisterContentObserver(mDataConnectionObserver);
    }

}
