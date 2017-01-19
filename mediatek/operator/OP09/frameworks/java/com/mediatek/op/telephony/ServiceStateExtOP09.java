package com.mediatek.op.telephony;

import android.content.Context;
import android.os.Environment;
import android.util.Log;
import android.util.Xml;

import com.android.internal.util.XmlUtils;

import com.mediatek.xlog.Xlog;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

public class ServiceStateExtOP09 extends ServiceStateExt {
    private static final String TAG = "CDMA";
    private static final String SPN_OVERRIDE_PATH = "etc/spn-conf-op09.xml";

    private Context mContext;

    public ServiceStateExtOP09() {
    }

    public ServiceStateExtOP09(Context context) {
        mContext = context;
    }

    @Override
    public Map<String, String> loadSpnOverrides() {
        Map<String, String> items = new HashMap<String, String>();

        final File spnFile = new File(Environment.getRootDirectory(), SPN_OVERRIDE_PATH);
        Log.d(TAG, "load files: " + Environment.getRootDirectory() + "/" + SPN_OVERRIDE_PATH);
        FileReader spnReader = null;
        try {
            spnReader = new FileReader(spnFile);

            XmlPullParser parser = Xml.newPullParser();
            parser.setInput(spnReader);

            XmlUtils.beginDocument(parser, "spnOverrides");
            while (true) {
                XmlUtils.nextElement(parser);
                String name = parser.getName();
                if (!"spnOverride".equals(name)) {
                    break;
                }
                String numeric = parser.getAttributeValue(null, "numeric");
                String data = parser.getAttributeValue(null, "spn");
                items.put(numeric, data);
            }

            Log.d(TAG, "load spn overrides ok.");
            return items;
        } catch (FileNotFoundException e) {
            Log.d(TAG, "Exception in spn-conf parser " + e);
            return null;
        } catch (XmlPullParserException e) {
            Log.d(TAG, "Exception in spn-conf parser " + e);
            return null;
        } catch (IOException e) {
            Log.d(TAG, "Exception in spn-conf parser " + e);
            return null;
        } finally {
            if (spnReader != null) {
                try {
                    spnReader.close();
                } catch (IOException e) {
                    Log.d(TAG, "Exception in spn-conf parser " + e);
                }
            }
        }
    }

    public boolean allowSpnDisplayed() {
        return false;
    }

    public boolean supportEccForEachSIM() {
        return true;
    }

    public void updateOplmn(Context context, Object ci) {
        new OplmnUpdateCenter().startOplmnUpdater(context, ci);
        Xlog.d(TAG, "custom for oplmn update");
    }
}
