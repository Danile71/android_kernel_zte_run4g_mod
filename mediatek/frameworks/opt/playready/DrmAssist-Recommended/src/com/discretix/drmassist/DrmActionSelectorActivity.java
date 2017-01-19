package com.discretix.drmassist;

import java.util.List;

import android.app.ListActivity;
import android.net.Uri;
import android.os.Bundle;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.Toast;

import com.discretix.drmactions.DrmRigistry;

/**
 * DRM action selection activity
 *
 */
public class DrmActionSelectorActivity extends ListActivity {
	private String drmFileName = null;
	private ArrayAdapter<DrmActionBase> adapter;
	static{
		DrmRigistry.register();
	}
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		drmFileName = (String) getIntent().getExtras().get("path");
		setTitle(Uri.parse(drmFileName).getLastPathSegment());
		List<DrmActionBase> drmActions = DrmActionStore.getInstance().getActions(getFileExtension(drmFileName),drmFileName);
		if (null == drmActions){
			
			Toast.makeText(this, "Current file is not supported", Toast.LENGTH_LONG).show();
			finish();
			return;
		}
		
		adapter = new ArrayAdapter<DrmActionBase>(this,
											  android.R.layout.simple_list_item_1,
											  drmActions);
		setListAdapter(adapter);
	}
	
	@Override
	protected void onListItemClick(ListView l, View v, int position, long id) {
		DrmActionBase selectedAction = adapter.getItem(position);
		selectedAction.run(this, drmFileName, getFileExtension(drmFileName));
	}

	private String getFileExtension(String fullFileName) {

	    String separator = System.getProperty("file.separator");
	    String fileName;
	    String extension = null;

	    // Remove the path upto the filename.
	    int lastSeparatorIndex = fullFileName.lastIndexOf(separator);
	    if (lastSeparatorIndex == -1) {
	        fileName = fullFileName;
	    } else {
	        fileName = fullFileName.substring(lastSeparatorIndex + 1);
	    }

	    // Remove the extension.
	    int extensionIndex = fileName.lastIndexOf(".");
	    if (extensionIndex != -1)
	    	extension = fileName.substring(extensionIndex+1,fileName.length());

	    return (null == extension) ? null : extension.toLowerCase();
	}

	
}
