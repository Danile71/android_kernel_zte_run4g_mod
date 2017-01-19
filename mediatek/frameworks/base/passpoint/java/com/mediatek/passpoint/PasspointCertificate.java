package com.mediatek.passpoint;

import android.util.Log;
import android.util.Base64;
import android.os.Environment;
import android.os.SystemProperties;
import android.os.Process;
import android.os.Binder;
import android.os.RemoteException;
import android.content.Intent;
import android.content.Context;
import android.security.*;
import android.widget.Toast;

import java.io.*;
import java.net.*;
import java.security.cert.Certificate;
import java.security.KeyStore;
import java.security.KeyStore.PasswordProtection;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.Provider;
import java.security.Security;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.KeyFactory;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.security.cert.CertificateException;
import java.security.spec.PKCS8EncodedKeySpec;

import java.math.BigInteger;

import java.util.*;
import java.util.zip.GZIPInputStream;

import javax.net.ssl.*;
import javax.security.auth.x500.X500Principal;

import com.android.org.bouncycastle.asn1.*;
import com.android.org.bouncycastle.jce.PKCS10CertificationRequest;
import com.android.org.bouncycastle.asn1.util.ASN1Dump;
import com.android.org.bouncycastle.asn1.pkcs.PKCSObjectIdentifiers;
import com.android.org.bouncycastle.asn1.x509.*;
import com.android.org.bouncycastle.asn1.util.ASN1Dump;
import com.android.org.bouncycastle.jce.provider.BouncyCastleProvider;
import com.android.org.bouncycastle.asn1.x509.ExtendedKeyUsage;
import com.android.org.bouncycastle.util.io.pem.*;

import org.apache.http.auth.UsernamePasswordCredentials;
import org.apache.http.HttpResponse;
import org.apache.http.message.BasicHeader;
import org.apache.http.Header;

public class PasspointCertificate{
    private static final String TAG = "PasspointCertificate";
    private static final String TAG2 = "PasspointCertAdvance";
    
    private static Context mContext;
    public static KeyStore hs20KeyStore; // store ca cert (root ca, aaa trust root ca)
    public static KeyStore hs20PKCS12KeyStore; //store client cert (extract from PKCS12)
    private static android.security.KeyStore mKeyStore;
    public static String passWord = "wifi@123"; //hs20KeyStore password
    public static final String ENROLL = "Enroll";
    public static final String REENROLL = "ReEnroll";
    public static final String AAA_ROOT = "AAA_ROOT";
    public static final String SUBSCRIPTION_ROOT = "SUBSCRIPTION_ROOT";
    public static final String POLICY_ROOT = "POLICY_ROOT";
    private static PrivateKey enrollPrivKey;
    public final static String CACERT_ALIAS = "osu_ca_cert";
    private final static String ENROLL_CACERT_ALIAS = "enroll_cacert";
    private final static String ENROLL_CLIENTCERT_ALIAS= "enroll_clientcert";
    private static String clientCertAlias;

    private static String macAddress = "";
    private static String challengePassword = "";
    private static String SHAalgorithm = "";
    private static String commonName = "";
    private static String imeiMeid = "";

    private static boolean macAddressRequired = false;
    private static boolean challengePasswordRequired = false;
    private static boolean SHAalgorithmRequired = false;
    private static boolean commonNameRequired = false;
    private static boolean imeiRequired = false;
    private static boolean meidRequired = false;
    private static boolean devidRequired = false;
    private static boolean idkphs20authRequired = false;

    private static TrustManager[] myTrustManagerArray = new TrustManager[]{new CertTrustManager()};
    
    private static class HeaderProperty {
        private String key;
        private String value;

        public HeaderProperty(String key, String value) {
            this.key = key;
            this.value = value;
        }

        public String getKey() {
            return key;
        }
        public void setKey(String key) {
            this.key = key;
        }
        public String getValue() {
            return value;
        }
        public void setValue(String value) {
            this.value = value;
        }
    }    
    
    private static PasspointCertificate instance = null;
    
    private PasspointCertificate(){
        initKeyStore();
    }
    
    public static PasspointCertificate getInstance(Context ctxt){
        if(instance == null){
            instance = new PasspointCertificate();
        }

        if(ctxt != null){
            mContext = ctxt;
        }
        return instance;
    }
    
    public void initKeyStore(){
        try{            
            //init keystore
            mKeyStore = android.security.KeyStore.getInstance();
            // store the key and the certificate chain for client certificate
            hs20PKCS12KeyStore = KeyStore.getInstance("PKCS12", "BC");
            hs20PKCS12KeyStore.load(null, null);
        }catch(Exception e){
            Log.e(TAG, "initKeyStore err:" + e );
        }
    }

    private boolean testPutCertInKeyStore(android.security.KeyStore keyStore, String alias,
            Certificate cert) {
        try{
            byte[] certData = Credentials.convertToPem(cert);
            String certKeyName = Credentials.WIFI + Credentials.CA_CERTIFICATE + alias;
            Log.d(TAG, "putting certificate " + certKeyName + " in keystore");
            if (!keyStore.put(certKeyName, certData, android.security.KeyStore.UID_SELF, android.security.KeyStore.FLAG_ENCRYPTED)) {
                Log.e(TAG, "[testPutCertInKeyStore] Failed to install " + certKeyName + " as user " + android.security.KeyStore.UID_SELF);
                return false;
            } else {
                saveMappingOfEnrollCertAliasAndSha256(alias, "SHA256FINGERPRINT");
                String aliasConverted = new String(keyStore.get(Credentials.WIFI + "SHA256FINGERPRINT"));
                Log.d(TAG, "[testPutCertInKeyStore] converted alias = " + aliasConverted + ", original alias = " + alias);
                if (aliasConverted.equals(alias)) {
                    List<X509Certificate> certRestore = Credentials.convertFromPem(keyStore.get(Credentials.WIFI + Credentials.CA_CERTIFICATE + aliasConverted));
                    for (X509Certificate x509Cert : certRestore) {
                        Log.d(TAG, "[testPutCertInKeyStore] cert restored: " + x509Cert.toString());
                    }
                    return true;
                }
                
            }
            
        } catch (IOException e1) {
            Log.e(TAG, "testPutCertInKeyStore err:" + e1);
        } catch (CertificateException e2) {
            Log.e(TAG, "testPutCertInKeyStore err:" + e2);
        }

        return false;
    }

    /**
     * For test only.
     * Install /data/data/com.mediatek.security/ca.cer in to system trust store.
     * Browser and self trust manager can run checkServerTrusted() to access it.
     */
    public void testInstallAndroidCACert() {
        try {
            CredentialAssistant mCredentialAssistant = CredentialAssistant.getInstance();
            ArrayList<X509Certificate> caCerts = new ArrayList<X509Certificate>();
            CertificateFactory cf = CertificateFactory.getInstance("X.509");
            X509Certificate cert = (X509Certificate)cf
                                .generateCertificate(new ByteArrayInputStream(FileOperationUtil.Read(new File("/data/data/com.mediatek.security/ca.cer"))));     
            
            caCerts.add(cert);
            mCredentialAssistant.installCaCertsToKeyChain(mContext, caCerts);                                                    
        } catch (Exception e) {
            Log.e(TAG, "testInstallAndroidCACert err:" + e );
        }
    }
    
    private void testGetAndroidCACert() {
        try {
            Log.d(TAG, "testGetAndroidCACert");
            KeyStore ks = KeyStore.getInstance("AndroidCAStore");
            ks.load(null, null);
            Enumeration<String> aliases = ks.aliases();
            while (aliases.hasMoreElements()) {
                String alias = aliases.nextElement();
                Log.d(TAG, "Alias: " + alias);
                X509Certificate cert = (X509Certificate) 
                   ks.getCertificate(alias);
                Log.d(TAG, "Subject DN: " + 
                   cert.getSubjectDN().getName());
                Log.d(TAG, "Issuer DN: " + 
                   cert.getIssuerDN().getName());
            }
        } catch (Exception e) {
            Log.e(TAG, "testGetAndroidCACert err:" + e );
        }

            }     

    private void blockingSetGrantPermission(int senderUid, String alias, boolean value)
            throws RemoteException {
        Log.d(TAG, "UID: " + Binder.getCallingUid());
        KeyChain.KeyChainConnection connection = null;
        try {
            connection = KeyChain.bind(mContext);
            connection.getService().setGrant(senderUid, alias, value);
        } catch (InterruptedException e) {
            // should never happen. if it does we will not grant the requested permission
            Log.e(TAG, "interrupted while granting access");
            Thread.currentThread().interrupt();
        } finally {
            if (connection != null) {
                connection.close();
            }
        }
    }  

    private void testGetCertificateChain() {
        try {
            String alias = "osucacert";
            blockingSetGrantPermission(Process.SYSTEM_UID, alias, true);
            X509Certificate[] chain = KeyChain.getCertificateChain(mContext,
                alias);
            Log.d(TAG, "chain length: " + chain.length);
            for (X509Certificate x : chain) {
                Log.d(TAG, "Subject DN: "
                    + x.getSubjectDN().getName());
                Log.d(TAG, "Issuer DN: "
                    + x.getIssuerDN().getName());
            }     
        }catch(Exception e){
            Log.e(TAG, "testGetCertificateChain err:" + e );
        }

    }

    public void testSystemKeystore() {
        Log.d(TAG, "testSystemKeystore");
        String osuCertBase64 = "MIIJSDCCCDCgAwIBAgIQCejKXtsBdi5ba2R/oW10mDANBgkqhkiG9w0BAQsFADBP"+
                    "MQswCQYDVQQGEwJVUzERMA8GA1UEChMIRGlnaUNlcnQxLTArBgNVBAMTJERpZ2lD"+
                    "ZXJ0IEhvdHNwb3QgMi4wIEludGVybWVkaWF0ZSBDQTAeFw0xMzEyMDQwMDAwMDBa"+
                    "Fw0xNTEyMDkxMjAwMDBaMIGrMQswCQYDVQQGEwJVUzETMBEGA1UECBMKQ2FsaWZv"+
                    "cm5pYTEUMBIGA1UEBxMLU2FudGEgQ2xhcmExFzAVBgNVBAoTDldpLUZpIEFsbGlh"+
                    "bmNlMSowKAYDVQQLEyFIb3RzcG90IDIuMCBPbmxpbmUgU2lnbiBVcCBTZXJ2ZXIx"+
                    "LDAqBgNVBAMTI29zdS1zZXJ2ZXIucjItdGVzdGJlZC1hcnUud2ktZmkub3JnMIIB"+
                    "IjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4f4pX37fFMrw2JegmBQYPSQo"+
                    "3XsF+GAmEQZ3r2F0vDpTN400E1tfUFs1C8/XVR2QehtxZEi0eyPewgNEQ8KkkFwr"+
                    "c3IHHDApdPTmZE0fI/8IqzBYPKNDsTOC7RgZz/CRqCHughPxUxGIyAteKA3nFtTX"+
                    "PeQZKlJU+CYDkbz5Srh2YLORahLy3QX8DTmgEzSaMaUN+bokP3Jzshki9Ut8hMOT"+
                    "BRMpL5cOPJq5oTmBj0ubl9FTaIwmYPBrQyAtzllt9zssCbB8ItcmkJOZlfrA5h0U"+
                    "sPDXed1kB9Nj0uurPwAKfZLVr4UGFM3RnGhWif4DQ5tQc74ZDu/2QFap/gBhwwID"+
                    "AQABo4IFwTCCBb0wHwYDVR0jBBgwFoAUAtROoBracpdT0GvOtDINqapGspIwHQYD"+
                    "VR0OBBYEFKSj+9hwm8cG8330NNYRA4YBBwHnMIIBFAYDVR0RAQH/BIIBCDCCAQSC"+
                    "I29zdS1zZXJ2ZXIucjItdGVzdGJlZC1hcnUud2ktZmkub3JngixzdWJzY3JpcHRp"+
                    "b24tc2VydmVyLnIyLXRlc3RiZWQtYXJ1LndpLWZpLm9yZ4ImcG9saWN5LXNlcnZl"+
                    "ci5yMi10ZXN0YmVkLWFydS53aS1maS5vcmeCK3JlbWVkaWF0aW9uLXNlcnZlci5y"+
                    "Mi10ZXN0YmVkLWFydS53aS1maS5vcmegJwYLKwYBBAGCvmgBAQGgGAQWZW5nU1Ag"+
                    "T3JhbmdlIFRlc3QgT25seaAxBgsrBgEEAYK+aAEBAaAiBCBrb3JTUCDsmKTroIzs"+
                    "p4Ag7YWM7Iqk7Yq4IOyghOyaqTAOBgNVHQ8BAf8EBAMCBaAwHQYDVR0lBBYwFAYI"+
                    "KwYBBQUHAwEGCCsGAQUFBwMCMIGTBgNVHR8EgYswgYgwQqBAoD6GPGh0dHA6Ly9j"+
                    "cmwzLmRpZ2ljZXJ0LmNvbS9EaWdpQ2VydEhvdHNwb3QyMEludGVybWVkaWF0ZUNB"+
                    "LmNybDBCoECgPoY8aHR0cDovL2NybDQuZGlnaWNlcnQuY29tL0RpZ2lDZXJ0SG90"+
                    "c3BvdDIwSW50ZXJtZWRpYXRlQ0EuY3JsMIIBxAYDVR0gBIIBuzCCAbcwggGzBglg"+
                    "hkgBhv1sAQEwggGkMDoGCCsGAQUFBwIBFi5odHRwOi8vd3d3LmRpZ2ljZXJ0LmNv"+
                    "bS9zc2wtY3BzLXJlcG9zaXRvcnkuaHRtMIIBZAYIKwYBBQUHAgIwggFWHoIBUgBB"+
                    "AG4AeQAgAHUAcwBlACAAbwBmACAAdABoAGkAcwAgAEMAZQByAHQAaQBmAGkAYwBh"+
                    "AHQAZQAgAGMAbwBuAHMAdABpAHQAdQB0AGUAcwAgAGEAYwBjAGUAcAB0AGEAbgBj"+
                    "AGUAIABvAGYAIAB0AGgAZQAgAEQAaQBnAGkAQwBlAHIAdAAgAEMAUAAvAEMAUABT"+
                    "ACAAYQBuAGQAIAB0AGgAZQAgAFIAZQBsAHkAaQBuAGcAIABQAGEAcgB0AHkAIABB"+
                    "AGcAcgBlAGUAbQBlAG4AdAAgAHcAaABpAGMAaAAgAGwAaQBtAGkAdAAgAGwAaQBh"+
                    "AGIAaQBsAGkAdAB5ACAAYQBuAGQAIABhAHIAZQAgAGkAbgBjAG8AcgBwAG8AcgBh"+
                    "AHQAZQBkACAAaABlAHIAZQBpAG4AIABiAHkAIAByAGUAZgBlAHIAZQBuAGMAZQAu"+
                    "MIGBBggrBgEFBQcBAQR1MHMwJAYIKwYBBQUHMAGGGGh0dHA6Ly9vY3NwLmRpZ2lj"+
                    "ZXJ0LmNvbTBLBggrBgEFBQcwAoY/aHR0cDovL2NhY2VydHMuZGlnaWNlcnQuY29t"+
                    "L0RpZ2lDZXJ0SG90c3BvdDIwSW50ZXJtZWRpYXRlQ0EuY3J0MAwGA1UdEwEB/wQC"+
                    "MAAwggFCBggrBgEFBQcBDASCATQwggEwoIIBLDCCASiggZEwgY4wgYswdxYJaW1h"+
                    "Z2UvcG5nMDMwMTANBglghkgBZQMEAgEFAAQgz6p0qK2vhYIGyPW1v+5Fcoru6r1H"+
                    "q1DTYgySwVPDTGswNRYzaHR0cDovL3d3dy5yMi10ZXN0YmVkLndpLWZpLm9yZy9p"+
                    "Y29uX29yYW5nZV96eHgucG5nMBACAhBYAgIAgAIBPYQDenh4oIGRMIGOMIGLMHcW"+
                    "CWltYWdlL3BuZzAzMDEwDQYJYIZIAWUDBAIBBQAEIMs1XLp6IVnfjgrh2J+kgZ5B"+
                    "j69YDAjWKH9mIpgTV5WNMDUWM2h0dHA6Ly93d3cucjItdGVzdGJlZC53aS1maS5v"+
                    "cmcvaWNvbl9vcmFuZ2VfZW5nLnBuZzAQAgItcwICAKACAUyEA2VuZzANBgkqhkiG"+
                    "9w0BAQsFAAOCAQEANjdsq4cwI+j9w7U9kiWjq1xV9jYX1uShtUUA6N8pH1FZ/oB0"+
                    "8hSF34rmfIqPr8+nIDJ8etWGJY9/STvC2Web5VpXH+KvQC8nWm29kjfHoPYpVBuI"+
                    "6tWbFlgiI5AvX5HSjvCP59Y5WUEXveOWX5UNQNRPlOrbQJAg2sY8gwZcPJtCUPHd"+
                    "Rth+SAbJLlAoVSk0OCDicXHQ2Z0idgbRvtmTGk59BdRTWNits8h449w2Xmf+kJc7"+
                    "Acor7FioIgEHsUyUfM7bMChmGYWgvxR1leARfjnNTbhwkngZ1gQDmUtHeX6GkH3E"+
                    "snlZYLQKzrJZdBq7gz3GOzDlFSJdwpp8RXr7nQ==";  

        
        try {
            byte[] osuCert = Base64.decode(osuCertBase64, Base64.DEFAULT);
            CertificateFactory cf = CertificateFactory.getInstance("X.509");
            X509Certificate x509OsuCert = (X509Certificate)cf
                                .generateCertificate(new ByteArrayInputStream(osuCert));     
            android.security.KeyStore mKeyStore = android.security.KeyStore.getInstance();
            if (!testPutCertInKeyStore(mKeyStore, "osucacert", x509OsuCert)) {
                return;
            }      
/*
            if (mKeyStore.contains(Credentials.USER_CERTIFICATE + "osuca")) {
                String certificate = new String(mKeyStore.get(Credentials.USER_CERTIFICATE + "osuca"));
                Log.d(TAG, "certificate: " + certificate);
            } else {
                Log.d(TAG, "no USER_CERTIFICATE in keystore");
            }
            testGetCertificateChain();
            testGetAndroidCACert();  */          
        } catch (Exception e) {
            Log.d(TAG, "testSystemKeystore err:" + e);
        }
    }

    public static String getSubjectX500PrincipalFromPKCS12Keystore(String sha256FingerPrint){
        if (!mKeyStore.contains(Credentials.WIFI + sha256FingerPrint)) {
            Log.e(TAG, "[getSubjectX500PrincipalFromPKCS12Keystore] client cert (SHA256: " + sha256FingerPrint + ") does not exist !!!");
            return null;
        }

        try{
            String alias = new String(mKeyStore.get(Credentials.WIFI + sha256FingerPrint));
            Log.d(TAG, "[getSubjectX500PrincipalFromPKCS12Keystore] alias: " + alias);
            X509Certificate x509Cert = (X509Certificate)hs20PKCS12KeyStore.getCertificate(alias);
            return x509Cert.getSubjectX500Principal().toString();
        }catch(Exception e){
            Log.d(TAG, "getSubjectX500PrincipalFromPKCS12Keystore err:" + e);
        }
        return null;
    }   

    public static BigInteger getSerialNumberFromPKCS12Keystore(){
        Log.d(TAG, "[getSerialNumberFromPKCS12Keystore] clientCertAlias: " + clientCertAlias);
        if(clientCertAlias == null){
            return null;
        }
        
        try{
            X509Certificate x509Cert = (X509Certificate)hs20PKCS12KeyStore.getCertificate(clientCertAlias);
            BigInteger serialNumber = x509Cert.getSerialNumber();
            Log.d(TAG, "serialNumber: " + serialNumber);
            return serialNumber;
        }catch(Exception e){
            Log.d(TAG, "getSerialNumberFromPKCS12Keystore err:" + e);
            return null;
        }
    }   
    
    public static void setMacAddress(String address){
        macAddress = address;
    }

    public static void setImeiOrMeid(String s){
        imeiMeid = s;
    }

    public static String getClientKeystorePassword(){
        return passWord;
    }
    
    public static void cleanClientCertStore(){
        try{
            Enumeration enu = hs20PKCS12KeyStore.aliases();
            while(enu.hasMoreElements()){
                String alias = (String) enu.nextElement();
                Log.d(TAG2,"[cleanClientCertStore] KeyStore alias: " + alias);
                Log.d(TAG2,"[cleanClientCertStore] Certificate " + alias + ": " +hs20PKCS12KeyStore.getCertificate(alias).toString());
                hs20PKCS12KeyStore.deleteEntry(alias);
            }
        }catch(Exception e){
            Log.e(TAG, "cleanClientCertStore err:" + e);
        }

    }

    public boolean verifyCertFingerprint(X509Certificate x509Cert, String sha256){
        try{
            String fingerPrintSha256 = computeHash(x509Cert.getEncoded(), "SHA-256");
            
            if(fingerPrintSha256.equals(sha256)){
                return true;
            }
            
        }catch(Exception e){
            Log.e(TAG, "verifyCertFingerprint err:" + e);
        }
        return false;
    }    
    
    public static String computeHash(byte[] input, String type) throws NoSuchAlgorithmException, UnsupportedEncodingException{
        if(input == null){
            return null;
        }
        
        MessageDigest digest = MessageDigest.getInstance(type);
        digest.reset();

        byte[] byteData = digest.digest(input);
        StringBuffer sb = new StringBuffer();

        for (int i = 0; i < byteData.length; i++){
          sb.append(Integer.toString((byteData[i] & 0xff) + 0x100, 16).substring(1));
        }
        return sb.toString();
    }
    
    private static void csrAttrsParse(byte[] csrattr, String chanPassword){
        macAddressRequired = false;
        challengePasswordRequired = false;
        SHAalgorithmRequired = false;
        commonNameRequired = false;
        imeiRequired = false;
        meidRequired = false;   
        devidRequired = false;
        idkphs20authRequired = false;
        
        DataOutputStream dataOutputStream;       
        challengePassword = chanPassword;
        try {            
            ByteArrayInputStream bais = new ByteArrayInputStream(csrattr);
            ASN1InputStream asn1_is = new ASN1InputStream(bais);
            ASN1Sequence derObj = (ASN1Sequence)asn1_is.readObject();
            for(int i = 0; i< derObj.size(); i++){
                String oid = derObj.getObjectAt(i).toString();
                Log.d(TAG2,oid);
                if(oid.equals("1.3.6.1.1.1.1.22")){ //macAddress
                    macAddressRequired = true;
                    Log.d(TAG2,"macAddress required");
                }else if(oid.equals("1.3.6.1.4.1.40808.1.1.2")){ //id-kp-HS2.0-auth
                    Log.d(TAG2,"id-kp-HS2.0-auth required");
                    idkphs20authRequired = true;
                }else if(oid.equals("1.3.6.1.4.1.40808.1.1.3")){ //IMEI
                    Log.d(TAG2,"IMEI required");
                    imeiRequired = true;
                }else if(oid.equals("1.3.6.1.4.1.40808.1.1.4")){ //MEID
                    Log.d(TAG2,"MEID required");
                    meidRequired = true;
                }else if(oid.equals("1.2.840.113549.1.9.7")){ // challenge password
                    Log.d(TAG2,"challengePassword required");
                    challengePasswordRequired = true;
                }else if(oid.equals("1.3.132.0.34")){
//                    Description:    
//                        NIST curve P-384 (covers "secp384r1", the elliptic curve domain listed in See SEC 2: Recommended Elliptic Curve Domain Parameters)
//                        Information:    
//                        The SEC (Standards for Efficient Cryptography) curves provide elliptic curve domain parameters at commonly required security levels for use by implementers of ECC standards like ANSI X9.62, ANSI X9.63, IEEE P1363, and other standards.
                }else if(oid.equals("2.16.840.1.101.3.4.2.2")){ //SHA algorithm: "sha384"
                    SHAalgorithm = "SHA-384";
                }else if(oid.equals("2.5.4.3")){
                    Log.d(TAG2,"commonName required");
                    commonNameRequired = true;
                }else if(oid.equals("1.3.6.1.4.1.40808.1.1.5")){
                    Log.d(TAG2,"DevID required");
                    devidRequired = true;
                }
            }
            
        }catch(Exception e){
            Log.e(TAG, "csrAttrsParse err:" + e);
        }
    }

    private static KeyPair createKeyPair() {
        KeyPair keyPair = null;
        try {
            KeyPairGenerator keyGen = KeyPairGenerator.getInstance("RSA");
            keyGen.initialize(2048);
            keyPair = keyGen.generateKeyPair();
        } catch (NoSuchAlgorithmException e) {
            Log.e(TAG, "createKeyPair err:" + e);
        }
        return keyPair;
    }

    private static void genDERAttribute(DERObjectIdentifier oid, Object derObj, Vector oids, Vector values){               
        try {
            if(derObj instanceof GeneralName){
                oids.add(oid);
                values.add(new X509Extension(false, new DEROctetString(new GeneralNames((GeneralName)derObj))));            
            }else if(derObj instanceof DERPrintableString){
                oids.add(oid);
                values.add(new X509Extension(false, new DEROctetString((DERPrintableString)derObj)));            
            }else if(derObj instanceof DERIA5String){
                oids.add(oid);
                values.add(new X509Extension(false, new DEROctetString((DERIA5String)derObj)));            
            }else if(derObj instanceof DERBitString){
                oids.add(oid);
                values.add(new X509Extension(false, new DEROctetString((DERBitString)derObj)));            
            }else if(derObj instanceof DERObjectIdentifier){
                oids.add(oid);
                values.add(new X509Extension(false, new DEROctetString((DERObjectIdentifier)derObj)));            
            }else if(derObj instanceof DERInteger){
                oids.add(oid);
                values.add(new X509Extension(false, new DEROctetString((DERInteger)derObj)));            
            }else if(derObj instanceof ExtendedKeyUsage){
                oids.add(oid);
                values.add(new X509Extension(false, new DEROctetString((ExtendedKeyUsage)derObj)));            
            }                     
        } catch (IOException e){
            Log.e(TAG, "genDERAttribute err:" + e);
        }
    }


    
    private static PKCS10CertificationRequest csrGenerate(String subjectDN){
        try{
            final KeyPair kp = createKeyPair();
            enrollPrivKey = kp.getPrivate();

            //sha384
            if(!SHAalgorithm.isEmpty()){
                if("2.16.840.1.101.3.4.2.2".equals(SHAalgorithm)){
                    Log.d(TAG2,"SHA 384 required");
                }
            }

            String signatureAlgorithm = "sha1withRSA";           
            X500Principal subject; 
            subject = new X500Principal(subjectDN); // this is the username
            //Attributes
            ASN1EncodableVector attributesVector = new ASN1EncodableVector();
            Vector  oids = new Vector();
            Vector  values = new Vector();
            X509Extensions  extensions;
            Attribute  attribute;

            if(challengePasswordRequired == true){
                //challenge password
                ASN1ObjectIdentifier attrType = PKCSObjectIdentifiers.pkcs_9_at_challengePassword;
                ASN1Set attrValues = new DERSet(new  DERPrintableString(challengePassword));
                attribute = new Attribute(attrType, attrValues);
                attributesVector.add(attribute);
            }            

            //Extension
            //Extended Key Usage 
            if(idkphs20authRequired == true){
                genDERAttribute(X509Extensions.ExtendedKeyUsage, new ExtendedKeyUsage(new KeyPurposeId("1.3.6.1.4.1.40808.1.1.2")), oids, values);//id-kp-HS2.0-auth          
            }

            if(macAddressRequired == true){
                //mac address
                genDERAttribute(new DERObjectIdentifier("1.3.6.1.1.1.1.22"), new DERIA5String(macAddress), oids, values);
            }

            if(imeiRequired == true){
                //IMEI
                genDERAttribute(new DERObjectIdentifier("1.3.6.1.4.1.40808.1.1.3"), new DERIA5String(imeiMeid), oids, values);
            }            

            if(meidRequired == true){
                //MEID
                genDERAttribute(new DERObjectIdentifier("1.3.6.1.4.1.40808.1.1.4"), new DERBitString(imeiMeid.getBytes()), oids, values);
            }            

            if(devidRequired == true){
                //DevID
                genDERAttribute(new DERObjectIdentifier("1.3.6.1.4.1.40808.1.1.5"), new DERPrintableString("imei:" + imeiMeid), oids, values);
            }

            //complete attributes
            extensions = new X509Extensions(oids, values); 
            attribute = new Attribute(
                    PKCSObjectIdentifiers.pkcs_9_at_extensionRequest, 
                    new DERSet(extensions));             
            attributesVector.add(attribute);             
            DERSet attributesFinal = new DERSet(attributesVector);
            
            Security.addProvider(new BouncyCastleProvider());
            PKCS10CertificationRequest csr;
            if(challengePasswordRequired || macAddressRequired || SHAalgorithmRequired){
                DERSet attrs;
                //debugging for CSR attributes
                if("true".equals(SystemProperties.get("persist.service.nocsrattrs"))){
                    attrs = new DERSet();
                }else{
                    attrs = attributesFinal;                
                }
                csr = new PKCS10CertificationRequest(signatureAlgorithm, subject, kp.getPublic()/*identity.getPublicKey()*/, attrs, kp.getPrivate()/*privKey*/);
            }else{
                csr = new PKCS10CertificationRequest(signatureAlgorithm, subject, kp.getPublic()/*identity.getPublicKey()*/, new DERSet(), kp.getPrivate()/*privKey*/);
            }
            Log.d(TAG2,ASN1Dump.dumpAsString((Object)csr, true));
            
            return csr;
        }catch(Exception e){
            Log.e(TAG, "PKCS10CertificationRequest err:" + e );
        }
        return null;

    }

    public KeyStore getCredentialCertKeyStore (String sha256FingerPrint) {
        if (!mKeyStore.contains(Credentials.WIFI + sha256FingerPrint)) {
            return null;
        }

        try {
            String certAlias = new String(mKeyStore.get(Credentials.WIFI + sha256FingerPrint));
            List<X509Certificate> userCert = Credentials.convertFromPem(mKeyStore.get(Credentials.WIFI + "HS20" + Credentials.USER_CERTIFICATE + certAlias));
            List<X509Certificate> caCerts = Credentials.convertFromPem(mKeyStore.get(Credentials.WIFI + "HS20" + Credentials.CA_CERTIFICATE + certAlias));
            CredentialAssistant mCredentialAssistant = CredentialAssistant.getInstance();
            String key = CredentialAssistant.toMd5(userCert.get(0).getPublicKey().getEncoded());
            Map<String, byte[]> map = mCredentialAssistant.getPkeyMap(certAlias);
            byte[] privKey = map.get(key);
            PKCS8EncodedKeySpec ks = new PKCS8EncodedKeySpec(privKey);            
            KeyFactory kf = KeyFactory.getInstance("RSA");
            PrivateKey pk = kf.generatePrivate(ks); 
            
            int index = 0;
            Certificate[] chain = new Certificate[userCert.size() + caCerts.size()];
            
            for(Certificate item : userCert){
                chain[index] = item;
                index++;     
            }
            
            for(Certificate item : caCerts){
                chain[index] = item;
                index++;
            }     
            
            cleanClientCertStore();
            hs20PKCS12KeyStore.setKeyEntry(certAlias, pk, passWord.toCharArray(), chain);
            return hs20PKCS12KeyStore;
        } catch (Exception e) {
            Log.e(TAG, "getCredentialCertKeyStore err:" + e );
        } 

        return null;
    }

    public boolean saveMappingOfEnrollCertAliasAndSha256(String alias, String sha256FingerPrint) {
        if (alias == null || sha256FingerPrint == null) {
            return false;
        }
        Log.d(TAG, "[saveMappingOfEnrollCertAliasAndSha256] SHA256FingerPrint: " + sha256FingerPrint);
        return mKeyStore.put(Credentials.WIFI + sha256FingerPrint, alias.getBytes(), android.security.KeyStore.UID_SELF, android.security.KeyStore.FLAG_ENCRYPTED);
    }
    
    public String getEnrollCertAlias() {
        return clientCertAlias;
    }
    
    private boolean installEnrolledClientCert(PrivateKey privKey, List<X509Certificate> certificatePKCS7,
            List<X509Certificate> trustedRootCACert){

            try {
                 if (!mKeyStore.isUnlocked()) {
                     Log.e(TAG, "Credential storage locked!!!");
                     return false;
                 }
             } catch (NullPointerException e) {
                 Log.e(TAG, "Credential storage is uninitialized!!! " + e);
                return false;
            }
        
        try{
            CredentialAssistant mCredentialAssistant = CredentialAssistant.getInstance();            
            int rootCaAmount = trustedRootCACert.size();
            int clientCertAmount = certificatePKCS7.size();
            Certificate[] chain = new Certificate[rootCaAmount + clientCertAmount];

            //chain[2] = createMasterCert(caPubKey, caPrivKey);
            Log.d(TAG2,"rootCaAmount: " + rootCaAmount);
            Log.d(TAG2,"clientCertAmount: " + clientCertAmount);

            clientCertAlias = mCredentialAssistant.getSha1FingerPrint(certificatePKCS7.get(0));

            //Install client cert
            X509Certificate[] userCerts = certificatePKCS7.toArray(new X509Certificate[certificatePKCS7.size()]); 
            byte[] userCertsData = Credentials.convertToPem(userCerts);
            
            if (!mKeyStore.put(Credentials.WIFI + "HS20" + Credentials.USER_CERTIFICATE + clientCertAlias, userCertsData, android.security.KeyStore.UID_SELF, android.security.KeyStore.FLAG_ENCRYPTED)) {
                Log.e(TAG, "Failed to install " + Credentials.WIFI + "HS20" + Credentials.USER_CERTIFICATE + clientCertAlias + " as user " + android.security.KeyStore.UID_SELF);
                return false;
            }

            if (!mKeyStore.put(Credentials.WIFI + "HS20" + Credentials.USER_CERTIFICATE + clientCertAlias, userCertsData, Process.WIFI_UID, android.security.KeyStore.FLAG_NONE)) {
                Log.e(TAG, "Failed to install " + Credentials.WIFI + "HS20" + Credentials.USER_CERTIFICATE + clientCertAlias + " as user " + Process.WIFI_UID);
                return false;
            }    

            //Install keypair
            byte[] prikey = privKey.getEncoded();
            byte[] pubkey = userCerts[0].getPublicKey().getEncoded();        
            int flags = android.security.KeyStore.FLAG_ENCRYPTED;

            if (!mCredentialAssistant.saveKeyPair(pubkey, prikey, clientCertAlias)) {
                Log.e(TAG, "Failed to install private key as user " + android.security.KeyStore.UID_SELF);
                return false;
            }   
            
            if (mCredentialAssistant.isHardwareBackedKey(prikey)) {
                // Hardware backed keystore is secure enough to allow for WIFI stack 
                // to enable access to secure networks without user intervention
                Log.d(TAG, "Saving private key with FLAG_NONE for WIFI_UID");
                flags = android.security.KeyStore.FLAG_NONE;
            }

            if (!mKeyStore.importKey(Credentials.WIFI + "HS20" + Credentials.USER_PRIVATE_KEY + clientCertAlias, prikey, Process.WIFI_UID, flags)) {
                Log.e(TAG, "Failed to install " + Credentials.WIFI + "HS20" + Credentials.USER_PRIVATE_KEY + clientCertAlias + " as user " + Process.WIFI_UID);
                return false;
            }                                         

            //Install trusted ca certs     
            X509Certificate[] caCerts = trustedRootCACert.toArray(new X509Certificate[trustedRootCACert.size()]);  
            byte[] caCertsData = Credentials.convertToPem(caCerts);
            
            if (!mKeyStore.put(Credentials.WIFI + "HS20" + Credentials.CA_CERTIFICATE + clientCertAlias, caCertsData, android.security.KeyStore.UID_SELF, android.security.KeyStore.FLAG_ENCRYPTED)) {
                Log.e(TAG, "Failed to install " + Credentials.WIFI + "HS20" + Credentials.CA_CERTIFICATE + clientCertAlias + " as user " + android.security.KeyStore.UID_SELF);
                return false;
            }

            if (!mKeyStore.put(Credentials.WIFI + "HS20" + Credentials.CA_CERTIFICATE + clientCertAlias, caCertsData, Process.WIFI_UID, android.security.KeyStore.FLAG_NONE)) {
                Log.e(TAG, "Failed to install " + Credentials.WIFI + "HS20" + Credentials.CA_CERTIFICATE + clientCertAlias + " as user " + Process.WIFI_UID);
                return false;
            }        


            //Save SHA256 SHA1 mapping
            String sha256FingerPrint = computeHash(userCerts[0].getEncoded(), "SHA-256");
            saveMappingOfEnrollCertAliasAndSha256(clientCertAlias, sha256FingerPrint);
            
            if ("true".equals(SystemProperties.get("persist.service.hs20.cert.dbg"))){
                int index = 0;
                
                for(Certificate item : certificatePKCS7){
                    chain[index] = item;
                    index++;     
                }
                
                for(Certificate item : trustedRootCACert){
                    chain[index] = item;
                    index++;
                }     

                KeyStore ks = KeyStore.getInstance("PKCS12", "BC");
                ks.load(null, null);
                
                ks.setKeyEntry(ENROLL_CLIENTCERT_ALIAS, privKey, null, chain);
                FileOutputStream fOut = new FileOutputStream("/data/data/com.mediatek.security/est_client_cert.p12");                           
                ks.store(fOut, passWord.toCharArray());                
                fOut.close();
            }            
        }catch(Exception e){
            Log.e(TAG, "installEnrolledClientCert err:" + e);
            return false;
        }

        return true;
    }

    public static byte[] httpClient(String serverUrl, String method){
        try{
            boolean bGzipContent = false;  
            boolean bBase64 = false;
            URL url = new URL(serverUrl);
            //get response content
            InputStream in = null;
            
            if(serverUrl.startsWith("HTTPS://") || serverUrl.startsWith("https://")){                        
                //KeyStore keyStore = ...;
                //TrustManagerFactory tmf = TrustManagerFactory.getInstance("X509");
                //tmf.init(keyStore)
                // TODO: Implement  HostnameVerifier
                HostnameVerifier hv = new HostnameVerifier()
                {
                    @Override
                    public boolean verify(String urlHostName, SSLSession session){
                        Log.d(TAG2,"verify:" + urlHostName);
                        return true;
                    }
                };                

                SSLContext context = SSLContext.getInstance("TLS");
                context.init(null, myTrustManagerArray, null);
                HttpsURLConnection urlConnection = (HttpsURLConnection) url.openConnection();                        
                urlConnection.setSSLSocketFactory(context.getSocketFactory());
                urlConnection.setHostnameVerifier(hv);
                urlConnection.setDoOutput(true);
                urlConnection.setRequestMethod(method);
                //urlConnection.setRequestProperty("Connection", "close");
                urlConnection.setRequestProperty("Accept-Encoding", "gzip");
                
                //urlConnection.setRequestProperty("Content-Transfer-Encoding", "base64");
                     
                //get response header
                boolean bPKCS7 = false;
                Log.d(TAG2,"Response code :"+String.valueOf(urlConnection.getResponseCode()));
                Log.d(TAG2,"Header :"+String.valueOf(urlConnection.getHeaderFields().toString()));

                if(urlConnection.getResponseCode() == 200){
                    Map properties = urlConnection.getHeaderFields();
                    Set keys = properties.keySet();
                    List retList = new LinkedList();

                    for (Iterator i = keys.iterator(); i.hasNext();) {
                        String key = (String) i.next();
                        List values = (List) properties.get(key);
                
                        for (int j = 0; j < values.size(); j++) {
                            retList.add(new HeaderProperty(key, (String) values.get(j)));
                        }
                    }
                    
                    for (int i = 0; i < retList.size(); i++) {
                        HeaderProperty hp = (HeaderProperty)retList.get(i);
                        // HTTP response code has null key
                        if (null == hp.getKey()) {
                            continue;
                        }

                        if(hp.getKey().equalsIgnoreCase("Content-Type")){
                           if(hp.getValue().equals("application/pkcs7-mime") ||
                            hp.getValue().equals("application/x-x509-ca-cert")){ //cacert and client cert
                               bPKCS7 = true;
                           }
                        }

                        if(hp.getKey().equalsIgnoreCase("Content-Encoding") && 
                           hp.getValue().equalsIgnoreCase("gzip")) {
                            bGzipContent = true;
                        }

                        if(hp.getKey().equalsIgnoreCase("Content-Transfer-Encoding") && 
                                hp.getValue().equalsIgnoreCase("base64")) {
                                 bBase64= true;
                        }
                    }                    
                }
                
                
                if(bPKCS7){
                    in = urlConnection.getInputStream();
                    if(bGzipContent){
                        in = getUnZipInputStream(in);
                    }
                    
                    if(in != null)
                    {
                        ByteArrayOutputStream bos = new ByteArrayOutputStream();
                        byte[] buf = new byte[8192];
                                
                        while (true) {
                            int rd = in.read(buf, 0, 8192);
                            if (rd == -1) {
                                break;
                            }
                            bos.write(buf, 0, rd);
                        }
                                
                        bos.flush();
                        byte[] byteArray = bos.toByteArray();
                        if(bBase64){
                           String s = new String(byteArray);
                           Log.d(TAG2,"Content : " + s);
                           return Base64.decode(s, Base64.DEFAULT);                            
                        }                    
                        
                        return byteArray;
                    }                      
                }                        
                
            }
            else
            {
                HttpURLConnection urlConnection = (HttpURLConnection) url.openConnection();                        
                urlConnection.setDoOutput(true);
                urlConnection.setRequestMethod(method);
                //urlConnection.setRequestProperty("Connection", "close");
                urlConnection.setRequestProperty("Accept-Encoding", "gzip");             
                
                //urlConnection.setRequestProperty("Content-Transfer-Encoding", "base64");

                //get response header  
                boolean bPKCS7 = false;
                int responseCode = urlConnection.getResponseCode();
                Log.d(TAG2,"Response code :"+String.valueOf(responseCode));    
                Log.d(TAG2,"Header :"+String.valueOf(urlConnection.getHeaderFields().toString()));

                if(responseCode == 200){
                    Map properties = urlConnection.getHeaderFields();
                    Set keys = properties.keySet();
                    List retList = new LinkedList();

                    for (Iterator i = keys.iterator(); i.hasNext();) {
                        String key = (String) i.next();
                        List values = (List) properties.get(key);
                
                        for (int j = 0; j < values.size(); j++) {
                            retList.add(new HeaderProperty(key, (String) values.get(j)));
                        }
                    }
                    
                    for (int i = 0; i < retList.size(); i++) {
                        HeaderProperty hp = (HeaderProperty)retList.get(i);
                        // HTTP response code has null key
                        if (null == hp.getKey()) {
                            continue;
                        }

                        if(hp.getKey().equalsIgnoreCase("Content-Type")){
                            if(hp.getValue().equals("application/pkcs7-mime") ||
                            hp.getValue().equals("application/x-x509-ca-cert")){
                                bPKCS7 = true;
                            }
                         }

                        if(hp.getKey().equalsIgnoreCase("Content-Encoding") && 
                           hp.getValue().equalsIgnoreCase("gzip")) {
                            bGzipContent = true;
                        }
                        
                        if(hp.getKey().equalsIgnoreCase("Content-Transfer-Encoding") && 
                                hp.getValue().equalsIgnoreCase("base64")) {
                                 bBase64= true;
                        }
                    }                    
                }
                
                 if(bPKCS7){
                    in = urlConnection.getInputStream();
                    if(bGzipContent){
                        in = getUnZipInputStream(in);
                    }
                    
                    if(in != null)
                    {
                        ByteArrayOutputStream bos = new ByteArrayOutputStream();
                        byte[] buf = new byte[8192];
                                
                        while (true) {
                            int rd = in.read(buf, 0, 8192);
                            if (rd == -1) {
                                break;
                            }
                            bos.write(buf, 0, rd);
                        }
                                
                        bos.flush();
                        byte[] byteArray = bos.toByteArray();
                        if(bBase64){
                           String s = new String(byteArray);
                           Log.d(TAG2,"Content : " + s);
                           return Base64.decode(s, Base64.DEFAULT);                            
                        }                    
                        
                        return byteArray;
                    }                      
                }                
            }          
        }catch(Exception e){
            Log.e(TAG, "httpClient err:" + e);
        }
        return null;
    }

    public boolean installServerTrustRoot(String certURL, String CertSHA256Fingerprint, String certName, boolean remediation){
        Log.d(TAG, "[installServerTrustRoot] start for " + certName);                                                            
        Log.d(TAG, "[installServerTrustRoot] certURL " + certURL);                                                            

        if(certURL == null){ 
            return true;
        }else if(certURL.isEmpty()){
            return true;
        }

        int retryCount = 3;
        boolean isConnected = false;
        byte[] response = null; 
        
        try{
            
            if(!mKeyStore.contains(Credentials.WIFI + CertSHA256Fingerprint)){ //not having yet
                Log.d(TAG, "[installServerTrustRoot] get ca from:" + certURL);                        
            
                while(retryCount > 0 && !isConnected){ 
                    response = httpClient(certURL, "GET");

                    if(response == null){
                        Log.d(TAG, "[installServerTrustRoot] response is null");
                        retryCount--;
                        Log.d(TAG, "Wait for retry:" + retryCount);
                        Thread.sleep(3 * 1000);                        
                    }else{ 
                        isConnected = true;
                        if ("true".equals(SystemProperties.get("persist.service.hs20.cert.dbg"))){
                            String CaPath = "/data/data/com.mediatek.security/" + certName + ".crt";
                            FileOperationUtil.writeBytesToFile(response, CaPath);
                        }
                        byte[] trustRoot = response;
                        CertificateFactory cf = CertificateFactory.getInstance("X.509");
                        X509Certificate x509Cert = (X509Certificate) cf
                                .generateCertificate(new ByteArrayInputStream(trustRoot));
                    
                        if(verifyCertFingerprint(x509Cert, CertSHA256Fingerprint)){
                            CredentialAssistant mCredentialAssistant = CredentialAssistant.getInstance();
                            String aliasSHA1 = mCredentialAssistant.getSha1FingerPrint(x509Cert);
                    
                            if (certName.equals(AAA_ROOT)) {
                                if (!mKeyStore.put(Credentials.WIFI + "HS20" + Credentials.CA_CERTIFICATE + aliasSHA1, x509Cert.getEncoded(), Process.WIFI_UID, android.security.KeyStore.FLAG_NONE)) {
                                    Log.e(TAG, "[installServerTrustRoot] Failed to install " + Credentials.WIFI + "HS20" + Credentials.CA_CERTIFICATE + aliasSHA1 + " as user " + Process.WIFI_UID);
                                }                     
                            } else {
                                if (!mKeyStore.put(Credentials.WIFI + "HS20" + Credentials.CA_CERTIFICATE + aliasSHA1, x509Cert.getEncoded(), android.security.KeyStore.UID_SELF, android.security.KeyStore.FLAG_ENCRYPTED)) {
                                    Log.e(TAG, "[installServerTrustRoot] Failed to install " + Credentials.WIFI + "HS20" + Credentials.CA_CERTIFICATE + aliasSHA1 + " as user " + android.security.KeyStore.UID_SELF);
                                }      
                    
                                ByteArrayInputStream bais = new ByteArrayInputStream(trustRoot);
                                List<X509Certificate> caCerts = (List<X509Certificate>) cf
                                        .generateCertificates(bais);
                                mCredentialAssistant.installCaCertsToKeyChain(mContext, caCerts);                                                    
                            }
                    
                            saveMappingOfEnrollCertAliasAndSha256(aliasSHA1, CertSHA256Fingerprint);
                        }else{
                            Log.d(TAG, "Server Trust Root fingerprint verify fail!");                            
                            return false;
                        }
                    }
                }

                if (isConnected) {
                    if(remediation){
                        // TODO: remove old AAA server trust root
                    } 
                } else {
                    return false;
                }
            }     
        }catch(Exception e){
            Log.e(TAG, "installServerTrustRoot err:" + e);
            return false;
        }

        return true;
    }

    public void installServerTrustRootsFromPEM(String certFile, String certType){
        Log.d(TAG, "[installServerTrustRootsFromPEM] certFile: " + certFile);                                                            

        try{                    
            String serverCAs = FileOperationUtil.Read(certFile);
            String[] firstParts = serverCAs.split("-----BEGIN CERTIFICATE-----");
            for(String firstPart : firstParts){
                if(firstPart.indexOf("-----END CERTIFICATE-----") == -1){
                    continue;
                }
                String secPart = firstPart.substring(0, firstPart.indexOf("-----END CERTIFICATE-----"));
                Log.d(TAG2,"secPart: " + secPart);
                byte[] serverRoot = Base64.decode(secPart, Base64.DEFAULT);
                CertificateFactory cf = CertificateFactory.getInstance("X.509");                
                X509Certificate x509ServerRoot = (X509Certificate) cf
                        .generateCertificate(new ByteArrayInputStream(serverRoot));

                String aliasSHA1 = computeHash(x509ServerRoot.getEncoded(), "SHA-1");
                String fingerPrintSHA256 = computeHash(x509ServerRoot.getEncoded(), "SHA-256");
                Log.d(TAG, "SHA1 : "+ aliasSHA1);  
                Log.d(TAG, "SHA256 : "+ fingerPrintSHA256);        

                CredentialAssistant mCredentialAssistant = CredentialAssistant.getInstance();
                
                if (certType.equals("aaaroot")) {
                    if (!mKeyStore.put(Credentials.WIFI + "HS20" + Credentials.CA_CERTIFICATE + aliasSHA1, x509ServerRoot.getEncoded(), Process.WIFI_UID, android.security.KeyStore.FLAG_NONE)) {
                        Log.e(TAG, "[installServerTrustRoot] Failed to install " + Credentials.WIFI + "HS20" + Credentials.CA_CERTIFICATE + aliasSHA1 + " as user " + Process.WIFI_UID);
                    }                     
                } else {
                    if (!mKeyStore.put(Credentials.WIFI + "HS20" + Credentials.CA_CERTIFICATE + aliasSHA1, x509ServerRoot.getEncoded(), android.security.KeyStore.UID_SELF, android.security.KeyStore.FLAG_ENCRYPTED)) {
                        Log.e(TAG, "[installServerTrustRoot] Failed to install " + Credentials.WIFI + "HS20" + Credentials.CA_CERTIFICATE + aliasSHA1 + " as user " + android.security.KeyStore.UID_SELF);
                    }      
                
                    ByteArrayInputStream bais = new ByteArrayInputStream(serverRoot);
                    List<X509Certificate> caCerts = (List<X509Certificate>) cf
                            .generateCertificates(bais);
                    mCredentialAssistant.installCaCertsToKeyChain(mContext, caCerts);                                                    
                }
                
                saveMappingOfEnrollCertAliasAndSha256(aliasSHA1, fingerPrintSHA256);
                
            } 
        }catch(Exception e){
            Log.d(TAG2,"installServerTrustRootsFromPEM err:" + e);
        }            
    } 

    public boolean connectESTServer(final String serverUrl, String operation, final String digestUsername, final String digestPassword, final String subjectDN) { 
        Log.d(TAG2,"Certificate Enrollment Starts...(EST)");
        
        if("true".equals(SystemProperties.get("persist.service.soapenroltest"))){
            return true;
        }

        Security.addProvider(new BouncyCastleProvider());
        try{
            byte[] in = null;
            //Get CACert
            in = estHttpClient(serverUrl, "GET", "/cacerts", operation, null, digestUsername, digestPassword);
            if(in == null){
                Log.d(TAG2,"Something wrong getting CACert"); 
                return false;
            }
            if ("true".equals(SystemProperties.get("persist.service.hs20.cert.dbg"))){
                FileOperationUtil.writeBytesToFile(in, "/data/data/com.mediatek.security/est_ca_cert.p7b");
            }
            
            Provider provBC = Security.getProvider("BC");
            CertificateFactory cf = CertificateFactory.getInstance("X.509", provBC);

            ByteArrayInputStream bais = new ByteArrayInputStream(in);
            List<X509Certificate> gRootCA = (List<X509Certificate>) cf
                    .generateCertificates(bais);
            for(X509Certificate item : gRootCA){
                Log.d(TAG2,"gRootCA: \r" + item);
            }                
            //Get CSR Attributes
            in = estHttpClient(serverUrl, "GET", "/csrattrs", operation, null, digestUsername, digestPassword);
            if(in == null){
                Log.d(TAG2,"Something wrong getting CSR Attributes"); 
                return false;
            }
            //Parse CSR Attribute
            csrAttrsParse(in, digestPassword);
            
            //Enroll
            //Build CSR according to CSR Attributes 
            PKCS10CertificationRequest csrPkcs10;
            if(operation.equals(REENROLL)){
                csrPkcs10 = csrGenerate(subjectDN);
            }else{
                csrPkcs10 = csrGenerate("CN=" + digestUsername + ", OU=Mediatek, O=Mediatek, L=Taipei, ST=TW, C=TW");
                //csrPkcs10 = csrGenerate("CN=" + SystemProperties.get("persist.service.cn", "www.mediatek.com") + ", OU=Mediatek, O=Mediatek, L=Taipei, ST=TW, C=TW");
            }
             
             
            byte[] csr = csrPkcs10.getEncoded();
            //start Enroll
            if(operation.equals(ENROLL)){
                in = estHttpClient(serverUrl, "POST", "/simpleenroll", operation, csr, digestUsername, digestPassword);
                if(in == null){
                    Log.d(TAG2,"Something wrong Enrolling"); 
                    return false;
                }
                Log.d(TAG2,"Enrolled Client Certificate: \r" + new String(in));
                
                if ("true".equals(SystemProperties.get("persist.service.hs20.cert.dbg"))){
                    FileOperationUtil.writeBytesToFile(in, "/data/data/com.mediatek.security/est_client_cert.p7b");
                }
                
                bais = new ByteArrayInputStream(in);
                List<X509Certificate> x509Cert = (List<X509Certificate>) cf
                        .generateCertificates(bais);
                for(X509Certificate item : x509Cert){
                    Log.d(TAG2,"x509Cert: \r" + item);
                }
                                                    
                return installEnrolledClientCert(enrollPrivKey, x509Cert, gRootCA);                
            }else if(operation.equals(REENROLL)){
                in = estHttpClient(serverUrl, "POST", "/simplereenroll", operation, csr, digestUsername, digestPassword);
                if(in == null){
                    Log.d(TAG2,"Something wrong Re-Enrolling"); 
                    return false;
                }      
                Log.d(TAG2,"Enrolled Client Certificate: \r" + new String(in));
                
                if ("true".equals(SystemProperties.get("persist.service.hs20.cert.dbg"))){
                    FileOperationUtil.writeBytesToFile(in, "/data/data/com.mediatek.security/est_client_cert.p7b");
                }
                
                bais = new ByteArrayInputStream(in);
                List<X509Certificate> x509Cert = (List<X509Certificate>) cf
                        .generateCertificates(bais);
                for(X509Certificate item : x509Cert){
                    Log.d(TAG2,"x509Cert: \r" + item);
                }
               
                return installEnrolledClientCert(enrollPrivKey, x509Cert, gRootCA);                 
            }                       
            
        } catch (Exception e){
            Log.d(TAG2,"Certificate Enrollment fail");            
            Log.e(TAG, "err:" + e);
            return false;
        }

        return true;
    }   
    
    public byte[] estHttpClient(String serverUrl, String method, String suffix, String operation, byte[] csr, final String digestUsername, final String digestPassword){
        try{
            boolean bGzipContent = false; 
            URI url = new URI(serverUrl+suffix);
            PasspointHttpClient hc = null;                      
            //get response content
            InputStream in = null;
            
            if(serverUrl.startsWith("HTTPS://") || serverUrl.startsWith("https://")){                                        
                if(operation.equals(REENROLL)){
                    Log.d(TAG, "[estHttpClient]: re-enroll");
                    if(hs20PKCS12KeyStore.aliases().hasMoreElements()){
                        hc = new PasspointHttpClient(hs20PKCS12KeyStore, passWord.toCharArray());
                    }else{                            
                        Log.d(TAG, "client cert is not installed in passpoint PKCS12 keystore");   
                        return null;
                    }
                }else{
                    Log.d(TAG, "[estHttpClient]: enroll");
                    hc = new PasspointHttpClient(null, null);
                }

                if(digestUsername != null && digestPassword != null){
                    hc.setAuthenticationCredentials(new UsernamePasswordCredentials(digestUsername, 
                            digestPassword));  

                }

                HttpResponse httpResp = null;
                Header[] requestHeaders;
                List<BasicHeader> basicHeaders = new ArrayList<BasicHeader>();
                
                basicHeaders.add(new BasicHeader(hc.ACCEPT_ENCODING_HEADER, "gzip"));                        
                
                if(method.equals("POST") && csr != null){
                    byte[] base64csr = Base64.encode(csr, Base64.DEFAULT);
                    String base64String = new String(base64csr);
                    String pemCsr = "-----BEGIN CERTIFICATE REQUEST-----\n" + 
                            base64String.replaceAll("(.{64})", "$1\n")+
                                    "\n-----END CERTIFICATE REQUEST-----";                    
                    pemCsr = base64String.replaceAll("(.{64})", "$1\n");
                    Log.d(TAG, "CSR: " + pemCsr);
                    basicHeaders.add(new BasicHeader(hc.CONTENT_LENGTH_HEADER, 
                            "" + pemCsr.getBytes().length));
                    basicHeaders.add(new BasicHeader(hc.CONTENT_TRANSFER_ENCODING, "base64"));
                    requestHeaders = basicHeaders.toArray(new Header[basicHeaders.size()]);                    
                    httpResp = hc.post(url, "application/pkcs10", pemCsr.getBytes(), requestHeaders);                            
                } else {
                    Log.d(TAG, "[estHttpClient]: method is not POST");
                    return null;
                }
                
                     
                //get response header
                boolean bPKCS7 = false;
                boolean bCSRAttrs = false;
                int statusCode = httpResp.getStatusLine().getStatusCode();
                Log.d(TAG2,"Response code :"+String.valueOf(statusCode));
                if(statusCode == 200){
                    String contentType = httpResp.getEntity().getContentType().getValue();
                    if("application/pkcs7-mime".equals(contentType)){ //cacert and client cert
                        bPKCS7 = true;
                    }else if("application/csrattrs".equals(contentType)){ // csr attributes
                        bCSRAttrs = true;
                    }

                    if("gzip".equalsIgnoreCase(httpResp.getEntity().getContentEncoding().getValue())) {
                        bGzipContent = true;
                    }
                }
                
                
                if(bPKCS7 || bCSRAttrs){
                    in = httpResp.getEntity().getContent();
                    if(bGzipContent){
                        in = getUnZipInputStream(in);
                    }

                    if(in != null)
                    {
                        ByteArrayOutputStream bos = new ByteArrayOutputStream();
                        byte[] buf = new byte[8192];
                                
                        while (true) {
                            int rd = in.read(buf, 0, 8192);
                            if (rd == -1) {
                                break;
                            }
                            bos.write(buf, 0, rd);
                        }
                                
                        bos.flush();
                        byte[] byteArray = bos.toByteArray();
                        if(org.apache.commons.codec.binary.Base64.isArrayByteBase64(byteArray)) {
                           String s = new String(byteArray);
                           Log.d(TAG2,"Content : " + s);
                           return Base64.decode(new String(byteArray), Base64.DEFAULT);                            
                        }                    
                        
                        return byteArray;
                    }                      
                }                                        
            }else {
                hc = new PasspointHttpClient(null, null);
                if(digestUsername != null && digestPassword != null){
                    hc.setAuthenticationCredentials(new UsernamePasswordCredentials(digestUsername, 
                            digestPassword));  

                }

                HttpResponse httpResp = null;
                Header[] requestHeaders;
                List<BasicHeader> basicHeaders = new ArrayList<BasicHeader>();
                
                basicHeaders.add(new BasicHeader(hc.ACCEPT_ENCODING_HEADER, "gzip"));                        
                if(method.equals("POST") && csr != null){
                    byte[] base64csr = Base64.encode(csr, Base64.DEFAULT);
                    String base64String = new String(base64csr);
                    String pemCsr = "-----BEGIN CERTIFICATE REQUEST-----\n" + 
                            base64String.replaceAll("(.{64})", "$1\n")+
                                    "\n-----END CERTIFICATE REQUEST-----";
                    
                    pemCsr = base64String.replaceAll("(.{64})", "$1\n");
                    Log.d(TAG, "CSR: " + pemCsr);
                    basicHeaders.add(new BasicHeader(hc.CONTENT_LENGTH_HEADER, 
                            "" + pemCsr.getBytes().length));
                    basicHeaders.add(new BasicHeader(hc.CONTENT_TRANSFER_ENCODING, "base64"));
                    requestHeaders = basicHeaders.toArray(new Header[basicHeaders.size()]);                    
                    httpResp = hc.post(url, "application/pkcs10", pemCsr.getBytes(), requestHeaders);                            

                } else {
                    Log.d(TAG, "[estHttpClient]: method is not POST");
                    return null;
                }                

                //get response header  
                boolean bPKCS7 = false;
                boolean bCSRAttrs = false;
                int statusCode = httpResp.getStatusLine().getStatusCode();
                Log.d(TAG2,"Response code :"+String.valueOf(statusCode));                
                if(statusCode == 200){
                    String contentType = httpResp.getEntity().getContentType().getValue();
                    if("application/pkcs7-mime".equals(contentType)){ //cacert and client cert
                        bPKCS7 = true;
                    }else if("application/csrattrs".equals(contentType)){ // csr attributes
                        bCSRAttrs = true;
                    }

                    if("gzip".equalsIgnoreCase(httpResp.getEntity().getContentEncoding().getValue())) {
                        bGzipContent = true;
                    }
                }
                
                
                if(bPKCS7 || bCSRAttrs){
                    in = httpResp.getEntity().getContent();
                    if(bGzipContent){
                        in = getUnZipInputStream(in);
                    }
                    
                    if(in != null)
                    {
                        ByteArrayOutputStream bos = new ByteArrayOutputStream();
                        byte[] buf = new byte[8192];
                                
                        while (true) {
                            int rd = in.read(buf, 0, 8192);
                            if (rd == -1) {
                                break;
                            }
                            bos.write(buf, 0, rd);
                        }
                                
                        bos.flush();
                        byte[] byteArray = bos.toByteArray();
                        if(org.apache.commons.codec.binary.Base64.isArrayByteBase64(byteArray)) {
                           String s = new String(byteArray);
                           Log.d(TAG2,"Content : " + s);
                           return Base64.decode(new String(byteArray), Base64.DEFAULT);                            
                        }                              
                        return byteArray;
                    }                      
                }                 
            }          
        }catch(Exception e){
            Log.e(TAG, "estHttpClient err:" + e);
        }
        return null;
    }

    private static InputStream getUnZipInputStream(InputStream inputStream) throws IOException {
        /* workaround for Android 2.3 
           (see http://stackoverflow.com/questions/5131016/)
        */
        try {
            return (GZIPInputStream) inputStream;
        } catch (ClassCastException e) {
            return new GZIPInputStream(inputStream);
        }
    }
    
    private static class CertTrustManager implements X509TrustManager {
        public void checkClientTrusted(X509Certificate[] arg0, String arg1){
            Log.d(TAG, "[checkClientTrusted] " + arg0 + arg1);
        }
        public void checkServerTrusted(final X509Certificate[] arg0, String arg1)
                throws CertificateException{
            Log.d(TAG, "[checkServerTrusted] X509Certificate amount:" + arg0.length + ", cryptography: " + arg1);
        }

        public X509Certificate[] getAcceptedIssuers() {            
            Log.d(TAG, "[getAcceptedIssuers] ");
            return null;
        }
    }
}
