package com.mediatek.gba.header;


import android.util.Log;

import java.util.HashMap;
import java.util.Map;

import org.apache.http.auth.AUTH;
import org.apache.http.auth.MalformedChallengeException;
import org.apache.http.protocol.HTTP;

import org.apache.http.HeaderElement;
import org.apache.http.message.BasicHeaderValueParser;
import org.apache.http.message.HeaderValueParser;
import org.apache.http.message.ParserCursor;
import org.apache.http.util.CharArrayBuffer;


public class WwwAuthHeader {
    private static final String TAG = "WwwAuthenticateHeader";

    private static final String DIGEST_SCHEME = "Digest";
    private static final String REALM = "realm";
    private static final String NONCE = "nonce";
    private static final String ALGO = "algorithm";
    private static final String QOP   = "qop";
    private static final String QOP_AUTH = "auth";
    private static final String QOP_AUTH_INT = "auth-int";
    private static final String OPAQUE = "opaque";

    private String mSchemeName;
    private String mRealm;
    private String mNonce;
    private String mAlgorithm;
    private String mQop;
    private String mOpaque;    

    public WwwAuthHeader() {
        
    }

    public WwwAuthHeader(String schemeName, String realm, String nonce, String algorithm,
            String qop, String opaque) {
        mSchemeName = schemeName;
        mRealm = realm;
        mNonce = nonce;
        mAlgorithm = algorithm;
        mQop = qop;
        mOpaque = opaque;        
    }

    public String getRealm() {
        return mRealm;
    }

    public void setRealm(String realm) {
        mRealm = realm;
    }

    public String getNonce() {
        return mNonce;        
    }

    public void setNonce(String nonce) {
        mNonce = nonce;
    }

    public String getAlgorithm() {
        return mAlgorithm;
    }

    public void setAlgorithm(String algorithm) {
        mAlgorithm = algorithm;
    }

    public String getQop() {
        return mQop;        
    }

    public void setQop(String qop) {
        mQop = qop;
    }

    public String getOpaque() {
        return mOpaque;
    }

    public void setOpaque(String opaque) {
        mOpaque = opaque;
    }

    public String getSchemeName() {
        return mSchemeName;
    }
    
    public void setSchemeName(String schemeName) {
        mSchemeName = schemeName;
    }

    @Override
    public String toString() {
        return "WwwAuthHeader [realm=" + mRealm + ", schema name=" + mSchemeName + ", nonce=" + mNonce
                + ", algorithm=" + mAlgorithm + ", qop=" + mQop + ", opaque=" + mOpaque + "]";
    }

    public static WwwAuthHeader parse(String headerValue) throws MalformedChallengeException{
        WwwAuthHeader obj = null;
        
        if(headerValue == null){
            throw new MalformedChallengeException("Header value is null");
        }
        
        if(!headerValue.startsWith(DIGEST_SCHEME)){
            throw new MalformedChallengeException("Invalid scheme identifier: " + headerValue);
        }        
        
        CharArrayBuffer buffer;
        int pos;
        
        buffer = new CharArrayBuffer(headerValue.length());
        buffer.append(headerValue);
        pos = 0;

        while (pos < buffer.length() && !HTTP.isWhitespace(buffer.charAt(pos))) {
            pos++;
        }
        String schemeName = buffer.substring(0, pos);

        HeaderValueParser parser = BasicHeaderValueParser.DEFAULT;
        ParserCursor cursor = new ParserCursor(pos, buffer.length());
        HeaderElement[] elements = parser.parseElements(buffer, cursor);
        if (elements.length == 0) {
            throw new MalformedChallengeException("Authentication challenge is empty");
        }
        Map<String, String> params = new HashMap<String, String>(elements.length);

        for (HeaderElement element : elements) {
            params.put(element.getName(), element.getValue());
        }

        String realm  = params.get(REALM);
        String nonce  = params.get(NONCE);
        String algo    = params.get(ALGO);
        String qop    = params.get(QOP);
        String opaque = params.get(OPAQUE);

        obj = new WwwAuthHeader(schemeName, realm, nonce, algo, qop, opaque);
        Log.d(TAG, "Dump:" + obj);
        return obj;
    }    
}
