package com.mediatek.voiceextension.cfg;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.HashMap;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import android.util.Log;
import android.util.Xml;
import android.content.Context;

import com.mediatek.voiceextension.R;

public class ConfigurationXml {

    static final String TAG = "ConfigurationXml";
    private final Context mContext;
    
    public ConfigurationXml(Context context) {
        mContext = context;
    }
    
    /*
     * read voice file path info from res
     *
     * @param pathMap
     */
    public void readVoiceFilePathFromXml(HashMap<String, String> pathMap) {

        XmlPullParser parser = mContext.getResources().getXml(
                R.xml.viepath);
        if (parser == null) {
            Log.e(TAG, "the package has no voice command path ");
            return;
        }

        try {
            int xmlEventType;
            String processName = null;
            String path = null;
            while ((xmlEventType = parser.next()) != XmlPullParser.END_DOCUMENT) {
                String name = parser.getName();
                if (xmlEventType == XmlPullParser.START_TAG
                        && "Path".equals(name)) {
                    processName = parser.getAttributeValue(null, "Name");
                    path = parser.getAttributeValue(null, "Path");
                } else if (xmlEventType == XmlPullParser.END_TAG
                        && "Path".equals(name)) {
                    if (processName != null & path != null) {
                        pathMap.put(processName, path);
                    } else {
                        Log.v(TAG, "the package has no voice command path ");
                    }
                }
            }
        } catch (XmlPullParserException e) {
            Log.v(TAG, "Got execption parsing paths.", e);
        } catch (IOException e) {
            Log.v(TAG, "Got execption parsing paths.", e);
        } catch (Exception e) {
            Log.v(TAG, "readVoiceFilePathFromXml Got execption. ", e);
        }
    }
    
}
