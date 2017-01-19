package com.orangelabs.rcs.core.ims.network.registration;

import java.lang.reflect.Method;

import javax2.sip.header.AuthorizationHeader;
import javax2.sip.header.WWWAuthenticateHeader;

import android.content.Context;
import android.telephony.TelephonyManager;
import android.util.Base64;

import com.orangelabs.rcs.core.CoreException;
import com.orangelabs.rcs.core.ims.ImsModule;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.protocol.sip.SipResponse;
import com.orangelabs.rcs.platform.AndroidFactory;

//import com.android.internal.telephony.PhoneConstants;

public class AKADigestRegisterationProcedure extends RegistrationProcedure {

	private String responseDigest = null;

	private final static int RAND_LEN = 16;
	private final static int AUTN_LEN = 16;

	private String cnonce = "" + System.currentTimeMillis();
	private String nonce = null;
	private String opaque = null;
	/**
	 * Cnonce counter
	 */
	private int nc = 0;

	@Override
	public void init() {
	}

	/**
	 * Build the cnonce counter
	 * 
	 * @return String (ie. "00000001")
	 */
	public String buildNonceCounter() {
		String result = Integer.toHexString(nc);
		while (result.length() != 8) {
			result = "0" + result;
		}
		return result;
	}

	@Override
	public String getHomeDomain() {
		return ImsModule.IMS_USER_PROFILE.getHomeDomain();

	}

	@Override
	public String getPublicUri() {
		// TODO Auto-generated method stub
		return "sip:" + ImsModule.IMS_USER_PROFILE.getUsername() + "@"
				+ ImsModule.IMS_USER_PROFILE.getHomeDomain();
	}

	@Override
	public void writeSecurityHeader(SipRequest request) throws CoreException {
		try {

			String realm = ImsModule.IMS_USER_PROFILE.getRealm();

			// Build the Authorization header
			String auth = "Digest username=\""
					+ ImsModule.IMS_USER_PROFILE.getPrivateID() + "\""
					+ ",uri=\"" + request.getRequestURI() + "\""
					+ ",algorithm=AKAv1-MD5" + ",realm=\"" + realm + "\""
					+ ",nonce=\"" + nonce + "\"" + ",cnonce=\"" + cnonce + "\""
					+ ",nc=\"" + buildNonceCounter() + "\"" + ",qop=\""
					+ "auth-int" + "\"" + ",response=\"" + responseDigest
					+ "\"";

			if (opaque != null) {
				auth += ",opaque=\"" + opaque + "\"";
			}

			// Set header in the SIP message
			request.addHeader(AuthorizationHeader.NAME, auth);
		} catch (Exception e) {

			throw new CoreException("Can't write the security header");
		}

	}

	@Override
	public void readSecurityHeader(SipResponse response) throws CoreException {

		WWWAuthenticateHeader wwwHeader = (WWWAuthenticateHeader) response
				.getHeader(WWWAuthenticateHeader.NAME);
		try {
			TelephonyManager tm = (TelephonyManager) AndroidFactory
					.getApplicationContext().getSystemService(
							Context.TELEPHONY_SERVICE);

			Class c = Class.forName(tm.getClass().getName());
			Method m = c.getDeclaredMethod("getITelephony");
			m.setAccessible(true);
			Object telephonyService = m.invoke(tm);
			c = Class.forName(telephonyService.getClass().getName());

			nonce = wwwHeader.getNonce();
			opaque = wwwHeader.getOpaque();

			Class[] cArg = new Class[4];
			cArg[0] = Integer.class;
			cArg[1] = Byte[].class;
			cArg[2] = Byte[].class;
			cArg[3] = Integer.class;

			m = c.getDeclaredMethod("simAkaAuthentication", cArg);

			m.setAccessible(true); // Make it accessible
			// Decode nonce field with Base64

			byte[] decodedData = Base64.decode(nonce, Base64.DEFAULT);
			if (decodedData.length < RAND_LEN + AUTN_LEN) {
				throw new IllegalStateException(
						"The length of decoded content is less then required.");
			}
			// Split nonce into RAND (Length: 16) + AUTN (Length: 16) two
			// fields.
			final byte[] rand = new byte[RAND_LEN];
			final byte[] autn = new byte[AUTN_LEN];

			System.arraycopy(decodedData, 0, rand, 0, RAND_LEN);
			System.arraycopy(decodedData, RAND_LEN, autn, 0, AUTN_LEN);

			Object[] cObjArg = new Object[4];
			cObjArg[0] = 3;// IMS
			cObjArg[1] = rand;
			cObjArg[2] = autn;
			cObjArg[3] = 0;// PhoneConstants.GEMINI_SIM_1;//considering single
							// sim

			responseDigest = (String) m.invoke(telephonyService, cArg);
		} catch (Exception e) {
			e.printStackTrace();
		}

	}

	@Override
	public String getPublicUri_ex() {
		// TODO Auto-generated method stub
		return null;
	}
}

/*
 * 1) Initial request
 * 
 * REGISTER sip:home.mobile.biz SIP/2.0
 * 
 * 
 * 2) Response containing a challenge
 * 
 * SIP/2.0 401 Unauthorized WWW-Authenticate: Digest
 * realm="RoamingUsers@mobile.biz",
 * nonce="CjPk9mRqNuT25eRkajM09uTl9nM09uTl9nMz5OX25PZz==", qop="auth,auth-int",
 * opaque="5ccc069c403ebaf9f0171e9517f40e41", algorithm=AKAv1-MD5
 * 
 * 3) Request containing credentials
 * 
 * REGISTER sip:home.mobile.biz SIP/2.0 Authorization: Digest
 * username="jon.dough@mobile.biz", realm="RoamingUsers@mobile.biz",
 * nonce="CjPk9mRqNuT25eRkajM09uTl9nM09uTl9nMz5OX25PZz==",
 * uri="sip:home.mobile.biz", qop=auth-int, nc=00000001, cnonce="0a4f113b",
 * response="6629fae49393a05397450978507c4ef1",
 * opaque="5ccc069c403ebaf9f0171e9517f40e41"
 * 
 * 4) Successful response
 * 
 * SIP/2.0 200 OK Authentication-Info: qop=auth-int,
 * rspauth="6629fae49393a05397450978507c4ef1", cnonce="0a4f113b", nc=00000001
 */
