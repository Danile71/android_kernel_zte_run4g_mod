package com.mediatek.connectivity;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.os.SystemProperties;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.connectivity.R;
import com.mediatek.common.ims.mo.ImsIcsi;
import com.mediatek.common.ims.mo.ImsAuthInfo;
import com.mediatek.common.ims.mo.ImsLboPcscf;
import com.mediatek.common.ims.mo.ImsPhoneCtx;
import com.mediatek.common.ims.mo.ImsXcapInfo;
import com.mediatek.common.gba.NafSessionKey;
import com.mediatek.ims.ImsManager;
import com.mediatek.ims.GbaManager;
import com.mediatek.ims.ImsConstants;


import org.apache.http.auth.AuthScope;
import org.apache.http.auth.UsernamePasswordCredentials;
import org.apache.http.client.HttpClient;
import org.apache.http.conn.params.ConnManagerParams;
import org.apache.http.conn.scheme.Scheme;
import org.apache.http.conn.scheme.PlainSocketFactory;
import org.apache.http.conn.ssl.SSLSocketFactory;
import org.apache.http.params.BasicHttpParams;
import org.apache.http.params.HttpProtocolParams;
import org.apache.http.HttpEntityEnclosingRequest;
import org.apache.http.HttpVersion;
import org.apache.http.Header;
import org.apache.http.HttpHost;
import org.apache.http.HttpRequest;
import org.apache.http.conn.scheme.SchemeRegistry;
import org.apache.http.conn.ClientConnectionManager;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.client.CredentialsProvider;
import org.apache.http.entity.ByteArrayEntity;
import org.apache.http.impl.client.BasicCredentialsProvider;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.HttpResponse;
import org.apache.http.HttpStatus;
import org.apache.http.params.HttpParams;
import org.apache.http.impl.conn.tsccm.ThreadSafeClientConnManager;
import org.apache.http.auth.AuthSchemeRegistry;

import java.lang.Character;
import java.security.KeyStore;



public class CdsImsMoActivity extends Activity implements View.OnClickListener {
    private static final String TAG = "CDSINFO/CdsImsMoActivity";
    private Context mContext;
    private TextView mImsMoInfo = null;
    private EditText mXcapRoot = null;
    private EditText mXcapUser = null;
    private EditText mXcapPasswd = null;
    private EditText mXcapAuthType = null;
    private Button   mSetButton = null;
    private Button   mGetButton = null;
    private Button   mTestButton = null;
    private Button   mRunGbaButton = null;
    private boolean  mToggle = true;
    private Toast mToast;


    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        setContentView(R.layout.cds_ims_mo);
        mContext = this.getBaseContext();
        mImsMoInfo = (TextView) findViewById(R.id.ims_mo_info);

        mXcapRoot = (EditText) findViewById(R.id.xcap_root);
        mXcapUser = (EditText) findViewById(R.id.xcap_user);
        mXcapPasswd = (EditText) findViewById(R.id.xcap_passwd);
        mXcapAuthType = (EditText) findViewById(R.id.xcap_authtype);

        mSetButton = (Button) findViewById(R.id.set_btn);
        mSetButton.setOnClickListener(this);
        mGetButton = (Button) findViewById(R.id.get_btn);
        mGetButton.setOnClickListener(this);
        mTestButton = (Button) findViewById(R.id.test_btn);
        mTestButton.setOnClickListener(this);
        mRunGbaButton = (Button) findViewById(R.id.run_gba_btn);
        mRunGbaButton.setOnClickListener(this);

        updateXcapInfo();

        mToast = Toast.makeText(this, null, Toast.LENGTH_SHORT);

        getImsMoInfo();
    }

    @Override
    protected void onResume() {
        super.onResume();
        updateXcapInfo();
        getImsMoInfo();
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
        case R.id.set_btn:
            setXcapInfo();
            break;
        case R.id.get_btn:
            updateXcapInfo();
            break;
        case R.id.test_btn:
            testImsMo();
            break;
        case R.id.run_gba_btn:
            int runType = SystemProperties.getInt("gba.run", -1);

            if (runType != -1) {
                runGbaProcedure();
            } else {
                runHttpClient();
            }

            break;
        }
    }

    private void showToast(String info) {
        mToast.setText(info);
        mToast.show();
    }

    private void testImsMo() {
        ImsManager imsManager = (ImsManager) mContext.getSystemService(Context.IMS_SERVICE);

        if (imsManager == null) {
            Log.e(TAG, "Null ImsManager");
            return;
        }

        StringBuilder builder = new StringBuilder();

        int ve = imsManager.readImsMoInt(ImsConstants.IMS_MO_VOICE_E);
        int uv = imsManager.readImsMoInt(ImsConstants.IMS_MO_VOICE_U);
        boolean mm = imsManager.readImsMoBool(ImsConstants.IMS_MO_MOBILITY);

        builder.append("[before]Voice_Domain_Preference_E_UTRAN:").append(ve).append("\r\n");
        builder.append("[before]Voice_Domain_Preference_UTRAN:").append(uv).append("\r\n");
        builder.append("[before]Mobility_Management_IMS_Voice_Termination:").append(mm).append("\r\n");

        Log.d(TAG, "testImsMo:" + mToggle);

        if (mToggle) {
            imsManager.writeImsMoInt(ImsConstants.IMS_MO_TIMER_1, 1000000);
            imsManager.writeImsMoInt(ImsConstants.IMS_MO_TIMER_2, 1000000);
            imsManager.writeImsMoInt(ImsConstants.IMS_MO_TIMER_4, 1000000);
            imsManager.writeImsMoBool(ImsConstants.IMS_MO_RESOURCE, true);
            imsManager.writeImsMoInt(ImsConstants.IMS_MO_REG_BASE, 10);
            imsManager.writeImsMoInt(ImsConstants.IMS_MO_REG_MAX, 10);
            imsManager.writeImsMoBool(ImsConstants.IMS_MO_SMS, true);
            imsManager.writeImsMoBool(ImsConstants.IMS_MO_KEEPALIVE, true);
            imsManager.writeImsMoInt(ImsConstants.IMS_MO_VOICE_E, 3);
            imsManager.writeImsMoInt(ImsConstants.IMS_MO_VOICE_U, 3);
            imsManager.writeImsMoBool(ImsConstants.IMS_MO_MOBILITY, true);

            try {
                testIcsList(true);
            } catch (Exception e) {
                e.printStackTrace();
            }

            try {
                testImsPhoneCtx(true);
            } catch (Exception e) {
                e.printStackTrace();
            }
        } else {
            imsManager.writeImsMoInt(ImsConstants.IMS_MO_VOICE_E, 1);
            imsManager.writeImsMoInt(ImsConstants.IMS_MO_TIMER_1, 600000);
            imsManager.writeImsMoInt(ImsConstants.IMS_MO_TIMER_2, 600000);
            imsManager.writeImsMoInt(ImsConstants.IMS_MO_TIMER_4, 600000);
            imsManager.writeImsMoBool(ImsConstants.IMS_MO_RESOURCE, false);
            imsManager.writeImsMoInt(ImsConstants.IMS_MO_REG_BASE, 30);
            imsManager.writeImsMoInt(ImsConstants.IMS_MO_REG_MAX, 30);
            imsManager.writeImsMoBool(ImsConstants.IMS_MO_SMS, false);
            imsManager.writeImsMoBool(ImsConstants.IMS_MO_KEEPALIVE, false);
            imsManager.writeImsMoInt(ImsConstants.IMS_MO_VOICE_E, 1);
            imsManager.writeImsMoInt(ImsConstants.IMS_MO_VOICE_U, 1);
            imsManager.writeImsMoBool(ImsConstants.IMS_MO_MOBILITY, false);

            try {
                testIcsList(false);
            } catch (Exception e) {
                e.printStackTrace();
            }

            try {
                testImsPhoneCtx(false);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        mToggle = !mToggle;

        getImsMoInfo();
    }

    private void setXcapInfo() {
        ImsManager imsManager = (ImsManager) mContext.getSystemService(Context.IMS_SERVICE);

        if (imsManager == null) {
            Log.e(TAG, "Null ImsManager");
            return;
        }

        String root = mXcapRoot.getText().toString();
        String user = mXcapUser.getText().toString();
        String passwd = mXcapPasswd.getText().toString();
        String authType = mXcapAuthType.getText().toString();

        ImsXcapInfo xcapInfo = new ImsXcapInfo(root, user, passwd, authType);
        Log.d(TAG, "[Info]" + xcapInfo);

        try {
            imsManager.writeImsXcapInfoMo(xcapInfo);
        } catch (Exception e) {
            e.printStackTrace();
        }

        updateXcapInfo();
    }

    private void runGbaProcedure() {
        final String nafAddress = "mms.msg.eng.t-mobile.com";

        final GbaManager gbaManager = GbaManager.getDefaultGbaManager(mContext);

        if (gbaManager == null) {
            Log.e(TAG, "gbaManager is null");
            return;
        }

        StringBuilder builder = new StringBuilder();
        builder.append("GBA support type:" + gbaManager.getGbaSupported() + "\r\n");

        new Thread() {
            @Override
            public void run() {
                super.run();

                try {
                    Log.d(TAG, "Before runGbaProcedure ");

                    synchronized (CdsImsMoActivity.this) {
                        final byte[] DEFAULT_UA_SECURITY_PROTOCOL_ID0 = new byte[] {(byte) 0x01, (byte) 0x00, (byte) 0x01, (byte) 0x00, (byte) 0x2F};
                        NafSessionKey nafSessionKey = gbaManager.runGbaAuthentication(nafAddress, DEFAULT_UA_SECURITY_PROTOCOL_ID0, false);

                        if (nafSessionKey != null) {
                            Log.i(TAG, "GBA Session Key:" + nafSessionKey);
                        }
                    }

                    Log.d(TAG, "After runGbaProcedure ");
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        } .start();

    }

    private void runHttpClient() {
        Log.i(TAG, "run http client");
        new Thread() {
            @Override
            public void run() {
                super.run();

                try {
                    final String nafAddress = "mms.msg.eng.t-mobile.com";
                    final String nafUrl = "https://mms.msg.eng.t-mobile.com/mms/wapenc";

                    java.util.logging.Logger.getLogger("org.apache.http.wire").setLevel(java.util.logging.Level.ALL);
                    java.util.logging.Logger.getLogger("org.apache.http.headers").setLevel(java.util.logging.Level.ALL);

                    // Create and initialize HTTP parameters
                    HttpParams params = new BasicHttpParams();
                    ConnManagerParams.setMaxTotalConnections(params, 10);
                    HttpProtocolParams.setVersion(params, HttpVersion.HTTP_1_1);
                    HttpProtocolParams.setContentCharset(params, "UTF-8");

                    CredentialsProvider credProvider = new BasicCredentialsProvider();
                    credProvider.setCredentials(new AuthScope(AuthScope.ANY_HOST, AuthScope.ANY_PORT), new GbaCredentials(mContext, nafAddress));

                    // Create and initialize scheme registry
                    SchemeRegistry schemeRegistry = new SchemeRegistry();
                    schemeRegistry.register(new Scheme("http", PlainSocketFactory.getSocketFactory(), 80));

                    KeyStore trustStore = KeyStore.getInstance(KeyStore.getDefaultType());
                    trustStore.load(null, null);
                    SSLSocketFactory sf = new CdsSSLSocketFactory(trustStore);
                    sf.setHostnameVerifier(SSLSocketFactory.ALLOW_ALL_HOSTNAME_VERIFIER);
                    schemeRegistry.register(new Scheme("https", sf, 443));

                    // Create an HttpClient with the ThreadSafeClientConnManager.
                    ClientConnectionManager cm = new ThreadSafeClientConnManager(params, schemeRegistry);
                    DefaultHttpClient httpClient = new DefaultHttpClient(cm, params);
                    httpClient.setCredentialsProvider(credProvider);

                    HttpRequest request = createHttpRequest(nafUrl);

                    HttpResponse response = httpClient.execute(new HttpHost(nafAddress, 443, "https"), request);

                    Log.i(TAG, "StatusCode:" + response.getStatusLine().getStatusCode());

                    if (response.getStatusLine().getStatusCode() == HttpStatus.SC_OK) {
                        showToast("http is ok");
                    } else {
                        showToast("http status:" + response.getStatusLine().getStatusCode());
                    }

                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        } .start();

    }

    private HttpRequest createHttpRequest(String host) {

        HttpRequest req = new HttpPost(host);

        req.setHeader("Accept", "*/*, application/vnd.wap.mms-message, application/vnd.wap.sic");
        req.setHeader("x-wap-profile", "http://www-ccpp.tcl-ta.com/files/ALCATEL_ONE_TOUCH_5020T.xml");
        req.setHeader("Accept-Language", "zh-TW, en-US");
        req.setHeader("User-Agent", "ALCATEL_ONE_TOUCH_5020T-MMS/2.0 3gpp-gba");        

        String raw = "8c839846545850705f364d354d7444008d9295839181";
        byte[] pdu = hexStringToByteArray(raw);
        ByteArrayEntity entity = new ByteArrayEntity(pdu);
        entity.setContentType("application/vnd.wap.mms-message");
        ((HttpEntityEnclosingRequest) req).setEntity(entity);
        
        return req;
    }

    private static byte[] hexStringToByteArray(String s) {
        int len = s.length();
        byte[] data = new byte[len / 2];
        for (int i = 0; i < len; i += 2) {
                data[i / 2] = (byte) ((Character.digit(s.charAt(i), 16) << 4)
                                 + Character.digit(s.charAt(i+1), 16));
        }
        return data;
    }


    private void updateXcapInfo() {
        ImsManager imsManager = (ImsManager) mContext.getSystemService(Context.IMS_SERVICE);

        if (imsManager == null) {
            Log.e(TAG, "Null ImsManager");
            return;
        }

        ImsXcapInfo xcapInfo = imsManager.readImsXcapInfoMo();

        if (xcapInfo != null) {
            mXcapRoot.setText(xcapInfo.getXcapRootUri());
            mXcapUser.setText(xcapInfo.getXcapAuth());
            mXcapPasswd.setText(xcapInfo.getXcapAuthSecret());
            mXcapAuthType.setText(xcapInfo.getXcapAuthType());
        }
    }

    public void testIcsList(boolean bValue) throws Exception {
        ImsManager imsManager = (ImsManager) mContext.getSystemService(Context.IMS_SERVICE);

        if (imsManager == null) {
            Log.e(TAG, "Null ImsManager");
            return;
        }

        String icsi = "";
        int count = 4;
        int i = 0;

        ImsIcsi[] objs = new ImsIcsi[count];

        for (i = 0; i < count; i++) {
            if (bValue) {
                icsi = "urn:urn-7:3gpp-service.ims.icsi.mmtel";
            } else {
                icsi = "ICSI" + "*" + i;
            }

            objs[i] = new ImsIcsi(icsi, bValue);
        }

        imsManager.writeImsIcsiMo(objs);
    }

    private void getImsMoInfo() {
        int i = 0;

        ImsManager imsManager = (ImsManager) mContext.getSystemService(Context.IMS_SERVICE);

        if (imsManager == null) {
            Log.e(TAG, "Null ImsManager");
            return;
        }

        StringBuilder builder = new StringBuilder();
        builder.append("P-CSCF_Address:").append(imsManager.readImsMoString(ImsConstants.IMS_MO_PCSCF)).append("\r\n");
        builder.append("Timer_T1:").append(imsManager.readImsMoInt(ImsConstants.IMS_MO_TIMER_1)).append("\r\n");
        builder.append("Timer_T2:").append(imsManager.readImsMoInt(ImsConstants.IMS_MO_TIMER_2)).append("\r\n");
        builder.append("Timer_T2:").append(imsManager.readImsMoInt(ImsConstants.IMS_MO_TIMER_4)).append("\r\n");
        builder.append("Private_user_identity:").append(imsManager.readImsMoString(ImsConstants.IMS_MO_IMPI)).append("\r\n");
        String[] impus = imsManager.readImsMoStringArray(ImsConstants.IMS_MO_IMPU);

        if (impus == null) {
            builder.append("impus:\r\n");
        } else {
            for (i = 0; i < impus.length; i++) {
                builder.append("Public_user_identity[" + i + "]:").append(impus[i]).append("\r\n");
            }
        }

        builder.append("Home_network_domain_name:").append(imsManager.readImsMoString(ImsConstants.IMS_MO_PCSCF)).append("\r\n");

        ImsIcsi[] icsis = imsManager.readImsIcsiMo();
        builder.append(getObjectsInfo("ICSI", icsis));

        ImsLboPcscf[] pcscfs = imsManager.readImsLboPcscfMo();
        builder.append(getObjectsInfo("LBO_P-CSCF", pcscfs));

        builder.append("Resource_Allocation_Mode:").append(imsManager.readImsMoBool(ImsConstants.IMS_MO_RESOURCE)).append("\r\n");
        builder.append("Voice_Domain_Preference_E_UTRAN:").append(imsManager.readImsMoInt(ImsConstants.IMS_MO_VOICE_E)).append("\r\n");
        builder.append("SMS_Over_IP_Networks_Indication:").append(imsManager.readImsMoBool(ImsConstants.IMS_MO_SMS)).append("\r\n");
        builder.append("Keep_Alive_Enabled:").append(imsManager.readImsMoBool(ImsConstants.IMS_MO_KEEPALIVE)).append("\r\n");
        builder.append("Voice_Domain_Preference_UTRAN:").append(imsManager.readImsMoInt(ImsConstants.IMS_MO_VOICE_U)).append("\r\n");
        builder.append("Mobility_Management_IMS_Voice_Termination:").append(imsManager.readImsMoBool(ImsConstants.IMS_MO_MOBILITY)).append("\r\n");
        builder.append("RegRetryBaseTime:").append(imsManager.readImsMoInt(ImsConstants.IMS_MO_REG_BASE)).append("\r\n");
        builder.append("RegRetryMaxTime:").append(imsManager.readImsMoInt(ImsConstants.IMS_MO_REG_MAX)).append("\r\n");

        ImsPhoneCtx[] phoneCtxs = imsManager.readImsPhoneCtxMo();
        builder.append(getObjectsInfo("PhoneContext", phoneCtxs));

        ImsAuthInfo authInfo = imsManager.readImsAuthInfoMo();
        builder.append(getSingleObjectInfo("AppAuth", authInfo));
        ImsXcapInfo xcapInfo = imsManager.readImsXcapInfoMo();
        builder.append(getSingleObjectInfo("XCAP info", xcapInfo));

        mImsMoInfo.setText(builder.toString());
    }

    private String getSingleObjectInfo(String title, Object obj) {
        if (obj == null) {
            return title + ":null\r\n";
        }

        return title + ":" + obj.toString() + "\r\n";
    }

    private String getObjectsInfo(String title, Object[] objs) {
        StringBuilder builder = new StringBuilder();
        int i = 0;

        if (objs == null) {
            return title + ":null\r\n";
        }

        for (Object obj: objs) {
            i++;
            builder.append(title + "-" + i + ":[").append(obj.toString() + "]\r\n");
        }

        return builder.toString();
    }

    public void testImsPhoneCtx(boolean flag) throws Exception {

        ImsManager imsManager = (ImsManager) mContext.getSystemService(Context.IMS_SERVICE);

        if (imsManager == null) {
            Log.e(TAG, "Null ImsManager");
            return;
        }

        ImsPhoneCtx[] phoneCtxArray = imsManager.readImsPhoneCtxMo();

        if (phoneCtxArray == null) {
            Log.e(TAG, "[PhoneCtx][write] readImsPhoneCtxMo is NULL!!!");
            return;
        }

        ImsPhoneCtx phoneCtx = null;
        String phoneCtxStr = null;
        String [] userID = null;
        int index = 0;
        int pcLength = phoneCtxArray.length;

        Log.i(TAG, "[write]pcLength: " + pcLength);

        for (; index < pcLength; index++) {
            Log.i(TAG, "write PhoneCtx " + index);

            if (flag) {
                phoneCtxStr = "phoneCtx-" + index;
            } else {
                phoneCtxStr = index + "-phoneCtx";
            }

            userID = phoneCtxArray[index].getPhoneCtxIpuis();
            int userIdLength = userID.length;

            for (int j = 0; j < userIdLength; ++j) {
                Log.i(TAG, "write userID " + j);

                if (flag) {
                    userID[j] = index + ":" + j;
                } else {
                    userID[j] = j + "/" + index;
                }
            }

            phoneCtx = new ImsPhoneCtx(phoneCtxStr, userID);
            Log.i(TAG, "phoneCtx:" + phoneCtx);
            phoneCtxArray[index] = phoneCtx;
        }

        imsManager.writeImsImsPhoneCtxMo(phoneCtxArray);
    }
}
