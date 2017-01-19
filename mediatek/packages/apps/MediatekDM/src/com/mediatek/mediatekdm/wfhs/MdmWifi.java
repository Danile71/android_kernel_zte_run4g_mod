package com.mediatek.mediatekdm.wfhs;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.net.Authenticator;
import java.net.PasswordAuthentication;
import java.security.KeyManagementException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;
import java.security.UnrecoverableKeyException;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.util.HashMap;
import java.util.List;
import java.util.Set;

import javax.net.ssl.KeyManager;
import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;

import com.mediatek.common.passpoint.WifiTree;
import com.mediatek.mediatekdm.DmConst;
import com.mediatek.mediatekdm.DmService;
import com.mediatek.mediatekdm.mdm.MdmEngine;
import com.mediatek.mediatekdm.mdm.MdmException;
import com.mediatek.mediatekdm.wfhs.io.AAAServerTrustRootIOHandlerInfo;
import com.mediatek.mediatekdm.wfhs.io.PreferredRoamingPartnerIOHandlerInfo;
import com.mediatek.passpoint.WifiTreeHelper;

import android.R.bool;
import android.R.integer;
import android.R.string;
import android.content.Context;
import android.content.Intent;

import android.text.TextUtils;
import android.util.Base64;
import android.util.Log;

public class MdmWifi {

	private static final String TAG = "DM/Wifi.MdmWifi";

	public MdmWifi(DmService service) {
		mContext = service;
		mTreeMngr = new WifiTreeManager(service, this);
		//mObserver = new SubscriptionSessionStateObserver(this);
	}

	public void onInit() {
		Log.i(TAG, "MdmWifi.onInit...");
		mHotspot = mTreeMngr.getHotspotUri(null);
		Log.i(TAG, "mHotspot is: " + mHotspot);
		mTreeMngr.registerOnAddHandler();
		mTreeMngr.registerExecuteHandler();

		/* do not need IO handler now */
		// mTreeMngr.registerNodeIOHandler();
	}

	public void waitForExecute() {
		Log.i(TAG, "block the execute handler thread and wait for the response...");
		mIsExecuteDone = false;
		synchronized (mLock) {
			while (!mIsExecuteDone) {
				try {
					mLock.wait();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
		}
	}

	public void notifyExecute(Object response) {
		Log.i(TAG, "execute done, notify...");
		mResponse = response;

		if (mResponseHandler != null) {
			mResponseHandler.handleResponse(response);
		}
		mIsExecuteDone = true;
		synchronized (mLock) {
			mLock.notify();
		}
	}

	public Object getExecuteResponse() {
		return mResponse;
	}

	public void initOSUSession(Intent intent) {
		Log.i(TAG, "initOSUSession");

		mSessionId = intent.getIntExtra("type", 0);
		sSessionAction = intent.getAction();
		Log.i(TAG, "[OSU] session action : " + sSessionAction);
		Log.i(TAG, "[OSU] session id : " + mSessionId);

		String redirectUrl = intent.getStringExtra("redirecturl");
		Log.i(TAG, "[OSU] intent redirect url : " + redirectUrl);
		// sRedirectUrl = TextUtils.isEmpty(redirectUrl) ? DATA.REDIRECT_URI_OSU
		// : redirectUrl;
		sRedirectUrl = DATA.REDIRECT_URI_OSU;
		Log.i(TAG, "[OSU] redirect url : " + sRedirectUrl);
		String url = intent.getStringExtra("serverurl");
		Log.i(TAG, "[OSU] server address : " + url);
		initOSUSessionURL(url);
	}

	public void initRemediationSession(Intent intent) {
		Log.i(TAG, "initRemediationSession");

		mSessionId = getRemediationSessionID(intent);
		if (mSessionId != MdmWifi.SessionID.REMEDIATION_UPDATE_SIM_OMA) {
			setCredentialUsernamePassword(intent, ServerType.REMEDIATION);
		} else if (mSessionId == MdmWifi.SessionID.REMEDIATION_UPDATE_SIM_OMA) {
			String imsi = intent.getStringExtra("imsi");
			Log.i(TAG, "imsi is : " + imsi);
			sSimRemPackage1 = String.format("%s%s%s", DATA.CDATA_REMEDIATION_HEAD, imsi, DATA.CDATA_REMEDIATION_TAIL);

			Log.i(TAG, "SIM Remediation package1 is : " + sSimRemPackage1);
		}
		sSessionAction = intent.getAction();
		sRemSimProvision = true;
		Log.i(TAG, "[Remediation] session action : " + sSessionAction);
		Log.i(TAG, "[Remediation] session id : " + mSessionId);

		String redirectUrl = intent.getStringExtra("redirecturl");
		Log.i(TAG, "[Remediation] intent redirect url : " + redirectUrl);
		// sRedirectUrl = TextUtils.isEmpty(redirectUrl) ? DATA.REDIRECT_URI_REM
		// : redirectUrl;
		sRedirectUrl = DATA.REDIRECT_URI_REM;
		Log.i(TAG, "[Remediation] redirect url : " + sRedirectUrl);
		if (needDoAuthenticationUernamePassword(mSessionId)) {
			Log.i(TAG, "Digest Authentication");
			String username = intent.getStringExtra("subscriptiondmaccusername");
			String password = intent.getStringExtra("subscriptiondmaccpassword");
			Log.i(TAG, "[Remediation] subscriptiondmaccusername : " + username);
			Log.i(TAG, "[Remediation] subscriptiondmaccpassword : " + password);
			setWifiHTTPAuthentication(username, password);
		} else {
			Log.i(TAG, "Certificate");
		}

		String url = intent.getStringExtra("serverurl");
		Log.i(TAG, "[Remediation] server address : " + url);
		initRemediationSessionURL(url);
	}

	public void initPolicyUpdateSession(Intent intent) {
		Log.i(TAG, "initPolicyUpdateSession");
		setCredentialUsernamePassword(intent, ServerType.POLICY_UPDATE);
		mSessionId = intent.getIntExtra("type", 0);
		sSessionAction = intent.getAction();
		Log.i(TAG, "[Policy] session action : " + sSessionAction);
		Log.i(TAG, "[Policy] session type : " + mSessionId);

		if (needDoAuthenticationUernamePassword(mSessionId)) {
			Log.i(TAG, "Digest Authentication");
			String username = intent.getStringExtra("policydmaccusername");
			String password = intent.getStringExtra("policydmaccpassword");
			Log.i(TAG, "[Policy] policydmaccusername : " + username);
			Log.i(TAG, "[Policy] policydmaccpassword : " + password);
			setWifiHTTPAuthentication(username, password);
		} else {
			Log.i(TAG, "Certificate");
		}

		String url = intent.getStringExtra("serverurl");
		Log.i(TAG, "[Policy] server address : " + url);
		initPolicyUpdateSessionURL(url, intent);
	}

	public void initOSUSessionURL(String serverUrl) {
		Log.i(TAG, "initSession server address : " + serverUrl);
		if (TextUtils.isEmpty(serverUrl)) {
			Log.i(TAG, "use default server address");
			mTreeMngr.replaceStringValue(URI_SERVER_ADDR, obtainServerAddr(mSessionId));
			mTreeMngr.replaceStringValue(URI_SERVER_PORT, obtainPortNbr(mSessionId));
		} else {
			Log.i(TAG, "use wifi dm client specified server address");
			String port = serverUrl.substring(serverUrl.indexOf(":", 8) + 1, serverUrl.indexOf("/", 8));
			if (port.startsWith("https")) {
				Log.i(TAG, "server port use default port");
				port = Server.PORT_DEFAULT_HTTPS;
			}
			Log.i(TAG, "server address : " + serverUrl);
			Log.i(TAG, "server port : " + port);
			mTreeMngr.replaceStringValue(URI_SERVER_ADDR, serverUrl);
			mTreeMngr.replaceStringValue(URI_SERVER_PORT, port);
		}
		//MdmEngine.getInstance().registerSessionStateObserver(mObserver);
	}

	public void initRemediationSessionURL(String serverUrl) {
		Log.i(TAG, "initRemediationSessionURL server address : " + serverUrl);
		if (TextUtils.isEmpty(serverUrl)) { // SubscriptionUpdate/URI
			String remUriPath = "SubscriptionUpdate/URI";
			String curHotspot = mTreeMngr.getHotspotUri(WifiTreeManager.URI_SERVICE_PROVIDER);
			Log.i(TAG, "use tree /SubscriptionUpdate/URI");
			Log.i(TAG, "curHotspot :" + curHotspot);
			String serverURlPath = String.format("%s/%s", curHotspot, remUriPath);
			Log.i(TAG, "serverURlPath :" + serverURlPath);
			String RemServerURI = mTreeMngr.getValue(serverURlPath, WifiTreeManager.FORMAT_CHR);
			Log.i(TAG, "RemServerURI :" + RemServerURI);
			String port = RemServerURI.substring(RemServerURI.indexOf(":", 8) + 1, RemServerURI.indexOf("/", 8));
			if (port.startsWith("https")) {
				Log.i(TAG, "server port use default port");
				port = Server.PORT_DEFAULT_HTTPS;
			}
			Log.i(TAG, "server address : " + RemServerURI);
			Log.i(TAG, "server port : " + port);
			mTreeMngr.replaceStringValue(URI_SERVER_ADDR, RemServerURI);
			mTreeMngr.replaceStringValue(URI_SERVER_PORT, port);

		} else {
			Log.i(TAG, "use wifi dm client specified server address");
			String port = serverUrl.substring(serverUrl.indexOf(":", 8) + 1, serverUrl.indexOf("/", 8));
			if (port.startsWith("https")) {
				Log.i(TAG, "server port use default port");
				port = Server.PORT_DEFAULT_HTTPS;
			}
			Log.i(TAG, "server address : " + serverUrl);
			Log.i(TAG, "server port : " + port);
			mTreeMngr.replaceStringValue(URI_SERVER_ADDR, serverUrl);
			mTreeMngr.replaceStringValue(URI_SERVER_PORT, port);
		}
		//MdmEngine.getInstance().registerSessionStateObserver(mObserver);
	}

	public void initPolicyUpdateSessionURL(String serverUrl, Intent intent) {
		Log.i(TAG, "initPolicyUpdateSessionURL server address : " + serverUrl);
		String PolicyUpdateServerURI = null;
		String port = null;
		String PolicyUpdateUriPath = "Policy/PolicyUpdate/URI";
		String curHotspot = mTreeMngr.getHotspotUri(WifiTreeManager.URI_SERVICE_PROVIDER);
		Log.i(TAG, "use tree Policy/PolicyUpdate/URI");
		Log.i(TAG, "curHotspot :" + curHotspot);
		String serverURlPath = String.format("%s/%s", curHotspot, PolicyUpdateUriPath);
		Log.i(TAG, "serverURlPath :" + serverURlPath);
		PolicyUpdateServerURI = mTreeMngr.getValue(serverURlPath, WifiTreeManager.FORMAT_CHR);
		if (PolicyUpdateServerURI == null || TextUtils.isEmpty(PolicyUpdateServerURI)) {
			Log.i(TAG, "tree serverUrl is null");
			if (!TextUtils.isEmpty(serverUrl)) {
				PolicyUpdateServerURI = serverUrl;
			} else {
				Log.i(TAG, "intent serverUrl is null");
			}
		}
		Log.i(TAG, "PolicyUpdateServerURI :" + PolicyUpdateServerURI);
		port = PolicyUpdateServerURI.substring(PolicyUpdateServerURI.indexOf(":", 8) + 1,
				PolicyUpdateServerURI.indexOf("/", 8));
		if (port.startsWith("https")) {
			Log.i(TAG, "server port use default port");
			port = Server.PORT_DEFAULT_HTTPS;
		}
		Log.i(TAG, "server address : " + PolicyUpdateServerURI);
		Log.i(TAG, "server port : " + port);
		mTreeMngr.replaceStringValue(URI_SERVER_ADDR, PolicyUpdateServerURI);
		mTreeMngr.replaceStringValue(URI_SERVER_PORT, port);

		//MdmEngine.getInstance().registerSessionStateObserver(mObserver);
	}

	/**
	 * only use for demo
	 */
	private void replaceFile(String src, String dest) {
		Log.i(TAG, "replaceFile src : " + src);
		Log.i(TAG, "replaceFile dest : " + dest);
		FileInputStream in = null;
		FileOutputStream out = null;
		try {
			File treeInData = new File(dest);
			if (treeInData.exists()) {
				treeInData.delete();
				treeInData.createNewFile();
			}
			in = new FileInputStream(new File(src));
			out = new FileOutputStream(treeInData);
			byte[] data = new byte[1024];
			int size = 0;
			while ((size = in.read(data)) != -1) {
				out.write(data);
			}
			out.flush();
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		} finally {
			try {
				if (in != null)
					in.close();
				if (out != null)
					out.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}

	public int getSessionId() {
		return mSessionId;
	}

	public void setSessionId(int sessionId) {
		mSessionId = sessionId;
	}

	public void addResult(String key, String val) {
		Log.i(TAG, "add " + key + " : " + val);
		mResults.put(key, val);
	}

	private void clearResults() {
		if (mResults != null) {
			mResults.clear();
		}
	}

	public void onPerProviderSubscriptionMoAdded(String uri) {
		Log.i(TAG, "onPerProviderSubscriptionMoAdded newUri : " + uri);
		// For test case
		sIsWiFiTreeUpdate = true;
		// add node io handler for the new PerProviderSubscriptionMo sub tree
		mHotspot = mTreeMngr.getHotspotUri(uri);
	}

	public void onSessionEnd(int ret) {
		Log.i(TAG, "onSessionEnd : " + ret);
		Log.i(TAG, "mSessionId is : " + mSessionId);

		notifyResult(ret);
		onEnd();
	}

	private void onOSUEndUserNamePassWord() {
		Log.i(TAG, "online signup end,UserNamePassWord...");
		for (int i = 0; i < mOSUPreferUrisUPC.length; i++) {
			String val = mTreeMngr.getValue(mHotspot + mOSUPreferUrisUPC[i], mOSUPreferFormatsUPC[i]);
			addResult(mOSUPreferKeysUPC[i], val);
		}
		/* do not need IO handler now */
		// mTreeMngr.registerNodeIOHandlerForHotspot(mHotspot);
	}

	private void onOSUEndDeviceCertificate() {
		Log.i(TAG, "online signup end,Certificate...");
		for (int i = 0; i < mOSUPreferUrisCC.length; i++) {
			String val = mTreeMngr.getValue(mHotspot + mOSUPreferUrisMDPC[i], mOSUPreferFormatsMDPC[i]);
			addResult(mOSUPreferKeysMDPC[i], val);
		}
		/* do not need IO handler now */
		// mTreeMngr.registerNodeIOHandlerForHotspot(mHotspot);
	}

	private void onOSUEndCertificate() {
		Log.i(TAG, "online signup end,Certificate...");
		for (int i = 0; i < mOSUPreferUrisCC.length; i++) {
			String val = mTreeMngr.getValue(mHotspot + mOSUPreferUrisCC[i], mOSUPreferFormatsCC[i]);
			addResult(mOSUPreferKeysCC[i], val);
		}
		// Set AAAServerTrustRoot Extra key
		List<String> nodes = new AAAServerTrustRootIOHandlerInfo().getRegisterUris(mHotspot);
		Log.i(TAG, "nodes.size() = :" + nodes.size());
		if (nodes.size() != 0) {
			for (String node : nodes) {
				Log.i(TAG, "need to get value of node:" + node);
			}
			for (int i = 0; i < nodes.size(); i++) {
				String node = nodes.get(i);
				Log.i(TAG, "AAAServerTrustRoot node : " + node);
				String val = mTreeMngr.getValue(node, mOSUGetAAAServerTrustRootFormats[i]);
				Log.i(TAG, "add " + mOSUGetAAAServerTrustRootKeys[i] + " : " + val);
				addResult(mOSUGetAAAServerTrustRootKeys[i], val);
			}

		} else {
			Log.i(TAG, "AAAServerTrustRoot node is not exist");
			for (int i = 0; i < mOSUGetAAAServerTrustRootKeys.length; i++) {
				Log.i(TAG, "add " + mOSUGetAAAServerTrustRootKeys[i] + " null");
				addResult(mOSUGetAAAServerTrustRootKeys[i], null);
			}
		}
		/* do not need IO handler now */
		// mTreeMngr.registerNodeIOHandlerForHotspot(mHotspot);
	}

	private void onRemediationEnd() {
		Log.i(TAG, "remediation end...");
		// setHotSpotForRemediation();
		for (int i = 0; i < mRemPreferUris.length; i++) {
			String val = mTreeMngr.getValue(mHotspot + mRemPreferUris[i], mRemPreferFormats[i]);
			addResult(mRemPreferKeys[i], val);
		}
		addUpdateIdentifierNodeValue();
	}

	private void onRemediationEndUpdateCert() {
		Log.i(TAG, "remediation Update Certificate end...");
		// setHotSpotForRemediation();
		for (int i = 0; i < mOSUPreferUrisCC.length; i++) {
			String val = mTreeMngr.getValue(mHotspot + mRemUpdateCertUris[i], mmRemUpdateCertFormats[i]);
			addResult(mmRemUpdateCertKeys[i], val);
		}
		addUpdateIdentifierNodeValue();
	}

	private void onRemediationEndSimProvision() {
		Log.i(TAG, "RemediationEndSimProvision end...");
		// setHotSpotForRemediation();
		List<String> nodes = new PreferredRoamingPartnerIOHandlerInfo().getRegisterUris(mHotspot);
		Log.i(TAG, "nodes.size() = :" + nodes.size());
		for (String node : nodes) {
			Log.i(TAG, "need to get value of node:" + node);
		}
		for (int i = 0; i < nodes.size(); i++) {
			String node = nodes.get(i);
			Log.i(TAG, "Policy Update prefer node : " + node);
			String val = mTreeMngr.getValue(node, mPolicyPreferFormats[i]);
			addResult(mPolicyPreferKeys[i], val);
		}

		Log.i(TAG, "Hotspot is : " + mHotspot);
		for (int i = 0; i < mPolicyPreferSimOmaUris.length; i++) {
			String val = mTreeMngr.getValue(mHotspot + mPolicyPreferSimOmaUris[i], mPolicyPreferSimOmaFormats[i]);
			Log.i(TAG, mHotspot + mPolicyPreferSimOmaUris[i] + "value is : " + val);
			addResult(mPolicyPreferSimOmaKeys[i], val);
		}
	}

	private void onRemediationEndCertificate() {
		Log.i(TAG, "remediation Certificate end...");
		// setHotSpotForRemediation();
		for (int i = 0; i < mRemTypeofSubscriptionUris.length; i++) {
			String val = mTreeMngr.getValue(mHotspot + mRemTypeofSubscriptionUris[i], mRemTypeofSubscriptionFormats[i]);
			addResult(mRemTypeofSubscriptionKeys[i], val);
		}
		addUpdateIdentifierNodeValue();
	}

	private void onPolicyUpdateEnd() {
		Log.i(TAG, "policy update end...");
		// setHotSpotForRemediation();
		List<String> nodes = new PreferredRoamingPartnerIOHandlerInfo().getRegisterUris(mHotspot);
		Log.i(TAG, "nodes.size() = :" + nodes.size());
		for (String node : nodes) {
			Log.i(TAG, "need to get value of node:" + node);
		}
		for (int i = 0; i < nodes.size(); i++) {
			String node = nodes.get(i);
			Log.i(TAG, "Policy Update prefer node : " + node);
			String val = mTreeMngr.getValue(node, mPolicyPreferFormats[i]);
			addResult(mPolicyPreferKeys[i], val);
		}

		/* do not need IO handler now */
		// mTreeMngr.registerNodeIOHandlerForNode(mHotspot + "/Policy");
	}

	private void setHotSpotForRemediation() {
		if (mHotspot.contains("PerProviderSubscription")) {
			Log.i(TAG, "mHotspot is:" + mHotspot);
		} else {
			mHotspot = "./Wi-Fi/wi-fi.org/PerProviderSubscription/Cred01";
			Log.i(TAG, "Reset mHotspot is:" + mHotspot);
		}
	}

	private void onEnd() {
		Log.i(TAG, "MdmWifi.onDeinit...");
		mSessionId = 0;
		mIsExecuteDone = false;
		clearResults();
		//MdmEngine.getInstance().unregisterSessionStateObserver(mObserver);
	}

	public void addUpdateIdentifierNodeValue() {
		Log.i(TAG, "get UpdateIdentifier Node Value");
		Log.i(TAG, "mHotspot is:" + mHotspot);
		if (mHotspot.contains("PerProviderSubscription")) {
			String spNode = mHotspot.substring(0, mHotspot.indexOf("PerProviderSubscription")
					+ new String("PerProviderSubscription").length());
			String val = mTreeMngr.getValue(spNode + mUpdateIdentifierNodeName[0], mUpdateIdentifierFormats[0]);
			addResult(mUpdateIdentifierKeys[0], val);
		} else {
			Log.i(TAG, "mHotspot is wrong:" + mHotspot);
			addResult(mUpdateIdentifierKeys[0], null);

		}
	}

	private void notifyResult(int ret) {
		Log.i(TAG, "notifyResult : " + ret);
		if (getSessionId() == MdmWifi.SessionID.PROVISION_INIT) {
			setSessionId(MdmWifi.SessionID.PROVISION_USERNAME_PASSWORD);
			Log.i(TAG, "reset sessionid : " + getSessionId());
		}
		Log.i(TAG, "sessionid is: " + getSessionId());
		Intent intent = new Intent(obtainAction(mSessionId));
		intent.putExtra("result", ret);
		WifiTree wifiTree = getDMTreeToSOAPTree();
		intent.putExtra("wifitree", wifiTree);
		Log.i(TAG, "before send broadcast");
		mContext.sendBroadcast(intent);
		Log.i(TAG, "after send broadcast");
		dumpTreeInfo(wifiTree);
	}

	public boolean addSubTree(String uri, byte[] data) {
		return mTreeMngr.addSubTree(uri, data);
	}

	public boolean replaceSubTree(String uri, byte[] data) {
		return mTreeMngr.replaceSubTree(uri, data);
	}

	public boolean deleteNode(String uri) {
		return mTreeMngr.deleteNode(uri);
	}

	public WifiTree getDMTreeToSOAPTree() {
		return mTreeMngr.getDMTreeToSOAPTree();
	}

	public void dumpTreeInfo(WifiTree tree) {
		WifiTreeHelper helper = new WifiTreeHelper();
		helper.dumpTreeInfo(tree);
	}

	public void parseWifiMO(String nodeUri) {
		try {
			mTreeMngr.parseWifiMO(nodeUri);
		} catch (MdmException e) {
			// TODO: handle exception
			Log.d(TAG, "parseWifiMO error :" + e.getMessage());
		}

	}

	private boolean needDoAuthenticationUernamePassword(int sessionType) {
		switch (sessionType) {
		case MdmWifi.SessionID.PROVISION_USERNAME_PASSWORD:
		case MdmWifi.SessionID.REMEDIATION_MACHINE_USERNAME_PASSWORD:
		case MdmWifi.SessionID.REMEDIATION_USER_USERNAME_PASSWORD:
		case MdmWifi.SessionID.POLICY_UPDATE_USERNAME_PASSWORD:
			return true;
		default:
			return false;
		}
	}

	public static boolean needDoAuthenticationWithClientCertificate(int sessionType) {
		switch (sessionType) {
		case MdmWifi.SessionID.REMEDIATION_MACHINE_CERTIFICATE:
		case MdmWifi.SessionID.REMEDIATION_USER_CERTIFICATE:
		case MdmWifi.SessionID.REMEDIATION_UPDATE_CERTIFICATE:
		case MdmWifi.SessionID.POLICY_UPDATE_CERTIFICATE:
			return true;
		default:
			return false;
		}
	}

	private void setWifiHTTPAuthentication(final String username, final String password) {
		Authenticator.setDefault(new Authenticator() {
			protected PasswordAuthentication getPasswordAuthentication() {
				Log.d(TAG, "Authentication info:" + username + ":" + password);
				return new PasswordAuthentication(username, password.toCharArray());
			}
		});
	}

	public void setCredentialUsernamePassword(Intent intent, String serverType) {
		String PolicyUsername = "Policy/PolicyUpdate/UsernamePassword/Username";
		String PolicyPassword = "Policy/PolicyUpdate/UsernamePassword/Password";
		String RemUsername = "SubscriptionUpdate/UsernamePassword/Username";
		String RemPassword = "SubscriptionUpdate/UsernamePassword/Password";
		String CreUsername = "Credential/UsernamePassword/Username";
		String CrePassword = "Credential/UsernamePassword/Password";
		String username = null;
		String password = null;
		String username_policy = null;
		String password_policy = null;
		String username_subscription_update = null;
		String password_subscription_update = null;
		String username_credential = null;
		String password_credential = null;

		String curHotspot = mTreeMngr.getHotspotUri(WifiTreeManager.URI_SERVICE_PROVIDER);

		if (serverType.equals(ServerType.POLICY_UPDATE)) {
			String credentialnodename = intent.getStringExtra("credentialnodename");
			Log.d(TAG, "credentialnodename is :" + credentialnodename);
			curHotspot = String.format("%s/%s", WifiTreeManager.URI_SERVICE_PROVIDER, credentialnodename);
			Log.d(TAG, "curHotspot is :" + curHotspot);
		} else {
			// do nothing
			Log.d(TAG, "curHotspot is :" + curHotspot);
		}
		String[] credentialChildrenNodeNames = mTreeMngr.getChildrenNodeName(curHotspot);
		for (String credentialInfoChildrenNodeName : credentialChildrenNodeNames) {
			String credentialInfoChilNodeUri = String.format("%s/%s", curHotspot, credentialInfoChildrenNodeName);
			// credentialInfoChilNodeUri = ./Wi-Fi/ X
			// /PerProviderSubscription/<X+>/node
			if (credentialInfoChildrenNodeName.equals("Policy")) {
				String[] policyChilNodeNames = mTreeMngr.getChildrenNodeName(credentialInfoChilNodeUri);
				for (String policyChlNodeName : policyChilNodeNames) {
					String policyChilNodeUri = String.format("%s/%s", credentialInfoChilNodeUri, policyChlNodeName);
					if (policyChlNodeName.equals("PolicyUpdate")) {
						String[] PUChilNodeNames = mTreeMngr.getChildrenNodeName(policyChilNodeUri);
						for (String PUChilNodeName : PUChilNodeNames) {
							if (PUChilNodeName.equals("UsernamePassword")) {
								username_policy = mTreeMngr.getValue(
										String.format("%s/%s", curHotspot, PolicyUsername), WifiTreeManager.FORMAT_CHR);
								String format = null;
								String base64Password = null;
								format = mTreeMngr.getProperty(String.format("%s/%s", curHotspot, PolicyPassword),
										"Format");
								Log.i(TAG, "password format = " + format);
								if (format != null) {
									if (format.equalsIgnoreCase("bin")) {
										password_policy = mTreeMngr.getValue(
												String.format("%s/%s", curHotspot, PolicyPassword),
												WifiTreeManager.FORMAT_CHR);
										Log.i(TAG, "password_policy = " + password_policy);
									}
								} else {
									base64Password = mTreeMngr.getValue(
											String.format("%s/%s", curHotspot, PolicyPassword),
											WifiTreeManager.FORMAT_CHR);
									Log.i(TAG, "get string base64Password = " + base64Password);
									password_policy = decodeBase64(base64Password);
									Log.i(TAG, "password_policy = " + password_policy);
								}
								Log.i(TAG, "base64Password :" + base64Password);
								Log.i(TAG, "username_policy:" + username_policy);
								Log.i(TAG, "password_policy:" + password_policy);
								break;
							}
						}
						break;
					}
				}
			}

			if (credentialInfoChildrenNodeName.equals("Credential")) {
				String[] creChilNodeNames = mTreeMngr.getChildrenNodeName(credentialInfoChilNodeUri);
				for (String creChilNodeName : creChilNodeNames) {
					if (creChilNodeName.equals("UsernamePassword")) {
						username_credential = mTreeMngr.getValue(String.format("%s/%s", curHotspot, CreUsername),
								WifiTreeManager.FORMAT_CHR);
						String format = null;
						String base64Password = null;
						format = mTreeMngr.getProperty(String.format("%s/%s", curHotspot, CrePassword), "Format");
						Log.i(TAG, "password format = " + format);
						if (format != null) {
							if (format.equalsIgnoreCase("bin")) {
								password_credential = mTreeMngr.getValue(
										String.format("%s/%s", curHotspot, CrePassword), WifiTreeManager.FORMAT_CHR);
								Log.i(TAG, "password_credential = " + password_credential);
							}
						} else {
							base64Password = mTreeMngr.getValue(String.format("%s/%s", curHotspot, CrePassword),
									WifiTreeManager.FORMAT_CHR);
							Log.i(TAG, "get string base64Password = " + base64Password);
							password_credential = decodeBase64(base64Password);
							Log.i(TAG, "password_credential = " + password_credential);
						}
						Log.i(TAG, "base64Password :" + base64Password);
						Log.i(TAG, "username_credential:" + username_credential);
						Log.i(TAG, "password_credential:" + password_credential);
						break;
					}
				}
			}
			if (credentialInfoChildrenNodeName.equals("SubscriptionUpdate")) {
				String[] SUChilNodeNames = mTreeMngr.getChildrenNodeName(credentialInfoChilNodeUri);
				for (String SUChilNodeName : SUChilNodeNames) {
					if (SUChilNodeName.equals("UsernamePassword")) {
						username_subscription_update = mTreeMngr.getValue(
								String.format("%s/%s", curHotspot, RemUsername), WifiTreeManager.FORMAT_CHR);
						String format = null;
						String base64Password = null;
						format = mTreeMngr.getProperty(String.format("%s/%s", curHotspot, RemPassword), "Format");
						Log.i(TAG, "password format = " + format);
						if (format != null) {
							if (format.equalsIgnoreCase("bin")) {
								password_subscription_update = mTreeMngr.getValue(
										String.format("%s/%s", curHotspot, RemPassword), WifiTreeManager.FORMAT_CHR);
								Log.i(TAG, "password_subscription_update = " + password_subscription_update);
							}
						} else {
							base64Password = mTreeMngr.getValue(String.format("%s/%s", curHotspot, RemPassword),
									WifiTreeManager.FORMAT_CHR);
							Log.i(TAG, "get string base64Password = " + base64Password);
							password_subscription_update = decodeBase64(base64Password);
							Log.i(TAG, "password_subscription_update = " + password_subscription_update);
						}
						Log.i(TAG, "base64Password :" + base64Password);
						Log.i(TAG, "username_subscription_update:" + username_subscription_update);
						Log.i(TAG, "password_subscription_update:" + password_subscription_update);
						break;
					}
				}
			}
		}
		if (serverType.equals(ServerType.REMEDIATION)) {
			Log.i(TAG, "Set REMEDIATION username password");
			Log.i(TAG, "username_subscription_update:" + username_subscription_update);
			Log.i(TAG, "password_subscription_update:" + password_subscription_update);
			if (username_subscription_update != null && password_subscription_update != null) {

				username = username_subscription_update;
				password = password_subscription_update;
			} else {
				username = username_credential;
				password = password_credential;
			}
			Log.i(TAG, "username:" + username);
			Log.i(TAG, "password:" + password);
			intent.putExtra("subscriptiondmaccusername", username);
			intent.putExtra("subscriptiondmaccpassword", password);

		} else if (serverType.equals(ServerType.POLICY_UPDATE)) {
			Log.i(TAG, "Set POLICY_UPDATE username password");
			Log.i(TAG, "username_policy:" + username_policy);
			Log.i(TAG, "password_policy:" + password_policy);
			if (username_policy != null && password_policy != null) {
				username = username_policy;
				password = password_policy;
			} else {

				username = username_credential;
				password = password_credential;
			}
			Log.i(TAG, "username:" + username);
			Log.i(TAG, "password:" + password);
			intent.putExtra("policydmaccusername", username);
			intent.putExtra("policydmaccpassword", password);
		} else {
			Log.e(TAG, "Unknown Server type :" + serverType);
		}
	}

	private String decodeBase64(String s) {
		try {
			byte[] decoded = Base64.decode(s, Base64.DEFAULT);
			return new String(decoded);
		} catch (Exception e) {
			Log.d(TAG, "decode err:" + e);
		}
		return null;
	}

	private int getRemediationSessionID(Intent intent) {
		int sessionID = 0;
		String redirectUrl = intent.getStringExtra("redirecturl");
		String username = intent.getStringExtra("subscriptiondmaccusername");
		String password = intent.getStringExtra("subscriptiondmaccpassword");
		sessionID = intent.getIntExtra("type", -1);

		Log.i(TAG, "[Remediation] redirecturl : " + redirectUrl);
		Log.i(TAG, "[Remediation] subscriptiondmaccusername : " + username);
		Log.i(TAG, "[Remediation] subscriptiondmaccpassword : " + password);
		Log.i(TAG, "[Remediation] sessionID : " + sessionID);
		switch (sessionID) {
		case 21:
			sessionID = MdmWifi.SessionID.REMEDIATION_MACHINE_USERNAME_PASSWORD;
			break;
		case 23:
			if (!TextUtils.isEmpty(redirectUrl)) {
				Log.i(TAG, "[Remediation] redirectUrl is not null ");
				sessionID = MdmWifi.SessionID.REMEDIATION_USER_CERTIFICATE;
			} else {
				Log.i(TAG, "[Remediation] redirectUrl is null ");
				sessionID = MdmWifi.SessionID.REMEDIATION_UPDATE_CERTIFICATE;
			}
			break;
		case 27:
			sessionID = MdmWifi.SessionID.REMEDIATION_UPDATE_SIM_OMA;
			break;
		default:
			break;
		}
		Log.i(TAG, "[Remediation] sessionID reset: " + sessionID);
		return sessionID;
	}

	private String obtainAction(int sessionId) {
		switch (sessionId) {
		case SessionID.PROVISION_INIT:
		case SessionID.PROVISION_USERNAME_PASSWORD:
		case SessionID.PROVISION_CERTIFICATE:
		case SessionID.PROVISION_DEVICE_CERTIFICATE:
			return SessionAction.PROVISION_END;

		case SessionID.REMEDIATION_MACHINE_USERNAME_PASSWORD:
		case SessionID.REMEDIATION_USER_USERNAME_PASSWORD:
		case SessionID.REMEDIATION_MACHINE_CERTIFICATE:
		case SessionID.REMEDIATION_USER_CERTIFICATE:
		case SessionID.REMEDIATION_UPDATE_CERTIFICATE:
		case SessionID.REMEDIATION_UPDATE_CERTIFICATE_UNPW:
			return SessionAction.REMEDIATION_END;
		case SessionID.REMEDIATION_UPDATE_SIM_OMA:
			return SessionAction.REMEDIATION_END_SIM_PROVISION;
		case SessionID.POLICY_UPDATE_USERNAME_PASSWORD:
		case SessionID.POLICY_UPDATE_CERTIFICATE:

			return SessionAction.POLICY_UPDATE_END;
		}
		Log.e(TAG, "error action because of error id : " + sessionId);
		return null;
	}

	private String obtainServerAddr(int sessionId) {
		switch (sessionId) {
		case SessionID.PROVISION_INIT:
		case SessionID.PROVISION_USERNAME_PASSWORD:
		case SessionID.PROVISION_CERTIFICATE:
		case SessionID.PROVISION_DEVICE_CERTIFICATE:
			return Server.ADDR_PROVIOSN;

		case SessionID.REMEDIATION_MACHINE_USERNAME_PASSWORD:
		case SessionID.REMEDIATION_USER_USERNAME_PASSWORD:
		case SessionID.REMEDIATION_MACHINE_CERTIFICATE:
		case SessionID.REMEDIATION_USER_CERTIFICATE:
		case SessionID.REMEDIATION_UPDATE_CERTIFICATE:
		case SessionID.REMEDIATION_UPDATE_CERTIFICATE_UNPW:
		case SessionID.REMEDIATION_UPDATE_SIM_OMA:
			return Server.ADDR_REMEDIATION_DIGEST;

		case SessionID.POLICY_UPDATE_USERNAME_PASSWORD:
		case SessionID.POLICY_UPDATE_CERTIFICATE:
			return Server.ADDR_POLICY_UPDATE_DIGEST;
		}
		Log.e(TAG, "error server address because of error id : " + sessionId);
		return null;
	}

	private String obtainPortNbr(int sessionId) {
		switch (sessionId) {
		case SessionID.PROVISION_INIT:
		case SessionID.PROVISION_USERNAME_PASSWORD:
		case SessionID.PROVISION_CERTIFICATE:
		case SessionID.PROVISION_DEVICE_CERTIFICATE:
			return Server.PORT_PROVISION;

		case SessionID.REMEDIATION_MACHINE_USERNAME_PASSWORD:
		case SessionID.REMEDIATION_USER_USERNAME_PASSWORD:
		case SessionID.REMEDIATION_MACHINE_CERTIFICATE:
		case SessionID.REMEDIATION_USER_CERTIFICATE:
		case SessionID.REMEDIATION_UPDATE_CERTIFICATE:
		case SessionID.REMEDIATION_UPDATE_CERTIFICATE_UNPW:
		case SessionID.REMEDIATION_UPDATE_SIM_OMA:
			return Server.PORT_REMEDIATION;

		case SessionID.POLICY_UPDATE_USERNAME_PASSWORD:
		case SessionID.POLICY_UPDATE_CERTIFICATE:
			return Server.PORT_POLICY_UPDATE;
		}
		Log.e(TAG, "error server port number because of error id : " + sessionId);
		return null;
	}

	public static String sRedirectUrl;
	// TODO : more session run will be error
	public static String sSessionAction;
	private int mSessionId;
	public static String sSimRemPackage1;
	public static boolean sIsWiFiTreeUpdate;
	public static boolean sRemSimProvision;
	private Context mContext;
	private WifiTreeManager mTreeMngr;
	//private SubscriptionSessionStateObserver mObserver;
	private HashMap<String, String> mResults = new HashMap<String, String>();

	private String mHotspot;

	private boolean mIsExecuteDone = false;
	private Object mResponse;
	private ExecuteResponseHandler mResponseHandler;
	private Object mLock = new Object();

	private String[] mOSUPreferUrisUPC = { "/Credential/Realm", "/Credential/UsernamePassword/Username",
			"/Credential/UsernamePassword/Password", "/Credential/UsernamePassword/EAPMethod/EAPType",
			"/Credential/UsernamePassword/EAPMethod/InnerMethod", "/HomeSP/FQDN", "/CredentialPriority"

	};

	private int[] mOSUPreferFormatsUPC = { WifiTreeManager.FORMAT_CHR, WifiTreeManager.FORMAT_CHR,
			WifiTreeManager.FORMAT_CHR, WifiTreeManager.FORMAT_CHR, WifiTreeManager.FORMAT_CHR,
			WifiTreeManager.FORMAT_CHR, WifiTreeManager.FORMAT_INT };

	private String[] mOSUPreferKeysUPC = { "realm", "username", "password", "eapmethod", "innermethod", "homespfqdn",
			"credentialpriority" };

	private String[] mOSUPreferUrisCC = { "/Credential/DigitalCertificate/CertificateType",
			"/Credential/DigitalCertificate/CertSHA256Fingerprint", "/Credential/Realm", "/HomeSP/FQDN",
			"/CredentialPriority" };

	private int[] mOSUPreferFormatsCC = { WifiTreeManager.FORMAT_CHR, WifiTreeManager.FORMAT_CHR,
			WifiTreeManager.FORMAT_CHR, WifiTreeManager.FORMAT_CHR, WifiTreeManager.FORMAT_INT };

	private String[] mOSUPreferKeysCC = { "certificatetype", "certsha256fingerprint", "realm", "homespfqdn",
			"credentialpriority"

	};

	private String[] mOSUPreferUrisMDPC = { "/Credential/DigitalCertificate/CertificateType",
			"/Credential/DigitalCertificate/CertSHA256Fingerprint" };

	private int[] mOSUPreferFormatsMDPC = { WifiTreeManager.FORMAT_CHR, WifiTreeManager.FORMAT_CHR };

	private String[] mOSUPreferKeysMDPC = { "certificatetype", "certsha256fingerprint"

	};

	private String[] mRemUpdateCertUris = { "/Credential/DigitalCertificate/CertificateType",
			"/Credential/DigitalCertificate/CertSHA256Fingerprint" };

	private int[] mmRemUpdateCertFormats = { WifiTreeManager.FORMAT_CHR, WifiTreeManager.FORMAT_CHR };

	private String[] mmRemUpdateCertKeys = { "certificatetype", "certsha256fingerprint"

	};
	//
	private int[] mOSUGetAAAServerTrustRootFormats = { WifiTreeManager.FORMAT_CHR, WifiTreeManager.FORMAT_CHR };

	private String[] mOSUGetAAAServerTrustRootKeys = { "aaacerturl", "aaacertsha256fingerprint" };

	private String[] mRemPreferUris = { "/Credential/UsernamePassword/Password" };

	private int[] mRemPreferFormats = { WifiTreeManager.FORMAT_CHR };

	private String[] mRemPreferKeys = { "newpassword" };
	private String[] mRemTypeofSubscriptionUris = { "/SubscriptionParameters/TypeOfSubscription",
			"/Credential/DigitalCertificate/CertificateType" };

	private int[] mRemTypeofSubscriptionFormats = { WifiTreeManager.FORMAT_CHR, WifiTreeManager.FORMAT_CHR };

	private String[] mRemTypeofSubscriptionKeys = { "typeOfsubscription", "certificatetype" };

	private String[] mUpdateIdentifierNodeName = { "/UpdateIdentifier" };
	private int[] mUpdateIdentifierFormats = { WifiTreeManager.FORMAT_INT };

	private String[] mUpdateIdentifierKeys = { "updateidentifier" };
	private int[] mPolicyPreferFormats = { WifiTreeManager.FORMAT_CHR, WifiTreeManager.FORMAT_INT,
			WifiTreeManager.FORMAT_CHR };

	private String[] mPolicyPreferKeys = { "fqdnmatch", "priority", "country" };

	private int[] mPolicyPreferSimOmaFormats = { WifiTreeManager.FORMAT_CHR, WifiTreeManager.FORMAT_CHR };

	private String[] mPolicyPreferSimOmaKeys = { "policyUpdatecerturl", "subscriptionUpdatecerturl" };

	private String[] mPolicyPreferSimOmaUris = { "/Policy/PolicyUpdate/TrustRoot/CertURL",
			"/SubscriptionUpdate/TrustRoot/CertURL" };

	public static final class ServerType {
		public static final String REMEDIATION = "remediation_server";
		public static final String POLICY_UPDATE = "policy_update_server";
	}

	public static final String TEST_TREE_OSU = "/system/etc/dm/tree_osu.xml";
	public static final String TEST_TREE_POLICY = "/system/etc/dm/tree_policy.xml";

	public static final String ACCOUNT = "WiFiAcc";
	public static final String URI_SERVER_ADDR = "./DMAcc/WiFiAcc/AppAddr/SrvAddr/Addr";
	public static final String URI_SERVER_PORT = "./DMAcc/WiFiAcc/AppAddr/SrvAddr/Port/Port/PortNbr";

	public static final class Server {
		public static final String ADDR_PROVIOSN = "https://10.123.103.1:9446/OnlineSignup/syncml/newUser/digest";
		public static final String ADDR_REMEDIATION_DIGEST = "https://10.123.103.1:9445/OnlineSignup/syncml/doAuthenticate";
		public static final String ADDR_POLICY_UPDATE_DIGEST = "https://10.123.103.1:9445/OnlineSignup/syncml/doAuthenticate";
		public static final String ADDR_REMEDIATION_CERT = "https://10.123.103.1:9445/OnlineSignup/syncml/doAuthenticate";
		public static final String ADDR_POLICY_UPDATE_CERT = "https://10.123.103.1:9445/OnlineSignup/syncml/doAuthenticate";

		public static final String PORT_PROVISION = "9446";
		public static final String PORT_REMEDIATION = "9445";
		public static final String PORT_POLICY_UPDATE = "9445";
		// HTTPS default port
		public static final String PORT_DEFAULT_HTTPS = "443";
	}

	public static final class AlertType {
		public static final String HEADER = "Reversed-Domain-Name:";
		public static final String SUBSCRIPTION_REGISTRATION = HEADER + "org.wi-fi.hotspot2dot0.SubscriptionCreation";
		public static final String CERTIFICATE_ENROLLMENT = HEADER + "org.wi-fi.hotspot2dot0.CertificateEnrollment";
		public static final String PERPROVIDERSUBSCRIPTION_MO_REQUEST = HEADER
				+ "org.wi-fi.hotspot2dot0.PerProviderSubscriptionMORequest";
		public static final String SUBSCRIPTION_REMEDIATION = HEADER + "org.wi-fi.hotspot2dot0.SubscriptionRemediation";
		public static final String SUBSCRIPTION_REMEDIATION_SIM = HEADER
				+ "org.wi-fi.hotspot2dot0.SubscriptionProvisioning";
		public static final String POLICY_UPDATE_FOR_NDS = HEADER + "org.wi-fi.hotspot2dot0.PolicyUpdate";
	}

	public static final class Source {
		public static final String PROVIOSN = "./Wi-Fi/SPFQDN1/PerProviderSubscription";
		public static final String REMEDIATION = PROVIOSN;
		public static final String POLICY_UPDATE = "./Wi-Fi/SPFQDN1/PerProviderSubscription/X/Policy";
	}

	public static final class Format {
		public static final String XML = "xml";
	}

	public static final class SessionID {
		public static final int PROVISION_INIT = 10;
		public static final int PROVISION_USERNAME_PASSWORD = 11;
		public static final int PROVISION_CERTIFICATE = 12;
		public static final int PROVISION_DEVICE_CERTIFICATE = 13;
		public static final int REMEDIATION_MACHINE_USERNAME_PASSWORD = 21;
		public static final int REMEDIATION_USER_USERNAME_PASSWORD = 22;
		public static final int REMEDIATION_MACHINE_CERTIFICATE = 23;
		public static final int REMEDIATION_USER_CERTIFICATE = 24;
		public static final int REMEDIATION_UPDATE_CERTIFICATE = 25;
		public static final int REMEDIATION_UPDATE_CERTIFICATE_UNPW = 26;
		public static final int REMEDIATION_UPDATE_SIM_OMA = 27;
		public static final int POLICY_UPDATE_USERNAME_PASSWORD = 31;
		public static final int POLICY_UPDATE_CERTIFICATE = 32;

	}

	public static final class SessionType {
		public static final String PROVISION_USERNAME_PASSWORD = "provisioning.username/password";
		public static final String PROVISION_CERTIFICATE = "provisioning.certificate";
		public static final String PROVISION_DEVICE_CERTIFICATE = "provisioning.device.credentials";
		public static final String REMEDIATION_MACHINE_USERNAME_PASSWORD = "remediation.username/password.machine";
		public static final String REMEDIATION_USER_USERNAME_PASSWORD = " remediation.username/password.user";
		public static final String REMEDIATION_MACHINE_CERTIFICATE = "remediation.certificate.machine";
		public static final String REMEDIATION_USER_CERTIFICATE = "remediation.certificate.user";
		public static final String REMEDIATION_UPDATE_CERTIFICATE = "remediation.certificate.update";
		public static final String REMEDIATION_UPDATE_CERTIFICATE_USERNAME_PASSWORD = "remediation.certificate.update.username/password";
		public static final String POLICY_UPDATE_USERNAME_PASSWORD = "policy.username/password";
		public static final String POLICY_UPDATE_SERTIFICATE = "policy.certificate";
	}

	public static final class SessionAction {
		public static final String PROVISION_START = "com.mediatek.dm.provision.start";
		public static final String PROVISION_END = "com.mediatek.dm.provision.end";
		public static final String PROVISION_MO_REQUEST = "com.mediatek.dm.provision.mo.request";
		public static final String REMEDIATION_START = "com.mediatek.dm.remediation.start";
		public static final String REMEDIATION_END = "com.mediatek.dm.remediation.end";
		public static final String REMEDIATION_END_SIM_PROVISION = "com.mediatek.dm.sim.provision.end";
		public static final String POLICY_UPDATE_START = "com.mediatek.dm.policy.start";
		public static final String POLICY_UPDATE_END = "com.mediatek.dm.policy.end";

		public static final String LAUNCH_BROWSER = Intent.ACTION_VIEW;
		public static final String LAUNCH_BROWSER_RESPONSE = "com.mediatek.passpoint.browser.redirect.dm";
		public static final String LAUNCH_BROWSER_RESPONSE_REM = "com.mediatek.passpoint.browser.remredirect.dm";
		public static final String ENROLLMENT = "com.mediatek.dm.certificateenrollment";
		public static final String REENROLLMENT = "com.mediatek.dm.certificatereenrollment";
		public static final String ENROLLMENT_RESPONSE = "com.mediatek.dm.certificateenrollment.done";
		public static final String REENROLLMENT_RESPONSE = "com.mediatek.dm.certificatereenrollment.done";
		public static final String NEGOTIATE = "com.mediatek.dm.negotiate";
		public static final String NEGOTIATE_RESPONSE = "com.mediatek.dm.negotiate.done";

		public static final String SOAP_PROVISION_START = "com.mediatek.soap.provision.start";
		public static final String SOAP_PROVISION_END = "com.mediatek.soap.provision.end";
		public static final String SOAP_REMEDIATION_START = "com.mediatek.soap.remediation.start";
		public static final String SOAP_REMEDIATION_END = "com.mediatek.soap.remediation.end";
		public static final String SOAP_POLICY_UPDATE_START = "com.mediatek.soap.policy.start";
		public static final String SOAP_POLICY_UPDATE_END = "com.mediatek.soap.policy.end";
		public static final String SOAP_SIM_PROVISION_START = "com.mediatek.soap.sim.provision.start";
		public static final String SOAP_SIM_PROVISION_END = "com.mediatek.soap.sim.provision.end";

		public static final String GET_DM_TREE_START = "com.mediatek.dm.wifitree.start";
		public static final String GET_DM_TREE_END = "com.mediatek.dm.wifitree.end";
	}

	public static final class DATA {

		public static String REDIRECT_URI = "http://127.0.0.1:54685/index.htm";
		public static String REDIRECT_URI_OSU = "http://127.0.0.1:54685/index.htm";
		public static String REDIRECT_URI_REM = "http://127.0.0.1:54686/index.htm";

		public static final String CDATA_POLICY_UPDATE = "<MgmtTree xmlns=\"syncml:dmddf1.2\"><VerDTD>1.2</VerDTD>"
				+ "<Node><NodeName>DevDetail</NodeName>" + "<Node><NodeName>Ext</NodeName>"
				+ "<Node><NodeName>org.wi-fi</NodeName>" + "<Node><NodeName>Wi-Fi</NodeName>"
				+ "<Node><NodeName>ClientTriggerRedirectURI</NodeName>"
				+ "<Value>http://127.0.0.1:54685/index.htm</Value>" + "</Node>" + "<Node><NodeName>IMSI</NodeName>"
				+ "<Value>310026000000004</Value>" + "</Node>" + "</Node></Node></Node></Node></MgmtTree>";
		public static final String CDATA_OSU = "<MgmtTree xmlns=\"syncml:dmddf1.2\"><VerDTD>1.2</VerDTD>"
				+ "<Node><NodeName>DevDetail</NodeName>" + "<Node><NodeName>Ext</NodeName>"
				+ "<Node><NodeName>org.wi-fi</NodeName>" + "<Node><NodeName>Wi-Fi</NodeName>"
				+ "<Node><NodeName>ClientTriggerRedirectURI</NodeName>"
				+ "<Value>http://127.0.0.1:54685/index.htm</Value>" + "</Node>" + "<Node><NodeName>IMSI</NodeName>"
				+ "<Value>310026000000004</Value>" + "</Node>" + "</Node></Node></Node></Node></MgmtTree>";
		public static final String CDATA_REMEDIATION = "<MgmtTree xmlns=\"syncml:dmddf1.2\"><VerDTD>1.2</VerDTD>"
				+ "<Node><NodeName>DevDetail</NodeName>" + "<Node><NodeName>Ext</NodeName>"
				+ "<Node><NodeName>org.wi-fi</NodeName>" + "<Node><NodeName>Wi-Fi</NodeName>"
				+ "<Node><NodeName>ClientTriggerRedirectURI</NodeName>"
				+ "<Value>http://127.0.0.1:54686/index.htm</Value>" + "</Node>" + "<Node><NodeName>IMSI</NodeName>"
				+ "<Value>310026000000004</Value>" + "</Node>" + "</Node></Node></Node></Node></MgmtTree>";
		public static final String CDATA_REMEDIATION_HEAD = "<MgmtTree xmlns=\"syncml:dmddf1.2\"><VerDTD>1.2</VerDTD>"
				+ "<Node><NodeName>DevDetail</NodeName>" + "<Node><NodeName>Ext</NodeName>"
				+ "<Node><NodeName>org.wi-fi</NodeName>" + "<Node><NodeName>Wi-Fi</NodeName>"
				+ "<Node><NodeName>ClientTriggerRedirectURI</NodeName>"
				+ "<Value>http://127.0.0.1:54686/index.htm</Value>" + "</Node>" + "<Node><NodeName>IMSI</NodeName>"
				+ "<Value>";
		public static final String CDATA_REMEDIATION_TAIL = "</Value>" + "</Node>"
				+ "</Node></Node></Node></Node></MgmtTree>";

	}
}
