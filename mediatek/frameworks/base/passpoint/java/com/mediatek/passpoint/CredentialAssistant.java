/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.passpoint;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.RemoteException;
import android.security.Credentials;
import android.security.KeyChain;
import android.security.IKeyChainService;
import android.security.KeyChain.KeyChainConnection;
import android.text.Html;
import android.text.TextUtils;
import android.util.Log;

import com.android.org.bouncycastle.asn1.ASN1InputStream;
import com.android.org.bouncycastle.asn1.ASN1Sequence;
import com.android.org.bouncycastle.asn1.DEROctetString;
import com.android.org.bouncycastle.asn1.x509.BasicConstraints;
import com.android.org.bouncycastle.asn1.pkcs.PrivateKeyInfo;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.lang.String;
import java.lang.StringBuilder;
import java.security.KeyFactory;
import java.security.KeyStore.PasswordProtection;
import java.security.KeyStore.PrivateKeyEntry;
import java.security.KeyStore;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.security.cert.Certificate;
import java.security.cert.CertificateEncodingException;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.PKCS8EncodedKeySpec;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.LinkedHashMap;

import org.apache.harmony.security.utils.AlgNameMapper;
/**
 * A helper class for accessing the raw data in the intent extra and handling
 * certificates.
 */
class CredentialAssistant {
    private static final String TAG = "CredentialAssistant";
    private static final String TAG2 = "CredentialAssistAdvance";
    private static final String PKEY_MAP_KEY = "HS20_PKEY_MAP_";

    private static CredentialAssistant instance;    
    
    private CredentialAssistant() {
    }

    public static CredentialAssistant getInstance(){
        if(instance == null){
            instance = new CredentialAssistant();
        }
        
        return instance;
    }
    
    public String getSha1FingerPrint(X509Certificate cert){
        try {
            MessageDigest sha1 = MessageDigest.getInstance("SHA1");
            Log.d(TAG2,"  Subject " + cert.getSubjectDN());
            Log.d(TAG2,"   Issuer  " + cert.getIssuerDN());
            sha1.update(cert.getEncoded());
            return bytesToHex(sha1.digest());
        } catch (Exception e) {
            e.printStackTrace();
        }

        return null;
    }

    public boolean isHardwareBackedKey(byte[] keyData) {
        try {
            ASN1InputStream bIn = new ASN1InputStream(new ByteArrayInputStream(keyData));
            PrivateKeyInfo pki = PrivateKeyInfo.getInstance(bIn.readObject());
            String algId = pki.getAlgorithmId().getAlgorithm().getId();
            String algName = AlgNameMapper.map2AlgName(algId);
            return KeyChain.isBoundKeyAlgorithm(algName);
        } catch (IOException e) {
            Log.e(TAG, "Failed to parse key data");
            return false;
        }
    }    

    public boolean installCaCertsToKeyChain(Context ctxt, List<X509Certificate> caCerts) {
        try {
            KeyChainConnection keyChainConnection = KeyChain.bind(ctxt);
            try {
                for (X509Certificate caCert : caCerts) {
                    byte[] bytes = null;
                    try {
                        bytes = caCert.getEncoded();
                    } catch (CertificateEncodingException e) {
                        throw new AssertionError(e);
                    }
                    if (bytes != null) {
                        try {
                            keyChainConnection.getService().installCaCertificate(bytes);
                        } catch (RemoteException e) {
                            Log.w(TAG, "installCaCertsToKeyChain(): " + e);
                            return false;
                        }
                    }
                }
            } finally {
                keyChainConnection.close();
            }
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            return false;
        }

        return true;
    }
    
    boolean saveKeyPair(byte[] pubkey, byte[] privatekey, String alias) {
        String key = toMd5(pubkey);
        LinkedHashMap<String, byte[]> map = new LinkedHashMap<String, byte[]>();
        map.put(key, privatekey);        
        Log.d(TAG, "save privatekey: " + key);
        return savePkeyMap(map, alias);
    }

    boolean savePkeyMap(Map<String, byte[]> map, String alias) {
        byte[] bytes = toBytes(map);
        if (!android.security.KeyStore.getInstance().put(PKEY_MAP_KEY + alias, bytes, android.security.KeyStore.UID_SELF, android.security.KeyStore.FLAG_ENCRYPTED)) {
            Log.w(TAG, "savePkeyMap(): failed to write pkey map");
            return false;
        }
        return true;
    }

    Map<String, byte[]> getPkeyMap(String alias) {
        byte[] bytes = android.security.KeyStore.getInstance().get(PKEY_MAP_KEY + alias);
        Map<String, byte[]> map = null;
        if (bytes != null) {
            map = (Map<String, byte[]>) fromBytes(bytes);
        }
        
        return map;
    }

    private String bytesToHex(byte[] bytes) {
        char[] hexArray = "0123456789ABCDEF".toCharArray();        
        char[] hexChars = new char[bytes.length * 2];
        for ( int j = 0; j < bytes.length; j++ ) {
            int v = bytes[j] & 0xFF;
            hexChars[j * 2] = hexArray[v >>> 4];
            hexChars[j * 2 + 1] = hexArray[v & 0x0F];
        }
        return new String(hexChars);
    }

    static byte[] toBytes(Object object) {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        try {
            ObjectOutputStream os = new ObjectOutputStream(baos);
            os.writeObject(object);
            os.close();
        } catch (Exception e) {
            Log.w(TAG, "toBytes(): " + e + ": " + object);
        }
        return baos.toByteArray();
    }

    static String toMd5(byte[] bytes) {
        try {
            MessageDigest algorithm = MessageDigest.getInstance("MD5");
            algorithm.reset();
            algorithm.update(bytes);
            return toHexString(algorithm.digest(), "");
        } catch(NoSuchAlgorithmException e){
            // should not occur
            Log.w(TAG, "toMd5(): " + e);
            throw new RuntimeException(e);
        }
    }

    private static String toHexString(byte[] bytes, String separator) {
        StringBuilder hexString = new StringBuilder();
        for (byte b : bytes) {
            hexString.append(Integer.toHexString(0xFF & b)).append(separator);
        }
        return hexString.toString();
    }

    static <T> T fromBytes(byte[] bytes) {
        if (bytes == null) return null;
        try {
            ObjectInputStream is =
                    new ObjectInputStream(new ByteArrayInputStream(bytes));
            return (T) is.readObject();
        } catch (Exception e) {
            Log.w(TAG, "fromBytes(): " + e);
            return null;
        }
    }    
}
