
package com.mediatek.systemui.plugin;

import android.content.Context;
import android.content.ContextWrapper;
import android.content.res.Resources;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.view.View;
import android.widget.TextView;
import android.provider.Telephony;
import android.content.IntentFilter;
import com.mediatek.xlog.Xlog;
import com.mediatek.systemui.ext.DefaultStatusBarPlmnPlugin;
import com.mediatek.systemui.ext.NetworkType;
import android.telephony.TelephonyManager;
import com.android.internal.telephony.TelephonyIntents;
import com.mediatek.systemui.ext.DataType;
import com.mediatek.systemui.ext.IStatusBarPlugin;
import com.mediatek.systemui.ext.IStatusBarPlmnPlugin;
import android.app.Activity;
import android.util.Log;
import android.telephony.TelephonyManager;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.PhoneConstants;
import android.util.Log;
import android.widget.Toast;
import com.mediatek.op07.plugin.R;

public class Op07StatusBarPlmnPluginExt extends DefaultStatusBarPlmnPlugin{
    static final String TAG = "Op07StatusBarPlmnPluginExt";
    private String mPlmn = null;
    private TextView mPlmnTextView = null;
    Context mconxt=null;
    Context mContext = null;
 
		
    public Op07StatusBarPlmnPluginExt(Context context){
        super(context);
        mContext = context;   
        //receive broadcasts
        IntentFilter filter = new IntentFilter();
        filter.addAction(Telephony.Intents.SPN_STRINGS_UPDATED_ACTION); /*listen for SPN_STRINGS_UPDATED_ACTION*/
        filter.addAction(TelephonyIntents.ACTION_SIM_INDICATOR_STATE_CHANGED);
        context.registerReceiver(mBroadcastReceiver, filter);
        if(mPlmnTextView == null)
            mPlmnTextView = new TextView(context);      
   }
    
      
    /********************************************************************************/
    /********* Broadcast receiver for change in network *****************************/
    /****Receives the intent to display and change the operator name on status bar***/
    /********************************************************************/
    
    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            Xlog.e(TAG, "into broadcast recevier");
            String action = intent.getAction();

            if(Telephony.Intents.SPN_STRINGS_UPDATED_ACTION.equals(action)){
                Xlog.e(TAG, "SPN_STRINGS_UPDATED_ACTION");
                updateNetworkName(
                intent.getBooleanExtra(Telephony.Intents.EXTRA_SHOW_SPN, false),
                intent.getStringExtra(Telephony.Intents.EXTRA_SPN),
                intent.getBooleanExtra(Telephony.Intents.EXTRA_SHOW_PLMN, false),
                intent.getStringExtra(Telephony.Intents.EXTRA_PLMN));

            } else if (action.equals(TelephonyIntents.ACTION_SIM_INDICATOR_STATE_CHANGED)) { ///for AT&T
                Xlog.e(TAG, "ACTION_SIM_INDICATOR_STATE_CHANGED");
                int simStatus = intent.getIntExtra(TelephonyIntents.INTENT_KEY_ICC_STATE, -1);
                if (simStatus == PhoneConstants.SIM_INDICATOR_SEARCHING) {
                    Xlog.e(TAG, "updateSIMState. simStatus is " + simStatus);
                    updatePLMNSearchingStateView(true);
                } else {
                    updatePLMNSearchingStateView(false);
                }
            }
         }

    };

    /*****************************************************************************/
    /***************************updateNetworkName*********************************/
    /***************************receive the textview and add the text*************/
    /*****************************************************************************/

    
    private String mOldPlmn = null;
    private void updateNetworkName(boolean showSpn, String spn, boolean showPlmn, String plmn) {
        Xlog.e(TAG, "For AT&T updateNetworkName, showSpn=" + showSpn + " spn=" + spn + " showPlmn=" + showPlmn + " plmn=" + plmn);
        
        StringBuilder str = new StringBuilder();
        boolean something = false;
        if (showPlmn && plmn != null) {
            str.append(plmn);
            something = true;
        }
        if (showSpn && spn != null) {
            if (something) {
                str.append(mContext.getString(R.string.status_bar_network_name_separator_plug));
            }
            str.append(spn);
            something = true;
        }
        if (something) {
            mOldPlmn = str.toString();
        } else {
            mOldPlmn = mContext.getResources().getString(com.android.internal.R.string.lockscreen_carrier_default);
        }
        mPlmnTextView.setText(str);
        mPlmnTextView.setVisibility(View.VISIBLE);
    }
    
    
    
    public TextView getPlmnTextView(Context context){
        Xlog.e(TAG, "return mPlmnTextView");
        Xlog.e(TAG, "return mPlmnTextView");            
        return mPlmnTextView;
    } 
    

/*
** update the plmn when in search mode
*/
    private void updatePLMNSearchingStateView(boolean searching) {
        if(searching) {
             mPlmnTextView.setText(R.string.plmn_searching);
             Xlog.e(TAG, "updatePLMNSearchingStateView");
        } else {
            mPlmnTextView.setText(mOldPlmn);
            Xlog.e(TAG, "updatePLMNSearchingStateView");
        }
        mPlmnTextView.setVisibility(View.VISIBLE);
    }
   
};
