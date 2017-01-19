
package com.mediatek.settings.plugin;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.widget.SearchView;
import android.widget.SearchView.OnCloseListener;
import android.widget.SearchView.OnQueryTextListener;
import android.widget.TextView;

import com.mediatek.op09.plugin.R;
import com.mediatek.xlog.Xlog;

/**
 * There are three entry points to  <code>TimeZoneActivity</code>:
 * 1. From Notification list.
 * 2. From International Roaming Settings screen.
 * 3. Pops up directly
 */
public class TimeZoneActivity extends Activity {
    private static final String TAG = "TimeZoneActivity";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.activity_international_roaming);
        final TextView title = (TextView) findViewById(R.id.time_zone_tilte);
        final SearchView searchView = (SearchView) findViewById(R.id.searchView);
        searchView.setOnSearchClickListener(new OnClickListener() {
            @Override
            public void onClick(View arg0) {
                title.setVisibility(View.GONE);
                Xlog.i(TAG, "setOnSearchClickListener!");
            }
        });
        searchView.setOnCloseListener(new OnCloseListener() {
            @Override
            public boolean onClose() {
                title.setVisibility(View.VISIBLE);
                return false;
            }
        });
        searchView.setOnQueryTextListener(new OnQueryTextListener() {
            @Override
            public boolean onQueryTextSubmit(String query) {
                return false;
            }
            @Override
            public boolean onQueryTextChange(String newText) {
                CharSequence str = searchView.getQuery();
                Xlog.i(TAG, str.toString());
                Intent intent = new Intent(ZonePicker.INPUT_ACTION);
                intent.putExtra(ZonePicker.INPUT_CONTENT, str.toString());
                sendBroadcast(intent);
                return false;
            }
        });
    }
}
