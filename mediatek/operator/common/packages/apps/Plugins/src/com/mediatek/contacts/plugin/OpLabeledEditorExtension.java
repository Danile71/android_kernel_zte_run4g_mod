package com.mediatek.contacts.plugin;

import android.content.Context;
import android.content.Intent;

import com.mediatek.contacts.ext.LabeledEditorExtension;

public class OpLabeledEditorExtension extends LabeledEditorExtension {
    private final static String TAG = "OpLabeledEditorExtension";

    private Context mContext = null;

    public OpLabeledEditorExtension(Context context) {
        mContext = context;
    }

    public String getCommond() {
         return ContactsPlugin.COMMD_FOR_AAS;
    }


    public void onTypeSelectionChange(int position, String commd) {
        if (ContactsPlugin.COMMD_FOR_AAS.equals(commd)) {
            LogUtils.d(TAG, " startActivity() accountType:" + SimUtils.getCurAccount());
            if (SimUtils.isUsim(SimUtils.getCurAccount())) {
                Intent intent = new Intent(Intent.ACTION_VIEW);
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                intent.setAction("com.mediatek.contacts.action.EDIT_AAS");
                intent.putExtra(SimUtils.KEY_SLOT, SimUtils.getCurSlotId());
                mContext.startActivity(intent);
            }
        }
    }
}
