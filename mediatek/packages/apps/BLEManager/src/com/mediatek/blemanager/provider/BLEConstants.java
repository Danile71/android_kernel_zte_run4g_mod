package com.mediatek.blemanager.provider;

import android.net.Uri;

public final class BLEConstants {

    public static final String HEADER = "content://";

    public static final String BLEMANAGER_AUTHORITY =
        "com.mediatek.blemanager.provider.BLEManagerProvider";

    public static final String BLE_AUTHORITY =
        "com.mediatek.bluetoothle.provider.BluetoothLeProvider";

    public static final String COLUMN_ID = "_id";

    public static final String COLUMN_BT_ADDRESS = "bt_address";
    
    public static final String SERVICE_LIST_SEPERATER = ";";
    
    public static final String TABLE_ANS_URI_STRING =
        "content://com.mediatek.bluetoothle.provider.BluetoothLeProvider/ans";
    public static final Uri TABLE_ANS_URI = Uri.parse(TABLE_ANS_URI_STRING);
    
    public static final String TABLE_CLIENT_STRING =
        "content://com.mediatek.bluetoothle.provider.BluetoothLeProvider/client_table";
    public static final Uri TABLE_CLIENT_URI = Uri.parse(TABLE_CLIENT_STRING);
    
    public static final String TABLE_UX_URI_STRING =
        HEADER + BLEMANAGER_AUTHORITY + "/" + DEVICE_SETTINGS.TABLE_NAME;
    public static final Uri TABLE_UX_URI = Uri.parse(TABLE_UX_URI_STRING);

    public static final String COMMON_TAG = "[BT][BLE-UX]";

    public static final class PXP_CONFIGURATION {
        public static final String ALERT_ENABLER = "alert_enabler";
        public static final String RANGE_ALERT_ENABLER = "range_alert_enabler";
        public static final String RANGE_ALERT_INFO_DIALOG_ENABLER =
                                "range_alert_info_dialog_enabler";
        public static final String RANGE_TYPE = "range_type";
        public static final String RANGE_VALUE = "range_value";
        public static final String RINGTONE_ENABLER = "ringtone_enabler";
        public static final String RINGTONE = "ringtone";
        public static final String VOLUME = "volume";
        public static final String VIBRATION_ENABLER = "vibration_enabler";
        public static final String DISCONNECTION_WARNING_ENABLER = "dis_warning_enabler";
//        public static final String IS_SUPPORT_OPTIONAL = "is_support_optional";
    }

    public static final class DEVICE_SETTINGS {
        public static final String TABLE_NAME = "device_setting";
        
        public static final String DEVICE_DISPLAY_ORDER = "device_display_order";
        public static final String DEVICE_NAME = "device_name";
        public static final String DEVICE_IAMGE_DATA = "_data";
        public static final String DEVICE_SERVICE_LIST = "service_list";
    }

    public static final class ANS_CONFIGURATION {
        public static final String ANS_HOST_SIMPLE_ALERT = "h_simple";
        public static final String ANS_HOST_EMAIL_ALERT = "h_email";
        public static final String ANS_HOST_NEWS_ALERT = "h_news";
        public static final String ANS_HOST_CALL_ALERT = "h_call";
        public static final String ANS_HOST_MISSED_CALL_ALERT = "h_missed_call";
        public static final String ANS_HOST_SMSMMS_ALERT = "h_smsmms";
        public static final String ANS_HOST_VOICE_MAIL_ALERT = "h_voice_mail";
        public static final String ANS_HOST_SCHEDULE_ALERT = "h_schedule";
        public static final String ANS_HOST_HIGH_PRIORITIZED_ALERT = "h_high_prioritized";
        public static final String ANS_HOST_INSTANT_MESSAGE_ALERT = "h_instant_message";

        public static final String ANS_REMOTE_SIMPLE_ALERT = "r_simple";
        public static final String ANS_REMOTE_EMAIL_ALERT = "r_email";
        public static final String ANS_REMOTE_NEWS_ALERT = "r_news";
        public static final String ANS_REMOTE_CALL_ALERT = "r_call";
        public static final String ANS_REMOTE_MISSED_CALL_ALERT = "r_missed_call";
        public static final String ANS_REMOTE_SMSMMS_ALERT = "r_smsmms";
        public static final String ANS_REMOTE_VOICE_MAIL_ALERT = "r_voice_mail";
        public static final String ANS_REMOTE_SECHEDULE_ALERT = "r_schedule";
        public static final String ANS_REMOTE_HIGH_PRIORITIZED_ALERT = "r_high_prioritized";
        public static final String ANS_REMOTE_INSTANT_MESSAGE_ALERT = "r_instant_message";

        public static final String ANS_NEW_CLIENT_CONFIG = "new_client_config";
        public static final String ANS_UNREAD_CLIENT_CONFIG = "unread_client_config";

        public static final String ANS_REMOTE_EXTRA_CATEGORY = "r_extra_category";
        public static final String ANS_HOST_EXTRA_CATEGORY = "h_extra_category";
    }

}
