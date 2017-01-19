package com.mediatek.blemanager.ui;


import android.app.AlertDialog;
import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.content.DialogInterface;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;

import java.util.ArrayList;

import com.mediatek.blemanager.R;
import com.mediatek.blemanager.provider.BLEConstants;

public class ScanDeviceAlertDialog {
    private static final String TAG = BLEConstants.COMMON_TAG + "[ScanAlertActivity]";
    public static final int SCAN = 1;
    public static final int DELETE = 2;
    private static ScanDeviceAlertDialog sInstance;
    
    private AlertDialog mDialog;
    private ProgressBar mProgressBar;
    private ListView mDeviceListView;
    private Context mContext;
    private ArrayList<BluetoothDevice> mFoundDevices;
    
    private ScanDeviceAlertDialog(Context context, ArrayList<BluetoothDevice> foundDevices) {
        mContext = context;
//        mFoundDevices = foundDevices;
    }
    
    public static AlertDialog show(
            int which,Context context, ArrayList<BluetoothDevice> foundDevices,
            DialogInterface.OnClickListener cancelListener,
            AdapterView.OnItemClickListener litItemClickListener) {
        if (context == null) {
            Log.d(TAG, "[show] context is null!");
            return null;
        }
        if (sInstance == null) {
            sInstance = new ScanDeviceAlertDialog(context, foundDevices);
        }
        if (sInstance.mDialog != null && sInstance.mDialog.isShowing()) {
            sInstance.mDialog.dismiss();
            sInstance.mDialog = null;
        }
        AlertDialog.Builder builder = new AlertDialog.Builder(context);
        View customTitleView;
        if (which == SCAN) {
            customTitleView =
                LayoutInflater.from(context).inflate(R.layout.scan_dialog_title_view, null);
        } else {
            customTitleView =
                LayoutInflater.from(context).inflate(R.layout.delete_dialog_title_view, null);
        }
        View view = LayoutInflater.from(context).inflate(R.layout.scan_dialog_content_view, null);
        builder.setCustomTitle(customTitleView);
        builder.setView(view);
        sInstance.mFoundDevices = foundDevices;
        sInstance.mProgressBar =
            (ProgressBar)customTitleView.findViewById(R.id.scan_dialog_progress);
        sInstance.mDeviceListView = (ListView)view.findViewById(R.id.scanned_device_list);
        sInstance.mDeviceListView.setAdapter(sInstance.mDeviceAdater);
        sInstance.mDeviceListView.setOnItemClickListener(litItemClickListener);
        builder.setNegativeButton(R.string.cancel, cancelListener);
        builder.setCancelable(false);
        sInstance.mDialog = builder.create();
        sInstance.mDialog.show();
        return sInstance.mDialog;
    }
    
    /**
     * dismiss current dialog
     */
    public static void dismiss() {
        if (sInstance == null) {
            return;
        }
        if (sInstance.mDialog == null) {
            return;
        }
        if (sInstance.mDialog.isShowing()) {
            sInstance.mDialog.setCancelable(true);
            sInstance.mDialog.dismiss();
        }
        if (sInstance.mFoundDevices != null) {
            sInstance.mFoundDevices.clear();
        }
        sInstance = null;
    }

    public static void notifyUi() {
        if (sInstance == null) {
            return;
        }
        if (sInstance.mDeviceAdater == null) {
            return;
        }
        sInstance.mDeviceAdater.notifyDataSetChanged();
    }
    
    public static void hideProgressBar() {
        if (sInstance != null && sInstance.mProgressBar != null) {
            sInstance.mProgressBar.setVisibility(View.INVISIBLE);
        }
    }
    
    private BaseAdapter mDeviceAdater = new BaseAdapter() {

        @Override
        public int getCount() {
            // TODO Auto-generated method stub
            return mFoundDevices.size();
        }

        @Override
        public Object getItem(int arg0) {
            // TODO Auto-generated method stub
            return mFoundDevices.get(arg0);
        }

        @Override
        public long getItemId(int arg0) {
            // TODO Auto-generated method stub
            return arg0;
        }

        @Override
        public View getView(int arg0, View arg1, ViewGroup arg2) {
            // TODO Auto-generated method stub
            Log.d(TAG, "[getView] enter");
            ViewHolder holder = null;
            if (arg1 == null) {
                arg1 =
                    LayoutInflater.from(mContext).inflate(R.layout.scan_device_item_detail, null);
                holder = new ViewHolder();
                holder.mNameText = (TextView) arg1.findViewById(R.id.device_name);
                arg1.setTag(holder);
            } else {
                holder = (ViewHolder) arg1.getTag();
            }

            String name = mFoundDevices.get(arg0).getName();
            String addr = mFoundDevices.get(arg0).getAddress();
            if (name == null || name.trim().length() == 0) {
                holder.mNameText.setText(addr);
            } else {
                holder.mNameText.setText(name);
            }
            return arg1;
        }

        class ViewHolder {
            TextView mNameText;
        }
    };
}
