package com.mediatek.ut;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.FileInputStream;

import android.content.Context;
import android.util.Log;

import org.json.JSONObject;
import org.json.JSONArray;

public class JsonParser {
	static private final String TAG = "SFT";
	private JSONObject mReader;
	private JSONObject mGlobalConfig;
	private JSONArray mFrameArray;
	private JSONObject mVirtualDisplayConfig;
	private JSONArray mVirtualDisplayFrameArray;

	public JsonParser(Context context, File file) {
		int length = (int) file.length();
		byte[] bytes = new byte[length];

		try {
            FileInputStream in = new FileInputStream(file);

			try {
		    	in.read(bytes);
			} finally {
			    in.close();
			}
        } catch (Exception e) {
            Log.e(TAG, "[Json]can't open file "+ e);
        } finally {

        }

		String s = new String(bytes);
		Log.d(TAG, "[Json] file string: "+s);

		try {
			mReader = new JSONObject(s);
			mGlobalConfig = new JSONObject(mReader.getString("GlobalConfig"));
			mVirtualDisplayConfig = new JSONObject(mReader.getString("VirtualDisplayConfig"));
			mFrameArray = new JSONArray(mReader.getString("Frame"));
			mVirtualDisplayFrameArray = new JSONArray(mReader.getString("VirtualDisplayFrame"));
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		if (mGlobalConfig == null)
			Log.e(TAG, "[Json] can't get GlobalConfig");

		if (mVirtualDisplayConfig == null)
			Log.e(TAG, "[Json] can't get VirtualDisplayConfig");

		if (mFrameArray == null)
			Log.e(TAG, "[Json] can't get Frame array");

		if (mVirtualDisplayFrameArray == null)
			Log.e(TAG, "[Json] can't get VirtualDisplayFrame array");
	}

	public String getGlobalConfig(String key) {
		String r = null;
		try {
			if (mGlobalConfig != null) {
				r = mGlobalConfig.getString(key);
				Log.d(TAG, "[Json] getGlobalConfig, key:"+key+", return:"+r);
			}
			else {
				Log.w(TAG, "[Json] mGlobalConfig is null ");
			}
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		return r;
	}

	public String getVirtualDisplayConfig(String key) {
		String r = null;
		try {
			if (mVirtualDisplayConfig != null) {
				r = mVirtualDisplayConfig.getString(key);
				Log.d(TAG, "[Json] getVirtualDisplayConfig, key:"+key+", return:"+r);
			}
			else {
				Log.w(TAG, "[Json] mVirtualDisplayConfig is null ");
			}
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		return r;
	}

	public String getFrameArray(int idx, String key) {
		String r = null;

		if (mFrameArray == null) {
			Log.e(TAG, "[Json] mFrameArray is null ");
			return r;
		}
		if (idx >= mFrameArray.length()) {
			Log.e(TAG, "[Json] Frame idx="+idx+" is too big than length "+ mFrameArray.length());
			return r;
		}

		try {
			JSONObject object = mFrameArray.getJSONObject(idx);
			if (object != null) {
				r = object.getString(key);
				Log.d(TAG, "[Json] getFrameArray["+idx+"], key:"+key+", return:"+r);
			}
			else {
				Log.w(TAG, "[Json] frame object is null ");
			}
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		return r;
	}

	public String getVirtualDisplayFrameArray(int idx, String key) {
		String r = null;

		if (mVirtualDisplayFrameArray == null) {
			Log.e(TAG, "[Json] mVirtualDisplayFrameArray is null ");
			return r;
		}
		if (idx >= mVirtualDisplayFrameArray.length()) {
			Log.e(TAG, "[Json] VirtualDisplayFrame idx="+idx+" is too big than length "+ mVirtualDisplayFrameArray.length());
			return r;
		}

		try {
			JSONObject object = mVirtualDisplayFrameArray.getJSONObject(idx);
			if (object != null) {
				r = object.getString(key);
				Log.d(TAG, "[Json] getVirtualDisplayFrameArray["+idx+"], key:"+key+", return:"+r);
			}
			else {
				Log.w(TAG, "[Json] virtual display frame object is null ");
			}
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		return r;
	}
}
