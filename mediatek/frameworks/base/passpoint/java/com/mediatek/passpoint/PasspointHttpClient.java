/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2012. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
package com.mediatek.passpoint;

import org.apache.http.Header;
import org.apache.http.HttpResponse;
import org.apache.http.HttpVersion;
import org.apache.http.auth.AuthSchemeRegistry;
import org.apache.http.auth.AuthScope;
import org.apache.http.auth.UsernamePasswordCredentials;
import org.apache.http.client.HttpRequestRetryHandler;
import org.apache.http.client.methods.HttpDelete;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.client.methods.HttpUriRequest;
import org.apache.http.client.params.AuthPolicy;
import org.apache.http.client.params.HttpClientParams;
import org.apache.http.conn.ClientConnectionManager;
import org.apache.http.conn.params.ConnManagerParams;
import org.apache.http.conn.params.ConnPerRoute;
import org.apache.http.conn.routing.HttpRoute;
import org.apache.http.conn.scheme.PlainSocketFactory;
import org.apache.http.conn.scheme.Scheme;
import org.apache.http.conn.scheme.SchemeRegistry;
import org.apache.http.conn.ssl.SSLSocketFactory;
import org.apache.http.entity.ByteArrayEntity;
import org.apache.http.impl.auth.DigestSchemeFactory;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.impl.conn.tsccm.ThreadSafeClientConnManager;
import org.apache.http.params.BasicHttpParams;
import org.apache.http.params.HttpConnectionParams;
import org.apache.http.params.HttpParams;
import org.apache.http.params.HttpProtocolParams;
import org.apache.http.protocol.HttpContext;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.Socket;
import java.net.URI;
import java.net.UnknownHostException;
import java.security.KeyManagementException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.UnrecoverableKeyException;

import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;
import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.KeyManager;

public class PasspointHttpClient {    
    protected final static String ACCEPT_ENCODING_HEADER = "Accept-Encoding";
    protected final static String CONTENT_LENGTH_HEADER = "Content-Length";
    protected final static String CONTENT_TRANSFER_ENCODING = "Content-Transfer-Encoding";
    protected final static String CONNECTION = "Connection";
    private final static String CONTENT_TYPE = "Content-Type";
    
    private DefaultHttpClient mClient = null;

    private UsernamePasswordCredentials mCredentials = null;

    private static final boolean TLS_TRUST_ALL = true;

    // Default connection and socket timeout of 60 seconds. Tweak to taste.
    private static final int SOCKET_OPERATION_TIMEOUT = 30 * 1000;
    private static final int MAX_SOCKET_CONNECTION = 30;

    public PasspointHttpClient(KeyStore clientCertStore, char[] passwd) {
        HttpParams params = new BasicHttpParams();

        HttpProtocolParams.setVersion(params, HttpVersion.HTTP_1_1);
        HttpProtocolParams.setUseExpectContinue(params, false);
        HttpProtocolParams.setUserAgent(params, "ksoap2-android/2.6.0+"); // tempoarily
        HttpProtocolParams.setContentCharset(params, "UTF-8");

        HttpConnectionParams.setStaleCheckingEnabled(params, false);
        HttpConnectionParams.setConnectionTimeout(params, SOCKET_OPERATION_TIMEOUT);
        HttpConnectionParams.setSoTimeout(params, SOCKET_OPERATION_TIMEOUT);
        HttpConnectionParams.setSoSndTimeout(params, SOCKET_OPERATION_TIMEOUT);
        HttpConnectionParams.setSocketBufferSize(params, 8192);

        HttpClientParams.setRedirecting(params, false);
        HttpClientParams.setAuthenticating(params, true);

        /** The default maximum number of connections allowed per host */
        ConnPerRoute connPerRoute = new ConnPerRoute() {

            public int getMaxForRoute(HttpRoute route) {
                return MAX_SOCKET_CONNECTION;
            }

        };

        ConnManagerParams.setTimeout(params, 3 * 1000);
        ConnManagerParams.setMaxConnectionsPerRoute(params, connPerRoute);
        ConnManagerParams.setMaxTotalConnections(params, MAX_SOCKET_CONNECTION);

        SchemeRegistry schemeRegistry = new SchemeRegistry();
        schemeRegistry.register(new Scheme("http",
                PlainSocketFactory.getSocketFactory(), 80));

        if (clientCertStore != null) {
            KeyStore trustStore;
            try {
                SSLSocketFactory sf = new SSLSocketFactoryEx(clientCertStore, passwd);
                // TODO: change to BROWSER_COMPATIBLE_HOSTNAME_VERIFIER or STRICT_HOSTNAME_VERIFIER
                sf.setHostnameVerifier(SSLSocketFactory.ALLOW_ALL_HOSTNAME_VERIFIER);
                schemeRegistry.register(new Scheme("https",
                        sf, 443));
            } catch (NoSuchAlgorithmException e){
                e.printStackTrace();
            } catch (Exception e) {
                e.printStackTrace();
            }
        } else {
            try {
                SSLSocketFactory sf = new SSLSocketFactoryEx(null, null);
                // TODO: change to BROWSER_COMPATIBLE_HOSTNAME_VERIFIER or STRICT_HOSTNAME_VERIFIER
                sf.setHostnameVerifier(SSLSocketFactory.ALLOW_ALL_HOSTNAME_VERIFIER);
                schemeRegistry.register(new Scheme("https",
                        sf, 443));
            } catch (NoSuchAlgorithmException e){
                e.printStackTrace();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        AuthSchemeRegistry registry = new AuthSchemeRegistry();
        registry.register(
                AuthPolicy.DIGEST,
                new DigestSchemeFactory());

        ClientConnectionManager manager =
                new ThreadSafeClientConnManager(params, schemeRegistry);

        if (mClient == null) {
            mClient = new DefaultHttpClient(manager, params);
        }

        mClient.setHttpRequestRetryHandler(new HttpRequestRetryHandler() {

            public boolean retryRequest(
                    final IOException exception, int executionCount, final HttpContext context) {
                return false;
            }

        });
    }

    // CLIENT MANAGEMENT

    public void setAuthenticationCredentials(UsernamePasswordCredentials credentials) {
        mCredentials = credentials;
    }

    public void shutdown() {
        if (mClient != null) {
            mClient.getConnectionManager().shutdown();
        }
    }

    private HttpResponse execute(HttpUriRequest request,
            Header[] additionalRequestHeaders)
            throws IOException {

        if (additionalRequestHeaders != null) {
            for (Header header : additionalRequestHeaders) {
                request.addHeader(header);
            }
        }

        if (mCredentials != null) {
            mClient.getCredentialsProvider().setCredentials(new AuthScope(AuthScope.ANY),
                    mCredentials);
        }

        final HttpResponse response = mClient.execute(request);
        return response;
    }

    private void setRequestEntity(HttpPost request, byte[] content,
            String mimetype) throws UnsupportedEncodingException {
        request.setHeader(CONTENT_TYPE, mimetype);
        request.setEntity(new ByteArrayEntity(content));
    }

    public HttpResponse get(URI uri, Header[] additionalRequestHeaders)
            throws IOException {
        final HttpGet request = new HttpGet(uri);
        return execute(request, additionalRequestHeaders);
    }

    public HttpResponse post(URI uri, String mimetype, byte[] content,
            Header[] additionalRequestHeaders)
            throws IOException {
        final HttpPost request = new HttpPost(uri);
        setRequestEntity(request, content, mimetype);
        return execute(request, additionalRequestHeaders);
    }

    private class SSLSocketFactoryEx extends SSLSocketFactory {

        SSLContext sslContext = SSLContext.getInstance("TLS");

        public SSLSocketFactoryEx(KeyStore truststore, char[] password)
                throws NoSuchAlgorithmException, KeyManagementException,
                KeyStoreException, UnrecoverableKeyException {
            super(truststore);

            TrustManager tm = new X509TrustManager() {
                public java.security.cert.X509Certificate[] getAcceptedIssuers() {
                    return null;
                }

                @Override
                public void checkClientTrusted(
                        java.security.cert.X509Certificate[] chain, String authType)
                        throws java.security.cert.CertificateException {
                }

                @Override
                public void checkServerTrusted(
                        java.security.cert.X509Certificate[] chain, String authType)
                        throws java.security.cert.CertificateException {
                }
            };

            if (truststore != null) {
                KeyManagerFactory kmf = KeyManagerFactory.getInstance(KeyManagerFactory.getDefaultAlgorithm());
                kmf.init(truststore, password);
                KeyManager[] keyManagers = kmf.getKeyManagers();
                sslContext.init(keyManagers, new TrustManager[] {
                    tm
                }, null, true);
            } else {
                sslContext.init(null, new TrustManager[] {
                    tm
                }, null, true);
            }
        }

        @Override
        public Socket createSocket(Socket socket, String host, int port, boolean autoClose)
                throws IOException, UnknownHostException {
            return sslContext.getSocketFactory().createSocket(socket, host, port, autoClose);
        }

        @Override
        public Socket createSocket() throws IOException {
            return sslContext.getSocketFactory().createSocket();
        }
    }
}


