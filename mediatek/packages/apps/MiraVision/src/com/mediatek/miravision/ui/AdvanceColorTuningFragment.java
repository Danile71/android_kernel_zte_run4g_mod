package com.mediatek.miravision.ui;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.widget.SeekBar;

import com.mediatek.miravision.setting.MiraVisionJni;
import com.mediatek.miravision.setting.MiraVisionJni.Range;

public class AdvanceColorTuningFragment extends BaseTuningFragment {

    private static final String TAG = "Miravision/AdvanceColorTuningFragment";
    private final int[] mTuningItems = { R.string.advance_color_item_sharpness,
            R.string.advance_color_item_temperature };

    public AdvanceColorTuningFragment() {
    }

    public AdvanceColorTuningFragment(int title) {
        mTuningTitle = title;
        for (int i = 0; i < mTuningItems.length; i++) {
            if (mTuningTitle == mTuningItems[i]) {
                index = i;
                break;
            }
        }
        Log.d(TAG, "mTuningTitle = " + mTuningTitle + ", index = " + index);
    }

    private Range mGammaRange;
    private Range mSharpnessRange;
    private Bitmap mBitmap;

    private boolean mHasMeasured;
    private int mSharpnessImageHeight = -1;
    private int mSharpnessImageWidth = -1;
    private int mImageViewHeight = -1;
    private int mImageViewWidth = -1;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View rootView = super.onCreateView(inflater, container, savedInstanceState);

        mImageBasicAfter.setVisibility(View.GONE);
        mLineView.setVisibility(View.GONE);

        mGammaRange = MiraVisionJni.getGammaIndexRange();
        mSharpnessRange = MiraVisionJni.getSharpnessIndexRange();
        mHasMeasured = false;

        updateUi();
        return rootView;
    }

    @Override
    public void onResume() {
        super.onResume();
        MiraVisionJni.nativeSetTuningMode(1);
    }

    @Override
    public void onPause() {
        super.onPause();
        MiraVisionJni.nativeSetTuningMode(0);
    }

    @Override
    public void onClick(View v) {
        v.setEnabled(false);
        super.onClick(v);
        updateUi();
        v.setEnabled(true);
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        if (fromUser) {
            onSeekBarChange(seekBar.getProgress());
        }
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
    }

    private void onSeekBarChange(int value) {
        Log.d(TAG, "onChange value = " + value);
        switch (mTuningTitle) {
        case R.string.advance_color_item_sharpness:
            MiraVisionJni.setSharpnessIndex(value - mSharpnessRange.min);
            Log.d(TAG, "MiraVisionJni.setSharpnessIndex(): " + (value - mSharpnessRange.min));
            sharpnessDecode();
            mImageAdvAfter.setImageBitmap(mBitmap);
            mImageAdvAfter.invalidate();
            break;
        case R.string.advance_color_item_temperature:
            MiraVisionJni.setGammaIndex(value - mGammaRange.min);
            break;
        default:
            break;
        }
    }

    private void updateUi() {
        Log.d(TAG, "updateUi");
        mTuningTitle = mTuningItems[index];
        mSeekBarText.setText(mTuningTitle);
        switch (mTuningTitle) {
        case R.string.advance_color_item_sharpness:
            mLineView.setVisibility(View.VISIBLE);
            mImageAdvAfter.setVisibility(View.VISIBLE);
            mLastButton.setVisibility(View.INVISIBLE);
            mNextButton.setVisibility(View.VISIBLE);
            mSeekBar.setMax(mSharpnessRange.max - mSharpnessRange.min);
            mSeekBar.setProgress(MiraVisionJni.getSharpnessIndex() - mSharpnessRange.min);

            // If not initial image resource, get the ImageView width and height firstly.
            if (mImageViewWidth == -1 || mImageViewHeight == -1) {
                ViewTreeObserver treeObserver = mImageAdvAfter.getViewTreeObserver();
                treeObserver.addOnPreDrawListener(new ViewTreeObserver.OnPreDrawListener() {
                    @Override
                    public boolean onPreDraw() {
                        if (!mHasMeasured) {
                            mImageViewHeight = mImageAdvAfter.getWidth();
                            mImageViewWidth = mImageAdvAfter.getHeight();
                            initSharpnessResource();
                            mHasMeasured = true;
                        }
                        return true;
                    }
                });
            } else {
                initSharpnessResource();
            }
            break;
        case R.string.advance_color_item_temperature:
            mLineView.setVisibility(View.GONE);
            mAfterText.setVisibility(View.GONE);
            mOrignalText.setVisibility(View.GONE);
            mImageAdvAfter.setVisibility(View.GONE);
            mLastButton.setVisibility(View.VISIBLE);
            mNextButton.setVisibility(View.INVISIBLE);
            mGammaRange = MiraVisionJni.getGammaIndexRange();
            mSeekBar.setMax(mGammaRange.max - mGammaRange.min);
            mSeekBar.setProgress(MiraVisionJni.getGammaIndex() - mGammaRange.min);
            mImageOrignal.setImageResource(R.drawable.advanced_tuning_temperature);
            break;
        default:
            break;
        }
    }

    private void initSharpnessResource() {
        Log.d(TAG, "initSharpnessResource");
        // Disable sharpness and initial the "before" image.
        int index = MiraVisionJni.getSharpnessIndex();
        MiraVisionJni.setSharpnessIndex(0);
        sharpnessDecode();
        mImageOrignal.setImageBitmap(mBitmap);

        // Reset the sharpness index and initial the "after" image.
        MiraVisionJni.setSharpnessIndex(index);
        sharpnessDecode();
        mImageAdvAfter.setImageBitmap(mBitmap);
    }

    private void sharpnessDecode() {
        Log.d(TAG, "Start decode image for sharpness");
        int resId = R.drawable.advanced_tuning_sharpness;
        BitmapFactory.Options options = new BitmapFactory.Options();
        if (mSharpnessImageHeight == -1 && mSharpnessImageWidth == -1) {
            options.inJustDecodeBounds = true;
            BitmapFactory.decodeResource(getResources(), resId, options);
            mSharpnessImageHeight = options.outHeight;
            mSharpnessImageWidth = options.outWidth;
        }

        int yRatio = (int) Math.ceil(mSharpnessImageHeight/mImageViewHeight);
        int xRatio = (int) Math.ceil(mSharpnessImageWidth/mImageViewWidth);
        Log.d(TAG, "yRatio: " + yRatio + ", xRatio: " + xRatio);
        if (yRatio > 1 || xRatio > 1) {
            if (yRatio > xRatio) {
                options.inSampleSize = yRatio;
            } else {
                options.inSampleSize = xRatio;
            }
        }

        // Set this flag for sharpness decoding.
        options.inPostProc = true;
        options.inJustDecodeBounds = false;
        mBitmap = BitmapFactory.decodeResource(getResources(), resId, options);
        Log.d(TAG, "Completed decoding image for sharpness!");
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        MiraVisionJni.nativeSetTuningMode(0);
        if (mBitmap != null) {
            mBitmap.recycle();
            mBitmap = null;
        }
    }
}
