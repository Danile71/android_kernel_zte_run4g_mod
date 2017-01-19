package com.mediatek.blemanager.ui;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ImageView;
import android.widget.RelativeLayout;

import com.mediatek.blemanager.R;
import com.mediatek.blemanager.common.CachedBluetoothLEDevice;

import java.io.InputStream;

public class ComposeRangeView extends RelativeLayout {

    private Context mContext;
    private ImageView mRangeBackgroundImage;
    private ImageView mDistanceFarImage;
    private ImageView mDistanceMiddleImage;
    private ImageView mDistanceNearImage;
    private ImageView mRangeArrowImage;

    private Bitmap mBackgroundBitmap;
    private Bitmap mArrowBitmap;
    private Bitmap mDistanceBitmap;

    private int mRange;
    private boolean mOutRangeChecked;
    private boolean mEnabled;
    
    public ComposeRangeView(Context context, AttributeSet attrs) {
        super(context, attrs);
        mContext = context;
        LayoutInflater.from(context).inflate(R.layout.compose_distance_image_layout, this, true);
    }

    public ComposeRangeView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        mContext = context;
    }

    public ComposeRangeView(Context context) {
        super(context);
        mContext = context;
    }

    @Override
    protected void onFinishInflate() {
        // TODO Auto-generated method stub
        super.onFinishInflate();
        mRangeBackgroundImage = (ImageView)this.findViewById(R.id.range_bg_image);
        mDistanceFarImage = (ImageView)this.findViewById(R.id.distance_far_image);
        mDistanceMiddleImage = (ImageView)this.findViewById(R.id.distance_middle_image);
        mDistanceNearImage = (ImageView)this.findViewById(R.id.distance_near_image);
        mRangeArrowImage = (ImageView)this.findViewById(R.id.range_indicator_image);
        updateViewState();
    }

    private void updateViewState() {
        if (mRangeBackgroundImage != null && mRangeArrowImage != null) {
            Bitmap backgroundBitmap;
            Bitmap arrowBitmap;
            if (mOutRangeChecked) {
                mRangeBackgroundImage.setImageResource(R.drawable.out_range_bg);
                backgroundBitmap = readBitMap(mContext, R.drawable.out_range_bg);
                arrowBitmap = readBitMap(mContext, R.drawable.ic_range_arrow_r);
            } else {
                backgroundBitmap = readBitMap(mContext, R.drawable.in_range_bg);
                arrowBitmap = readBitMap(mContext, R.drawable.ic_range_arrow_l);
            }
            // set image to view
            if (backgroundBitmap != null) {
                mRangeBackgroundImage.setImageBitmap(backgroundBitmap);
                if (mBackgroundBitmap != null) {
                    mBackgroundBitmap.recycle();
                }
                mBackgroundBitmap = backgroundBitmap;
            }
            if (arrowBitmap != null) {
                mRangeArrowImage.setImageBitmap(arrowBitmap);
                if (mArrowBitmap != null) {
                    mArrowBitmap.recycle();
                }
                mArrowBitmap = arrowBitmap;
            }
        }
        updateDistanceImage();
    }
    
    private void updateDistanceImage() {
        ImageView imageDis = null;
        Bitmap distanceBitmap = null;
        if (mDistanceFarImage != null && mDistanceMiddleImage != null
                && mDistanceNearImage != null) {
            if (mRange == CachedBluetoothLEDevice.PXP_RANGE_NEAR_VALUE) {
                mDistanceFarImage.setVisibility(View.GONE);
                mDistanceMiddleImage.setVisibility(View.GONE);
                mDistanceNearImage.setVisibility(View.VISIBLE);
                if (mOutRangeChecked) {
                    distanceBitmap = readBitMap(mContext, R.drawable.out_range_near);
                } else {
                    distanceBitmap = readBitMap(mContext, R.drawable.in_range_near);
                }
                imageDis = mDistanceNearImage;
            } else if (mRange == CachedBluetoothLEDevice.PXP_RANGE_MIDDLE_VALUE) {
                mDistanceFarImage.setVisibility(View.GONE);
                mDistanceMiddleImage.setVisibility(View.VISIBLE);
                mDistanceNearImage.setVisibility(View.GONE);
                if (mOutRangeChecked) {
                    distanceBitmap = readBitMap(mContext, R.drawable.out_range_middle);
                } else {
                    distanceBitmap = readBitMap(mContext, R.drawable.in_range_middle);
                }
                imageDis = mDistanceMiddleImage;
            } else if (mRange == CachedBluetoothLEDevice.PXP_RANGE_FAR_VALUE) {
                mDistanceFarImage.setVisibility(View.VISIBLE);
                mDistanceMiddleImage.setVisibility(View.GONE);
                mDistanceNearImage.setVisibility(View.GONE);
                if (mOutRangeChecked) {
                    distanceBitmap = readBitMap(mContext, R.drawable.out_range_far);
                } else {
                    distanceBitmap = readBitMap(mContext, R.drawable.in_range_far);
                }
                imageDis = mDistanceFarImage;
            }
            if (imageDis != null) {
                // set image to view
                if (distanceBitmap != null) {
                    imageDis.setImageBitmap(distanceBitmap);
                    if (mDistanceBitmap != null) {
                        mDistanceBitmap.recycle();
                    }
                    mDistanceBitmap = distanceBitmap;
                }
                if (mEnabled) {
                    imageDis.setImageAlpha(255);
                } else {
                    imageDis.setImageAlpha(125);
                }
            }
        }
        if (mRangeBackgroundImage != null && mRangeArrowImage != null) {
            if (mEnabled) {
                mRangeBackgroundImage.setImageAlpha(255);
                mRangeArrowImage.setImageAlpha(255);
            } else {
                mRangeBackgroundImage.setImageAlpha(125);
                mRangeArrowImage.setImageAlpha(125);
            }
        }
    }
    
    public void setState(boolean enabled, int range, boolean outRangeChecked) {
        mRange = range;
        mOutRangeChecked = outRangeChecked;
        mEnabled = enabled;
        updateViewState();
    }

    private Bitmap readBitMap(Context context, int resId) {
        BitmapFactory.Options opt = new BitmapFactory.Options();
        opt.inPurgeable = true;
        opt.inInputShareable = true;
        InputStream is = context.getResources().openRawResource(resId);
        return BitmapFactory.decodeStream(is, null, opt);
    }

    public void clearBitmap() {
        mRangeBackgroundImage.setVisibility(View.GONE);
        mDistanceFarImage.setVisibility(View.GONE);
        mDistanceMiddleImage.setVisibility(View.GONE);
        mDistanceNearImage.setVisibility(View.GONE);
        mRangeArrowImage.setVisibility(View.GONE);

        mBackgroundBitmap.recycle();
        mArrowBitmap.recycle();
        mDistanceBitmap.recycle();
    }

}
