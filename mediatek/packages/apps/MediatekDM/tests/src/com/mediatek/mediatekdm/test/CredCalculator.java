package com.mediatek.mediatekdm.test;

import android.util.Base64;

import java.io.UnsupportedEncodingException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public class CredCalculator {
    protected byte mNonce;
    protected final String mUserName;
    protected final String mPassword;

    public CredCalculator(byte firstNonce, String username, String password) {
        if (firstNonce < 1 || firstNonce > 8) {
            
        }
        mNonce = firstNonce;
        mUserName = username;
        mPassword = password;
    }

    protected byte nextNonce() {
        return (byte) ((mNonce & 7) + 1);
    }

    public byte updateNonce() {
        mNonce = nextNonce();
        return mNonce;
    }

    public String getNonce() {
        byte[] data = new byte[] {
            mNonce
        };
        return Base64.encodeToString(data, Base64.NO_WRAP);
    }

    public String calculateCred() {
        MessageDigest digest = null;
        String result = null;
        try {
            digest = MessageDigest.getInstance("MD5");
            byte[] identity = (mUserName + ":" + mPassword).getBytes("UTF-8");
            byte[] md5 = digest.digest(identity);
            byte[] enc = Base64.encode(md5, Base64.NO_WRAP);
            byte[] buf = new byte[enc.length + 1 + 1];
            System.arraycopy(enc, 0, buf, 0, enc.length);
            buf[enc.length] = (byte) ':';
            buf[enc.length + 1] = mNonce;
            md5 = digest.digest(buf);
            result = Base64.encodeToString(md5, Base64.NO_WRAP);
        } catch (NoSuchAlgorithmException e) {
            throw new Error(e);
        } catch (UnsupportedEncodingException e) {
            throw new Error(e);
        }
        return result;
    }
}
