package com.mediatek.rcse.activities;

import java.util.ArrayList;
import java.util.List;

import org.gsma.joyn.JoynServiceException;
import org.gsma.joyn.chat.ChatService;
import org.gsma.joyn.contacts.ContactsService;


import android.app.ActionBar;
import android.app.Activity;
import android.app.ListActivity;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.provider.ContactsContract;
import android.provider.ContactsContract.PhoneLookup;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.LayoutInflater;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView.AdapterContextMenuInfo;
import android.widget.ArrayAdapter;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.rcse.activities.widgets.AsyncImageView;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.fragments.GroupChatFragment;
import com.mediatek.rcse.mvc.ParticipantInfo;
import com.mediatek.rcse.service.ApiManager;
import com.orangelabs.rcs.R;
import com.orangelabs.rcs.core.ims.service.ContactInfo;
import com.orangelabs.rcs.core.ims.service.im.chat.event.User;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.eab.RichAddressBookData;

public class ParticipantsListActivity extends ListActivity implements OnClickListener {
    
    private ArrayList<String> participantsDisplayName = new ArrayList<String>();
    protected LayoutInflater mInflator = null;
    private View mArea;
    private ListView mBanner;
    private List <ParticipantInfo> participantsInfoList = new ArrayList<ParticipantInfo>();
    private ListArrayAdapter mAdapter;
    private static final String MIMETYPE_RCS_CONTACT = "vnd.android.cursor.item/com.orangelabs.rcs.rcs-status";

    private static final String TAG = "ParticipantsListActivity";
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        try {
            setContentView(R.layout.participantslistactivity_layout);
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        
        ActionBar actionBar = getActionBar();
        actionBar.setDisplayOptions(ActionBar.DISPLAY_SHOW_HOME | ActionBar.DISPLAY_SHOW_CUSTOM);
        LayoutInflater inflater = LayoutInflater.from(getApplicationContext());
        ViewGroup view = (ViewGroup) inflater.inflate(R.layout.participantlist_actionbar_layout, null);       
        TextView textView = (TextView) view.findViewById(R.id.text_title);
        textView.setText("Group participants");
        actionBar.setCustomView(view);

        ListView listView = (ListView) findViewById(R.id.list1);
        Button returnButton = (Button) findViewById(R.id.button1);
        returnButton.setOnClickListener(this);
        registerForContextMenu(listView);
        mAdapter = new ListArrayAdapter(); 
        listView.setAdapter(mAdapter);       
        
        Intent intent = getIntent();  
        participantsInfoList = intent.getParcelableArrayListExtra("participantsinfo");
         
    }
    
    @Override
	public void onCreateContextMenu(ContextMenu menu, View v,
			ContextMenuInfo menuInfo) {
		super.onCreateContextMenu(menu, v, menuInfo);
		menu.setHeaderTitle(getString(R.string.vcard_context_title));
		MenuInflater inflater = getMenuInflater();
		inflater.inflate(R.menu.vcard_context_menu, menu);
	}

    
	@Override
	public boolean onContextItemSelected(MenuItem item) {
		AdapterContextMenuInfo info = (AdapterContextMenuInfo) item.getMenuInfo();
		int position = info.position;
		switch (item.getItemId()) {
		    case R.id.vcard:
		        //save vcard
		    	try{
			    	ParticipantInfo participantInfo = participantsInfoList.get(position);
			    	String contact = participantInfo.getContact();
			    	Logger.d(TAG, "get vcard menu item clicked contact =" + contact + "position =" +position);
			    	Uri contactUri = getContactUri(contact);
			    	Logger.d(TAG, "get vcard menu item clicked uri =" + contactUri);
			    	ApiManager instance = ApiManager.getInstance();
			    	if (instance == null) {
			            Logger.i(TAG, "ApiManager instance is null");
			        }
			    	if(instance != null)
			        {
				        ContactsService contacsApi = instance.getContactsApi();
				        if (contacsApi == null) {
				            Logger.d(TAG, "chatApi instance is null");
				        }
				        if(contacsApi != null)
				        {
				        	try{
			                String filename = contacsApi.getVCard(contactUri);
			                Logger.d(TAG, "filename =" + filename);
			                String delims = "[/]+";
			                String[] tokens = filename.split(delims);
			                Toast.makeText(getApplicationContext(), "Vcard saved with name = "+ tokens[tokens.length-1],
                                    Toast.LENGTH_LONG).show();
				        }
				        	catch (JoynServiceException e) {
								e.printStackTrace();
							}
				        }
			        }
		    	}
		    	catch (Exception e) {
					e.printStackTrace();
					Toast.makeText(getApplicationContext(), "Some issue with generating Vcard",
                            Toast.LENGTH_LONG).show();
				}
		        return true;
		    default:
		        return super.onContextItemSelected(item);
	    }
	}


    
    private Uri getContactUri(String contact) {
		
    	String id = fetchContactIdFromPhoneNumber(contact);
    	Logger.d(TAG, "getContactUri, id is =" + id);
        /*String[] projection = {
        		ContactsContract.Data._ID
        };
        
        String selection = "(" + ContactsContract.CommonDataKinds.Phone.DISPLAY_NAME + "=? AND " 
                + ContactsContract.Data.MIMETYPE + "=? )";
        String selection = "(" + ContactsContract.CommonDataKinds.Phone.DISPLAY_NAME + "=?  )";
        String[] selectionArgs = {
                contact
                
        };
        Cursor cur = getApplicationContext().getContentResolver().query(ContactsContract.Contacts.CONTENT_URI, 
                projection, 
                selection, 
                selectionArgs, 
                null);
        Logger.d(TAG, "getContactUri cursor count =" + cur.getCount());
        while (cur.moveToNext()) {
            id = cur.getString(0);
            Logger.d(TAG, "getContactUri id =" + id);
        }
        cur.close();*/
        Uri contactUri = Uri.withAppendedPath(ContactsContract.Contacts.CONTENT_URI, id);
		return contactUri;
	}

    public String fetchContactIdFromPhoneNumber(String phoneNumber) {
        Uri uri = Uri.withAppendedPath(PhoneLookup.CONTENT_FILTER_URI,
            Uri.encode(phoneNumber));
        Cursor cursor = this.getContentResolver().query(uri,
            new String[] { PhoneLookup.DISPLAY_NAME, PhoneLookup._ID },
            null, null, null);

        String contactId = "";
        Logger.d(TAG, "fetchContactIdFromPhoneNumber, cursor count is =" + cursor.getCount());
        if (cursor.moveToFirst()) {
            do {
            contactId = cursor.getString(cursor
                .getColumnIndex(PhoneLookup._ID));
            } while (cursor.moveToNext());
        }

        return contactId;
      }


    
    public class ListArrayAdapter extends BaseAdapter {        
            
            
        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            
            View itemView = convertView;
            if(itemView ==null)
            {   LayoutInflater inflater = LayoutInflater.from(parent.getContext());
                itemView = inflater.inflate(R.layout.participantlist, parent, false);                
            }
            bindView(itemView, participantsInfoList.get(position));           
            return itemView;            
        }

        private void bindView(View itemView, ParticipantInfo info) {

            String contact = info.getContact();
            AsyncImageView avatar = (AsyncImageView) itemView
                    .findViewById(R.id.peer_avatar);
            boolean active = User.STATE_CONNECTED.equals(info.getState());
            avatar.setAsyncContact(contact, !active);
            TextView statusView = (TextView) itemView
                    .findViewById(R.id.participant_status);
            statusView.setText(active ? getString(R.string.group_active)
                    : getString(R.string.group_inactive));
            TextView remoteName = (TextView) itemView
                    .findViewById(R.id.remote_name);
            remoteName.setText(ContactsListManager.getInstance()
                    .getDisplayNameByPhoneNumber(contact));
            /*TextView aliasNameTextView = (TextView) itemView
                   .findViewById(R.id.alias_name);*/
            /*if(RichMessaging.getInstance() == null)
				RichMessaging.createInstance(AndroidFactory.getApplicationContext());
    		String aliasName = "~"+RichMessaging.getInstance().getContactAliasName(contact);
    		if(aliasName != null)
    		{
    			aliasNameTextView.setText(aliasName);
    		}*/
        }



        @Override
        public int getCount() {
            // TODO Auto-generated method stub
            return participantsInfoList.size();
        }

        @Override
        public Object getItem(int position) {
            // TODO Auto-generated method stub
            return participantsInfoList.get(position);
        }

        @Override
        public long getItemId(int position) {
            // TODO Auto-generated method stub
            return 0;
        }
    }


    @Override
    public void onClick(View v) {
        this.finish();
        
    }

}
