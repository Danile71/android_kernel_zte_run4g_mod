/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/
package com.orangelabs.rcs.provider.sharing;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;

import android.content.ContentProvider;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteQueryBuilder;
import android.net.Uri;
import android.os.Environment;
import android.text.TextUtils;

/**
 * Video sharing provider
 * 
 * @author Jean-Marc AUFFRET
 */
public class VideoSharingProvider extends ContentProvider {
	// Database table
	public static final String TABLE = "vsh";
	
	// Create the constants used to differentiate between the different
	// URI requests
	private static final int VIDEOSHARES = 1;
	private static final int VIDEOSHARE_ID = 2;
    private static final int RCSAPI = 3;
    private static final int RCSAPI_ID = 4;
	
	// Allocate the UriMatcher object
	private static final UriMatcher uriMatcher;
	static {
		uriMatcher = new UriMatcher(UriMatcher.NO_MATCH);
		uriMatcher.addURI("com.orangelabs.rcs.vsh", "vsh", VIDEOSHARES);
		uriMatcher.addURI("com.orangelabs.rcs.vsh", "vsh/#", VIDEOSHARE_ID);
		uriMatcher.addURI("org.gsma.joyn.provider.vsh", "vsh", RCSAPI);
		uriMatcher.addURI("org.gsma.joyn.provider.vsh", "vsh/#", RCSAPI_ID);
	}

    /**
     * Database helper class
     */
    private SQLiteOpenHelper openHelper;
    
    /**
     * Database name
     */
    public static final String DATABASE_NAME = "vsh.db";

    /**
     * Helper class for opening, creating and managing database version control
     */
    private static class DatabaseHelper extends SQLiteOpenHelper {
        private static final int DATABASE_VERSION = 2;

        public DatabaseHelper(Context ctx) {
            super(ctx, DATABASE_NAME, null, DATABASE_VERSION);
        }

        @Override
        public void onCreate(SQLiteDatabase db) {
        	db.execSQL("CREATE TABLE " + TABLE + " ("
        			+ VideoSharingData.KEY_ID + " integer primary key autoincrement,"
        			+ VideoSharingData.KEY_SESSION_ID + " TEXT,"
        			+ VideoSharingData.KEY_CONTACT + " TEXT,"
        			+ VideoSharingData.KEY_STATUS + " integer,"
        			+ VideoSharingData.KEY_DIRECTION + " integer,"
        			+ VideoSharingData.KEY_TIMESTAMP + " long,"
        			+ VideoSharingData.KEY_DURATION + " long);");
        }

        @Override
        public void onUpgrade(SQLiteDatabase db, int oldVersion, int currentVersion) {
            db.execSQL("DROP TABLE IF EXISTS " + TABLE);
            onCreate(db);
        }
    }

    @Override
    public boolean onCreate() {
        openHelper = new DatabaseHelper(getContext());
        return true;
    }

	@Override
	public String getType(Uri uri) {
		switch(uriMatcher.match(uri)){
			case VIDEOSHARES:
			case RCSAPI:
				return "vnd.android.cursor.dir/vsh";
			case VIDEOSHARE_ID:
			case RCSAPI_ID:
				return "vnd.android.cursor.item/vsh";
			default:
				throw new IllegalArgumentException("Unsupported URI " + uri);
		}
	}
	
    @Override
    public Cursor query(Uri uri, String[] projectionIn, String selection, String[] selectionArgs, String sort) {
        SQLiteQueryBuilder qb = new SQLiteQueryBuilder();
        qb.setTables(TABLE);

        // Generate the body of the query
        int match = uriMatcher.match(uri);
        switch(match) {
            case VIDEOSHARES:
        	case RCSAPI:
                break;
            case VIDEOSHARE_ID:
        	case RCSAPI_ID:
                qb.appendWhere(VideoSharingData.KEY_ID + "=" + uri.getPathSegments().get(1));
                break;
            default:
                throw new IllegalArgumentException("Unknown URI " + uri);
        }

        SQLiteDatabase db = openHelper.getReadableDatabase();
        Cursor c = qb.query(db, projectionIn, selection, selectionArgs, null, null, sort);

		// Register the contexts ContentResolver to be notified if the cursor result set changes.
        if (c != null) {
        	c.setNotificationUri(getContext().getContentResolver(), VideoSharingData.CONTENT_URI);
        }
        return c;
    }
    
    @Override
    public int update(Uri uri, ContentValues values, String where, String[] whereArgs) {
        int count = 0;
        SQLiteDatabase db = openHelper.getWritableDatabase();

        int match = uriMatcher.match(uri);
        switch (match) {
	        case VIDEOSHARES:
	            count = db.update(TABLE, values, where, null);
	            break;
            case VIDEOSHARE_ID:
                String segment = uri.getPathSegments().get(1);
                int id = Integer.parseInt(segment);
                count = db.update(TABLE, values, VideoSharingData.KEY_ID + "=" + id, null);
                break;
            default:
                throw new UnsupportedOperationException("Cannot update URI " + uri);
        }
        getContext().getContentResolver().notifyChange(uri, null);
        return count;
    }
    
    @Override
    public Uri insert(Uri uri, ContentValues initialValues) {
        SQLiteDatabase db = openHelper.getWritableDatabase();
        switch(uriMatcher.match(uri)){
	        case VIDEOSHARES:
	        case VIDEOSHARE_ID:
	    		long rowId = db.insert(TABLE, null, initialValues);
	    		uri = ContentUris.withAppendedId(VideoSharingData.CONTENT_URI, rowId);
	        	break;
	        default:
	    		throw new SQLException("Failed to insert row into " + uri);
        }
		getContext().getContentResolver().notifyChange(uri, null);
        return uri;
    }
    
    @Override
    public int delete(Uri uri, String where, String[] whereArgs) {
        SQLiteDatabase db = openHelper.getWritableDatabase();
        int count = 0;
        switch(uriMatcher.match(uri)){
	        case VIDEOSHARES:
	        case RCSAPI:
	        	count = db.delete(TABLE, where, whereArgs);
	        	break;
	        case VIDEOSHARE_ID:
	        case RCSAPI_ID:
	        	String segment = uri.getPathSegments().get(1);
				count = db.delete(TABLE, VideoSharingData.KEY_ID + "="
						+ segment
						+ (!TextUtils.isEmpty(where) ? " AND ("	+ where + ')' : ""),
						whereArgs);
				
				break;
	        	
	        default:
	    		throw new SQLException("Failed to delete row " + uri);
        }
		getContext().getContentResolver().notifyChange(uri, null);
        return count;    
   }	

	public static void backupVideoSharingDatabase(String account) {
		    	try {
			    	String packageName = "com.orangelabs.rcs";
			    	String dbFile = Environment.getDataDirectory() + "/data/" + packageName + "/databases/" + VideoSharingProvider.DATABASE_NAME;
			    	File file = new File(dbFile);
			    	if (file.exists())
			    	{
			    	File backupFileRoot =new File( Environment.getExternalStorageDirectory()+"/Rcs" + "/data/" + packageName + "/databases/");
			    	backupFileRoot.mkdirs();
			        File backupFile = new File(backupFileRoot, VideoSharingProvider.DATABASE_NAME +"_"+ account +".db");
			    	
			    	OutputStream outStream = new FileOutputStream(backupFile, false);
			    	InputStream inStream = new FileInputStream(dbFile);
		 		    byte[] buffer = new byte[1024];
				    int length;
				    while ((length = inStream.read(buffer))>0) {
						outStream.write(buffer, 0, length);
				    }
				    outStream.flush();
				    outStream.close();
				    inStream.close();		    	
			    	}
		    	} catch(Exception e) {
		    		e.printStackTrace();
		    	}
		    }	
    
	public static void backupVideoSharingDatabase(String account, String path) {
    	try {
	    	String packageName = "com.orangelabs.rcs";
	    	String dbFile = Environment.getDataDirectory() + "/data/" + packageName + "/databases/" + VideoSharingProvider.DATABASE_NAME;
	    	File file = new File(dbFile);
	    	if (file.exists())
	    	{
	    	File backupFileRoot =new File( path);
	    	backupFileRoot.mkdirs();
	        File backupFile = new File(backupFileRoot, VideoSharingProvider.DATABASE_NAME +"_"+ account +".db");
	    	
	    	OutputStream outStream = new FileOutputStream(backupFile, false);
	    	InputStream inStream = new FileInputStream(dbFile);
 		    byte[] buffer = new byte[1024];
		    int length;
		    while ((length = inStream.read(buffer))>0) {
				outStream.write(buffer, 0, length);
		    }
		    outStream.flush();
		    outStream.close();
		    inStream.close();		    	
	    	}
    	} catch(Exception e) {
    		e.printStackTrace();
    	}
    }	
	
	public static void restoreVideoSharingDatabase(String account)
    {
    	
    	try {
    		
   		// if (logger.isActivated()) {
        //        logger.debug("restoreAccountMessages : account:"+account);
         //   }
   		 
	    	String packageName = "com.orangelabs.rcs";
	    	String dbFile = Environment.getDataDirectory() + "/data/" + packageName + "/databases/" + VideoSharingProvider.DATABASE_NAME;
	    	File backupFileRoot =new File( Environment.getExternalStorageDirectory()+"/Rcs" + "/data/" + packageName + "/databases/");  
	    	String restoreFile = backupFileRoot+"/" + VideoSharingProvider.DATABASE_NAME +"_"+ account +".db";
           


	    	File file = new File(restoreFile);
	    	if (!file.exists()) {
	    	//	if (logger.isActivated()) {
	                 //logger.debug("error in restoreAccountMessages : account:"+account + "; "+restoreFile+" :file cant be created");
	       //      }
	    		return;
	    	}
	    	

                File dbOriginalFile = new File(dbFile);
	              if (!dbOriginalFile.exists()) {
	                          //     dbOriginalFile.delete();
				dbOriginalFile.createNewFile();
              } 

	              new FileOutputStream(dbFile,false).close();
	             // dbOriginalFile.createNewFile();
		    	OutputStream outStream = new FileOutputStream(dbFile, false);
	    	InputStream inStream = new FileInputStream(file);
		    byte[] buffer = new byte[1024];
		    int length;
		    while ((length = inStream.read(buffer))>0) {
				outStream.write(buffer, 0, length);
		    }
		    outStream.flush();
		    outStream.close();
		    inStream.close();		    	
   	} catch(Exception e) {

      // if (logger.isActivated()) {
      //             logger.debug("exception in restoreAccountMessages");
      //         }
   		e.printStackTrace();

   	    }
    }
    
	public static boolean restoreVideoSharingDatabase(String account, String path)
    {
    	
    	try {
    		
   		// if (logger.isActivated()) {
        //        logger.debug("restoreAccountMessages : account:"+account);
         //   }
   		 
	    	String packageName = "com.orangelabs.rcs";
	    	String dbFile = Environment.getDataDirectory() + "/data/" + packageName + "/databases/" + VideoSharingProvider.DATABASE_NAME;
	    	File backupFileRoot =new File( path);  
	    	String restoreFile = backupFileRoot+"/" + VideoSharingProvider.DATABASE_NAME +"_"+ account +".db";
           


	    	File file = new File(restoreFile);
	    	if (!file.exists()) {
	    	//	if (logger.isActivated()) {
	                 //logger.debug("error in restoreAccountMessages : account:"+account + "; "+restoreFile+" :file cant be created");
	       //      }
	    		return false;
	    	}
	    	

                File dbOriginalFile = new File(dbFile);
	              if (!dbOriginalFile.exists()) {
	                          //     dbOriginalFile.delete();
				dbOriginalFile.createNewFile();
              } 

	              new FileOutputStream(dbFile,false).close();
	             // dbOriginalFile.createNewFile();
		    	OutputStream outStream = new FileOutputStream(dbFile, false);
	    	InputStream inStream = new FileInputStream(file);
		    byte[] buffer = new byte[1024];
		    int length;
		    while ((length = inStream.read(buffer))>0) {
				outStream.write(buffer, 0, length);
		    }
		    outStream.flush();
		    outStream.close();
		    inStream.close();		    	
   	} catch(Exception e) {

      // if (logger.isActivated()) {
      //             logger.debug("exception in restoreAccountMessages");
      //         }
   		e.printStackTrace();
        return false;
   	    }
   	return true;
    }
    
}