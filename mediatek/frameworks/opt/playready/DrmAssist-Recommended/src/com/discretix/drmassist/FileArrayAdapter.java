package com.discretix.drmassist;

import java.io.File;
import java.util.List;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;
/**
 * Custom ArrayAdapter for showing list of Files
 */
public class FileArrayAdapter extends ArrayAdapter<File> {
	
	private List<File> fileList;
	private Context context;
	
	public FileArrayAdapter(Context context, List<File> objects) {
		super(context, android.R.layout.simple_list_item_1, objects);
		fileList = objects;
		this.context = context;
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		LayoutInflater inflater = (LayoutInflater) context
				.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		View rowView = inflater.inflate(android.R.layout.simple_list_item_1, parent, false);
		TextView textView = (TextView) rowView.findViewById(android.R.id.text1);
		File file = fileList.get(position);
		String name = file.getName();
		if (!file.isDirectory() || name.equals("..")){
			textView.setText(name);
		} else {//all folder names will be padded with slash for easier recognition 
			textView.setText(name  + "/");
		}
		
		return rowView;
	}



}
