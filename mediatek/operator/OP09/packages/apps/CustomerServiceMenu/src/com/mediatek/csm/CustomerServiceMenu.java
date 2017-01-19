package com.mediatek.csm;

import android.app.Activity;
import android.os.Bundle;

import com.mediatek.xlog.Xlog;

public class CustomerServiceMenu extends Activity {
    static final String LOG_TAG = "CustomerService";
    private CustomerServiceFragment mFragment;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Xlog.d(LOG_TAG, "CustomerServiceMenu onCreate");
        mFragment = new CustomerServiceFragment();
        // Display the fragment as the main content.
        getFragmentManager().beginTransaction().replace(android.R.id.content, mFragment).commit();
    }

    @Override
    protected void onDestroy() {
        Xlog.d(LOG_TAG, "CustomerServiceMenu onDestroy");
        super.onDestroy();
    }
}
