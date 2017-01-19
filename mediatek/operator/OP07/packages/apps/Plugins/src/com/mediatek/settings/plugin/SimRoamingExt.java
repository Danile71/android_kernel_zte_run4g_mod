package com.mediatek.settings.plugin;

import android.app.AlertDialog;
import android.app.Dialog;
import android.preference.Preference;
import android.content.Context;
import android.widget.Toast;
import com.mediatek.settings.ext.ISimRoamingExt;
import com.mediatek.settings.ext.DefaultSimRoamingExt;
import com.mediatek.xlog.Xlog;
import com.mediatek.op07.plugin.R;


public class SimRoamingExt extends DefaultSimRoamingExt{
    private String TAG = "SimRoamingExt";
    private AlertDialog.Builder mDialogBuild;
    private Context mContext;

    public SimRoamingExt(Context context) {
        mContext = context;
        Xlog.d(TAG,"SimRoamingExt");
    }    

    public String getRoamingWarningMsg(Context context, int res){
        String text = mContext.getString(R.string.data_roaming_warning);
	Xlog.d(TAG,"text="+text);
        return text;
    }
    
    public void setSummary(Preference pref){
        pref.setSummary(mContext.getString(R.string.data_roaming_warning));
    }
    public void showDialog(Context context){
        if (mDialogBuild == null){
            Xlog.d(TAG,"create new dialog");
            mDialogBuild = new AlertDialog.Builder(context);
            mDialogBuild.setMessage(mContext.getString(R.string.data_conn_under_roaming_hint))
            .setTitle(android.R.string.dialog_alert_title)
            .setIcon(android.R.drawable.ic_dialog_alert)
            .setNegativeButton(android.R.string.ok, null)
            .create();   
        }
        mDialogBuild.show();  
    }

    public void showPinToast(boolean enable) {
        if (enable) {
            Toast.makeText(mContext,
                    mContext.getString(R.string.sim_pin_enable),
                    Toast.LENGTH_SHORT).show();
        } else {
            Toast.makeText(mContext,
                    mContext.getString(R.string.sim_pin_disable),
                    Toast.LENGTH_SHORT).show();
        }
    }
}
