package com.mediatek.imsframework;;

import com.android.internal.util.FastXmlSerializer;
import com.android.internal.util.XmlUtils;

import com.mediatek.common.ims.mo.ImsLboPcscf;
import com.mediatek.common.ims.mo.ImsAuthInfo;
import com.mediatek.common.ims.mo.ImsXcapInfo;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlSerializer;

import android.util.AtomicFile;
import android.util.Log;
import android.util.Xml;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;

import libcore.io.IoUtils;
import libcore.util.Objects;

final class PersistentDataStore {
    private static final String TAG = "ImsFrameworkService";

    private static final boolean DBG = true;
    private static final boolean VDBG = false;

    // MO nodes information.
    private ArrayList<ImsMoInfo> mImsMoInfoList = new ArrayList<ImsMoInfo>();

    // The atomic file used to safely read or write the file.
    private final AtomicFile mAtomicFile;

    // True if the data has been loaded.
    private boolean mLoaded;

    // True if there are changes to be saved.
    private boolean mDirty;

    public PersistentDataStore() {
        mAtomicFile = new AtomicFile(new File("/data/data/com.mediatek.imsframework/ims-mo-list.xml"));
    }

    public void saveIfNeeded() {
        if (mDirty) {
            save();
            mDirty = false;
        }
    }

    public ImsMoInfo getImsMoInfo(String impi) {
        loadIfNeeded();
        int index = findImsMoInfo(impi);
        if (index >= 0) {
            return mImsMoInfoList.get(index);
        }
        return null;
    }

    public ImsMoInfo[] getImsMoInfoList() {
        loadIfNeeded();
        return mImsMoInfoList.toArray(new ImsMoInfo[mImsMoInfoList.size()]);
    }

    public boolean addImsMoInfo(ImsMoInfo moInfo) {
        loadIfNeeded();

        if(DBG) {
            Log.d(TAG, "addImsMoInfo:" + moInfo);
        }

        int index = findImsMoInfo(moInfo.getImpi());
        if (index >= 0) {
            ImsMoInfo other = mImsMoInfoList.get(index);
            mImsMoInfoList.set(index, moInfo);
        } else {
            mImsMoInfoList.add(moInfo);
        }
        setDirty();
        return true;
    }

    public boolean removeImsMoInfo(String impi) {
        int index = findImsMoInfo(impi);
        if (index >= 0) {
            mImsMoInfoList.remove(index);
            setDirty();
            return true;
        }
        return false;
    }

    private int findImsMoInfo(String impi) {
        int count = mImsMoInfoList.size();

        if(VDBG) {
            Log.d(TAG, "findImsMoInfo:" + impi);
        }

        for (int i = 0; i < count; i++) {
            if (mImsMoInfoList.get(i).getImpi().equals(impi)) {
                return i;
            }
        }
        return -1;
    }

    private void loadIfNeeded() {
        if (!mLoaded) {
            load();
            mLoaded = true;
        }
    }

    private void setDirty() {
        mDirty = true;
    }

    private void clearMoInfoList() {
        mImsMoInfoList.clear();
    }

    private void load() {
        clearMoInfoList();

        Log.i(TAG, "load");

        final InputStream is;
        try {
            is = mAtomicFile.openRead();
        } catch (FileNotFoundException ex) {
            return;
        }

        Log.i(TAG, "start to load");

        XmlPullParser parser;
        try {
            parser = Xml.newPullParser();
            parser.setInput(new BufferedInputStream(is), null);
            loadFromXml(parser);
        } catch (IOException ex) {
            Log.w(TAG, "Failed to load display manager persistent store data.", ex);
            clearMoInfoList();
        } catch (XmlPullParserException ex) {
            Log.w(TAG, "Failed to load display manager persistent store data.", ex);
            clearMoInfoList();
        } finally {
            IoUtils.closeQuietly(is);
        }
    }

    private void save() {
        final FileOutputStream os;
        try {
            os = mAtomicFile.startWrite();
            boolean success = false;
            try {
                XmlSerializer serializer = new FastXmlSerializer();
                serializer.setOutput(new BufferedOutputStream(os), "utf-8");
                saveToXml(serializer);
                serializer.flush();
                success = true;
            } finally {
                if (success) {
                    mAtomicFile.finishWrite(os);
                } else {
                    mAtomicFile.failWrite(os);
                }
            }
        } catch (IOException ex) {
            Log.w(TAG, "Failed to save display manager persistent store data.", ex);
        }
    }

    private void loadFromXml(XmlPullParser parser)
    throws IOException, XmlPullParserException {
        XmlUtils.beginDocument(parser, "Ims-mo-list-info");
        final int outerDepth = parser.getDepth();
        while (XmlUtils.nextElementWithin(parser, outerDepth)) {
            if (parser.getName().equalsIgnoreCase("Ims-mo-info")) {
                loadImsMoInfoListFromXml(parser);
            }
        }
    }

    private void loadImsMoInfoListFromXml(XmlPullParser parser)
    throws IOException, XmlPullParserException {
        final int outerDepth = parser.getDepth();

        ImsMoInfo imsMoInfo = null;

        while (XmlUtils.nextElementWithin(parser, outerDepth)) {
            //Log.d(TAG, "parser.getName():" + parser.getName());

            if (parser.getName().equalsIgnoreCase("Impi")) {
                if(imsMoInfo != null) {
                    Log.i(TAG, "Add ims MO info:" + imsMoInfo);
                    mImsMoInfoList.add(imsMoInfo);
                    imsMoInfo = null;
                }

                String impi = parser.nextText();
                if(impi == null) {
                    throw new XmlPullParserException(
                        "Missing Impi on imsMoInfo");
                }

                if (findImsMoInfo(impi) >= 0) {
                    throw new XmlPullParserException(
                        "Found duplicate impi");
                }
                Log.i(TAG, "Impi mo node:" + impi);
                imsMoInfo = new ImsMoInfo(impi);
            }

            if (parser.getName().equalsIgnoreCase("Pcscf")) {
                String pcscf = parser.nextText();
                imsMoInfo.setPcscf(pcscf);
            }

            if (parser.getName().equals(ImsLboPcscf.class.getSimpleName())) {
                String addressType = parser.getAttributeValue(null, "addressType");
                String address = parser.nextText();
                imsMoInfo.addLboPcscfList(new ImsLboPcscf(address, addressType));
            }

            if (parser.getName().equals(ImsAuthInfo.class.getSimpleName())) {
                String authType = parser.getAttributeValue(null, "authType");
                String relam = parser.getAttributeValue(null, "relam");
                String userName = parser.getAttributeValue(null, "userName");
                String userPwd = parser.getAttributeValue(null, "userPwd");
                imsMoInfo.setImsAuthInfo(new ImsAuthInfo(authType, relam, userName, userPwd));
            }

            if (parser.getName().equals(ImsXcapInfo.class.getSimpleName())) {
                String rootUri = parser.getAttributeValue(null, "rootUri");
                String auth = parser.getAttributeValue(null, "auth");
                String secret = parser.getAttributeValue(null, "secret");
                String authType = parser.getAttributeValue(null, "authType");
                imsMoInfo.setImsXcapInfo(new ImsXcapInfo(rootUri, auth, secret, authType));
            }
        }

        if(imsMoInfo != null) {
            Log.i(TAG, "Add ims MO info:" + imsMoInfo);
            mImsMoInfoList.add(imsMoInfo);
        }
    }

    private void saveToXml(XmlSerializer serializer) throws IOException {
        serializer.startDocument(null, true);
        serializer.setFeature("http://xmlpull.org/v1/doc/features.html#indent-output", true);
        serializer.startTag(null, "Ims-mo-list-info");
        serializer.startTag(null, "Ims-mo-info");
        for (ImsMoInfo imsMoInfo : mImsMoInfoList) {

            serializer.startTag(null, "Impi");
            serializer.text(imsMoInfo.getImpi());
            serializer.endTag(null, "Impi");

            if(imsMoInfo.getPcscf().length() > 0) {
                serializer.startTag(null, "Pcscf");
                serializer.text(imsMoInfo.getPcscf());
                serializer.endTag(null, "Pcscf");
            }

            for (ImsLboPcscf pcscf : imsMoInfo.getLboPcscfList()) {
                serializer.startTag(null, ImsLboPcscf.class.getSimpleName());
                serializer.attribute(null, "addressType", pcscf.getLboPcscfAddressType());
                serializer.text(pcscf.getLboPcscfAddress());
                serializer.endTag(null, ImsLboPcscf.class.getSimpleName());
            }

            ImsAuthInfo authInfo = imsMoInfo.getImsAuthInfo();
            if(authInfo != null) {
                serializer.startTag(null, ImsAuthInfo.class.getSimpleName());
                serializer.attribute(null, "authType", authInfo.getAuthType());
                serializer.attribute(null, "relam", authInfo.getRelam());
                serializer.attribute(null, "userName", authInfo.getUserName());
                serializer.attribute(null, "userPwd", authInfo.getUserPwd());
                serializer.endTag(null, ImsAuthInfo.class.getSimpleName());
            }

            ImsXcapInfo xcapInfo = imsMoInfo.getImsXcapInfo();
            if(xcapInfo != null) {
                serializer.startTag(null, ImsXcapInfo.class.getSimpleName());
                serializer.attribute(null, "rootUri", xcapInfo.getXcapRootUri());
                serializer.attribute(null, "auth", xcapInfo.getXcapAuth());
                serializer.attribute(null, "secret", xcapInfo.getXcapAuthSecret());
                serializer.attribute(null, "authType", xcapInfo.getXcapAuthType());
                serializer.endTag(null, ImsXcapInfo.class.getSimpleName());
            }
        }
        serializer.endTag(null, "Ims-mo-info");
        serializer.endTag(null, "Ims-mo-list-info");
        serializer.endDocument();
    }

}