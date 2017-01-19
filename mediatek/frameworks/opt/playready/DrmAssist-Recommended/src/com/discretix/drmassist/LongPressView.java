package com.discretix.drmassist;

import java.io.File;

import android.app.ListActivity;
import android.net.Uri;
import android.os.Bundle;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.Toast;

/**
 * DRM action selection activity
 *
 */
public class LongPressView extends ListActivity {
	private String drmFileName = null;
	private ArrayAdapter<String> adapter;
		
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		drmFileName = (String) getIntent().getExtras().get("path");
		setTitle(Uri.parse(drmFileName).getLastPathSegment());
		
		String[] actions = new String[] { "Delete" };
		
		adapter = new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1, actions);
		setListAdapter(adapter);
	}
	
	
	@Override
	protected void onListItemClick(ListView l, View v, int position, long id) {
		if (adapter.getItem(position).equals("Delete")){
			File file = new File(drmFileName);
			if (deleteDir(file)){
				Toast.makeText(this, "Done", Toast.LENGTH_SHORT).show();
			} else {
				Toast.makeText(this, "Failed", Toast.LENGTH_SHORT).show();
			}
		}
		finish();
	}
	
	
	private boolean deleteDir(File dir) {
	    if (dir.isDirectory()) {
	        String[] children = dir.list();
	        for (int i=0; i<children.length; i++) {
	            boolean success = deleteDir(new File(dir, children[i]));
	            if (!success) {
	                return false;
	            }
	        }
	    }

	    // The directory is now empty so delete it
	    return dir.delete();
	}
	
}
