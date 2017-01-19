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

package com.mediatek.rcse.provider;

import com.mediatek.rcse.plugin.message.IntegratedMessagingData;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;

import android.database.sqlite.SQLiteOpenHelper;

public class MediatekRichProviderHelper extends SQLiteOpenHelper{
	private static final String DATABASE_NAME = "eventlog.db";
	private static final int DATABASE_VERSION = 10;

	@Override
	public void onCreate(SQLiteDatabase db){	
		/** INTEGRATED MESSAGING MAPPING TABLE*/
		db.execSQL("create table " + IntegratedMessagingData.TABLE_MESSAGE_INTEGRATED + " ("
				+ IntegratedMessagingData.KEY_INTEGRATED_MODE_GROUP_SUBJECT + " TEXT , "
				+ IntegratedMessagingData.KEY_INTEGRATED_MODE_THREAD_ID + " long primary key ); "
				);
		/** INTEGRATED MESSAGING MAPPING TABLE TAG */
		db.execSQL("create table " + IntegratedMessagingData.TABLE_MESSAGE_INTEGRATED_TAG + " ("
				+ IntegratedMessagingData.KEY_INTEGRATED_MODE_TAG + " TEXT , "
				+ IntegratedMessagingData.KEY_INTEGRATED_MODE_THREAD_ID + " long primary key ); "
				);
	}
	
	@Override
	public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion){	
		db.execSQL("DROP TABLE IF EXISTS " + IntegratedMessageMappingProvider.TABLE);
		db.execSQL("DROP TABLE IF EXISTS " + IntegratedMessageMappingProvider.TABLE_2);
		onCreate(db);
	}
	
	
	/**
	 * To manage an unique instance.
	 */
	private static MediatekRichProviderHelper instance = null;
	
	public static synchronized void createInstance(Context ctx) {
		if (instance == null) {
			instance = new MediatekRichProviderHelper(ctx);
		}
	}
	
	public static MediatekRichProviderHelper getInstance() {
		return instance;
	}

	private MediatekRichProviderHelper(Context context) {
		 super(context, DATABASE_NAME, null, DATABASE_VERSION);
	}
}
