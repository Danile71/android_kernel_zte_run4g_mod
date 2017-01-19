package com.mediatek.blemanager.ui;

import android.content.Context;
import android.graphics.Color;
import android.util.AttributeSet;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.TextView;

import com.mediatek.blemanager.R;
import com.mediatek.blemanager.common.CachedBluetoothLEDevice;
import com.mediatek.blemanager.provider.BLEConstants;

public class SeekbarWithText extends RelativeLayout {

    private static final String TAG = BLEConstants.COMMON_TAG + "[SeekbarWithText]";
    
    private static final int CUSTOM_COLOR = Color.rgb(0, 153, 204);
    
    private SeekBar mDistanceSeek;
    private ImageView mNearDotImage;
    private ImageView mMiddleDotImage;
    private ImageView mFarDotImage;
    
    private TextView mNearTopicText;
    private TextView mMiddleTopicText;
    private TextView mFarTopicText;
    
    private TextView mNearSumText;
    private TextView mMiddleSumText;
    private TextView mFarSumText;
    
    public SeekbarWithText(Context context, AttributeSet attrs) {
        super(context, attrs);
        LayoutInflater.from(context)
                .inflate(R.layout.distance_seekbar_with_text, this, true);
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
        this.mDistanceSeek = (SeekBar)this.findViewById(R.id.distance_seek_bar);
        this.mNearDotImage = (ImageView)this.findViewById(R.id.near_dot_image);
        this.mMiddleDotImage = (ImageView)this.findViewById(R.id.middle_dot_image);
        this.mFarDotImage = (ImageView)this.findViewById(R.id.far_dot_image);
        
        this.mNearTopicText = (TextView)this.findViewById(R.id.distance_topic_near_text);
        this.mMiddleTopicText = (TextView)this.findViewById(R.id.distance_topic_middle_text);
        this.mFarTopicText = (TextView)this.findViewById(R.id.distance_topic_far_text);
        
        this.mNearSumText = (TextView)this.findViewById(R.id.distance_sum_near_text);
        this.mMiddleSumText = (TextView)this.findViewById(R.id.distance_sum_middle_text);
        this.mFarSumText = (TextView)this.findViewById(R.id.distance_sum_far_text);
    }

    private void updateDotImages(int pro) {
        if (pro == CachedBluetoothLEDevice.PXP_RANGE_NEAR_VALUE) {
            mNearDotImage.setVisibility(View.INVISIBLE);
            mMiddleDotImage.setVisibility(View.VISIBLE);
            mFarDotImage.setVisibility(View.VISIBLE);
            
            mNearTopicText.setTextColor(CUSTOM_COLOR);
            mMiddleTopicText.setTextColor(Color.BLACK);
            mFarTopicText.setTextColor(Color.BLACK);
            
            mNearSumText.setTextColor(CUSTOM_COLOR);
            mMiddleSumText.setTextColor(Color.BLACK);
            mFarSumText.setTextColor(Color.BLACK);
        } else if (pro == CachedBluetoothLEDevice.PXP_RANGE_MIDDLE_VALUE) {
            mNearDotImage.setVisibility(View.VISIBLE);
            mMiddleDotImage.setVisibility(View.INVISIBLE);
            mFarDotImage.setVisibility(View.VISIBLE);
            
            mNearTopicText.setTextColor(Color.BLACK);
            mMiddleTopicText.setTextColor(CUSTOM_COLOR);
            mFarTopicText.setTextColor(Color.BLACK);
            
            mNearSumText.setTextColor(Color.BLACK);
            mMiddleSumText.setTextColor(CUSTOM_COLOR);
            mFarSumText.setTextColor(Color.BLACK);
        } else if (pro == CachedBluetoothLEDevice.PXP_RANGE_FAR_VALUE) {
            mNearDotImage.setVisibility(View.VISIBLE);
            mMiddleDotImage.setVisibility(View.VISIBLE);
            mFarDotImage.setVisibility(View.INVISIBLE);
            
            mNearTopicText.setTextColor(Color.BLACK);
            mMiddleTopicText.setTextColor(Color.BLACK);
            mFarTopicText.setTextColor(CUSTOM_COLOR);
            
            mNearSumText.setTextColor(Color.BLACK);
            mMiddleSumText.setTextColor(Color.BLACK);
            mFarSumText.setTextColor(CUSTOM_COLOR);
        }
    }
    
    /**
     * 
     * @param progress 0(near),1(middle),2(far)
     */
    public void setProgress(int progress) {
        int pro;
        if (progress == CachedBluetoothLEDevice.PXP_RANGE_NEAR_VALUE) {
            pro = 0;
        } else if (progress == CachedBluetoothLEDevice.PXP_RANGE_FAR_VALUE) {
            pro = 40;
        } else {
            pro = 20;
        }
        mDistanceSeek.setProgress(pro);
        updateDotImages(progress);
    }
    
    public void setOnSeekBarChangedListener(SeekBar.OnSeekBarChangeListener listener) {
        if (listener == null) {
            Log.d(TAG, "[setOnSeekBarChangedListener] listener is null!!");
            return;
        }
        mDistanceSeek.setOnSeekBarChangeListener(listener);
    }
}
