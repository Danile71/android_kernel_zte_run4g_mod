package com.mediatek.mediatekdm.wfhs.parser;

import java.util.LinkedList;
import java.util.List;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;

import android.util.Log;

public class WifiIOHandlerInfoParseHandler extends DefaultHandler {
    
    private static final String TAG = "DM/Wifi.parser.NodeIOHandlerInfoParser";
    
    @Override
    public void startDocument() throws SAXException {
        Log.d(TAG, "startDocument");
    }

    @Override
    public void endDocument() throws SAXException {
        Log.d(TAG, "endDocument");
    }

    @Override
    public void startElement(String uri, String localName, String qName,
            Attributes attributes) throws SAXException {
        Log.d(TAG, "[startElement] uri : " + uri);
        Log.d(TAG, "[startElement] localName : " + localName);
        Log.d(TAG, "[startElement] qName : " + qName);
        Log.d(TAG, "[startElement] attributes : " + attributes);
        
        if (Element.INFOS.equals(localName)) {
            mInfos = new LinkedList<IOHandlerInfo>();
        } else if (Element.INFO.equals(localName)) {
            mCurrInfo = new IOHandlerInfo();
        }
        mNode = localName;
    }
    
    @Override
    public void characters(char[] ch, int start, int length)
            throws SAXException {
        Log.d(TAG, "[characters] ch : " + ch);
        Log.d(TAG, "[characters] start : " + start);
        Log.d(TAG, "[characters] length : " + length);
        
        mValue = new String(ch, start, length);
        Log.i(TAG, "the element <" + mNode + "> value : " + mValue); 
        
        if (Element.LOCAL_URI.equals(mNode)) {
            mCurrInfo.localUri = mValue;
        } else if (Element.HANDLER_NAME.equals(mNode)) {
            mCurrInfo.handlerName = mValue;
        } else if (Element.BROTHER.equals(mNode)) {
            mCurrInfo.hasBrother = "yes".equals(mValue);
        } else if (Element.PREFER_SESSION.equals(mNode)) {
            mCurrInfo.addPreferSessionId(Integer.valueOf(mValue));
        } else if (Element.NAME.equals(mNode)) {
            mCurrInfo.addPreferNode(mValue);
        } else if (Element.KEY.equals(mNode)) {
            mCurrInfo.addPreferKey(mValue);
        }
    }
    
    @Override
    public void endElement(String uri, String localName, String qName)
            throws SAXException {
        Log.d(TAG, "[endElement] uri : " + uri);
        Log.d(TAG, "[endElement] localName : " + localName);
        Log.d(TAG, "[endElement] qName : " + qName);
        
        if (Element.INFO.equals(localName)) {
            mInfos.add(mCurrInfo);
        }
        mNode = "";
    }
    
    public List<IOHandlerInfo> infos() {
        return mInfos;
    }

    private static final class Element {
        public static final String INFOS            = "infos";
        public static final String INFO             = "info";
        public static final String LOCAL_URI        = "local-uri";
        public static final String HANDLER_NAME     = "handler-name";
        public static final String BROTHER          = "brother";
        public static final String PREFER_SESSION   = "prefer-session";
        public static final String PREFER_NODE      = "prefer-node";
        public static final String NAME             = "name";
        public static final String KEY              = "key";
    }

    private List<IOHandlerInfo> mInfos;
    private IOHandlerInfo mCurrInfo;
    private String mNode;
    private String mValue;
}
