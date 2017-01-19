package com.mediatek.settings.plugin;

import android.app.Activity;
import android.app.AlarmManager;
import android.app.ListFragment;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.XmlResourceParser;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.ListView;
import android.widget.SimpleAdapter;

import com.mediatek.op09.plugin.R;
import com.mediatek.xlog.Xlog;

import org.xmlpull.v1.XmlPullParserException;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Collections;
import java.util.Comparator;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.TimeZone;

public class ZonePicker extends ListFragment {
    private static final String TAG = "ZonePicker";

    private static final String KEY_ID = "id";  // value: String
    private static final String KEY_DISPLAYNAME = "name";  // value: String
    private static final String KEY_GMT = "gmt";  // value: String
    private static final String KEY_TIME_AND_DATE = "time_date";  // value: String
    private static final String KEY_OFFSET = "offset";  // value: int (Integer)
    private static final String XMLTAG_TIMEZONE = "timezone";

    public static final String INPUT_ACTION = "INPUT_ACTION";
    public static final String INPUT_CONTENT = "INPUT_CONTENT";

    private static final int HOURS_1 = 60 * 60000;

    private static int sCurrentOffset = 0;

    private String mQueryString = "";
    private SimpleDateFormat mSimpleDateFormat = new SimpleDateFormat("HH:mm  yyyy-MM-dd");
    private SimpleAdapter mAdapter;
    private boolean mFirstCreate = true;
    private ZoneSelectionListener mListener;
    private IntentFilter mTimeZoneIntentFilter;

    public SimpleAdapter constructTimezoneAdapter(Context context, int layoutId) {
        Xlog.d(TAG, "constructTimezoneAdapter");
        final String[] from = new String[] { KEY_DISPLAYNAME, KEY_GMT,
                KEY_TIME_AND_DATE };
        final int[] to = new int[] { R.id.displayname_text, R.id.gmt_text,
                R.id.time_date_text };
        final List<HashMap<String, Object>> sortedList = getZones(context);
        final MyComparator comparator = new MyComparator(KEY_OFFSET);
        Collections.sort(sortedList, comparator);
        final SimpleAdapter adapter = new SimpleAdapter(context, sortedList,
                layoutId, from, to);
        return adapter;
    }

    @Override
    public void onActivityCreated(Bundle savedInstanseState) {
        super.onActivityCreated(savedInstanseState);
        Xlog.d(TAG, "onActivityCreated");
        mFirstCreate = true;
        final Activity activity = getActivity();
        sCurrentOffset = getCurrentOffset(activity);
        // Sets the adapter
        mAdapter = constructTimezoneAdapter(activity, R.layout.zone_picker);
        setListAdapter(mAdapter);
        final int defaultIndex = getTimeZoneIndex(mAdapter, TimeZone.getDefault());
        if (defaultIndex >= 0) {
            setSelection(defaultIndex);
        }
        setHasOptionsMenu(true);
        mTimeZoneIntentFilter = new IntentFilter(Intent.ACTION_TIME_TICK);
        mTimeZoneIntentFilter.addAction(INPUT_ACTION);
    }

    @Override
    public void onListItemClick(ListView listView, View v, int position, long id) {
        final Map<?, ?> map = (Map<?, ?>)listView.getItemAtPosition(position);
        final String tzId = (String) map.get(KEY_ID);
        // Update the system timezone value
        final Activity activity = getActivity();
        final AlarmManager alarm = (AlarmManager) activity.getSystemService(Context.ALARM_SERVICE);        
        alarm.setTimeZone(tzId);        
        final TimeZone tz = TimeZone.getTimeZone(tzId);
        if (mListener != null) {
            mListener.onZoneSelected(tz);
        } else {
            getActivity().onBackPressed();
        }
    }

    private final BroadcastReceiver mTimeReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            //Log.i(TAG, "[onReceive] intent:" + intent);
            if (intent.getAction().equals(Intent.ACTION_TIME_TICK)) {
                 // Sets the adapter
                if (mAdapter != null) {
                    int position = getListView().getFirstVisiblePosition();
                    Activity activity = getActivity();
                    mAdapter = constructTimezoneAdapter(activity, R.layout.zone_picker);
                    setListAdapter(mAdapter);
                    setSelection(position);
                }
            } else if (intent.getAction().equals(INPUT_ACTION)) {
                String str = intent.getStringExtra(INPUT_CONTENT);
                Log.i(TAG, str);
                mQueryString = str;
                Activity activity = getActivity();
                mAdapter = constructTimezoneAdapter(activity, R.layout.zone_picker);
                setListAdapter(mAdapter);
            }
        }
    };
    @Override
    public void onResume() {
        super.onResume();
        Log.i(TAG, "onResume");
        final Activity activity = getActivity();
        activity.registerReceiver(mTimeReceiver, mTimeZoneIntentFilter);
        if (!mFirstCreate) {
            mAdapter = constructTimezoneAdapter(activity, R.layout.zone_picker);
            setListAdapter(mAdapter);
        }
        mFirstCreate = false;
    }

    @Override
    public void onPause() {
        super.onPause();
        final Activity activity = getActivity();
        activity.unregisterReceiver(mTimeReceiver);
    }

    public static int getTimeZoneIndex(SimpleAdapter adapter, TimeZone tz) {
        final String defaultId = tz.getID();
        final int listSize = adapter.getCount();
        for (int i = 0; i < listSize; i++) {
            final HashMap<?,?> map = (HashMap<?,?>)adapter.getItem(i);
            final String id = (String)map.get(KEY_ID);
            if (defaultId.equals(id)) {
                // If current timezone is in this list, move focus to it
                return i;
            }
        }
        return -1;
    }

    private static int getCurrentOffset(Context context) {
        //final List<HashMap<String, Object>> myData = new ArrayList<HashMap<String, Object>>();
        final long date = Calendar.getInstance().getTimeInMillis();
        /// M: @ {
        int currentOffset = 0;
        final String defaultId = TimeZone.getDefault().getID();
        /// @ }
        try {
            XmlResourceParser xrp = context.getResources().getXml(R.xml.timezones);
            while (xrp.next() != XmlResourceParser.START_TAG) {
                continue;
            }
            xrp.next();
            while (xrp.getEventType() != XmlResourceParser.END_TAG) {
                while (xrp.getEventType() != XmlResourceParser.START_TAG) {
                    if (xrp.getEventType() == XmlResourceParser.END_DOCUMENT) {
                        return currentOffset;
                    }
                    xrp.next();
                }
                if (xrp.getName().equals(XMLTAG_TIMEZONE)) {
                    String id = xrp.getAttributeValue(0);
                    //String displayName = xrp.nextText();
                    /// M: @ {
                    if (defaultId.equals(id)) {
                        final TimeZone currentTimeZone  = TimeZone.getTimeZone(id);
                        currentOffset = currentTimeZone.getOffset(date);
                        //sDefaultTimeZone = TimeZone.getTimeZone(id);
                        //Log.i(TAG, "currentOffset:" + currentOffset);
                        return currentOffset;
                    }
                    /// @ }
                    //addItem(myData, id, displayName, date);
                }
                while (xrp.getEventType() != XmlResourceParser.END_TAG) {
                    xrp.next();
                }
                xrp.next();
            }
            xrp.close();
        } catch (XmlPullParserException xppe) {
            Log.e(TAG, "Ill-formatted timezones.xml file");
        } catch (java.io.IOException ioe) {
            Log.e(TAG, "Unable to read timezones.xml file");
        }
        return currentOffset;
    }

    private List<HashMap<String, Object>> getZones(Context context) {
        final List<HashMap<String, Object>> myData = new ArrayList<HashMap<String, Object>>();
        final long date = Calendar.getInstance().getTimeInMillis();
        try {
            XmlResourceParser xrp = context.getResources().getXml(R.xml.timezones);
            while (xrp.next() != XmlResourceParser.START_TAG) {
                continue;
            }
            xrp.next();
            while (xrp.getEventType() != XmlResourceParser.END_TAG) {
                while (xrp.getEventType() != XmlResourceParser.START_TAG) {
                    if (xrp.getEventType() == XmlResourceParser.END_DOCUMENT) {
                        return myData;
                    }
                    xrp.next();
                }
                if (xrp.getName().equals(XMLTAG_TIMEZONE)) {
                    String id = xrp.getAttributeValue(0);
                    String displayName = xrp.nextText();
                    
                     /// M: @ {
                    if (mQueryString == null || mQueryString.equals("")) { // No query string in SearchView.
                        addItem(myData, id, displayName, date);
                    } else { // Has query string in SearchView:
                        String tempName = displayName.toLowerCase();
                        String tempQuery = mQueryString.toLowerCase();
                        if (tempName.contains(tempQuery)) { // Only add item which is contained by displayName
                            addItem(myData, id, displayName, date);
                        }
                    }
                    /// @ }
                }
                while (xrp.getEventType() != XmlResourceParser.END_TAG) {
                    xrp.next();
                }
                xrp.next();
            }
            xrp.close();
        } catch (XmlPullParserException xppe) {
            Log.e(TAG, "Ill-formatted timezones.xml file");
        } catch (java.io.IOException ioe) {
            Log.e(TAG, "Unable to read timezones.xml file");
        }
        return myData;
    }

    private void addItem(
            List<HashMap<String, Object>> myData, String id, String displayName, long date) {
        final HashMap<String, Object> map = new HashMap<String, Object>();
        map.put(KEY_ID, id);
        map.put(KEY_DISPLAYNAME, displayName);
        final TimeZone tz = TimeZone.getTimeZone(id);
        final int offset = tz.getOffset(date);
        final int p = Math.abs(offset);
        final StringBuilder name = new StringBuilder();
        name.append("GMT");

        if (offset < 0) {
            name.append('-');
        } else {
            name.append('+');
        }
        int hour = p / (HOURS_1);
        final StringBuilder hourStr = new StringBuilder();
        if (hour < 10) {
            hourStr.append("0");
        } 
        hourStr.append(hour);
        name.append(hourStr);
        name.append(':');
        int min = p / 60000;
        min %= 60;
        if (min < 10) {
            name.append('0');
        }
        name.append(min);
        map.put(KEY_GMT, name.toString());
        map.put(KEY_OFFSET, offset);
        Date dt = new Date(date + offset - sCurrentOffset);
        String dateTime = mSimpleDateFormat.format(dt);  
        map.put(KEY_TIME_AND_DATE, String.valueOf(dateTime));
        myData.add(map);
    }

    private static class MyComparator implements Comparator<HashMap<?, ?>> {
        private String mSortingKey;
        public MyComparator(String sortingKey) {
            mSortingKey = sortingKey;
        }

        public int compare(HashMap<?, ?> map1, HashMap<?, ?> map2) {
            Object value1 = map1.get(mSortingKey);
            Object value2 = map2.get(mSortingKey);
            // This should never happen, but just in-case, put non-comparable items at the end.
            if (!isComparable(value1)) {
                return isComparable(value2) ? 1 : 0;
            } else if (!isComparable(value2)) {
                return -1;
            }
            return ((Comparable) value1).compareTo(value2);
        }

        private boolean isComparable(Object value) {
            return (value != null) && (value instanceof Comparable); 
        }
    }

    public interface ZoneSelectionListener {
        // You can add any argument if you really need it...
        void onZoneSelected(TimeZone tz);
    }
}
