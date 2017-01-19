package com.mediatek.rcse.ipcall;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.gsma.joyn.JoynServiceException;
import org.gsma.joyn.JoynServiceListener;
import org.gsma.joyn.ipcall.IPCall;
import org.gsma.joyn.ipcall.IPCallService;
import org.gsma.joyn.ipcall.NewIPCallListener;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.RemoteException;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.TextView;

import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.R;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * List of current IP calls
 */
public class IPCallSessionsList extends Activity implements JoynServiceListener {
	/**
	 * UI handler
	 */
	private Handler handler = new Handler();
	
	/**
	 * layout of activity
	 */
	private ListView sessionsList;
	
	/**
	 * refresh of sessions list when api is connected
	 */
	private Boolean refreshWhenApiConnected = false ;
	
	/**
	 * Logger
	 */
	private Logger logger = Logger.getLogger(IPCallSessionsList.class.getName());
	
	public NewIpcallListener imsEventListener = null;
	
	
	 /* *****************************************
     *                Activity
     ***************************************** */
    @Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		if (logger.isActivated()) {
			logger.info("onCreate()");
		}
		// Set layout
		setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
		setContentView(R.layout.ipcall_sessions_list);

		// Set title
		setTitle(R.string.menu_ipcall_sessions);
		
		imsEventListener = new NewIpcallListener();

		sessionsList = (ListView) findViewById(R.id.sessions_list);

		RcsSettings.createInstance(getApplicationContext());

		if (IPCallSessionsData.getInstance().isCallApiConnected) {
			// remove "old" listeners and set "new" listeners
			IPCallSessionsData.getInstance().imsEventListener = imsEventListener;
			try {
				IPCallSessionsData.getInstance().callApi
						.removeNewIPCallListener(IPCallSessionsData.getInstance().imsEventListener);
			} catch (JoynServiceException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			//IPCallSessionsData.getInstance().callApi.addApiEventListener(this);
			IPCallSessionsData.getInstance().callApiListener = this;
			try {
				IPCallSessionsData.getInstance().callApi.addNewIPCallListener(imsEventListener);
			} catch (JoynServiceException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			
		}
		else { // connect API - get sessions when connected		
			refreshWhenApiConnected = true;

			// instantiate callApi if null
			if (IPCallSessionsData.getInstance().callApi == null) { 
				IPCallSessionsData.getInstance().callApi = new IPCallService(getApplicationContext(), this);
			} 
			else { // callApi already exists remove "old" listeners
				
				try {
					IPCallSessionsData.getInstance().callApi
							.removeNewIPCallListener(IPCallSessionsData.getInstance().imsEventListener);
				} catch (JoynServiceException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}			
			//set "new" listeners
			IPCallSessionsData.getInstance().imsEventListener = imsEventListener;
			IPCallSessionsData.getInstance().callApiListener = this;
			try {
				IPCallSessionsData.getInstance().callApi.addNewIPCallListener(imsEventListener);
			} catch (JoynServiceException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			
			//connect api
			IPCallSessionsData.getInstance().callApi.connect();
		}
	
	}

   
    @Override
    public void onResume() {
    	super.onResume();
    	
    	if (logger.isActivated()) {
			logger.info("onResume()");
		}
    	
    	if (IPCallSessionsData.getInstance().isCallApiConnected) {
    		refreshSessionsList();
    	}	
    	else {
    		refreshWhenApiConnected = true;
    	} 	
    }
    
    
    @Override
    public void onDestroy() {
    	super.onDestroy();  	
    	if (logger.isActivated()) {
			logger.info("onDestroy()");
		}
    }
    
    public class NewIpcallListener extends NewIPCallListener
    {

		@Override
		public void onNewIPCall(String callId) {
			// TODO Auto-generated method stub
			
		}
    	
    }

    
    public void refreshSessionsList(){   	
    	
    	Thread thread = new Thread() {
			public void run() {
				List<String> sessionsLabel = new ArrayList();
				
				try {
					IPCallSessionsData.getInstance().sessions = new ArrayList<IPCall> (IPCallSessionsData.getInstance().callApi
							.getIPCalls());
					if (logger.isActivated()) {
						logger.info("sessions list initialized");
					}
				} catch (JoynServiceException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				} 

				handler.post(new Runnable() {
					public void run() {
				
						

						if (IPCallSessionsData.getInstance().sessions.isEmpty()) {
							Utils.showMessage(IPCallSessionsList.this,
									getString(R.string.label_list_empty));
						} else {
							sessionsList.setAdapter(new SessionsListAdapter(
									IPCallSessionsList.this,
									R.layout.ipcall_sessions_list_item,
									IPCallSessionsData.getInstance().sessions){
								
							});
							sessionsList
									.setOnItemClickListener(sessionsListListener);
						}
					}
				});

			}
		};
		thread.start();
    }

    
	public class SessionsListAdapter extends ArrayAdapter<IPCall> {

		Context context;
		int layoutResourceId;
		ArrayList<IPCall> sessionsList;

		// we use the constructor allowing to provide a List of objects for the
		// data
		// to be binded.
		public SessionsListAdapter(Context context, int layoutResourceId,
				ArrayList<IPCall> objects) {
			super(context, layoutResourceId, objects);
			this.context = context;
			this.layoutResourceId = layoutResourceId;
			this.sessionsList = (ArrayList<IPCall>) objects;
		}

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {

			View row = convertView;
			ViewHolder holder = null;

			if (row == null) {
				LayoutInflater inflater = ((Activity) context)
						.getLayoutInflater();
				row = inflater.inflate(layoutResourceId, parent, false);

				holder = new ViewHolder();
				holder.label = (TextView) row
						.findViewById(R.id.ipcall_sessions_list_item_label);

				row.setTag(holder);
			} else {
				holder = (ViewHolder) row.getTag();
			}

			// get session and session data
			IPCall session = sessionsList.get(position);			
			try {
				try {
					if (logger.isActivated()) {
						logger.info("session.getSessionDirection() ="+session.getDirection());
					}
				} catch (JoynServiceException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				String labelText;
				int direction = 0;
				try {
					direction = session.getDirection();
				} catch (JoynServiceException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				String contact = session.getRemoteContact() ;
				int beginIdx = contact.indexOf("tel");
				int endIdx = contact.indexOf(">", beginIdx);
				if (endIdx == -1) {endIdx = contact.length();}
				String remoteContact = contact.substring(beginIdx, endIdx);
				
				if (direction== IPCallSessionsData.TYPE_OUTGOING_IPCALL){
					labelText = "Outgoing call to:";
				}
				else {labelText = "Incoming call from:";}				
				labelText += remoteContact;
				holder.label.setText(labelText);			
			} catch (Exception e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			return row;
		}
     
        /**
         *
         * Inner holder class for a single row view in the ListView
         *
         */
        class ViewHolder {
            TextView label;

        }     
    }
    
	private OnItemClickListener sessionsListListener = new OnItemClickListener() {
	    @Override
	    public void onItemClick(AdapterView<?> parent, View view, int position,
	            long id) {
	    	if (logger.isActivated()) {logger.debug("onItemClick");	}
	        
	    	// get session
	    	//IBinder  iBinder  = (IBinder) parent.getItemAtPosition(position);
	    	//IPCall session = IIPCall.Stub.asInterface(iBinder);
	    	IPCall  session  =  (IPCall) parent.getItemAtPosition(position);
	        try {	        		        	
	        	//launch IPCallSessionActivity with recover action on sessionId
				try {
					getApplicationContext().startActivity(setIntentRecoverSession(session.getCallId()));
				} catch (JoynServiceException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				IPCallSessionsList.this.finish();
			} catch (Exception e) {
				// TODO Auto-generated catch block 
				e.printStackTrace();
			}
	    }
	};
	
	private Intent setIntentRecoverSession(String id) {
		// Initiate Intent to launch outgoing IP call
		Intent intent = new Intent(getApplicationContext(), IPCallView.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.putExtra("sessionId", id);
        intent.setAction("recover");
        
        return intent;
	}


	@Override
	public void onServiceConnected() {
		if (logger.isActivated()) {
			logger.debug("API, onServiceConnected");
		}
	
		IPCallSessionsData.getInstance().isCallApiConnected = true;

		 if (refreshWhenApiConnected){
			 refreshSessionsList();
			 refreshWhenApiConnected = false;}
		
	}


	@Override
	public void onServiceDisconnected(int error) {
		if (logger.isActivated()) {
			logger.debug("API, onServiceDisconnected");
		}
		IPCallSessionsData.getInstance().isCallApiConnected = false;
		
		String msg = IPCallSessionsList.this.getString(R.string.label_api_disconnected);

		// Service has been disconnected
		Intent intent = new Intent(IPCallSessionsList.this.getApplicationContext(), IPCallView.class);
		intent.setAction("ExitActivity");
		intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		intent.putExtra("messages", msg);
		getApplicationContext().startActivity(intent);
		
	}
	
}
