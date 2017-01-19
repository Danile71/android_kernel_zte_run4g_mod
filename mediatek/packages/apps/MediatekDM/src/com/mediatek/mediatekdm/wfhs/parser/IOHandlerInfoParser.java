package com.mediatek.mediatekdm.wfhs.parser;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.List;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;

import org.xml.sax.SAXException;

import android.util.Log;

public class IOHandlerInfoParser {
    
    private static final String TAG = "DM/Wifi.parser.NodeIOHandlerInfoParser";
    
    public IOHandlerInfoParser() {
        SAXParserFactory factory = SAXParserFactory.newInstance();
        try {
            mParser = factory.newSAXParser();            
        } catch (ParserConfigurationException e) {
            e.printStackTrace();
        } catch (SAXException e) {
            e.printStackTrace();
        }
    }
    
    public List<IOHandlerInfo> parse(String filePath) {
        Log.d(TAG, "start to parse path..");
        try {
            mParser.parse(new File(filePath), mHandler);
        } catch (SAXException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } 
        return mHandler.infos();
    }
    
    public List<IOHandlerInfo> parse(File file) {
        Log.d(TAG, "start to parse file..");
        try {
            mParser.parse(file, mHandler);
        } catch (SAXException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } 
        return mHandler.infos();
    }

    public List<IOHandlerInfo> parse(InputStream is) {
        Log.d(TAG, "start to parse stream..");
        try {
            mParser.parse(is, mHandler);
        } catch (SAXException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }       
        return mHandler.infos();
    }

    private SAXParser mParser;
    private WifiIOHandlerInfoParseHandler mHandler = new WifiIOHandlerInfoParseHandler();
}
