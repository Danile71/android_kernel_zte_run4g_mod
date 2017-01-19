
package com.mediatek.settings.plugin;

import android.content.Context;
import android.preference.Preference;
import android.util.AttributeSet;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.LinearLayout;

import com.mediatek.op09.plugin.R;
import com.mediatek.xlog.Xlog;

/**
 * Used in ManualNetworkSelection to do CDMA network selection.
 */
public class CdmaNetworkSwitcherPreference extends Preference implements OnClickListener {
    private static final String TAG = "CdmaNetworkSwitcherPreference";

    private Context mContext;

    private View mSearchingContainer;
    private View mCdmaSwitchContainer;

    private Button mSwitchNetworKButton;
    private boolean mPendingSwitchNetworkBtnEnable;
    private boolean mPendingSearchContainerVisibility;

    private OnSwitchNetworkListener mOnSwitchNetworkListener;

    public CdmaNetworkSwitcherPreference(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        mContext = context;
        initPreference();
    }

    public CdmaNetworkSwitcherPreference(Context context, AttributeSet attrs) {
        super(context, attrs);
        mContext = context;
        initPreference();
    }

    private void initPreference() {
        setLayoutResource(R.layout.cdma_network_switcher);
    }

    @Override
    protected void onBindView(View view) {
        super.onBindView(view);
        mSearchingContainer = (LinearLayout) view.findViewById(R.id.cdma_searching);
        mCdmaSwitchContainer = (LinearLayout) view.findViewById(R.id.cdma_switch_container);
        mSwitchNetworKButton = (Button) view.findViewById(R.id.cdma_switch_button);
        mSwitchNetworKButton.setOnClickListener(this);
        mSwitchNetworKButton.setEnabled(mPendingSwitchNetworkBtnEnable);
        mSearchingContainer.setVisibility(mPendingSearchContainerVisibility ? View.VISIBLE
                : View.GONE);
        mCdmaSwitchContainer.setVisibility(mPendingSearchContainerVisibility ? View.GONE
                : View.VISIBLE);
        Xlog.d(TAG, "onBindView: mSearchingContainer = " + mSearchingContainer
                + ", mCdmaSwitchContainer = " + mCdmaSwitchContainer);
    }

    public void updateSwitcherState(boolean enable) {
        Xlog.d(TAG, "updateSwitcherState: enable = " + enable + ", mSwitchNetworKButton = "
                + mSwitchNetworKButton);
        // TODO: check why only update variable when button is null.
        if (mSwitchNetworKButton != null) {
            mSwitchNetworKButton.setEnabled(enable);
        }
        mPendingSwitchNetworkBtnEnable = enable;
    }

    public void updateNetworkSearchingState(boolean searching) {
        Xlog.d(TAG, "updateNetworkSearchingState: searching = " + searching
                + ", mSearchingContainer = " + mSearchingContainer + ", mCdmaSwitchContainer = "
                + mCdmaSwitchContainer, new Throwable("updateNetworkSearchingState"));
        // TODO: check why only update variable when button is null.
        if (mSearchingContainer != null && mCdmaSwitchContainer != null) {
            mSearchingContainer.setVisibility(searching ? View.VISIBLE : View.GONE);
            mCdmaSwitchContainer.setVisibility(searching ? View.GONE : View.VISIBLE);
            Xlog.d(TAG, "updateNetworkSearchingState: searching = " + searching
                    + ", mSearchingContainer vis = " + mSearchingContainer.getVisibility()
                    + ", mCdmaSwitchContainer vis = " + mCdmaSwitchContainer.getVisibility());
        }
        mPendingSearchContainerVisibility = searching;
    }

    @Override
    public void onClick(View v) {
        Xlog.d(TAG, "onClick: v = " + v);
        if (v.getId() == mSwitchNetworKButton.getId()) {
            if (mOnSwitchNetworkListener != null) {
                mOnSwitchNetworkListener.onSwitchNetwork();
            }
        }
    }

    public void setOnPerformSwitchNetowrkListener(OnSwitchNetworkListener onSwitchNetworkListener) {
        mOnSwitchNetworkListener = onSwitchNetworkListener;
    }

    public interface OnSwitchNetworkListener {
        void onSwitchNetwork();
    }
}
