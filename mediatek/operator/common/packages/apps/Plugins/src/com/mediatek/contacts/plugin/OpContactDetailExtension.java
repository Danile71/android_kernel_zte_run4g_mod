package com.mediatek.contacts.plugin;

//import com.mediatek.op03.plugin.R;
import com.mediatek.contacts.plugin.AASSNEResources;

import android.content.Context;
import android.content.Intent;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import com.mediatek.contacts.ext.ContactDetailExtension;

public class OpContactDetailExtension extends ContactDetailExtension {
    private static final String TAG = "OpContactDetailExtension";

    private Context mContext = null;

    public OpContactDetailExtension(Context context) {
        mContext = context;
    }

    /***
     * Used for get string resource from plugin.
     */
    public String repChar(String phoneNumber, char pause, char p, char wait, char w, String commd) {
        if (ContactsPlugin.COMMD_FOR_AAS.equals(commd)) {
            if (w == STRING_PRIMART) {
                return mContext.getResources().getString(AASSNEResources.aas_phone_primary);
            }
            return mContext.getResources().getString(AASSNEResources.aas_phone_additional);
        }
        return phoneNumber;
    }

    /**
     * @param view The View to update
     * @param type Kind of View
     * @param action Operation action
     * @param cmd
     * @return
     */
    
    public boolean updateView(View view, int type, int action, String commd) {
        if (ContactsPlugin.COMMD_FOR_AAS.equals(commd)) {
            LogUtils.d(TAG, "updateView(), type=" + type + " action=" + action);
            switch (action) {
            case VIEW_UPDATE_HINT:
                if (SimUtils.isUsim(SimUtils.getCurAccount())) {
                    if (view instanceof TextView) {
                        ((TextView) view).setHint(repChar("", (char) 0, (char) 0, (char) 0, (char) type, ContactsPlugin.COMMD_FOR_AAS));
                    } else {
                        LogUtils.e(TAG, "updateView(), VIEW_UPDATE_HINT but view is not a TextView");
                    }
                }
                break;
            case VIEW_UPDATE_VISIBILITY:
                view.setVisibility(View.GONE);
                break;
            default:
                break;
            }
            return true;
        }
        return false;
    }

    public int getMaxEmptyEditors(String mimeType, String commd) {
        if (ContactsPlugin.COMMD_FOR_AAS.equals(commd)) {
            LogUtils.d(TAG, "getMaxEmptyEditors() curAccount=" + SimUtils.getCurAccount() + "mimeType=" + mimeType);
            if (SimUtils.isUsim(SimUtils.getCurAccount()) && SimUtils.isPhone(mimeType)) {
                int max = SimUtils.getAnrCount(SimUtils.getCurSlotId()) + 1;
                LogUtils.d(TAG, "getMaxEmptyEditors() max=" + max);
                return max;
            }
            return super.getMaxEmptyEditors(mimeType, commd);
        }
        return 1;
    }

    public int getAdditionNumberCount(int slotId, String commd) {
        if (ContactsPlugin.COMMD_FOR_AAS.equals(commd)) {
            int count = SimUtils.getAnrCount(slotId);
            LogUtils.d(TAG, "getAdditionNumberCount() count=" + count);
            return count;
        }
        return 0;
    }

    public boolean isDoublePhoneNumber(String[] buffer, String[] bufferName, String commd) {
        if (ContactsPlugin.COMMD_FOR_AAS.equals(commd)) {
            if (SimUtils.isUsim(SimUtils.getCurAccount())) {
                return bufferName[1] != null;
            }
        }
        return super.isDoublePhoneNumber(buffer, bufferName, commd);
    }
    
}
