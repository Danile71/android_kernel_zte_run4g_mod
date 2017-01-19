package com.mediatek.blemanager.ui;


import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.GridView;
import android.widget.ImageView;

import java.util.ArrayList;

import com.mediatek.blemanager.R;
import com.mediatek.blemanager.provider.BLEConstants;

public class DeviceDefaultImageChooser extends Activity {

    private static final String TAG = BLEConstants.COMMON_TAG + "[DeviceDefaultImageChooser]";
    
    private ArrayList<Integer> mDeviceImageList;
    
    private LayoutInflater mInflater;
    private GridView mView;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.default_image_chooser_layout);
        mView = (GridView)this.findViewById(R.id.default_image_list);
        
        mDeviceImageList = new ArrayList<Integer>();
        mDeviceImageList.add(R.drawable.image_baby);
        mDeviceImageList.add(R.drawable.image_key);
        mDeviceImageList.add(R.drawable.image_pet);
        mDeviceImageList.add(R.drawable.image_wallet);
        mDeviceImageList.add(R.drawable.image_watch);
        
        mInflater = LayoutInflater.from(this);
        mView.setAdapter(new DefaultImageAdapter());
        mView.setOnItemClickListener(new OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> arg0, View arg1, int arg2,
                    long arg3) {
                Log.d(TAG, "[onItemClick] which : " + arg2);
                int item = (Integer) arg0.getItemAtPosition(arg2);
                Log.d(TAG, "[onItemClick] item : " + item);
                setActivityResult(item);
            }
        });
    }
    
    private void setActivityResult(int result) {
        Intent intent = new Intent();
        intent.putExtra("which", result);
        setResult(RESULT_OK, intent);
        finish();
    }

    @Override
    protected void onStart() {
        super.onStart();
    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // TODO Auto-generated method stub
        if (item.getItemId() == android.R.id.home) {
//            this.setActivityResult(RESULT_CANCELED);
            this.finish();
        }
        return true;
    }

    private class DefaultImageAdapter extends BaseAdapter {

        @Override
        public int getCount() {
            return mDeviceImageList.size();
        }

        @Override
        public Object getItem(int arg0) {
            return mDeviceImageList.get(arg0);
        }

        @Override
        public long getItemId(int arg0) {
            return arg0;
        }

        @Override
        public View getView(int arg0, View arg1, ViewGroup arg2) {
            ViewHolder holder;
            if (arg1 == null) {
                arg1 = mInflater.inflate(R.layout.device_default_image_item, null);
                holder = new ViewHolder();
                holder.mImage = (ImageView)arg1.findViewById(R.id.device_default_image);
                arg1.setTag(holder);
            } else {
                holder = (ViewHolder)arg1.getTag();
            }
            holder.mImage.setImageResource(mDeviceImageList.get(arg0));
            return arg1;
        }
        
        private class ViewHolder {
            private ImageView mImage;
        }
    }

}
