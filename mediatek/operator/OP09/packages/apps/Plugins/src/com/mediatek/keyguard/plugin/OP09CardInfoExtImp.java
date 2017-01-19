package com.mediatek.keyguard.plugin;

import android.content.Context;
import android.graphics.drawable.Drawable;
import android.util.Log;
import android.widget.TextView;

import com.mediatek.keyguard.ext.DefaultCardInfoExt;
import com.mediatek.telephony.SimInfoManager;

public class OP09CardInfoExtImp extends DefaultCardInfoExt {
    private static final String TAG = "CardInfoExtImp";

    @Override
    public void addOptrNameByIdx(TextView v, long simIdx, Context context, String optrname) {
        Drawable d = getOptrDrawableByIdx(simIdx, context);
        if (d != null) {
            d.setBounds(0, 0, d.getIntrinsicWidth(), d.getIntrinsicHeight());
        }
        v.setCompoundDrawables(d, null, null, null);
        v.setCompoundDrawablePadding(15);

        if (null == optrname) {
            v.setText(com.mediatek.internal.R.string.searching_simcard);
        } else {
            v.setText(optrname);
        }
    }

    @Override
    public void addOptrNameBySlot(TextView v, int slot, Context context, String optrname) {
        Drawable d = getOptrDrawableBySlot(slot, context);
        if (d != null) {
            d.setBounds(0, 0, d.getIntrinsicWidth(), d.getIntrinsicHeight());
        }
        v.setCompoundDrawables(d, null, null, null);
        v.setCompoundDrawablePadding(15);
        
        if (null == optrname) {
            v.setText(com.mediatek.internal.R.string.searching_simcard);
        } else {
            v.setText(optrname);
        }
    }

    @Override
    public Drawable getOptrDrawableByIdx(long simIdx, Context context) {
        if (simIdx > 0) {
            Log.d(TAG, "getOptrDrawableById, xxsimIdx=" + simIdx);
            SimInfoManager.SimInfoRecord info = SimInfoManager.getSimInfoById(context, (int)simIdx); 
            if (null == info) {
                Log.d(TAG, "getOptrDrawableBySlotId, return null");
               return null;
            } else {
               return context.getResources().getDrawable(info.mSimBackgroundDarkSmallRes);
            }
        } else {
            return null;
        }
    }

    @Override
    public Drawable getOptrDrawableBySlot(long slot, Context context) {
        if (slot >= 0) {
            Log.d(TAG, "getOptrDrawableBySlot, xxslot=" + slot);
            SimInfoManager.SimInfoRecord info = SimInfoManager.getSimInfoBySlot(context, (int)slot); 
            if (null == info) {
                Log.d(TAG, "getOptrDrawableBySlotId, return null");
                return null;
            } else {
                return context.getResources().getDrawable(info.mSimBackgroundDarkSmallRes);
            }
        } else {
            throw new IndexOutOfBoundsException();
        }
    }
}
