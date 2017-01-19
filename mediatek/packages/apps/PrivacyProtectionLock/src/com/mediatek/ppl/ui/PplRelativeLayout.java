package com.mediatek.ppl.ui;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.widget.RelativeLayout;

public class PplRelativeLayout extends RelativeLayout {

    private static final String TAG = "PPL/PplRelativeLayout";


    public static final int PANEL_INIT = 0;
    public static final int PANEL_SHOW = 1;
    public static final int PANEL_HIDE = 2;
    
    private IOnResizeListener mListener;
    
    public PplRelativeLayout(Context context) {
        super(context);
    }

    public PplRelativeLayout(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public PplRelativeLayout(Context context, AttributeSet attrs, int defStyle) {  
        super(context, attrs, defStyle);  
    }   

    public void setOnResizeListener(IOnResizeListener l) {
        mListener = l;
    }

    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        
        super.onSizeChanged(w, h, oldw, oldh);
                        
        //if (h != oldh && oldh != 0) {
        if (h != oldh) {
            Log.d(TAG, "onSizeChanged: h=" + h + ", oldh=" + oldh);
            mListener.onPanelChange(h);
        } 
    }

    public interface IOnResizeListener {
        public void onPanelChange(int h);
    }

}
