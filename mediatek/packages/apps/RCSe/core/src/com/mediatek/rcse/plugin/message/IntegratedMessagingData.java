package com.mediatek.rcse.plugin.message;

import android.net.Uri;

public class IntegratedMessagingData {
	
    public static final Uri CONTENT_URI_INTEGRATED = Uri.parse("content://com.orangelabs.rcs.messaging.integrated/messaging");
    public static final Uri CONTENT_URI_INTEGRATED_TAG = Uri.parse("content://com.orangelabs.rcs.messaging.integrated/tag");
	public static final String TABLE_MESSAGE_INTEGRATED = "integrated_chatid_mapping";
	public static final String TABLE_MESSAGE_INTEGRATED_TAG = "integrated_tag_id_mapping";
	public static final String KEY_INTEGRATED_MODE_GROUP_SUBJECT = "group_subjectid";
	public static final String KEY_INTEGRATED_MODE_THREAD_ID = "thread_id";
	public static final String KEY_INTEGRATED_MODE_TAG = "_tag";


}
