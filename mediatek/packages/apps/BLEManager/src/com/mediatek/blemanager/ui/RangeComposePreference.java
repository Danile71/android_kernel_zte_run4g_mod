package com.mediatek.blemanager.ui;

import android.content.Context;
import android.preference.Preference;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.widget.RadioButton;
import android.widget.SeekBar;

import com.mediatek.blemanager.R;
import com.mediatek.blemanager.common.CachedBluetoothLEDevice;
import com.mediatek.blemanager.provider.BLEConstants;

public class RangeComposePreference extends Preference {

    private static final String TAG = BLEConstants.COMMON_TAG + "[RangeComposePreference]";

    private static final int OUT_OF_RANGE_CLICK_FLAG = 1;
    private static final int IN_RANGE_CLICK_FLAG = 2;
    
    private static final int RANGE_NEAR_FLAG = 1;
    private static final int RANGE_MIDDLE_FLAG = 2;
    private static final int RANGE_FAR_FLAG = 3;
    
    private static final int RANGE_NEAR_DISTANCE = 0;
    private static final int RANGE_MIDDLE_DISTANCE = 20;
    private static final int RANGE_FAR_DISTANCE = 40;
    
    private SeekbarWithText mRangeDistanceSeekBar;
    private View mOutOfRangeAlertLayout;
    private View mInRangeAlertLayout;
    private RadioButton mOutOfRangeRadio;
    private RadioButton mInRangeRadio;
    
    private ComposeRangeView mComposeRangeView;
    
    private int mSeekBarStopProgress;
    private int mSeekBarStartProgress;
    private boolean mOutOfRangeChecked;
    private boolean mEnabled;
    private ComposePreferenceChangedListener mChangeListener;
    
    public RangeComposePreference(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public RangeComposePreference(Context context, AttributeSet attrs,
            int defStyle) {
        super(context, attrs, defStyle);
        this.setLayoutResource(R.layout.range_distance_preference_layout);
    }

    public RangeComposePreference(Context context) {
        this(context, null);
    }

    @Override
    protected void onBindView(View view) {
        super.onBindView(view);
        mRangeDistanceSeekBar = (SeekbarWithText)view.findViewById(R.id.range_distance_seek_bar);
        mOutOfRangeAlertLayout = view.findViewById(R.id.out_range_alert_linear);
        mInRangeAlertLayout = view.findViewById(R.id.in_range_alert_linear);
        mOutOfRangeRadio = (RadioButton)view.findViewById(R.id.out_range_alert_radio);
        mInRangeRadio = (RadioButton)view.findViewById(R.id.in_range_alert_radio);
        mComposeRangeView = (ComposeRangeView)view.findViewById(R.id.compose_range_view);
        
        initView();
    }
    
    private void initView() {
        mOutOfRangeAlertLayout.setOnClickListener(
                new RadioLinearClickListener(OUT_OF_RANGE_CLICK_FLAG));
        mOutOfRangeRadio.setOnClickListener(new RadioLinearClickListener(OUT_OF_RANGE_CLICK_FLAG));
        mInRangeAlertLayout.setOnClickListener(new RadioLinearClickListener(IN_RANGE_CLICK_FLAG));
        mInRangeRadio.setOnClickListener(new RadioLinearClickListener(IN_RANGE_CLICK_FLAG));

        mRangeDistanceSeekBar.setOnSeekBarChangedListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                int dis = seekBar.getProgress();
                if (dis >= 0 && dis < 10) {
                    mSeekBarStopProgress = CachedBluetoothLEDevice.PXP_RANGE_NEAR_VALUE;
                } else if (dis >= 10 && dis < 30) {
                    mSeekBarStopProgress = CachedBluetoothLEDevice.PXP_RANGE_MIDDLE_VALUE;
                } else if (dis >= 30 && dis <= 40) {
                    mSeekBarStopProgress = CachedBluetoothLEDevice.PXP_RANGE_FAR_VALUE;
                }
                mChangeListener.onSeekBarProgressChanged(
                        mSeekBarStartProgress, mSeekBarStopProgress);
                updateViewState();
            }
            
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                mSeekBarStartProgress = seekBar.getProgress();
            }
            
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress,
                    boolean fromUser) {
                
            }
        });
        updateViewState();
    }
    
    private void updateViewState() {
        if (mOutOfRangeRadio != null && mInRangeRadio != null) {
            if (mOutOfRangeChecked) {
                mOutOfRangeRadio.setChecked(true);
                mInRangeRadio.setChecked(false);
            } else {
                mOutOfRangeRadio.setChecked(false);
                mInRangeRadio.setChecked(true);
            }
        }
        if (mRangeDistanceSeekBar != null) {
            mRangeDistanceSeekBar.setProgress(mSeekBarStopProgress);
        }
        if (mComposeRangeView != null) {
            mComposeRangeView.setState(mEnabled, mSeekBarStopProgress, mOutOfRangeChecked);
        }
    }
    
    private class RadioLinearClickListener implements View.OnClickListener {
        
        private int mWhich;
        public RadioLinearClickListener(int which) {
            mWhich = which;
        }

        @Override
        public void onClick(View v) {
            switch (mWhich) {
            case OUT_OF_RANGE_CLICK_FLAG:
                if (!mOutOfRangeChecked) {
                    mOutOfRangeChecked = true;
                }
                break;
                
            case IN_RANGE_CLICK_FLAG:
                if (mOutOfRangeChecked) {
                    mOutOfRangeChecked = false;
                }
                break;
            default:
                break;
            }
            updateViewState();
            mChangeListener.onRangeChanged(mOutOfRangeChecked);
        }
    }

    public void setState(boolean enabled, int rangeValue, int inOutAlert) {
        mSeekBarStopProgress = rangeValue;
        if (inOutAlert == 1) {
            mOutOfRangeChecked = true;
        } else {
            mOutOfRangeChecked = false;
        }
        mEnabled = enabled;
        updateViewState();
    }
    
    public void setChangeListener(ComposePreferenceChangedListener listener) {
        if (listener == null) {
            Log.d(TAG, "[setChangeListener] listener is null!!");
            return;
        }
        mChangeListener = listener;
    }
    
    public interface ComposePreferenceChangedListener {
        void onSeekBarProgressChanged(int startPorgress, int stopProgress);
        void onRangeChanged(boolean outRangeChecked);
    }

    public void clear() {
        if (mComposeRangeView != null) {
            mComposeRangeView.clearBitmap();
        }
    }
}

