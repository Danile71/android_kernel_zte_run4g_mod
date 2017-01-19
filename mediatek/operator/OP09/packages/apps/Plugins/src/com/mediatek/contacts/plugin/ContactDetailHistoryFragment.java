package com.mediatek.contacts.plugin;

import android.app.Fragment;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.net.Uri.Builder;
import android.os.Bundle;
import android.provider.CallLog.Calls;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.RawContacts;
import android.telephony.PhoneNumberUtils;
import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

import com.mediatek.op09.plugin.R;

import com.mediatek.phone.SIMInfoWrapper;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;

import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;

public class ContactDetailHistoryFragment extends Fragment {
    private static final String TAG = "ContactDetailHistoryFragment";

    private static final int HISTORY_MMS = 0;
    private static final int HISTORY_CALLLOG = 1;
    private static final int HISTORY_LIST_SEPERATOR = 2;
    private static final int HISTORY_EMPTY = 3;

    private static final int HISTORY_VIEW_COUNT = 4;

    public static final int MMS_THREAD_ID = 1;
    public static final int MMS_NUMBER = 2;
    public static final int MMS_DATE = 3;
    public static final int MMS_BODY = 4;
    public static final int MMS_SEND_RECEIVE_TYPE = 5;
    public static final int MMS_MMS_SMS = 6;

    private Uri mLookupUri;
    private List<String> mPhoneNumbers = new ArrayList<String>();
    private long mContactId;

    private Calendar mCalendar = Calendar.getInstance();

    private LayoutInflater mInflater;
    private Context mPluginContext;
    private View mView;

    private ListView mHistoryListView;
    private List<HistoryItem> mHistoryDataList = new ArrayList<HistoryItem>();
    private HistoryListAdapter mHistoryListAdapter;
    private TextView mEmptyView;

    /**
     * add construct method for history fragment
     */
//    public ContactDetailHistoryFragment(){
//        
//    }

    public ContactDetailHistoryFragment(Context pluginContext){
        this.mPluginContext = pluginContext;
//        String path = mPluginContext.getPackageCodePath();
//        setClassPath(path);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
//        if (mPluginContext == null) {
//            return;
//        }
        mInflater = (LayoutInflater) mPluginContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
//        if (mPluginContext == null) {
//            return null;
//        }
        mView = mInflater.inflate(R.layout.contact_detail_history_list, container, false);
        mHistoryListView = (ListView)mView.findViewById(R.id.contact_history_list);
        mEmptyView = (TextView)mView.findViewById(R.id.empty);
        mEmptyView.setText(R.string.contact_detail_history_empty);
        mEmptyView.setVisibility(View.GONE);

        mHistoryListAdapter = new HistoryListAdapter();
        return mView;
    }

    private List<HistoryItem> getHistoryDataList() {
        List<HistoryItem> historyList = new ArrayList<HistoryItem>();
        
        getMmsHistoryList(historyList);
        getCalllogHistroyList(historyList);
        
        return historyList;
    }

    private void getCalllogHistroyList(List<HistoryItem> historyList) {
        HistoryItem seperator = new HistoryItem();
        seperator.setFromApp(HISTORY_LIST_SEPERATOR);
        seperator.setAppName(mPluginContext.getResources().getString(
                R.string.contact_detail_history_calllog));
        
        Uri queryUri = Uri.parse("content://call_log/callsjoindataview");
        String selection = getSelection();
        Cursor callCursor = mPluginContext.getContentResolver().query(
                queryUri,
                new String[] { Calls.NUMBER, Calls.DATE, Calls.TYPE,
                        Calls.DURATION, Calls.SIM_ID }, selection, null, Calls.DATE);

        if (callCursor != null) {
            Log.i(TAG, "getCalllogHistroyList ==> callCursor.getCount(): " + callCursor.getCount());
            if (callCursor.getCount() > 0) {
                historyList.add(seperator);
                callCursor.moveToLast();
                do {
                    String number = callCursor.getString(0);
                    String time = callCursor.getString(1);
                    int callType = callCursor.getInt(2);
                    int simId = callCursor.getInt(4);
                    HistoryItem call = new HistoryItem();
                    call.setFromApp(HISTORY_CALLLOG);
                    call.setCallLogDate(time);
                    call.setCallLogPhoneNumber(number);
                    call.setCallDuration(formatDuration(callCursor.getLong(3)));
                    call.setCallLogPhoneIcon(callType);
                    call.setCallLogSimIcon(simId);
                    historyList.add(call);
                } while (callCursor.moveToPrevious());
            }
            // else {
            // HistoryItem empty = new HistoryItem();
            // empty.setEmptyText(getResources().getString(R.string.contact_detail_history_empty_calllog));
            // empty.setFromApp(HISTORY_EMPTY);
            // historyList.add(empty);
            // }
        }
        
        callCursor.close();

    }

    private String getSelection() {
        if (mPhoneNumbers != null) {
            return RawContacts.CONTACT_ID + " = " + this.mContactId;
        }

        return null;
    }

    private String formatDuration(long elapsedSeconds) {
        long minutes = 0;
        long seconds = 0;

        if (elapsedSeconds >= 60) {
            minutes = elapsedSeconds / 60;
            elapsedSeconds -= minutes * 60;
        }
        seconds = elapsedSeconds;
        String minute = "";
        String second = "";
        if (seconds < 10) {
            second = "0" + seconds;
        } else {
            second = String.valueOf(seconds);
        }
        if (minutes < 10) {
            minute = "0" + minutes;
        } else {
            minute = String.valueOf(minutes);
        }

        return mPluginContext.getResources().getString(R.string.contact_detail_history_duration) + " " + minute + ":" + second;
    }

    private void getMmsHistoryList(List<HistoryItem> historyList) {
        HistoryItem seperator = new HistoryItem();
        seperator.setFromApp(HISTORY_LIST_SEPERATOR);
        seperator.setAppName(mPluginContext.getResources()
                .getString(R.string.contact_detail_history_mms));
        
        Uri queryUri = Uri.parse("content://mms-sms/conversations/history");
        StringBuilder numbers = getMmsSelection();
        Builder builder = queryUri.buildUpon().appendQueryParameter(
                "numbers", numbers.toString());
        Cursor callCursor = mPluginContext.getContentResolver().query(builder.build(),
                null, null, null, null);

        if (callCursor != null) {
            Log.i(TAG, "getMmsHistoryList ==> callCursor.getCount(): " + callCursor.getCount());
            if (callCursor.getCount() > 0) {
                historyList.add(seperator);
                while (callCursor.moveToNext()) {
                    String number = callCursor.getString(MMS_NUMBER);
                    if (TextUtils.isEmpty(number)) {
                        continue;
                    }
                    String time = callCursor.getString(MMS_DATE);
                    
                    HistoryItem historyMMS = new HistoryItem();
                    historyMMS.setMmsThreadId(callCursor.getInt(MMS_THREAD_ID));
                    historyMMS.setMmsDate(time);
                    historyMMS.setMmsPhoneNumber(number);
                    historyMMS.setMmsContent(callCursor.getString(MMS_BODY));
                    historyMMS.setMmsSentReceive(callCursor.getInt(MMS_SEND_RECEIVE_TYPE));
                    historyMMS.setMmsMMSsms(callCursor.getInt(MMS_MMS_SMS));
                    historyMMS.setFromApp(HISTORY_MMS);
                    historyList.add(historyMMS);
                }
            }
            // else {
            // HistoryItem empty = new HistoryItem();
            // empty.setEmptyText(getResources().getString(R.string.contact_detail_history_empty_message));
            // empty.setFromApp(HISTORY_EMPTY);
            // historyList.add(empty);
            // }
        }
        
        callCursor.close();
    }

    private StringBuilder getMmsSelection() {
        StringBuilder numbers = new StringBuilder("");
        for (int i = 0; i < mPhoneNumbers.size(); i++) {
            String phoneNumber = mPhoneNumbers.get(i);
            phoneNumber = removeAllSimbol(phoneNumber);
            numbers.append(phoneNumber);
            if (i != mPhoneNumbers.size() - 1) {
                numbers.append(":");
            }
        }
        return numbers;
    }

    private String removeAllSimbol(String phoneNumber) {
        StringBuilder sb = new StringBuilder("");
        for (int i = 0; i < phoneNumber.length(); i++) {
            char c = phoneNumber.charAt(i);
            if (PhoneNumberUtils.isISODigit(c)) {
                sb.append(c);
            }
        }
        return sb.toString();
    }

    @Override
    public void onResume() {
        super.onResume();
        Log.i(TAG, " == onResume() == mPluginContext : " + mPluginContext);
        if (mPluginContext == null) {
            return;
        }
        mHistoryDataList = getHistoryDataList();
        if (mHistoryDataList.size() == 0) {
            mHistoryListView.setVisibility(View.GONE);
            mEmptyView.setVisibility(View.VISIBLE);
        } else {
            mHistoryListView.setVisibility(View.VISIBLE);
            mEmptyView.setVisibility(View.GONE);
        }
        
        if (mHistoryListView.getAdapter() != null) {
            mHistoryListAdapter.notifyDataSetChanged();
        } else {
            mHistoryListView.setAdapter(mHistoryListAdapter);
        }

    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
    }
    
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                // The home icon on the action bar is pressed
                    // "UP" icon press -- should be treated as "back".
                getActivity().onBackPressed();
                return true;
            default:
                break;
        }
        return false;
    }
    
    public void setData(Uri lookupUri, long contactId, List<String> mPhoneNumbers) {
        Log.i(TAG, " lookupUri : " + lookupUri + " | contactId: " + contactId + " | mPhoneNumbers: " + mPhoneNumbers);
        this.mPhoneNumbers = mPhoneNumbers;
        this.mContactId = contactId;
        if (mPhoneNumbers == null) {
            return;
        }
        mLookupUri = lookupUri;

        mHistoryDataList = getHistoryDataList();
        
        if (mHistoryDataList.size() == 0) {
            mHistoryListView.setVisibility(View.GONE);
            mEmptyView.setVisibility(View.VISIBLE);
        } else {
            mHistoryListView.setVisibility(View.VISIBLE);
            mEmptyView.setVisibility(View.GONE);
        }
        
        if (mHistoryListView.getAdapter() != null) {
            mHistoryListAdapter.notifyDataSetChanged();
        } else {
            mHistoryListView.setAdapter(mHistoryListAdapter);
        }
    }

    private class HistoryItem {
        private int mFromApp;
        private String mAppName;
        private String mEmptyText;

        public String getEmptyText() {
            return mEmptyText;
        }
        public void setEmptyText(String emptyText) {
            this.mEmptyText = emptyText;
        }

        private String mCallLogDate;
        private String mCallLogPhoneNumber;
        private int mCallLogPhoneIcon;
        private int mCallLogSimIcon;
        private String mCallDuration;

        private int mMmsThreadId;
        private String mMmsDate;
        private String mMmsContent;
        private String mMmsPhoneNumber;
        private int mMmsSentReceive;
        private int mMmsMMSsms;

        public int getMmsThreadId() {
            return mMmsThreadId;
        }
        public void setMmsThreadId(int mmsThreadId) {
            this.mMmsThreadId = mmsThreadId;
        }
        public int getMmsSentReceive() {
            return mMmsSentReceive;
        }
        public void setMmsSentReceive(int mmsSentReceive) {
            this.mMmsSentReceive = mmsSentReceive;
        }
        public int getMmsMMSsms() {
            return mMmsMMSsms;
        }
        public void setMmsMMSsms(int mmsMMSsms) {
            this.mMmsMMSsms = mmsMMSsms;
        }
        public int getCallLogSimIcon() {
            return mCallLogSimIcon;
        }
        public void setCallLogSimIcon(int callLogSimIcon) {
            this.mCallLogSimIcon = callLogSimIcon;
        }
        public String getAppName() {
            return mAppName;
        }
        public void setAppName(String appName) {
            this.mAppName = appName;
        }
        public String getMmsPhoneNumber() {
            return mMmsPhoneNumber;
        }
        public void setMmsPhoneNumber(String mmsPhoneNumber) {
            this.mMmsPhoneNumber = mmsPhoneNumber;
        }
        public int getFromApp() {
            return mFromApp;
        }
        public void setFromApp(int fromApp) {
            this.mFromApp = fromApp;
        }
        public String getCallLogDate() {
            return mCallLogDate;
        }
        public void setCallLogDate(String callLogDate) {
            this.mCallLogDate = formatDate(callLogDate);
        }
        public String getCallLogPhoneNumber() {
            return mCallLogPhoneNumber;
        }
        public void setCallLogPhoneNumber(String callLogPhoneNumber) {
            this.mCallLogPhoneNumber = callLogPhoneNumber;
        }
        public int getCallLogPhoneIcon() {
            return mCallLogPhoneIcon;
        }
        public void setCallLogPhoneIcon(int callLogPhoneIcon) {
            this.mCallLogPhoneIcon = callLogPhoneIcon;
        }
        public String getCallDuration() {
            return mCallDuration;
        }
        public void setCallDuration(String callDuration) {
            this.mCallDuration = callDuration;
        }
        public String getMmsDate() {
            return mMmsDate;
        }
        public void setMmsDate(String mmsDate) {
            this.mMmsDate = formatDate(mmsDate);
        }
        public String getMmsContent() {
            return mMmsContent;
        }
        public void setMmsContent(String mmsContent) {
            this.mMmsContent = mmsContent;
        }

        private String formatDate(String inputTime) {
            mCalendar.setTimeInMillis(Long.parseLong(inputTime));
            String result = mCalendar.get(Calendar.YEAR) + "/"
                    + (mCalendar.get(Calendar.MONTH) + 1) + "/"
                    + mCalendar.get(Calendar.DATE) + ", "
                    + mCalendar.get(Calendar.HOUR) + ":"
                    + mCalendar.get(Calendar.MINUTE);
            if (mCalendar.get(Calendar.AM_PM) == Calendar.AM) {
                result = result + "AM";
            } else if (mCalendar.get(Calendar.AM_PM) == Calendar.PM) {
                result = result + "PM";
            }

            return result;
        }

    }

    private class HistoryListAdapter extends BaseAdapter {

        @Override
        public int getCount() {
            return mHistoryDataList.size();
        }

        @Override
        public Object getItem(int position) {
            if (mHistoryDataList != null) {
                return mHistoryDataList.get(position);
            }
            return null;
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            switch (getItemViewType(position)) {
            case HISTORY_CALLLOG:
                return getCalllogHistoryItemView(position, convertView, parent);
            case HISTORY_MMS:
                return getMmsHistoryItemView(position, convertView, parent);
            case HISTORY_LIST_SEPERATOR:
                return getMmsHistoryListSeperatorView(position, convertView, parent);
            case HISTORY_EMPTY:
                return getEmptyHistoryView(position, convertView, parent);
                
            default:
                throw new IllegalStateException("Invalid view type ID "
                        + getItemViewType(position));
            }
        }
        
        private View getEmptyHistoryView(int position, View convertView,
                ViewGroup parent) {
            final HistoryItem emptyItem = mHistoryDataList.get(position);
            final View result = mInflater.inflate(R.layout.contact_detail_history_list_item_empty, parent, false);
            TextView emptyText = (TextView)result.findViewById(R.id.emptytext);
            
            emptyText.setText(emptyItem.getEmptyText());
            return result;
        }

        private View getMmsHistoryListSeperatorView(int position,
                View convertView, ViewGroup parent) {
            final HistoryItem mmsItem = mHistoryDataList.get(position);
            final View result = mInflater.inflate(R.layout.contact_detail_history_list_item_seperator, parent, false);
            TextView appNameView = (TextView)result.findViewById(R.id.app_name);
            appNameView.setText(mmsItem.getAppName());
            
            return result;
        }

        private View getMmsHistoryItemView(int position, View convertView,
                ViewGroup parent) {
            final HistoryItem mmsItem = mHistoryDataList.get(position);
            final View result = mInflater.inflate(R.layout.contact_detail_history_list_item_mms, parent, false);
            result.setBackground(mPluginContext.getResources().getDrawable(
                    R.drawable.group_list_item_background));
           
            TextView receiveFrom = (TextView)result.findViewById(R.id.received_from);
            TextView receivedTime = (TextView)result.findViewById(R.id.received_time);
            TextView mmsContent = (TextView)result.findViewById(R.id.mms_content);

            if (mmsItem.getMmsSentReceive() == 1) {
                receiveFrom.setText(mPluginContext.getResources().getString(
                        R.string.contact_detail_history_mms_received_from,
                        mmsItem.getMmsPhoneNumber()));
                receivedTime.setText(mPluginContext.getResources().getString(
                        R.string.contact_detail_history_mms_received_time,
                        mmsItem.getMmsDate()));
            } else if (mmsItem.getMmsSentReceive() == 2) {
                receiveFrom.setText(mPluginContext.getResources().getString(
                        R.string.contact_detail_history_mms_sent_to,
                        mmsItem.getMmsPhoneNumber()));
                receivedTime.setText(mPluginContext.getResources().getString(
                        R.string.contact_detail_history_mms_sent_time,
                        mmsItem.getMmsDate()));
            }
            
            if (mPhoneNumbers.size() == 1) {
                receiveFrom.setVisibility(View.GONE);
            }
            
            if (mmsItem.getMmsMMSsms() == 1) {
                mmsContent.setText(mmsItem.getMmsContent());
            } else if (mmsItem.getMmsMMSsms() == 2) {
                mmsContent.setText(mPluginContext.getResources().getString(
                        R.string.contact_detail_history_ismms));
            }
            
            
            result.setOnClickListener(new OnClickListener() {
                
                @Override
                public void onClick(View v) {
                    // goes to the MMS thread list page
                    Intent mmsIntent = new Intent();
                    mmsIntent.setAction("com.android.mms.ui.ComposeMessageActivity");
                    mmsIntent.putExtra("thread_id", (long)(mmsItem.getMmsThreadId()));
                    startActivity(mmsIntent);
                }
            });
            
            return result;
        }

        private View getCalllogHistoryItemView(int position, View convertView,
                ViewGroup parent) {
            final HistoryItem callItem = mHistoryDataList.get(position);
            final View result = mInflater.inflate(R.layout.contact_detail_history_list_item_calllog, parent, false);
            ImageView phoneIcon = (ImageView)result.findViewById(R.id.phone_icon);
            TextView callDate = (TextView)result.findViewById(R.id.call_date);
            TextView phoneNumber = (TextView)result.findViewById(R.id.phone_number);
            TextView duration = (TextView)result.findViewById(R.id.duration);
            TextView simIcon = (TextView)result.findViewById(R.id.sim_icon);
            
            switch (callItem.getCallLogPhoneIcon()) {
            case Calls.INCOMING_TYPE:
                phoneIcon.setImageResource(R.drawable.ic_btn_incoming);
                break;
            case Calls.OUTGOING_TYPE:
                phoneIcon.setImageResource(R.drawable.ic_btn_outgoing);
                break;
            case Calls.MISSED_TYPE:
                phoneIcon.setImageResource(R.drawable.ic_btn_missed);
                break;
            case Calls.AUTOREJECTED_TYPE:
                phoneIcon.setImageResource(R.drawable.ic_call_autorejected_holo_dark);
                break;

            default:
                break;
            }

            phoneNumber.setText(callItem.getCallLogPhoneNumber());
            duration.setText(callItem.getCallDuration());
            callDate.setText(callItem.getCallLogDate());

            SimInfoRecord simInfo = SIMInfoWrapper.getDefault().getSimInfoById(callItem.getCallLogSimIcon());
            if (simInfo == null) {
                return result;
            }
            int backgroudRes = simInfo.mSimBackgroundLightRes;
            simIcon.setBackground(mPluginContext.getResources().getDrawable(backgroudRes));
            simIcon.setText(simInfo.mDisplayName);

            result.setOnClickListener(new OnClickListener() {

                @Override
                public void onClick(View v) {
                    // Toast.makeText(getActivity().getApplicationContext(),
                    // callItem.getCallLogPhoneNumber(),
                    // Toast.LENGTH_SHORT).show();
                }
            });

            return result;
        }

        @Override
        public int getItemViewType(int position) {
            return mHistoryDataList.get(position).getFromApp();
        }
        
        @Override
        public int getViewTypeCount() {
            return HISTORY_VIEW_COUNT;
        }
    }

    public boolean handleKeyDown(int keyCode) {
        return false;
    }
//
//    public void setPluginContext(Context pluginContext) {
//        this.mPluginContext = pluginContext;
//    }


    
}
