package com.mediatek.dialer.plugin;

import android.app.Activity;
import android.app.ActionBar.Tab;
import android.app.ActionBar.TabListener;
import android.app.FragmentTransaction;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.Resources;
import android.preference.PreferenceManager;
import android.provider.CallLog.Calls;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.MenuItem.OnMenuItemClickListener;
import android.widget.TextView;

import com.mediatek.dialer.ext.DefaultCallLogExtension;
import com.mediatek.dialer.ext.ICallLogExtension.ICallLogAction;
import com.mediatek.op01.plugin.R;

public class Op01CallLogExtension extends DefaultCallLogExtension {
    private static final String TAG = "Op01CallLogExtension";

    public static final int CALL_TYPE_ALL = -1;
    public static final String TYPE_FILTER_PREF = "calllog_type_filter";
    public static final int FILTER_BASE = 20000;
    public static final int FILTER_TYPE_AUTO_REJECT = FILTER_BASE + 21;
    
    /**
     * for op01
     * @param typeFiler current query type
     * @param builder the query selection Stringbuilder
     */
    @Override
    public void appendQuerySelection(int typeFiler, StringBuilder builder) {
        Log.i(TAG, "appendQuerySelection");
        String strbuilder = null;
        if ( CALL_TYPE_ALL == typeFiler ) {
            if(builder.length()>0 ){
                builder.append(" and ");
            }
            strbuilder = Calls.TYPE + "!=" + Calls.AUTOREJECTED_TYPE;
            builder.append(strbuilder);
        }
        Log.i(TAG, "builder: " + builder);
    }

    /**
     * for op01
     * called when host create menu, to add plug-in own menu here
     * @param menu
     * @param callLogAction callback plug-in need if things need to be done by host
     */
    @Override
    public void createCallLogMenu(Activity callLogActivity, Menu menu, ICallLogAction callLogAction) {
        Log.i(TAG, "createCallLogMenu");
        mCallLogActivity = callLogActivity;
        final Activity fCallLogActivity = callLogActivity;
        final ICallLogAction fCallLogAction = callLogAction;
        try {
            final Context cont = mCallLogActivity.getApplicationContext().createPackageContext("com.mediatek.op01.plugin",Context.CONTEXT_INCLUDE_CODE | Context.CONTEXT_IGNORE_SECURITY);
            MenuItem autoRejectMenu = menu.add(cont.getText(R.string.call_log_auto_rejected_label));
            mAutoRejectMenuId = autoRejectMenu.getItemId();
            autoRejectMenu.setOnMenuItemClickListener(mAutoRejectMenuClickListener = new OnMenuItemClickListener() {
                @Override
                public boolean onMenuItemClick(MenuItem item) {
                    Log.i(TAG, "Auto reject onMenuItemClick");
                    SharedPreferences.Editor editor = PreferenceManager.getDefaultSharedPreferences(mCallLogActivity.getApplicationContext()).edit();
                    editor.putInt(TYPE_FILTER_PREF, Calls.AUTOREJECTED_TYPE);
                    editor.commit();
                    fCallLogAction.updateCallLogScreen();
                    fCallLogActivity.getActionBar().removeAllTabs();
                    final Tab autoRejectTab = fCallLogActivity.getActionBar().newTab();
                    final String autoRejectTitle = cont.getString(R.string.call_log_auto_rejected_label);
                    autoRejectTab.setContentDescription(autoRejectTitle);
                    autoRejectTab.setText(autoRejectTitle);
                    autoRejectTab.setTabListener(mTabListener);
                    fCallLogActivity.getActionBar().addTab(autoRejectTab);
                    return true;
                }
            });
        } catch(NameNotFoundException e){
            Log.i(TAG, "no com.mediatek.op01.plugin packages");
        }
    }

    /**
     * for op01 
     * @param context
     * @param menu
     */
    @Override
    public void prepareCallLogMenu(Menu menu) {
        Log.i(TAG, "prepareCallLogMenu");
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(mCallLogActivity.getApplicationContext());
        boolean isAutoRejectedFilterMode = (prefs.getInt(TYPE_FILTER_PREF, CALL_TYPE_ALL) ==  Calls.AUTOREJECTED_TYPE);
        Log.i(TAG, "prefs.getInt(TYPE_FILTER_PREF, CALL_TYPE_ALL): " + prefs.getInt(TYPE_FILTER_PREF, CALL_TYPE_ALL));
        Log.i(TAG, "Calls.AUTOREJECTED_TYPE: " + Calls.AUTOREJECTED_TYPE );
        Log.i(TAG, "isAutoRejectedFilterMode: " + isAutoRejectedFilterMode);
        if(mAutoRejectMenuId >= 0){
            menu.findItem(mAutoRejectMenuId).setVisible(!isAutoRejectedFilterMode);
        }
    }

    /**
     * for op01 
     * called when host press back key
     */
    @Override
    public void onBackPressed(ICallLogAction callLogAction){
        Log.i(TAG, "onBackHandled");
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(mCallLogActivity.getApplicationContext());
        boolean isAutoRejectedFilterMode = (prefs.getInt(TYPE_FILTER_PREF, CALL_TYPE_ALL) ==  Calls.AUTOREJECTED_TYPE);
        if(isAutoRejectedFilterMode){
            SharedPreferences.Editor editor = prefs.edit();
            editor.putInt(TYPE_FILTER_PREF, CALL_TYPE_ALL);
            editor.commit();
            final Intent intent = new Intent();
            intent.setClassName("com.android.dialer", "com.android.dialer.calllog.CallLogActivity");
            mCallLogActivity.startActivity(intent);
            mCallLogActivity.finish();
        }else{
            callLogAction.processBackPressed();
        }
    }
 
    private final TabListener mTabListener = new TabListener() {
        @Override
        public void onTabUnselected(Tab tab, FragmentTransaction ft) {
        }
        @Override
        public void onTabSelected(Tab tab, FragmentTransaction ft) {
        }
        @Override
        public void onTabReselected(Tab tab, FragmentTransaction ft) {
        }
    };
    
    /// M: for Auto Reject CallLog @{
    private OnMenuItemClickListener mAutoRejectMenuClickListener ;
    private int mAutoRejectMenuId = -1;
    private Activity mCallLogActivity;
    private static final int TAB_INDEX_ALL = 0;
    private static final int TAB_INDEX_MISSED = 1;

}
