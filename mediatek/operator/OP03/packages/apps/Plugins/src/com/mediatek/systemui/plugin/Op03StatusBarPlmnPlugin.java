
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
import com.mediatek.op03.plugin.R;
import com.mediatek.systemui.ext.DefaultStatusBarPlmnPlugin;
import com.mediatek.systemui.ext.NetworkType;
import android.telephony.TelephonyManager;
import com.android.internal.telephony.TelephonyIntents;
import com.mediatek.systemui.ext.DataType;
import com.mediatek.systemui.ext.IStatusBarPlugin;
import com.mediatek.systemui.ext.IStatusBarPlmnPlugin;
import android.preference.PreferenceCategory;
import android.content.SharedPreferences;
import android.preference.CheckBoxPreference;
import android.preference.PreferenceManager;
import android.app.Activity;
import java.io.FileOutputStream;
import java.io.OutputStreamWriter;
import java.io.File;
import java.io.Writer;
import java.io.BufferedWriter;
import java.io.FileWriter;
import com.mediatek.settings.plugin.PlmnDisplaySettingsExt;
//import com.mediatek.settings.plugin.IOP03SettingsInterface;
import android.content.ServiceConnection;
import android.content.ComponentName;
import android.os.IBinder;
import android.provider.Settings;


/**
 * M: OP03 implementation of Plug-in definition of Status bar.
 * This class extends TextView, implements IStatusBarPlugin interface
 * This is done to allow or class to inherit from the TextView class
 */
public class Op03StatusBarPlmnPlugin extends DefaultStatusBarPlmnPlugin{
    static final String TAG = "Op03StatusBarPlmnPlugin";
    private String mPlmn = null;
    //PlmnDisplaySettings mInst = null;
    private TextView mPlmnTextView = null;
    StringBuilder mstrSub = null;  
    StringBuilder mstr = null;  
    boolean mshowText = false;
    String mshowTextStr = "false";
    Context mconxt=null;
    Context msystemContext = null;
    SharedPreferences mprefs = null;
    //private IOP03SettingsInterface plmnInterface =null;
    private final String SETTING_SERVICE = "setting_service_intent";
    private final String PLMN_SETTING_STATE ="plmn_setting_state";
    private final String MSHOWTEXT = "mshowText";
    /************************************************************************/
    /*********************** constructor***************************************/
    /********** this is a TextView, we will receive the parameters in the constructor to**/
    /** use the plmn to show the text *******************************************/
		
   public Op03StatusBarPlmnPlugin(Context context){
      super(context);
      mconxt = context;   
      mstrSub = new StringBuilder();  
      mstr = new StringBuilder(); 
      //receive broadcasts
      IntentFilter filter = new IntentFilter();
      filter.addAction(Telephony.Intents.SPN_STRINGS_UPDATED_ACTION); /*listen for SPN_STRINGS_UPDATED_ACTION*/
      filter.addAction(PlmnDisplaySettingsExt.ACTION_PLMN_CHANGED);
      filter.addAction("com.mediatek.settings.PLMN_TEXT_SETTING");
      context.registerReceiver(mBroadcastReceiver, filter);
      if(mPlmnTextView == null)
        mPlmnTextView = new TextView(context);
      mshowText = getShowText();
       }
      
      
    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            Xlog.d(TAG, "into broadcast recevier");
            String action = intent.getAction();
            boolean dataShared = false;
            StringBuilder str = new StringBuilder();
           Xlog.d(TAG, "Into received intent "+action);
           if(PlmnDisplaySettingsExt.ACTION_PLMN_CHANGED.equals(action)){
               String message = intent.getStringExtra("plmnSettingCheck");
               Xlog.d(TAG, "received the intent");
               Xlog.d(TAG, message);
              if(message.equals("true")){
                  mshowText = true;
                  mshowTextStr = "true";
                }else{
                 mshowText = false;
                 mshowTextStr = "false";
             }
        try{
            Xlog.d(TAG, "write into setSettingparameter");
              //plmnInterface.setSettingparameter(PLMN_SETTING_STATE,mshowTextStr); 
           }catch(Exception e){
               Xlog.d(TAG, "Fail to write using setSettingparameter catch exception");
        }
               android.provider.Settings.System.putInt(mconxt.getContentResolver(), MSHOWTEXT, mshowText ? 1: 0);
        }

         if (Telephony.Intents.SPN_STRINGS_UPDATED_ACTION.equals(action)){
         StringBuilder strTemp = new StringBuilder(); 
         mstr = strTemp;
         mstr.append(intent.getStringExtra(Telephony.Intents.EXTRA_PLMN));
         boolean showPlmn = false;
         }


         Xlog.d(TAG, "called getPlmnSetting received =" );
         if(mshowText == true){
         mPlmnTextView.setText(mstr.toString(),TextView.BufferType.NORMAL);
         Xlog.d(TAG, "true to show now mstr= " +mstr);
         }else{
         mPlmnTextView.setText("",TextView.BufferType.NORMAL);
         Xlog.d(TAG, "false dont show now = mstr"+mstr);
         }
         Xlog.d(TAG, "set text of textview");
         Xlog.d(TAG, "set text of textview = "+str);
       }
     };
     
    public TextView getPlmnTextView(Context sysContx){
        Xlog.d(TAG, "return mPlmnTextView");
        msystemContext = sysContx;
        return mPlmnTextView;
    }
        
    public boolean getShowText(){
       int showText = android.provider.Settings.System.getInt(mconxt.getContentResolver(), MSHOWTEXT, -1);
       Xlog.d(TAG, "getShowTextParameter"+showText);
       if(showText == 1)
        return true;
       else 
        return false;
        
    }
   
}
