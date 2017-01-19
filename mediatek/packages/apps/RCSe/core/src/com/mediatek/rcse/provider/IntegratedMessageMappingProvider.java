package com.mediatek.rcse.provider;

import com.mediatek.rcse.plugin.message.IntegratedMessagingData;

import android.content.ContentProvider;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteQueryBuilder;
import android.net.Uri;

public class IntegratedMessageMappingProvider extends ContentProvider {

	
	private static final UriMatcher uriMatcher;

	public static final String TABLE = IntegratedMessagingData.TABLE_MESSAGE_INTEGRATED;
	public static final String TABLE_2 = IntegratedMessagingData.TABLE_MESSAGE_INTEGRATED_TAG;
	private static final int INTEGRATED_MESSAGING = 1;
	private static final int INTEGRATED_MESSAGING_TAG = 2;
	
	
    /**
     * Database helper class
     */
    private SQLiteOpenHelper openHelper;
    
	static {
		uriMatcher = new UriMatcher(UriMatcher.NO_MATCH);
		uriMatcher.addURI("com.orangelabs.rcs.messaging.integrated", "messaging", INTEGRATED_MESSAGING);
		uriMatcher.addURI("com.orangelabs.rcs.messaging.integrated", "tag", INTEGRATED_MESSAGING_TAG);
	}
	
	@Override
	public int delete(Uri uri, String where, String[] whereArgs) {
		SQLiteDatabase db = openHelper.getWritableDatabase();
        int count = 0;
        switch(uriMatcher.match(uri)){
	        case INTEGRATED_MESSAGING:
	        	count = db.delete(TABLE, where, whereArgs);
	        	break;
	        case INTEGRATED_MESSAGING_TAG:
	        	count = db.delete(TABLE_2, where, whereArgs);
	        	break;
        }
        return count; 
	}

	
	
	@Override
	public String getType(Uri arg0) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	  public Cursor query(Uri uri, String[] projectionIn, String selection, String[] selectionArgs, String sort) {
		// TODO Auto-generated method stub
		   Cursor cursor = null;
		   SQLiteQueryBuilder qb = new SQLiteQueryBuilder();	       
	    	Cursor raw_cursor = null;
	    	boolean rawCursorFlag = false; //enable if a query is raw query
	        // Generate the body of the query
	        String groupBy=null;
	        int match = uriMatcher.match(uri);
	        switch(match) {
	            case INTEGRATED_MESSAGING:
	            	qb.setTables(TABLE);
	    	        SQLiteDatabase db = openHelper.getReadableDatabase();
	               //SQLiteDatabase db = openHelper.getReadableDatabase();
	                cursor = qb.query(db, projectionIn, selection, selectionArgs, groupBy, null, sort);
	                break;
	            case INTEGRATED_MESSAGING_TAG:
	            	qb.setTables(TABLE_2);
	    	        db = openHelper.getReadableDatabase();
	               //SQLiteDatabase db = openHelper.getReadableDatabase();
	                cursor = qb.query(db, projectionIn, selection, selectionArgs, groupBy, null, sort);
	                break;
	        }
            return cursor;
	}
	
	
	 @Override
	    public Uri insert(Uri uri, ContentValues initialValues) {
	        SQLiteDatabase db = openHelper.getWritableDatabase();
	        switch(uriMatcher.match(uri)){
		        case INTEGRATED_MESSAGING:
		            // Insert the new row, will return the row number if successful
		        	// Use system clock to generate id : it should not be a common int otherwise it could be the 
		        	// same as an id present in MmsSms table (and that will create uniqueness problem when doing the tables merge) 
		    		long rowId = db.insert(TABLE, null, initialValues);
		    		uri = ContentUris.withAppendedId(IntegratedMessagingData.CONTENT_URI_INTEGRATED, rowId);
		        	break;
		        case INTEGRATED_MESSAGING_TAG:
		            // Insert the new row, will return the row number if successful
		        	// Use system clock to generate id : it should not be a common int otherwise it could be the 
		        	// same as an id present in MmsSms table (and that will create uniqueness problem when doing the tables merge) 
		    		long rowId2 = db.insert(TABLE_2, null, initialValues);
		    		uri = ContentUris.withAppendedId(IntegratedMessagingData.CONTENT_URI_INTEGRATED_TAG, rowId2);
		        	break;
		        default:
		    		throw new SQLException("Failed to insert row into " + uri);
	        }
			return uri;
	    }

	@Override
	public boolean onCreate() {
		// TODO Auto-generated method stub
		
		if(MediatekRichProviderHelper.getInstance()==null){
			MediatekRichProviderHelper.createInstance(this.getContext());
        }
        this.openHelper = MediatekRichProviderHelper.getInstance();
        return true;

	}



	@Override
	public int update(Uri uri, ContentValues values, String selection,
			String[] selectionArgs) {
		// TODO Auto-generated method stub
		return 0;
	}

}
