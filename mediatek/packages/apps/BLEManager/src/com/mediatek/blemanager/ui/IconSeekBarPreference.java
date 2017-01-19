
package com.mediatek.blemanager.ui;

import android.content.Context;
import android.preference.Preference;
import android.util.AttributeSet;
import android.view.View;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;

import com.mediatek.blemanager.R;

public class IconSeekBarPreference extends Preference {

    private int mProgress;
    private int mStartProgress;
    private int mStopProgress;
    
    private SeekBar mSeekBar;
    private OnSeekBarProgressChangedListener mListener;
    
    public IconSeekBarPreference(Context context) {
        this(context, null);
    }

    public IconSeekBarPreference(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public IconSeekBarPreference(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        this.setLayoutResource(R.layout.custom_seek_bar_layout);
    }

    @Override
    protected void onBindView(View view) {
        // TODO Auto-generated method stub
        super.onBindView(view);

        mSeekBar = (SeekBar)view.findViewById(R.id.volume_seekbar);
        mSeekBar.setMax(30);

        mSeekBar.setProgress(mProgress);
        mSeekBar.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress,
                    boolean fromUser) {
                mProgress = progress;
                mSeekBar.setProgress(progress);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                mStartProgress = seekBar.getProgress();
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                mStopProgress = seekBar.getProgress();
                mListener.onProgressChanged(mStopProgress);
            }
            
        });
    }
    
    public void setProgress(int progress) {
        mProgress = progress;
    }
    
    public void setOnProgressChanged(OnSeekBarProgressChangedListener listener) {
        if (listener == null) {
            return;
        }
        mListener = listener;
    }
    
    public interface OnSeekBarProgressChangedListener {
        void onProgressChanged(int progress);
    }
}
