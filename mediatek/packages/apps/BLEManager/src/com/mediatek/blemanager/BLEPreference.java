package com.mediatek.blemanager;


import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;

import java.util.Set;

import com.mediatek.blemanager.provider.BLEConstants;

/**
 * Which used to edit and get value from profile preference
 * Each profile has its own preference to avoid the same key in different profile.
 *
 */
public class BLEPreference {

    private static final String TAG = BLEConstants.COMMON_TAG + "[BLEPreference]";

    /*****Profile preference file name****/
    private static final String ANS_PROFILE_PREFERENCE_NAME = "ans_preference";
    private static final String PASP_PROFILE_PREFERENCE_NAME = "pasp_preference";
    private static final String TIP_PROFILE_PREFERENCE_NAME = "tip_preference";

    /*****Profile ID****/
    public static final int ANS_PROFILE_ID = 1;
    public static final int PASP_PROFILE_ID = 2;
    public static final int TIP_PROFILE_ID = 3;

    /**
    * get the profile preference which according to profileId.
    * If no preference exist, create it,else return the preference.
    */
    private static SharedPreferences getProfilePreference(Context context, int profileId) {
        if (context == null) {
            Log.d(TAG, "[getProfilePreference] context is null, pls check !!");
            return null;
        }
        SharedPreferences sp = null;
        switch (profileId) {
            case ANS_PROFILE_ID:
                sp = context.getSharedPreferences(
                        ANS_PROFILE_PREFERENCE_NAME, Context.MODE_PRIVATE);
            break;

            case PASP_PROFILE_ID:
                sp = context.getSharedPreferences(
                        PASP_PROFILE_PREFERENCE_NAME, Context.MODE_PRIVATE);
            break;

            case TIP_PROFILE_ID:
                sp = context.getSharedPreferences(
                        TIP_PROFILE_PREFERENCE_NAME, Context.MODE_PRIVATE);
                break;
            
            default:
                Log.d(TAG, "[getProfilePreference] wrong profileId, pls check !!");
                break;
        }
        return sp;
        
    }

    /**
     * Get preference editor which used to edit the preference
     */
    private static SharedPreferences.Editor getProfilePreferenceEditor(
            Context context, int profileId) {
        if (context == null) {
            Log.d(TAG, "[getProfilePreferenceEditor] context is null, pls check !!");
            return null;
        }
        SharedPreferences sp = getProfilePreference(context, profileId);
        if (sp == null) {
            Log.d(TAG, "[getProfilePreferenceEditor] sharedpreference is null!!");
            return null;
        }
        return sp.edit();
    }

    /**
     * Used to check the specified key is in the preference
     * @param context
     * @param profileId
     * @param key specified key string
     * @return if parameters is wrong, return false;
     *         if profile preference is null, return false;
     *         else check the preference contains or not.
     */
    public static boolean contains(Context context, int profileId, String key) {
        if (context == null || key == null || key.length() == 0) {
            Log.d(TAG, "[contains] WRONG PARAMETERS!!!");
            return false;
        }
        SharedPreferences sp = getProfilePreference(context, profileId);
        if (sp == null) {
            Log.d(TAG, "[getProfilePreferenceEditor] sharedpreference is null!!");
            return false;
        }
        return sp.contains(key);
    }

    /**
     * putString used to put a string value into the specified profile preference
     * @param context
     * @param profileId which used to identify profile preference
     * @param key which used to identify the unique value
     * @param value which will be saved to preference
     * @return if editor commit success, return true, else return false
     */
    public static boolean putString(Context context, int profileId, String key,
                    String value) {
        if (context == null || key == null || key.length() == 0) {
            Log.d(TAG, "[putString] WRONG PARAMETERS !!!!");
            return false;
        }
        SharedPreferences.Editor editor = getProfilePreferenceEditor(context, profileId);
        if (editor == null) {
            Log.d(TAG, "[putString] eidtor is null !!!!");
            return false;
        }
        editor.putString(key, value);
        return editor.commit();
    }

    /**
     * Which used to get saved string value from specified preference according to key.
     * @param context
     * @param profileId profile id
     * @param key which used to identify the unique value
     * @param defValue default value
     * @return if not exist the preference will return {@code defValue}.
     *         if parameters is wrong ,will return {@code defValue}
     *         else get value from preference
     */
    public static String getString(Context context, int profileId, String key,
                    String defValue) {
        if (context == null || key == null || key.length() == 0) {
            Log.d(TAG, "[getString] WRONG PARAMETERS !!!!");
            return defValue;
        }
        SharedPreferences sp = getProfilePreference(context, profileId);
        if (sp == null) {
            Log.d(TAG, "[getString] sp is null !!!!");
            return defValue;
        }
        return sp.getString(key, defValue);
    }

    /**
     * putString used to put a long value into the specified profile preference
     * @param context
     * @param profileId which used to identify profile preference
     * @param key which used to identify the unique value
     * @param value which will be saved to preference
     * @return if editor commit success, return true, else return false
     */
    public static boolean putLong(Context context, int profileId, String key,
                    long value) {
        if (context == null || key == null || key.length() == 0) {
            Log.d(TAG, "[putLong] WRONG PARAMETERS !!!!");
            return false;
        }
        SharedPreferences.Editor editor = getProfilePreferenceEditor(context, profileId);
        if (editor == null) {
            Log.d(TAG, "[putLong] eidtor is null !!!!");
            return false;
        }
        editor.putLong(key, value);
        return editor.commit();
    }

    /**
     * Which used to get saved long value from specified preference according to key.
     * @param context
     * @param profileId profile id
     * @param key which used to identify the unique value
     * @param defValue default value
     * @return if not exist the preference will return {@code defValue}.
     *         if parameters is wrong ,will return {@code defValue}
     *         else get value from preference
     */
    public static long getLong(Context context, int profileId, String key,
                    long defValue) {
        if (context == null || key == null || key.length() == 0) {
            Log.d(TAG, "[getLong] WRONG PARAMETERS !!!!");
            return defValue;
        }
        SharedPreferences sp = getProfilePreference(context, profileId);
        if (sp == null) {
            Log.d(TAG, "[getLong] sp is null !!!!");
            return defValue;
        }
        return sp.getLong(key, defValue);
    }

    /**
     * putString used to put a integer value into the specified profile preference
     * @param context
     * @param profileId which used to identify profile preference
     * @param key which used to identify the unique value
     * @param value which will be saved to preference
     * @return if editor commit success, return true, else return false
     */
    public static boolean putInt(Context context, int profileId, String key,
                    int value) {
        if (context == null || key == null || key.length() == 0) {
            Log.d(TAG, "[putInt] WRONG PARAMETERS !!!!");
            return false;
        }
        SharedPreferences.Editor editor = getProfilePreferenceEditor(context, profileId);
        if (editor == null) {
            Log.d(TAG, "[putInt] eidtor is null !!!!");
            return false;
        }
        editor.putInt(key, value);
        return editor.commit();
    }

    /**
     * Which used to get saved integer value from specified preference according to key.
     * @param context
     * @param profileId profile id
     * @param key which used to identify the unique value
     * @param defValue default value
     * @return if not exist the preference will return {@code defValue}.
     *         if parameters is wrong ,will return {@code defValue}
     *         else get value from preference
     */
    public static int getInt(Context context, int profileId, String key,
                    int defValue) {
        if (context == null || key == null || key.length() == 0) {
            Log.d(TAG, "[getInt] WRONG PARAMETERS !!!!");
            return defValue;
        }
        SharedPreferences sp = getProfilePreference(context, profileId);
        if (sp == null) {
            Log.d(TAG, "[getInt] sp is null !!!!");
            return defValue;
        }
        return sp.getInt(key, defValue);
    }

    /**
     * putString used to put a float value into the specified profile preference
     * @param context
     * @param profileId which used to identify profile preference
     * @param key which used to identify the unique value
     * @param value which will be saved to preference
     * @return if editor commit success, return true, else return false
     */
    public static boolean putFloat(Context context, int profileId, String key,
                    float value) {
        if (context == null || key == null || key.length() == 0) {
            Log.d(TAG, "[putFloat] WRONG PARAMETERS !!!!");
            return false;
        }
        SharedPreferences.Editor editor = getProfilePreferenceEditor(context, profileId);
        if (editor == null) {
            Log.d(TAG, "[putFloat] eidtor is null !!!!");
            return false;
        }
        editor.putFloat(key, value);
        return editor.commit();
    }

    /**
     * Which used to get saved float value from specified preference according to key.
     * @param context
     * @param profileId profile id
     * @param key which used to identify the unique value
     * @param defValue default value
     * @return if not exist the preference will return {@code defValue}.
     *         if parameters is wrong ,will return {@code defValue}
     *         else get value from preference
     */
    public static float getFloat(Context context, int profileId, String key,
                    float defValue) {
        if (context == null || key == null || key.length() == 0) {
            Log.d(TAG, "[getFloat] WRONG PARAMETERS !!!!");
            return defValue;
        }
        SharedPreferences sp = getProfilePreference(context, profileId);
        if (sp == null) {
            Log.d(TAG, "[getFloat] sp is null !!!!");
            return defValue;
        }
        return sp.getFloat(key, defValue);
    }

    /**
     * putString used to put a boolean value into the specified profile preference
     * @param context
     * @param profileId which used to identify profile preference
     * @param key which used to identify the unique value
     * @param value which will be saved to preference
     * @return if editor commit success, return true, else return false
     */
    public static boolean putBoolean(Context context, int profileId, String key,
                    boolean value) {
        if (context == null || key == null || key.length() == 0) {
            Log.d(TAG, "[putBoolean] WRONG PARAMETERS !!!!");
            return false;
        }
        SharedPreferences.Editor editor = getProfilePreferenceEditor(context, profileId);
        if (editor == null) {
            Log.d(TAG, "[putBoolean] eidtor is null !!!!");
            return false;
        }
        editor.putBoolean(key, value);
        return editor.commit();
    }

    /**
     * Which used to get saved boolean value from specified preference according to key.
     * @param context
     * @param profileId profile id
     * @param key which used to identify the unique value
     * @param defValue default value
     * @return if not exist the preference will return {@code defValue}.
     *         if parameters is wrong ,will return {@code defValue}
     *         else get value from preference
     */
    public static boolean getBoolean(Context context, int profileId, String key,
                    boolean defValue) {
        if (context == null || key == null || key.length() == 0) {
            Log.d(TAG, "[getBoolean] WRONG PARAMETERS !!!!");
            return defValue;
        }
        SharedPreferences sp = getProfilePreference(context, profileId);
        if (sp == null) {
            Log.d(TAG, "[getBoolean] sp is null !!!!");
            return defValue;
        }
        return sp.getBoolean(key, defValue);
    }

    /**
     * putString used to put a set<String> value into the specified profile preference
     * @param context
     * @param profileId which used to identify profile preference
     * @param key which used to identify the unique value
     * @param value which will be saved to preference
     * @return if editor commit success, return true, else return false
     */
    public static boolean putStringSet(Context context, int profileId, String key,
            Set<String> value) {
        if (context == null || key == null || key.length() == 0) {
            Log.d(TAG, "[getBoolean] WRONG PARAMETERS !!!!");
            return false;
        }
        SharedPreferences.Editor editor = getProfilePreferenceEditor(context, profileId);
        if (editor == null) {
            Log.d(TAG, "[putStringSet] eidtor is null !!!!");
            return false;
        }
        editor.putStringSet(key, value);
        return editor.commit();
    }

    /**
     * Which used to get saved set<String> value from specified preference according to key.
     * 
     * Note that you must not modify the set instance returned by this call. The
     * consistency of the stored data is not guaranteed if you do,
     * nor is your ability to modify the instance at all.
     * 
     * @param context
     * @param profileId profile id
     * @param key which used to identify the unique value
     * @param defValue default value
     * @return if not exist the preference will return {@code defValue}.
     *         if parameters is wrong ,will return {@code defValue}
     *         else get value from preference
     */
    public static Set<String> getStringSet(Context context, int profileId, String key,
            Set<String> defValue) {
        if (context == null || key == null || key.length() == 0) {
            Log.d(TAG, "[getStringSet] WRONG PARAMETERS !!!!");
            return defValue;
        }
        SharedPreferences sp = getProfilePreference(context, profileId);
        if (sp == null) {
            Log.d(TAG, "[getStringSet] sp is null !!!!");
            return defValue;
        }
        return sp.getStringSet(key, defValue);
    }

    /**
     * Mark in the editor that a preference value should be removed,
     * which will be done in the actual preferences once {@link commit()} is called.
     * @param context
     * @param profileId profile id
     * @param key key which saved in preference
     * @return if parameters is wrong, return false
     *         if preference is null, return false
     *         if editor is null, return false
     *         else return editor.commit();
     */
    public static boolean remove(Context context, int profileId, String key) {
        if (context == null || key == null || key.length() == 0) {
            Log.d(TAG, "[remove] WRONG PARAMETERS !!!!");
            return false;
        }
        SharedPreferences sp = getProfilePreference(context, profileId);
        if (sp == null) {
            Log.d(TAG, "[remove] sp is null !!!!");
            return false;
        }
        if (!sp.contains(key)) {
            Log.d(TAG, "[remove] key is not in preference!!!!");
            return false;
        }
        SharedPreferences.Editor editor = sp.edit();
        if (editor == null) {
            Log.d(TAG, "[remove] eidtor is null !!!!");
            return false;
        }
        editor.remove(key);
        return editor.commit();
    }

    /**
     * Registers a callback to be invoked when a change happens to a preference.
     * @param listener The callback that will run.
     */
    public static void registerOnSharedPreferenceChangeListener(Context context, int profileId,
            SharedPreferences.OnSharedPreferenceChangeListener listener) {
        if (context == null || listener == null) {
            Log.d(TAG, "[registerOnSharedPreferenceChangeListener]" +
                    " pls check context and listener should not be null");
            return;
        }
        SharedPreferences sp = getProfilePreference(context, profileId);
        if (sp == null) {
            Log.d(TAG, "[registerOnSharedPreferenceChangeListener] sp is null !!!!");
            return;
        }
        sp.registerOnSharedPreferenceChangeListener(listener);
    }

    /**
     * Unregisters a previous callback.
     * @param listener The callback that should be unregistered.
     */
    public static void unregisterOnSharedPreferenceChangeListener(Context context, int profileId,
            SharedPreferences.OnSharedPreferenceChangeListener listener) {
        if (context == null || listener == null) {
            Log.d(TAG, "[unregisterOnSharedPreferenceChangeListener]" +
                    " pls check context and listener should not be null");
            return;
        }
        SharedPreferences sp = getProfilePreference(context, profileId);
        if (sp == null) {
            Log.d(TAG, "[unregisterOnSharedPreferenceChangeListener] sp is null !!!!");
            return;
        }
        sp.unregisterOnSharedPreferenceChangeListener(listener);
    }
    
}
