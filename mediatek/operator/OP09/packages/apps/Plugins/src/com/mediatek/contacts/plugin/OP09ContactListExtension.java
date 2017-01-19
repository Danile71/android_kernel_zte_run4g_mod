package com.mediatek.contacts.plugin;

import android.telephony.PhoneNumberUtils;
import android.util.Log;
import android.view.MenuItem;
import android.view.Menu;
import android.content.Context;

import com.mediatek.contacts.ext.ContactListExtension;
import com.mediatek.contacts.ext.ContactPluginDefault;

import java.util.HashMap;
import java.util.List;

import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.app.AlertDialog;

import com.mediatek.op09.plugin.R;
import com.mediatek.phone.SIMInfoWrapper;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;
import com.mediatek.common.telephony.ITelephonyEx;
import android.graphics.drawable.Drawable;
import android.content.pm.PackageManager.NameNotFoundException;

public class OP09ContactListExtension extends ContactListExtension {
    private static final String TAG = "OP09ContactListExtension";
    private Context mContext = null;
    private static Context mContextHost = null;
    private static final int MENU_SIM_STORAGE = 9999;   

    @Override
    public String getCommand() {
        return ContactPluginDefault.COMMD_FOR_OP09;
    }

    @Override
    public void registerHostContext(Context context, Bundle args, String commd) {
        if (! ContactPluginDefault.COMMD_FOR_OP09.equals(commd)){
            return ;
        }
        
        mContextHost = context;
        try {
            mContext = context.createPackageContext("com.mediatek.op09.plugin", Context.CONTEXT_INCLUDE_CODE | Context.CONTEXT_IGNORE_SECURITY);
        } catch(NameNotFoundException e){
            Log.d(TAG, "no com.mediatek.op09.plugin packages");
        }
    }

    @Override
    public void addOptionsMenu(Menu menu, Bundle args, String commd) {
        Log.i(TAG, "addOptionsMenu"); 
        if (! ContactPluginDefault.COMMD_FOR_OP09.equals(commd)){
            return ;
        }

        MenuItem item = menu.findItem(MENU_SIM_STORAGE);
        if (item == null) {
            String string = mContext.getResources().getString(R.string.look_simstorage);
            menu.add(0, MENU_SIM_STORAGE, 0, string).setOnMenuItemClickListener(
                new MenuItem.OnMenuItemClickListener() {
                    public boolean onMenuItemClick(MenuItem item) {
                        ShowSimCardStorageInfoTask.showSimCardStorageInfo(mContext);
                        return true;
                    }
            });
        }
    }
    
    public static class ShowSimCardStorageInfoTask extends AsyncTask<Void, Void, Void> {
        private static ShowSimCardStorageInfoTask sInstance = null;
        private boolean mIsCancelled = false;
        private boolean mIsException = false;
        private String mDlgContent = null;
        private Context mContext = null;
        private static HashMap<Integer, Integer> sSurplugMap = new HashMap<Integer, Integer>();

        public static void showSimCardStorageInfo(Context context) {
            Log.i(TAG, "[ShowSimCardStorageInfoTask]_beg");
            if (sInstance != null) {
                sInstance.cancel();
                sInstance = null;
            }
            sInstance = new ShowSimCardStorageInfoTask(context);
            sInstance.execute();
            Log.i(TAG, "[ShowSimCardStorageInfoTask]_end");
        }

        public ShowSimCardStorageInfoTask(Context context) {
            mContext = context;
            Log.i(TAG, "[ShowSimCardStorageInfoTask] onCreate()");
        }

        @Override
        protected Void doInBackground(Void... args) {
            Log.i(TAG, "[ShowSimCardStorageInfoTask]: doInBackground_beg");
            sSurplugMap.clear();
//            List<SIMInfo> simInfos = SIMInfo.getInsertedSIMList(mContext);
            List<SimInfoRecord> simInfos =  SIMInfoWrapper.getDefault().getInsertedSimInfoList();
            Log.i(TAG, "[ShowSimCardStorageInfoTask]: simInfos.size = " + simInfos.size());
            if (!mIsCancelled && (simInfos != null) && simInfos.size() > 0) {
                StringBuilder build = new StringBuilder();
                int simId = 0;
                for (SimInfoRecord simInfo : simInfos) {
                    if (simId > 0) {
                        build.append("\n\n");
                    }
                    simId++;
                    int[] storageInfos = null;
                    Log.i(TAG, "[ShowSimCardStorageInfoTask] simName = " + simInfo.mDisplayName
                            + "; simSlot = " + simInfo.mSimSlotId + "; simId = " + simInfo.mSimInfoId);
                    build.append(simInfo.mDisplayName);
                    build.append(" " + simId);
                    build.append(":\n");
                    try {
                        ITelephonyEx phoneEx = ITelephonyEx.Stub.asInterface(ServiceManager
                              .checkService("phoneEx"));
                        if (!mIsCancelled && phoneEx != null) {
                            storageInfos = phoneEx.getAdnStorageInfo(simInfo.mSimSlotId);
                            if (storageInfos == null) {
                                mIsException = true;
                                Log.i(TAG, " storageInfos is null");
                                return null;
                            }
                            Log.i(TAG, "[ShowSimCardStorageInfoTask] infos: "
                                    + storageInfos.toString());
                        } else {
                            Log.i(TAG, "[ShowSimCardStorageInfoTask]: phone = null");
                            mIsException = true;
                            return null;
                        }
                    } catch (RemoteException ex) {
                        Log.i(TAG, "[ShowSimCardStorageInfoTask]_exception: " + ex);
                        mIsException = true;
                        return null;
                    }
                    Log.i(TAG, "slotId:" + simInfo.mSimSlotId + "||storage:"
                            + (storageInfos == null ? "NULL" : storageInfos[1]) + "||used:"
                            + (storageInfos == null ? "NULL" : storageInfos[0]));
                    if (storageInfos != null && storageInfos[1] > 0) {
                        sSurplugMap.put(simInfo.mSimSlotId, storageInfos[1] - storageInfos[0]);
                    }
                    build.append(mContext.getResources().getString(R.string.dlg_simstorage_content,
                            storageInfos[1], storageInfos[0]));
                    if (mIsCancelled) {
                        return null;
                    }
                }
                mDlgContent = build.toString();
            }
            Log.i(TAG, "[ShowSimCardStorageInfoTask]: doInBackground_end");
            return null;
        }

        public void cancel() {
            super.cancel(true);
            mIsCancelled = true;
            Log.i(TAG, "[ShowSimCardStorageInfoTask]: mIsCancelled = true");
        }

        @Override
        protected void onPostExecute(Void v) {
            Drawable icon = mContext.getResources().getDrawable(R.drawable.ic_menu_look_simstorage_holo_light);
            String string = mContext.getResources().getString(R.string.look_simstorage);
            sInstance = null;
            if (!mIsCancelled && !mIsException && mContextHost != null) {
                new AlertDialog.Builder(mContextHost).setIcon(icon).setTitle(
                        string).setMessage(mDlgContent).setPositiveButton(
                        android.R.string.ok, null).setCancelable(true).create()
                        .show();
            }
            mIsCancelled = false;
            mIsException = false;
        }
    }
}
