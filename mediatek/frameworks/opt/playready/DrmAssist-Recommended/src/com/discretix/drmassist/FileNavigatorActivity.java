package com.discretix.drmassist;

import java.io.File;
import java.io.FileFilter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView.OnItemLongClickListener;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.TextView.OnEditorActionListener;

/**
 * File browser activity
 */
public class FileNavigatorActivity extends Activity {
	private ListView fileView = null;
	private EditText path = null;
	private List<File> items = null;
	private int lastVisibleIndex;

	
	@Override
    public void onCreate(Bundle savedInstanceState) {
        
        setContentView(R.layout.main);
        fileView = (ListView) findViewById(R.id.listViewFiles);
        path = (EditText) findViewById(R.id.editTextPath);
               
        getFiles();
        
        Button refresh = (Button) findViewById(R.id.buttonRefresh);
        refresh.setOnClickListener(new OnClickListener() {
			
			public void onClick(View v) {
				getFiles();
			}
		});
        
        path.setOnEditorActionListener(new OnEditorActionListener() {
			
			public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
				File file = new File (v.getText().toString());
				if (file.exists() && file.isDirectory()){
					getFiles();
				}
		        InputMethodManager mgr = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
		        mgr.hideSoftInputFromWindow(path.getWindowToken(), 0);
				
				return false;
			}
		});
        
        fileView.setOnItemClickListener(new OnItemClickListener() {

			public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {		
				lastVisibleIndex = fileView.getFirstVisiblePosition();
				File selectedFile = items.get(arg2);
				if (selectedFile.getName().equals("..")){
					File file = new File(path.getText().toString());
					file = file.getParentFile();
					if (null != file){
						path.setText(file.getAbsolutePath());
						getFiles();
					}
				} else if(selectedFile.isDirectory()){
					path.setText(selectedFile.getAbsolutePath());
					getFiles();
				} else {
									
					Intent intent = new Intent(FileNavigatorActivity.this,DrmActionSelectorActivity.class);
					Bundle bundle = new Bundle();
					bundle.putString("path",selectedFile.getAbsolutePath());
					
					intent.putExtras(bundle);
									
					//Start next activity
					startActivity(intent);
				}
			}
		});
        fileView.setOnItemLongClickListener(new OnItemLongClickListener() {

			public boolean onItemLongClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
				lastVisibleIndex = fileView.getFirstVisiblePosition();
				File selectedFile = items.get(arg2);
				Intent intent = new Intent(FileNavigatorActivity.this,LongPressView.class);
				Bundle bundle = new Bundle();
				bundle.putString("path",selectedFile.getAbsolutePath());
				
				intent.putExtras(bundle);
								
				//Start next activity
				startActivity(intent);
				return true;
			}
        	
		});
        super.onCreate(savedInstanceState);
    }
    
    private void getFiles(){
    	String pathStr = path.getText().toString();
    	File root = new File(pathStr);
        items = new ArrayList<File>();
        items.add(new File(".."));
        
        File[] folders = root.listFiles(new FileFilter() {
			
			public boolean accept(File pathname) {
				return pathname.isDirectory();
			}
		});
        if (null != folders){
        	Arrays.sort(folders);
        	items.addAll(Arrays.asList(folders));
        }
        
        File[] files = root.listFiles(new FileFilter() {
			
			public boolean accept(File pathname) {
				return !pathname.isDirectory();
			}
		});
        if (null != files){
        	Arrays.sort(files);
        	items.addAll(Arrays.asList(files));
        }
        BaseAdapter fileList = new FileArrayAdapter(this, items);   
        //ArrayAdapter<File> fileList = new FileArrayAdapter(this, items);
        fileView.setAdapter(fileList);
    }

	@Override
	protected void onResume() {
		getFiles();
		fileView.setSelectionFromTop(lastVisibleIndex, 0);
		super.onResume();
	}

	@Override
	public void onBackPressed() {
		File file = new File(path.getText().toString());
		file = file.getParentFile();
		if (null != file){
			path.setText(file.getAbsolutePath());
			getFiles();
		}
	}

	@Override
	public boolean onKeyLongPress(int keyCode, KeyEvent event) {
		if (keyCode == KeyEvent.KEYCODE_BACK)  {              //If the back button was long pressed
			
			finish();
			return true;
	    }
		return super.onKeyLongPress(keyCode, event);
	}
    
	
    
}