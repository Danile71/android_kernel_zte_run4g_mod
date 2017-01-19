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

package com.mediatek.rcse.settings;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Environment;
import android.text.TextUtils;

import com.orangelabs.rcs.core.ims.service.capability.Capabilities;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * RCS settings
 *
 * @author jexa7410
 */
public class AppSettings {
	/**
	 * Current instance
	 */
	private static AppSettings instance = null;
	
	/**
	 * Content resolver
	 */
	private ContentResolver cr;

	/**
	 * Database URI
	 */
	private Uri databaseUri = AppSettingsData.CONTENT_URI;
	
	/**
     * Tag
     */
    private final static String TAG = "AppSettings";

    /**
     * Logger instance
     */
    private final static Logger LOGGER = Logger.getLogger(TAG);

    /**
     * Create instance
     *
     * @param ctx Context
     */
	public static synchronized void createInstance(Context ctx) {
		if (instance == null) {
			instance = new AppSettings(ctx);
		}
	}

	/**
     * Returns instance
     *
     * @return Instance
     */
	public static AppSettings getInstance() {
		return instance;
	}

	/**
     * Constructor
     *
     * @param ctx Application context
     */
	private AppSettings(Context ctx) {
		super();

        this.cr = ctx.getContentResolver();
	}

	/**
     * Read a parameter
     *
     * @param key Key
     * @return Value
     */
	public String readParameter(String key) {
		if (key == null) {
			return null;
		}

		String result = null;
        Cursor c = cr.query(databaseUri, null, AppSettingsData.KEY_KEY + "='" + key + "'", null, null);
        if (c != null) {
        	if ((c.getCount() > 0) && c.moveToFirst()) {
	        	result = c.getString(2);
        	}
	        c.close();
        }
        return result;
	}

	/**
     * Write a parameter
     *
     * @param key Key
     * @param value Value
     */
	public void writeParameter(String key, String value) {
		if ((key == null) || (value == null)) {
			return;
		}
		
        ContentValues values = new ContentValues();
        values.put(AppSettingsData.KEY_VALUE, value);
        String where = AppSettingsData.KEY_KEY + "='" + key + "'";
        cr.update(databaseUri, values, where, null);
	}

	/**
     * Insert a parameter
     *
     * @param key Key
     * @param value Value
     */
	public void insertParameter(String key, String value) {
		if ((key == null) || (value == null)) {
			return;
		}

		ContentValues values = new ContentValues();
        values.put(AppSettingsData.KEY_KEY, key);
        values.put(AppSettingsData.KEY_VALUE, value);
        cr.insert(databaseUri, values);
	}

	

	

	

    
	/**
     * Get the ringtone for CSh invitation
     *
     * @return Ringtone URI or null if there is no ringtone
     */
	public String getCShInvitationRingtone() {
		String result = null;
		if (instance != null) {
			result = readParameter(AppSettingsData.CSH_INVITATION_RINGTONE);
		}
		return result;
	}

	/**
     * Set the CSh invitation ringtone
     *
     * @param uri Ringtone URI
     */
	public void setCShInvitationRingtone(String uri) {
		if (instance != null) {
			writeParameter(AppSettingsData.CSH_INVITATION_RINGTONE, uri);
		}
	}

    /**
     * Is phone vibrate for CSh invitation
     *
     * @return Boolean
     */
	public boolean isPhoneVibrateForCShInvitation() {
		boolean result = false;
		if (instance != null) {
			result = Boolean.parseBoolean(readParameter(AppSettingsData.CSH_INVITATION_VIBRATE));
		}
		return result;
    }

	/**
     * Set phone vibrate for CSh invitation
     *
     * @param vibrate Vibrate state
     */
	public void setPhoneVibrateForCShInvitation(boolean vibrate) {
		if (instance != null) {
			writeParameter(AppSettingsData.CSH_INVITATION_VIBRATE, Boolean.toString(vibrate));
		}
    }

	

	/**
     * Get the ringtone for file transfer invitation
     *
     * @return Ringtone URI or null if there is no ringtone
     */
	public String getFileTransferInvitationRingtone() {
		String result = null;
		if (instance != null) {
			result = readParameter(AppSettingsData.FILETRANSFER_INVITATION_RINGTONE);
		}
		return result;
	}

	/**
     * Set the file transfer invitation ringtone
     *
     * @param uri Ringtone URI
     */
	public void setFileTransferInvitationRingtone(String uri) {
		if (instance != null) {
			writeParameter(AppSettingsData.FILETRANSFER_INVITATION_RINGTONE, uri);
		}
	}

    /**
     * Is phone vibrate for file transfer invitation
     *
     * @return Boolean
     */
	public boolean isPhoneVibrateForFileTransferInvitation() {
		boolean result = false;
		if (instance != null) {
			result = Boolean.parseBoolean(readParameter(AppSettingsData.FILETRANSFER_INVITATION_VIBRATE));
		}
		return result;
    }

	/**
     * Set phone vibrate for file transfer invitation
     *
     * @param vibrate Vibrate state
     */
	public void setPhoneVibrateForFileTransferInvitation(boolean vibrate) {
		if (instance != null) {
			writeParameter(AppSettingsData.FILETRANSFER_INVITATION_VIBRATE, Boolean.toString(vibrate));
		}
    }

	/**
     * Get the ringtone for chat invitation
     *
     * @return Ringtone URI or null if there is no ringtone
     */
	public String getChatInvitationRingtone() {
		String result = null;
		if (instance != null) {
			result = readParameter(AppSettingsData.CHAT_INVITATION_RINGTONE);
		}
		return result;
	}

	/**
     * Set the chat invitation ringtone
     *
     * @param uri Ringtone URI
     */
	public void setChatInvitationRingtone(String uri) {
		if (instance != null) {
			writeParameter(AppSettingsData.CHAT_INVITATION_RINGTONE, uri);
		}
	}

    /**
     * Is phone vibrate for chat invitation
     *
     * @return Boolean
     */
	public boolean isPhoneVibrateForChatInvitation() {
		boolean result = false;
		if (instance != null) {
			result = Boolean.parseBoolean(readParameter(AppSettingsData.CHAT_INVITATION_VIBRATE));
		}
		return result;
    }

	/**
     * Set phone vibrate for chat invitation
     *
     * @param vibrate Vibrate state
     */
	public void setPhoneVibrateForChatInvitation(boolean vibrate) {
		if (instance != null) {
			writeParameter(AppSettingsData.CHAT_INVITATION_VIBRATE, Boolean.toString(vibrate));
		}
	}
	
	/**
     * Check whether compressing image when send image. Do not call this method
     * in ui thread
     * 
     * @return True if compressing image is enabled, otherwise return false.
     */
    public boolean isEnabledCompressingImageFromDB() {
        String result = null;
        if (instance != null) {
            result = readParameter(AppSettingsData.RCSE_COMPRESSING_IMAGE);
            if (LOGGER.isActivated()) {
                LOGGER.debug("isEnabledCompressingImageFromDB(), result: " + result);
            }
            return Boolean.valueOf(result);
        }
        return true;
    }

    /**
     * Set the status which indicate whether compressing image when send image.
     * 
     * @param state True if compressing image is enabled, otherwise return
     *            false.
     */
    public void setCompressingImage(final boolean state) {
        if (LOGGER.isActivated()) {
            LOGGER.debug("setCompressingImage(), state: " + state);
        }
        writeParameter(AppSettingsData.RCSE_COMPRESSING_IMAGE, Boolean.toString(state));
    }
    
    /**
     * Get the remind flag
     * @return True if need remind compress again when send image, otherwise
     *         return false
     */
    public boolean restoreRemindCompressFlag() {
        String result = null;
        if (instance != null) {
            result = readParameter(AppSettingsData.COMPRESS_IMAGE_HINT);
            if (LOGGER.isActivated()) {
                LOGGER.debug("restoreRemindCompressFlag(), result: " + result);
            }
            return Boolean.valueOf(result);
        }
        if (LOGGER.isActivated()) {
            LOGGER.debug("instance is null, return false");
        }
        return false;
    }

    /**
     * Set the remind flag
     * 
     * @param notRemind Indicates whether need to remind user to compress image
     *            when send image
     */
    public void saveRemindCompressFlag(final boolean notRemind) {
        if (LOGGER.isActivated()) {
            LOGGER.debug("saveRemindFlag(), notRemind: " + notRemind);
        }
        writeParameter(AppSettingsData.COMPRESS_IMAGE_HINT, Boolean.toString(!notRemind));
    }
    
    /**
     * Set the warning remind flag
     * 
     * @param notRemind Indicates whether need to remind user to Large image
     *            when send image
     */
    public void saveRemindWarningLargeImageFlag(final boolean notRemindFlag) {
        if (LOGGER.isActivated()) {
            LOGGER.debug("saveRemindWarningLargeImageFlag(), notRemindFlag: " + notRemindFlag);
        }
        writeParameter(AppSettingsData.WARNING_LARGE_IMAGE_HINT, Boolean.toString(!notRemindFlag));
    }
    
    /**
     * Get the warning remind flag
     * @return True if need remind Large image again when send image, otherwise
     *         return false
     */
    public boolean restoreRemindWarningLargeImageFlag() {
        String result = null;
        if (instance != null) {
            result = readParameter(AppSettingsData.WARNING_LARGE_IMAGE_HINT);
            if (LOGGER.isActivated()) {
                LOGGER.debug("restoreRemindWarningLargeFlag(), result: " + result);
            }
            return Boolean.valueOf(result);
        }
        if (LOGGER.isActivated()) {
            LOGGER.debug("instance is null, return false");
        }
        return false;
    }
    
    /** M: ftAutAccept @{ */
    /**
     * Whether it is enable to auto accept ft when roaming.
     * 
     * @return whether it is enable.
     */
    public Boolean isEnableFtAutoAcceptWhenRoaming() {
        Boolean result = Boolean.FALSE;
        if (instance != null) {
            result = Boolean
                    .parseBoolean(readParameter(AppSettingsData.ENABLE_AUTO_ACCEPT_FT_ROMING));
        }
        if (LOGGER.isActivated()) {
            LOGGER.debug("isEnableFtAutoAcceptWhenRoaming() result: " + result);
        }
        return result;
    }

/**
     * Enable or disable to auto-accept ft when roaming
     * 
     * @param enable True to be enable, otherwise false.
     */
    public void setEnableFtAutoAcceptWhenRoaming(Boolean enable) {
        if (instance != null) {
            writeParameter(AppSettingsData.ENABLE_AUTO_ACCEPT_FT_ROMING, Boolean.toString(enable));
        }
        if (LOGGER.isActivated()) {
            LOGGER.debug("setEnableFtAutoAcceptWhenRoaming() enable: " + enable);
        }
    }
    
    /** M: ftAutAccept no roaming @{ */
    /**
     * Whether it is enable to auto accept ft when no roaming.
     * 
     * @return whether it is enable.
     */
    public Boolean isEnableFtAutoAcceptWhenNoRoaming() {
        Boolean result = Boolean.FALSE;
        if (instance != null) {
            result = Boolean
                    .parseBoolean(readParameter(AppSettingsData.ENABLE_AUTO_ACCEPT_FT_NOROMING));
        }
        if (LOGGER.isActivated()) {
            LOGGER.debug("isEnableFtAutoAcceptWhenNoRoaming() result: " + result);
        }
        return result;
    }

    /**
     * Enable or disable to auto-accept ft when no roaming
     * 
     * @param enable True to be enable, otherwise false.
     */
    public void setEnableFtAutoAcceptWhenNoRoaming(Boolean enable) {
        if (instance != null) {
            writeParameter(AppSettingsData.ENABLE_AUTO_ACCEPT_FT_NOROMING, Boolean.toString(enable));
        }
        if (LOGGER.isActivated()) {
            LOGGER.debug("setEnableFtAutoAcceptWhenNoRoaming() enable: " + enable);
        }
    }
    
    /**
     * M: Add to achieve the RCS-e set chat wall paper feature. @{
     */
    // The resource id of chat wall paper copy in memory
    private String mChatWallpaper = null;
    private final ArrayList<OnWallPaperChangedListener> mOnWallPaperChangedListenerList = new ArrayList<OnWallPaperChangedListener>();
    public void registerWallPaperChangedListener(OnWallPaperChangedListener listener) {
        if (LOGGER.isActivated()) {
            LOGGER.debug("registerWallPaperChangedListener, listener: " + listener);
        }
        mOnWallPaperChangedListenerList.add(listener);
    }

    public void unregisterWallPaperChangedListener(OnWallPaperChangedListener listener) {
        if (LOGGER.isActivated()) {
            LOGGER.debug("unregisterWallPaperChangedListener: " + listener);
        }
        mOnWallPaperChangedListenerList.remove(listener);
    }

    /**
     * Interface definition for a callback to be invoked when the RCS-e chat
     * wall paper is changed.
     */
    public interface OnWallPaperChangedListener {
        /**
         * Called when the RCS-e chat wall paper is changed.
         * 
         * @param wallPaper The wall paper's full file name or resource id.
         */
        public void onWallPaperChanged(String wallPaper);
    }
    /**
     * Set chat wall paper resource. This method can be called on UI thread.
     * 
     * @param chatWallpaper The resource id of chat wall paper or the file name of chat wall paper.
     */
    public void setChatWallpaper(final String chatWallpaper) {
        if (chatWallpaper == null) {
            if (LOGGER.isActivated()) {
                LOGGER.debug("setChatWallpaperId invalid chatWallpaper. chatWallpaper is null");
            }
            return;
        }
        if (instance != null) {
            // Save it in memory when write it to database.
            mChatWallpaper = chatWallpaper;
            for (OnWallPaperChangedListener listener : mOnWallPaperChangedListenerList) {
                listener.onWallPaperChanged(chatWallpaper);
            }
            AsyncTask.execute(new Runnable() {
                @Override
                public void run() {
                    writeParameter(AppSettingsData.RCSE_CHAT_WALLPAPER, chatWallpaper);
                }
            });
        }
    }
    
    /**
     * Get is Messaging Disable and Fully integrated mode is on.
     * 
     * @return The Messaging Disable and Fully integrated mode.
     */
    
    
    public boolean getJoynMessagingDisabledFullyIntegrated() {
    	boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(AppSettingsData.JOYN_MESSAGING_DISABLED_FULLY_INTEGRATED));
        }
        return result;
    }
    
    public int getDisableServiceStatus()
    {
    	int result = 0;
		if (instance != null) {
			try {
				result = Integer.parseInt(readParameter(AppSettingsData.JOYN_DISABLE_STATUS));
			} catch(Exception e) {}
		}
		return result;
    }
    
    public void setDisableServiceStatus(int status)
    {
    	if (instance != null) {
            writeParameter(AppSettingsData.JOYN_DISABLE_STATUS, "" + status);
        }
    }
    
    /**
     * Set is Messaging Disable and Fully integrated mode is on.
     * 
     */
    
    
    public void setJoynMessagingDisabledFullyIntegrated(boolean state) {
		if (instance != null) {
			writeParameter(AppSettingsData.JOYN_MESSAGING_DISABLED_FULLY_INTEGRATED, Boolean.toString(state));
		}
    }
    /**
     * Get chat wall paper resource id. If this method returns 0, please call 
     * {@link #getChatWallpaper()} to get the wall paper file name. This method
     * can be called on UI thread.
     * 
     * @return The resource id of chat wall paper or the file name of chat wall
     *         paper.
     */
    public int getChatWallpaperId() {

            return 0;
        
    }

    /**
     * Get chat wall paper resource. This method can be called on UI thread.
     * 
     * @return The resource id of chat wall paper or the file name of chat wall paper.
     */
    public String getChatWallpaper() {
        if (LOGGER.isActivated()) {
            LOGGER.debug("getChatWallpaper() from memory, mChatWallpaper: " + mChatWallpaper);
        }
        return mChatWallpaper;
    }
    

    
    

}
