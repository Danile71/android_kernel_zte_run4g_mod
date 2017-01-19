
package com.mediatek.blemanager.ui;

import android.content.Context;
import android.preference.Preference;
import android.util.AttributeSet;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.CheckBox;

import com.mediatek.blemanager.R;

public class NonChangeCheckBoxPreference extends Preference {

    private CheckBox mCheckBox;
    private boolean mChecked;
    private OnCheckStateChangeListener mCheckChangeListener;
    
    public interface OnCheckStateChangeListener {
        void onCheckdChangeListener(boolean checked);
    }
    
    public NonChangeCheckBoxPreference(Context context, AttributeSet attrs) {
        super(context, attrs);
        this.setWidgetLayoutResource(R.layout.right_checkbox);
    }

    @Override
    protected void onBindView(View view) {
        super.onBindView(view);
        mCheckBox = (CheckBox)view.findViewById(R.id.right_checkbox);
        mCheckBox.setChecked(mChecked);
        mCheckBox.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View arg0) {
                mCheckChangeListener.onCheckdChangeListener(mCheckBox.isChecked());
            }
        });
    }

    @Override
    protected void onClick() {
    }
    
    private void updateCheckState() {
        mCheckBox.setChecked(mChecked);
    }
    
    public void setCheckState(boolean checked) {
        mChecked = checked;
        updateCheckState();
    }
    
    public void setCheckStateChangeListener(
            boolean initCheckState, OnCheckStateChangeListener listener) {
        mChecked = initCheckState;
        if (listener == null) {
            return;
        }
        mCheckChangeListener = listener;
    }
    
}
