
package com.mediatek.mediatekdm.wfhs;

import android.os.SystemProperties;
import org.apache.harmony.security.provider.cert.X509CertImpl;
import org.conscrypt.TrustManagerImpl;
import org.bouncycastle.asn1.*;
import org.bouncycastle.asn1.x509.GeneralName;
import org.bouncycastle.asn1.x509.KeyPurposeId;
import org.bouncycastle.asn1.util.ASN1Dump;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.InetSocketAddress;
import java.net.MalformedURLException;
import java.net.Proxy;
import java.net.SocketTimeoutException;
import java.net.URL;
import java.security.KeyStore;
import java.security.cert.CertificateException;
import java.security.cert.CertificateParsingException;
import java.security.cert.X509Certificate;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.Enumeration;

import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.KeyManager;
import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSession;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;

import android.util.Log;

import com.mediatek.mediatekdm.DmOperation;
import com.mediatek.mediatekdm.DmOperationManager;
import com.mediatek.mediatekdm.mdm.PLHttpConnection;
import com.mediatek.mediatekdm.wfhs.WiFiHotSpotComponent;
import com.mediatek.passpoint.PasspointCertificateTest;

public class WFHSHttpConnection implements PLHttpConnection {
	public static final String TAG = "DM/Wifi.WFHSHttpConnection";
    class CertTrustManager implements X509TrustManager {
        public void checkClientTrusted(X509Certificate[] arg0, String arg1) {
            Log.d(TAG, "[checkClientTrusted] " + arg0 + arg1);
        }

        public void checkServerTrusted(X509Certificate[] arg0, String arg1)
                throws CertificateException {
        	Log.d(TAG, "[checkServerTrusted] X509Certificate amount:" + arg0.length
                    + ", cryptography: " + arg1);
            if("true".equals(SystemProperties.get("persist.service.ocsprevoke"))){
                throw new RuntimeException("Certificate is revoked (5.9h)");
            }else if("true".equals(SystemProperties.get("persist.service.servercertcheck","true"))){
                try {
                    int i;
                    for(i = 0; i < arg0.length; i++)
                    {
                        Log.d(TAG, "X509Certificate: " + arg0[i]);      
                        Log.d(TAG, "====================");                        

                        X509CertImpl cert = new X509CertImpl(arg0[i].getEncoded());
                        //check validity (not before and not after)
                        cert.checkValidity();                    
                    }                    

                    //check root CA chaining
                    KeyStore ks = KeyStore.getInstance("AndroidCAStore");
                    ks.load(null, null);
                    TrustManagerImpl tm = new TrustManagerImpl(ks);
                    tm.checkServerTrusted(arg0, arg0[0].getPublicKey().getAlgorithm(), mSPFQDN);
                    
                    if(mOSUServerUrl != null && !mOSUServerUrl.isEmpty()){ // only check on OSU
                        //check SP friendly name and Language code
                        if("true".equals(SystemProperties.get("persist.service.nonamecheck"))){
                
                        }else if("true".equals(SystemProperties.get("persist.service.test59b"))){
                            if(!checkSubjectAltNameOtherNameSPLangFriendlyName(PasspointCertificateTest.getID_H())){
                                throw new RuntimeException("[certificate test]id-wfa-hotspot-friendly-name check fail");                    
                            }
                        }else if("true".equals(SystemProperties.get("persist.service.test59c"))){
                            if(!checkSubjectAltNameOtherNameSPLangFriendlyName(PasspointCertificateTest.getID_I())){
                                throw new RuntimeException("[certificate test]id-wfa-hotspot-friendly-name check fail");                    
                            }
                        }else if("true".equals(SystemProperties.get("persist.service.test59d"))){
                            if(!checkSubjectAltNameOtherNameSPLangFriendlyName(PasspointCertificateTest.getID_J())){
                                throw new RuntimeException("[certificate test]id-wfa-hotspot-friendly-name check fail");                    
                            }
                        }else if("true".equals(SystemProperties.get("persist.service.test59e"))){
                            if(!checkSubjectAltNameOtherNameSPLangFriendlyName(PasspointCertificateTest.getID_K())){
                                throw new RuntimeException("[certificate test]id-wfa-hotspot-friendly-name check fail");                    
                            }                                
                        }else{
                            if(!checkSubjectAltNameOtherNameSPLangFriendlyName(arg0[0])){
                                throw new RuntimeException("id-wfa-hotspot-friendly-name check fail");                    
                            }
                        }                   
                        
                        //check icon type hash value 
                        if("true".equals(SystemProperties.get("persist.service.test59d"))){
                            if(!checkLogotypeExtn(PasspointCertificateTest.getID_J())){
                                throw new RuntimeException("[certificate test]Certificate Logo icon hash doesn't match");                    
                            }                                
                        }else if(!checkLogotypeExtn(arg0[0])){
                            throw new RuntimeException("Certificate Logo icon hash doesn't match");                    
                        }    
                    }
                
                
                    //check id-kp-clientAuth
                    if(!checkExtendedKeyUsageIdKpClientAuth(arg0[0])){
                        throw new RuntimeException("id-kp-clientAuth found");                    
                    }           
                
                
                    //check OCSP
                    //if(!checkOCSP(arg0[0], CaCert)){
                        //throw new RuntimeException("Certificate revoked");
                    //}
                
                    //check SP-FQDN
                    boolean suffixMatch;
                    if(mOSUServerUrl != null && !mOSUServerUrl.isEmpty()){ // check complete host name on OSU server
                        suffixMatch = false;
                    }else{// check SPFQDN on subscription and policy server
                        suffixMatch = true;
                    }
                    if(!checkSubjectAltNameDNSName(arg0[0], mSPFQDN, suffixMatch)){
                        throw new RuntimeException("Certificate Subject Alternative Name doesn't include SP-FQDN");                    
                    }                                
                } catch (CertificateException e) {
                    throw e;
                } catch(Exception e){
                    e.printStackTrace();
                    throw new RuntimeException(e);                    
                }
                    
            }else {
                Log.d(TAG, "[checkServerTrusted] don't check");
            }
        }

        public X509Certificate[] getAcceptedIssuers() {
        	Log.d(TAG, "[getAcceptedIssuers] ");
            return null;
        }
    }

    private HttpURLConnection mConnection;
    private Proxy.Type mProxyType = null;
    private URL mUrl = null;
    private TrustManager[] mTrustManagerArray = null;

    private String mSPFQDN;
    private String mOSUServerUrl;
    private String mOSUFriendlyName;
    private String mOSULanguage;
    private String mIconFileName;
    private String mIconHash;
    private MdmWifi mMdmWifi;

    public WFHSHttpConnection(MdmWifi wifi) {
    	mMdmWifi = wifi;
    	
    	DmOperation operation = DmOperationManager.getInstance().current();
    	String sessionAction = operation.getProperty(WiFiHotSpotComponent.IntentExtraKey.OPERATION_KEY_ACTION);
    	Log.d(TAG, "WiFI action is :" + sessionAction);
    	if (MdmWifi.SessionAction.PROVISION_START.equals(sessionAction)) {
    	    mSPFQDN = operation.getProperty("spfqdn");
    	    Log.d(TAG, "OSU spfqdn:" + mSPFQDN);
    	    mOSUServerUrl = operation.getProperty("serverurl");
    	    Log.d(TAG, "OSU serverurl:" + mOSUServerUrl);
    	    mOSUFriendlyName = operation.getProperty("friendlyname");
    	    Log.d(TAG, "OSU friendlyname:" + mOSUFriendlyName);
    	    mOSULanguage = operation.getProperty("language");
    	    Log.d(TAG, "OSU language:" + mOSULanguage);
    	    mIconFileName = operation.getProperty("iconfilename");
    	    Log.d(TAG, "OSU iconfilename:" + mIconFileName);
    	    mIconHash = operation.getProperty("iconhash");
    	    Log.d(TAG, "OSU iconhash:" + mIconHash);
    	}
    	mTrustManagerArray = new TrustManager[] {
    	        new CertTrustManager()
    	};
    	
    }

    @Override
    public boolean addRequestProperty(String field, String value) {
    	Log.d(TAG, "addRequestProperty: " + field + " = " + value);

        if (mConnection == null) {
        	Log.d(TAG, "AddRequestProperty: mConnection=" + mConnection);
            return false;
        }

        try {
            mConnection.setRequestProperty(field, value);
            return true;
        } catch (IllegalStateException e) {
        	Log.d(TAG, "AddRequestProperty: IllegalStateException");
        } catch (NullPointerException e) {
        	Log.d(TAG, "AddRequestProperty: NullPointerException");
        } catch (Exception e) {
        	Log.d(TAG, "AddRequestProperty: Exception");
        }

        return false;
    }

    @Override
    public boolean closeComm() {
    	Log.d(TAG, "closeComm()");
        if (mConnection == null) {
        	Log.d(TAG, "closeComm: mConnection=" + mConnection);
            return false;
        }

        mConnection.disconnect();
        return true;
    }

    @Override
    public void destroy() {
        // nothing to do
    }

    @Override
    public int getContentLength() {
    	Log.d(TAG, "getContentLength()");

        if (mConnection == null) {
        	Log.d(TAG, "getContentLength: mConnection=" + mConnection);
            return -1;
        }

        if (!waitResponse()) {
        	Log.d(TAG, "getHeadField: timeout");
            return -1;
        }

        int length = mConnection.getContentLength();
        if (length < 0) {
            try {
                /* for chunked ?? */
                length = mConnection.getInputStream().available();
            } catch (IOException e) {
            	Log.d(TAG, "in.available: IOException " + e.getMessage());
                e.printStackTrace();
            }
        }

        Log.d(TAG, "getContentLength() return " + length);

        return length;
    }

    @Override
    public String getHeadField(String field) {
    	Log.d(TAG, "getHeadField: field=" + field);

        if (mConnection == null) {
        	Log.d(TAG, "getHeadField: mConnection=" + mConnection);
            return null;
        }

        if (!waitResponse()) {
        	Log.d(TAG, "getHeadField: timeout");
            return null;
        }

        return mConnection.getHeaderField(field);

    }

    @Override
    public int getHeadFieldInt(String field, int defValue) {
    	Log.d(TAG, "getHeadFieldInt: field=" + field + " ,defValue=" + defValue);
        if (mConnection == null) {
        	Log.d(TAG, "getHeadFieldInt: mConnection=" + mConnection);
            return defValue;
        }

        if (!waitResponse()) {
        	Log.d(TAG, "getHeadField: timeout");
            return defValue;
        }

        return mConnection.getHeaderFieldInt(field, defValue);

    }

    @Override
    public String getURL() {
        if (mConnection == null) {
        	Log.d(TAG, "getURL: mConnection=" + mConnection);
            return null;
        }

        return mConnection.getURL().toString();
    }

    /**
     * @param uri
     * @param proxyType : 0 -- DIRECT, 1 -- PROXY(HTTP??), 2 --SOCKS
     * @param proxyAddr
     * @param proxyPort
     */
    public boolean initialize(String uri, int proxyType, String proxyAddr, int proxyPort) {
    	Log.d(TAG, 
                "initialize: uri=" + uri + 
                ", proxyType=" + proxyType + 
                ", proxyAddr=" + proxyAddr + 
                ", proxyPort=" + proxyPort);
        try {
            mUrl = new URL(uri);
            Log.d(TAG, "Host is " + mUrl.getHost());
            Log.d(TAG, "Port is " + mUrl.getPort());
        } catch (MalformedURLException e) {
        	Log.d(TAG, "SimpleHttpConnection: invalid URL: " + uri);
            return false;
        }

        switch (proxyType) {
            case 0:
                mProxyType = Proxy.Type.DIRECT;
                break;
            case 1:
                mProxyType = Proxy.Type.HTTP;
                break;
            case 2:
                mProxyType = Proxy.Type.SOCKS;
                break;
            default:
                return false;
        }

        try {
            if (mProxyType == Proxy.Type.DIRECT) {
                mConnection = (HttpURLConnection) mUrl.openConnection();
            } else {
                InetSocketAddress addr = new InetSocketAddress(proxyAddr, proxyPort);
                mConnection = (HttpURLConnection) mUrl.openConnection(new Proxy(mProxyType, addr));
            }
            return true;
        } catch (IOException e) {
        	Log.d(TAG, "SimpleHttpConnection: IOException");
            e.printStackTrace();
            return false;
        } catch (IllegalArgumentException e) {
            Log.d(TAG, "SimpleHttpConnection: IllegalArgumentException");
            return false;
        } catch (UnsupportedOperationException e) {
            Log.d(TAG, "SimpleHttpConnection: UnsupportedOperationException");
            return false;
        }
    }

    @Override
    public boolean openComm() {
        Log.d(TAG, "openComm()");
        if (mConnection == null) {
            Log.e(TAG, "openComm: mConnection=" + mConnection);
            return false;
        }

        if (mConnection instanceof HttpsURLConnection) {
            Log.d(TAG, "openComm(): https connection");
            HttpsURLConnection connection = (HttpsURLConnection) mConnection;
            try {
                // TODO: Implement HostnameVerifier
                HostnameVerifier hv = new HostnameVerifier() {
                    public boolean verify(String urlHostName, SSLSession session) {
                        Log.d(TAG, "verify:" + urlHostName);
                        return true;
                    }
                };

                SSLContext sc = SSLContext.getInstance("TLS");
                //add authentication with client certificate
                int sessionType = mMdmWifi.getSessionId();
                Log.d(TAG, "Session type is :" + sessionType);
                if ( mMdmWifi.needDoAuthenticationWithClientCertificate(sessionType)) {
                    String password = "wifi@123";
                    String certFilePath = "/data/misc/wpa_supplicant/est_client_cert.p12";
                    FileInputStream fiStream = new FileInputStream(certFilePath);
                    //init KeyStore
                    KeyStore hs20PKCS12KeyStore = KeyStore.getInstance("PKCS12", "BC");
                    hs20PKCS12KeyStore.load(fiStream, password.toCharArray());
                    
                    if(hs20PKCS12KeyStore.aliases().hasMoreElements()) {
                        KeyManagerFactory kmf = 
                            KeyManagerFactory.getInstance(KeyManagerFactory.getDefaultAlgorithm());
           
                        kmf.init(hs20PKCS12KeyStore, password.toCharArray());
                        KeyManager[] keyManagers = kmf.getKeyManagers();
                        sc.init(keyManagers, mTrustManagerArray, null, true);
                    } else {
                        Log.d(TAG, "client cert is not installed in passpoint PKCS12 keystore");
                        sc.init(null, mTrustManagerArray, null, true);
                    }
                } else {
                    Log.d(TAG, "not need authentication with client certificate");
                    sc.init(null, mTrustManagerArray, null, true);
                }
                //end
                SSLSocketFactory sslf = sc.getSocketFactory();
                connection.setHostnameVerifier(hv);
                connection.setSSLSocketFactory(sslf);
            } catch (Exception e) {
                Log.e(TAG, "openComm(): https exception!!!");
                e.printStackTrace();
            }
        } else {
            Log.d(TAG, "openComm(): http connection");
        }

        mConnection.setConnectTimeout(60 * 1000);
        mConnection.setReadTimeout(120 * 1000);
        mConnection.setDoOutput(true);
        mConnection.setDoInput(true);
        mConnection.setRequestProperty("Accept-Encoding", "identity");

        /* general header */
        addRequestProperty("Cache-Control", "private");
        addRequestProperty("Connection", "close");
        addRequestProperty("Accept", "application/vnd.syncml+xml, application/vnd.syncml+wbxml, */*");
        addRequestProperty("Accept-Language", "en");
        addRequestProperty("Accept-Charset", "utf-8");

        return true;
    }

    @Override
    public int recvData(byte[] buffer) {
        Log.d(TAG, "recvData: buflen=" + buffer.length);
        if (mConnection == null) {
            Log.e(TAG, "recvData: mConnection=" + mConnection);
            return -1;
        }

        try {
            InputStream in = mConnection.getInputStream();
            int ret = in.read(buffer);
            return ret;
        } catch (SocketTimeoutException e) {
            Log.e(TAG, "recvData: SocketTimeoutException!!");
        } catch (IOException e) {
            Log.e(TAG, "recvData: IOException!!");
        }

        return -1;
    }

    @Override
    public int sendData(byte[] data) {
        Log.d(TAG, "sendData: len=" + data.length);
        if (mConnection == null) {
            Log.e(TAG, "sendData: mConnection=" + mConnection);
            return -1;
        }

        addRequestProperty("Content-Length", String.valueOf(data.length));

        try {
            OutputStream out = mConnection.getOutputStream();
            out.write(data, 0, data.length);
            out.flush();
        } catch (IOException e) {
            Log.e(TAG, "sendData IOException: " + e);
            Log.e(TAG, "Message: " + e.getMessage());
            Log.e(TAG, "Cause: " + e.getCause());
            e.printStackTrace();
            return -1;
        } catch (IndexOutOfBoundsException e) {
            Log.e(TAG, "sendData: IndexOutOfBoundsException!!");
            return -1;
        }

        Log.d(TAG, "sendData: return " + data.length);
        return data.length;
    }

    private boolean waitResponse() {
        InputStream is = null;
        byte[] buf = new byte[8192];
        ByteArrayOutputStream bos = new ByteArrayOutputStream();
        boolean debug = true;

        try {
            Log.d(TAG, "waitResponse: enterring getInputStream...");
            mConnection.getInputStream();
            return true;
        } catch (IOException e) {
        	Log.e(TAG, "waitResponse: IOException");
            e.printStackTrace();
        }

        if (debug) {
            is = mConnection.getErrorStream();
            try {
                while (true) {
                    int rd = is.read(buf, 0, 8192);
                    if (rd == -1) {
                        break;
                    }
                    bos.write(buf, 0, rd);
                }
            } catch (IOException e) {
            	Log.e(TAG, "is.read: IOException");
                e.printStackTrace();
            }
            String responseDump = new String(buf);
            Log.d(TAG, "waitResponse: " + responseDump);
        }

        return false;
    }

    private boolean checkSubjectAltNameOtherNameSPLangFriendlyName(final X509Certificate x509Cert) {
        boolean result = false;

        if(mOSUFriendlyName == null){
            return false;
        }else if(mOSUFriendlyName.isEmpty()){
            return false;
        }

        try {
            Collection c = x509Cert.getSubjectAlternativeNames();
            
            if(c != null){
                Iterator it = c.iterator();
                while (it.hasNext()) {
                    List gn = (List) it.next();
                    Integer tag = (Integer) gn.get(0);
                    if(tag == GeneralName.otherName){
                        Log.d(TAG, "SubjectAltName OtherName:"
                                + gn.get(1).toString());
                        
                        ByteArrayInputStream bais = new ByteArrayInputStream((byte[]) gn.get(1));
                        ASN1InputStream asn1_is = new ASN1InputStream(bais);
                        Object asn1Obj = (Object)asn1_is.readObject();
                        Log.d(TAG, ASN1Dump.dumpAsString(asn1Obj, true));

                        DERTaggedObject derTagObj = (DERTaggedObject) asn1Obj;                       
                        DERSequence derSeq = (DERSequence)derTagObj.getObject();
                        Enumeration enu = derSeq.getObjects();
                        DERObjectIdentifier oid = (DERObjectIdentifier) ((ASN1Encodable) enu.nextElement()).toASN1Primitive();
                        Log.d(TAG, "    OID:" + oid.toString());
                        
                        if("1.3.6.1.4.1.40808.1.1.1".equals(oid.toString())){ //id-wfa-hotspot-friendly-name
                            DERTaggedObject dertagObj = (DERTaggedObject) ((ASN1Encodable) enu.nextElement()).toASN1Primitive();
                            ASN1Object derObj = dertagObj.getObject();
                            String spLangFriendlyName;
                            if(derObj instanceof DERUTF8String){
                                DERUTF8String spLangFriendlyNameDERString = (DERUTF8String)(dertagObj.getObject());
                                spLangFriendlyName = spLangFriendlyNameDERString.toString();
                            }else{
                                DEROctetString spLangFriendlyNameDERString = (DEROctetString)(dertagObj.getObject());                            
                                spLangFriendlyName = spLangFriendlyNameDERString.toString();
                            }
                             
                            Log.d(TAG, "language code and friendly name:" + spLangFriendlyName.toString());

                            //check language code                  
                            Log.d(TAG, "mOSULanguage = "+mOSULanguage);
                            if(spLangFriendlyName.substring(0,3).equals(mOSULanguage.toLowerCase())){ //ISO639
                                Log.d(TAG, "Language code match"); 
                                
                              //check friendly name
                                Log.d(TAG, "mOSUFriendlyName = "+mOSUFriendlyName);
                                if(mOSUFriendlyName != null && !mOSUFriendlyName.isEmpty() && mOSUFriendlyName.length() != 0){
                                    if(spLangFriendlyName.substring(3).equals(mOSUFriendlyName)){
                                        Log.d(TAG, "OSU friendly name match");                     
                                        result =  true;
                                        return result;
                                    }else{
                                        Log.d(TAG, "OSU friendly name not match");                     
                                    }
                                }                                
                            }else{
                                Log.d(TAG, "Language code not match"); 
                            }                                         
                        }
                    }
                }
                Log.d(TAG, "Subject Alternative Names:" + c.toString());                
            }           
        } catch (CertificateParsingException e) {
            e.printStackTrace();
        } catch(Exception e){
            e.printStackTrace();
        }

        return result;
    }

    private boolean checkSubjectAltNameDNSName(final X509Certificate x509Cert, final String fqdn, boolean suffixMatch) {
        boolean result = false;

        try {
            Collection c = x509Cert.getSubjectAlternativeNames();
            
            if(c != null){
                Log.d(TAG, "Subject Alternative Names:" + c.toString());                
                
                Iterator it = c.iterator();
                while (it.hasNext()) {
                    List gn = (List) it.next();
                    Integer tag = (Integer) gn.get(0);
                    if(tag == GeneralName.dNSName){
                        Log.d(TAG, "Subject Alternative Name:" + gn.get(1));
                        if(suffixMatch){
                            String value = (String)(gn.get(1));
                            if(value.endsWith(fqdn)){                            
                                Log.d(TAG, "Subject Alternative DNS Name suffix match SPFQDN"); 
                                result = true;
                            }
                        }else{//complete match
                            if(gn.get(1).equals(fqdn)){                            
                                Log.d(TAG, "Subject Alternative DNS Name complete match SPFQDN"); 
                                result = true;
                            }
                        }

                    }
                }
            }           
        } catch (CertificateParsingException e) {
            e.printStackTrace();
        }

        return result;
    }

    private boolean checkExtendedKeyUsageIdKpClientAuth(final X509Certificate x509Cert) {
        boolean result = true;
        try {
            List<String> extKeyUsages = x509Cert.getExtendedKeyUsage();
            if(extKeyUsages != null){
                result = false;
                for (String extKeyUsage : extKeyUsages) {
                    Log.d(TAG, "ExtendedKeyUsage:" + extKeyUsage);
                    if(extKeyUsage.equals(KeyPurposeId.id_kp_serverAuth.toString())){ 
                        Log.d(TAG, "Server certificate EKU includes id_kp_serverAuth, true");
                        result = true;
                        break;
                    }                    
                }               
            }
        
        } catch (CertificateParsingException e) {
            e.printStackTrace();
        }

        if (!result) Log.d(TAG, "Server certificate EKU not includes id_kp_serverAuth, false");
        return result;

    }

    private boolean checkLogotypeExtn(final X509Certificate x509Cert){
        if(mIconHash == null){ //icon doesn't successfully downloaded and displayed, bypass the check
            return true;
        }

        boolean result = true;

        try {        
            // Extension Value
            
            byte[] logoType = x509Cert.getExtensionValue("1.3.6.1.5.5.7.1.12");
            if(logoType != null){
                ByteArrayInputStream bais = new ByteArrayInputStream(logoType);
                ASN1InputStream asn1_is = new ASN1InputStream(bais);
                DEROctetString derObj = (DEROctetString) asn1_is.readObject(); 
                                      
                bais = (ByteArrayInputStream)derObj.getOctetStream();
                asn1_is = new ASN1InputStream(bais);            
                DERSequence logoTypeExt = (DERSequence) asn1_is.readObject();   
                Log.d(TAG, "LogotypeExtn:" + logoTypeExt.toString()); 
                
                Enumeration LogotypeExtnSequence = logoTypeExt.getObjects();
                while(LogotypeExtnSequence.hasMoreElements()){
                    
                    DERTaggedObject LogotypeExtnTaggedObj = (DERTaggedObject) ((ASN1Encodable)LogotypeExtnSequence.nextElement()).toASN1Primitive();
                    Log.d(TAG, "LogotypeExtnTaggedObj:" + LogotypeExtnTaggedObj.toString());
                    Log.d(TAG, "LogotypeExtnTaggedObj CHOICE: " + LogotypeExtnTaggedObj.getTagNo());

                    /*LogotypeExtn ::= SEQUENCE {
                            communityLogos  [0] EXPLICIT SEQUENCE OF LogotypeInfo OPTIONAL,
                            issuerLogo      [1] EXPLICIT LogotypeInfo OPTIONAL,
                            subjectLogo     [2] EXPLICIT LogotypeInfo OPTIONAL,
                            otherLogos      [3] EXPLICIT SEQUENCE OF OtherLogotypeInfo OPTIONAL }
                 
                             */                    
                    if(LogotypeExtnTaggedObj.getTagNo() == 0){//communityLogos 
                     
                        Log.d(TAG, "");
                        DERSequence CommunityLogos = (DERSequence) LogotypeExtnTaggedObj.getObject();
                        Log.d(TAG, "communityLogos:" + CommunityLogos.toString());                    
                        
                      
                        Enumeration enu;
                        Enumeration CommunityLogosEnu = CommunityLogos.getObjects();
                        while(CommunityLogosEnu.hasMoreElements()){
                            result = true;
                            
                            DERTaggedObject CommunityLogosTaggedObj = (DERTaggedObject) ((ASN1Encodable)CommunityLogosEnu.nextElement()).toASN1Primitive();
                            Log.d(TAG, "CommunityLogosTaggedObj CHOICE: " + CommunityLogosTaggedObj.getTagNo());
                            /* LogotypeInfo ::= CHOICE {
                                           * direct          [0] LogotypeData,
                                           * indirect        [1] LogotypeReference }                    
                                           */                           
                            if(CommunityLogosTaggedObj.getTagNo() == 0){//direct
                                /*********************************************
                                                 ************         image      *************
                                                 *********************************************/ 
                                /* LogotypeData ::= SEQUENCE {
                                                * image           SEQUENCE OF LogotypeImage OPTIONAL,
                                                * audio           [1] SEQUENCE OF LogotypeAudio OPTIONAL }                 
                                                */                             
                                DERSequence LogotypeData = (DERSequence) CommunityLogosTaggedObj.getObject(); ;
                                Log.d(TAG, "LogotypeImage:" + LogotypeData.toString());
                                Enumeration LogotypeDataEnu = LogotypeData.getObjects();                      
                                while(LogotypeDataEnu.hasMoreElements()){
                                    /* LogotypeImage ::= SEQUENCE {
                                                      * imageDetails    LogotypeDetails,
                                                      * imageInfo       LogotypeImageInfo OPTIONAL }             
                                                      */                        
                                    DERSequence LogotypeImage = (DERSequence) ((ASN1Encodable)LogotypeDataEnu.nextElement()).toASN1Primitive();
                                    Log.d(TAG, "LogotypeImage:" + LogotypeImage.toString()); 
                                    Enumeration LogotypeImageEnu = LogotypeImage.getObjects();
                                    while(LogotypeImageEnu.hasMoreElements()){
                                        DERSequence imageDetails = (DERSequence) ((ASN1Encodable)LogotypeImageEnu.nextElement()).toASN1Primitive();                                
                                        /* LogotypeImageInfo ::= SEQUENCE {
                                                            * type            [0] LogotypeImageType DEFAULT color,
                                                            * fileSize        INTEGER,  -- In octets
                                                            * xSize           INTEGER,  -- Horizontal size in pixels
                                                            * ySize           INTEGER,  -- Vertical size in pixels
                                                            * resolution      LogotypeImageResolution OPTIONAL,
                                                            * language        [4] IA5String OPTIONAL }  -- RFC 3066 Language Tag                       
                                                            */
                                        DERSequence imageInfo = (DERSequence) ((ASN1Encodable)LogotypeImageEnu.nextElement()).toASN1Primitive();
                                        Log.d(TAG, "imageInfo:" + imageInfo.toString()); 
                                        enu = imageInfo.getObjects();
                                        while(enu.hasMoreElements()){
                                            ASN1Object info = ((ASN1Encodable)enu.nextElement()).toASN1Primitive();
                                            Log.d(TAG, "object:" + info.toString());
                                            if(info instanceof DERTaggedObject){
                                                if(((DERTaggedObject)info).getTagNo() == 4){
                                                    DEROctetString language = (DEROctetString) ((DERTaggedObject) info).getObject();
                                                    String languageCode = new String(language.getEncoded()).substring(2);
                                                    Log.d(TAG, "imageInfo language code:" + languageCode);
                                                    /*if(mOSULanguage.equals(languageCode)){
                                                        result = true;
                                                    }else{
                                                        result = false;
                                                    }*/
                                                }
                                            }
                                        }                                                                                                                             
                                    
                                        /*if(result == false){
                                            continue;
                                        }*/
                                        
                                        /* LogotypeDetails ::= SEQUENCE {
                                                            * mediaType       IA5String, -- MIME media type name and optional
                                                                                   -- parameters
                                                            * logotypeHash    SEQUENCE SIZE (1..MAX) OF HashAlgAndValue,
                                                            * logotypeURI     SEQUENCE SIZE (1..MAX) OF IA5String }
                                                            */                             
                                        Log.d(TAG, "imageDetails:" + imageDetails.toString());            
                                        enu = imageDetails.getObjects();
                                    
                                        //mediaType                             
                                        DERIA5String mediaType = (DERIA5String) ((ASN1Encodable)enu.nextElement()).toASN1Primitive();
                                        Log.d(TAG, "mediaType:" + mediaType.toString());
                                        DERSequence logotypeHash = (DERSequence)((ASN1Encodable)enu.nextElement()).toASN1Primitive();
                                        Log.d(TAG, "logotypeHash:" + logotypeHash.toString());            
                                        DERSequence logotypeURI = (DERSequence)((ASN1Encodable)enu.nextElement()).toASN1Primitive();
                                        Log.d(TAG, "logotypeURI:" + logotypeURI.toString());
                                    
                                        //logotypeURI
                                        enu = logotypeURI.getObjects();
                                        DERIA5String logotypeURIStr = (DERIA5String) ((ASN1Encodable)enu.nextElement()).toASN1Primitive();
                                        Log.d(TAG, "logotypeURIStr:" + logotypeURIStr.toString());
                                        Log.d(TAG, "filename : (" + filenameFromURI(logotypeURI.toString()) + ")");                    
                                        if(mIconFileName.equals(filenameFromURI(logotypeURIStr.toString()))){
                                            Log.d(TAG, "Icon filename match");                    
                                            result = true;
                                        }else{
                                            Log.d(TAG, "Icon filename not match");                    
                                            result = false;
                                            continue;
                                        }
                                        
                                        //logotypeHash
                                        enu = logotypeHash.getObjects();
                                        DERSequence HashAlgAndValue = (DERSequence) ((ASN1Encodable)enu.nextElement()).toASN1Primitive();
                                        Log.d(TAG, "HashAlgAndValue:" + HashAlgAndValue.toString());
                                        enu = HashAlgAndValue.getObjects();
                                        //hashAlg
                                        DERSequence hashAlg = (DERSequence) ((ASN1Encodable)enu.nextElement()).toASN1Primitive();           
                                        Log.d(TAG, "hashAlg:" + hashAlg.toString());            
                                        //hashValue
                                        DEROctetString hashValue = (DEROctetString) ((ASN1Encodable)enu.nextElement()).toASN1Primitive();            
                                        Log.d(TAG, "hashValue:" + hashValue.toString());
                                        //hashAlg --> AlgorithmIdentifier
                                        enu = hashAlg.getObjects();
                                        DERObjectIdentifier AlgorithmIdentifier = (DERObjectIdentifier) ((ASN1Encodable)enu.nextElement()).toASN1Primitive();
                                        Log.d(TAG, "AlgorithmIdentifier:" + AlgorithmIdentifier.toString());            
                                        //hashValue --> OctetString
                                        byte[] hashValueOctetString = hashValue.getOctets();
                                        Log.d(TAG, "hashValueOctetString:" + hashValueOctetString.toString()); 
                                        //String certIconHash = octetStringToString(hashValue.toString().substring(1));
                                        String certIconHash = hashValue.toString().substring(1);
                                        Log.d(TAG, "hashValue String:" + certIconHash);
                                        if(mIconHash.equals(certIconHash)){
                                            Log.d(TAG, "Icon hash match");                    
                                            return true;
                                        }else{
                                            Log.d(TAG, "Icon hash not match"); 
                                            result = false;
                                            continue;
                                        }
                                    
                                    } 

                                }
                                  
                            }  
                            
                        }                                   
                    }                    
                }   
                Log.d(TAG, "LogotypeExtn parsing done");
                return result;                                     
            }
        } catch (SecurityException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }                

        return false;
    }

    private static String filenameFromURI(String uri){
        String filename = uri.substring(uri.lastIndexOf("/") + 1);
        return filename;
    }

}
