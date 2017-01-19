package com.mediatek.gba.header;

import android.util.Log;

import java.util.HashMap;
import java.util.Map;

import org.apache.http.auth.MalformedChallengeException;
import org.apache.http.HeaderElement;
import org.apache.http.message.BasicHeaderValueParser;
import org.apache.http.message.HeaderValueParser;

public class AuthInfoHeader {

    public static final String HEADER_NAME = "Authentication-Info";
    
    public static final String RSPAUTH  = "rspauth";
    public static final String NONCE    = "nonce";
    public static final String CNONCE   = "cnonce";
    public static final String QOP      = "qop";
    public static final String NC       = "nc";    
    public static final String OPAQUE   = "opaque";
        
    private String mQop;
    private String mRspauth;
    private String mCnonce;
    private String mNonceCount;
    private String mOpaque;
    private String mNonce;

    public AuthInfoHeader() {
        
    }

    public AuthInfoHeader(String qop, String rspauth, String cnonce, String nonceCount,
            String opaque, String nonce) {        
        mQop = qop;
        mRspauth = rspauth;
        mCnonce = cnonce;
        mNonceCount = nonceCount;
        mOpaque = opaque;
        mNonce = nonce;
    }

    public String getQop() {
        return mQop;
    }

    public void setQop(String qop) {
        mQop = qop;
    }

    public String getRspauth() {
        return mRspauth;
    }

    public void setRspauth(String rspauth) {
        mRspauth = rspauth;
    }

    public String getCnonce() {
        return mCnonce;
    }

    public void setCnonce(String cnonce) {
        mCnonce = cnonce;
    }

    public String getNonceCount() {
        return mNonceCount;
    }

    public void setNonceCount(String nc) {
        mNonceCount = nc;
    }

    public String getOpaque() {
        return mOpaque;
    }

    public void setOpaque(String opaque) {
        mOpaque = opaque;
    }

    public String getNonce() {
        return mNonce;
    }

    public void setNonce(String nonce) {
        mNonce = nonce;
    }

    @Override
    public String toString() {
        return "AuthenticationInfoHeader [qop=" + mQop + ", rspauth=" + mRspauth + ", cnonce="
                + mCnonce + ", nonceCount=" + mNonceCount + ", opaque=" + mOpaque + ", nonce=" + mNonce
                + "]";
    }
    
    public static AuthInfoHeader parse(String headerValue) throws MalformedChallengeException{
        AuthInfoHeader obj = null;
        
        if(headerValue == null){
            throw new MalformedChallengeException("Header value is null");
        }

        HeaderValueParser parser = BasicHeaderValueParser.DEFAULT;
        HeaderElement[] elements = BasicHeaderValueParser.parseElements(headerValue, parser);
        if (elements.length == 0) {
            throw new MalformedChallengeException("Authentication challenge is empty");
        }
        Map<String, String> params = new HashMap<String, String>(elements.length);
        
        for (HeaderElement element : elements) {            
            params.put(element.getName(), element.getValue());
        }
        
        String qop     = params.get(QOP);
        String rspauth = params.get(RSPAUTH);        
        String cnonce  = params.get(CNONCE);
        String nc      = params.get(NC);
        String opaque  = params.get(OPAQUE);
        String nonce   = params.get(NONCE);        
                
        obj = new AuthInfoHeader(qop, rspauth, cnonce, nc, opaque, nonce);        
        return obj;     
    }
}
