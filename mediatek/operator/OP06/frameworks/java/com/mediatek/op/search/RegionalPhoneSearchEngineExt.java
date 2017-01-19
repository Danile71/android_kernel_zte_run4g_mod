package com.mediatek.op.search;

import android.content.Context;
import android.database.ContentObserver;
import android.database.Cursor;
import android.net.Uri;
import android.os.Handler;
import android.util.Log;

import com.mediatek.common.search.SearchEngineInfo;
import com.mediatek.common.regionalphone.RegionalPhone;
import com.mediatek.op.search.DefaultRegionalPhoneSearchEngineExt;

import java.util.List;
import java.util.ArrayList;

public class RegionalPhoneSearchEngineExt extends
        DefaultRegionalPhoneSearchEngineExt {
    
    private  Context mContext;
    private static final String TAG = "RegionalPhoneSearchEngineExt";
    private List<SearchEngineInfo> mSearchEngineInfos;

    public  List<SearchEngineInfo> initSearchEngineInfosFromRpm(Context context) {
    	 	  mContext = context;
              Log.d(TAG, "initSearchEngineInfosFromRpm");
              String sp = "--";
              String configInfo = "";
              String nilString = "nil";
              Uri uri = RegionalPhone.SEARCHENGINE_URI;
              mSearchEngineInfos = new ArrayList<SearchEngineInfo>();
              if (mContext == null) {
                  Log.d(TAG, "context is null");
                  return null;
              }
              if (mContext.getContentResolver() == null) {
                  Log.d(TAG, "content resolver is null");
                  return null;
              }
              Cursor cursor = null;
              try {
                  cursor = mContext.getContentResolver().query(uri, null, null, null, null);
                  if (cursor != null && cursor.getCount() > 0) {
                      int nameIndex = cursor.getColumnIndex(RegionalPhone.SEARCHENGINE.SEARCH_ENGINE_NAME);
                      int labelIndex = cursor.getColumnIndex(RegionalPhone.SEARCHENGINE.SEARCH_ENGINE_LABEL);
                      int keyWordIndex = cursor.getColumnIndex(RegionalPhone.SEARCHENGINE.KEYWORD);
                      int faviconIndex = cursor.getColumnIndex(RegionalPhone.SEARCHENGINE.FAVICON);
                      int searchUriIndex = cursor.getColumnIndex(RegionalPhone.SEARCHENGINE.SEARCH_URL);
                      int encodeIndex = cursor.getColumnIndex(RegionalPhone.SEARCHENGINE.ENCODING);
                      int suggestionUriIndex = cursor.getColumnIndex(RegionalPhone.SEARCHENGINE.SUGGESTION_URL);
                      while (cursor.moveToNext()) {
                          configInfo = (cursor.getString(nameIndex) != null
                                  ? cursor.getString(nameIndex) : nilString) + sp;
                          configInfo += (cursor.getString(labelIndex) != null
                                  ? cursor.getString(labelIndex) : nilString) + sp;
                          configInfo += (cursor.getString(keyWordIndex) != null
                                  ? cursor.getString(keyWordIndex) : nilString) + sp;
                          configInfo += (cursor.getString(faviconIndex) != null
                                  ? cursor.getString(faviconIndex) : nilString) + sp;
                          configInfo += (cursor.getString(searchUriIndex) != null
                                  ? cursor.getString(searchUriIndex) : nilString) + sp;
                          configInfo += (cursor.getString(encodeIndex) != null
                                  ? cursor.getString(encodeIndex) : nilString) + sp;
                          configInfo += cursor.getString(suggestionUriIndex) != null
                                  ? cursor.getString(suggestionUriIndex) : nilString;
                          String []configData = configInfo.split(sp);
                          if (configData.length != 7) {
                              Log.e(TAG, "configData length is wrong.");
                              return null;
                          }
                          SearchEngineInfo info = SearchEngineInfo.parseFrom(configInfo, sp);
                          mSearchEngineInfos.add(info);
                      }
                      return mSearchEngineInfos;
                  }
              } catch (Exception e) {
                  Log.e(TAG, "initSearchEngineInfosFromRpm failed." + e.toString());
              } finally {
                  if (cursor != null) {
                      cursor.close();
                  }
              }
              return null;
          }
}

