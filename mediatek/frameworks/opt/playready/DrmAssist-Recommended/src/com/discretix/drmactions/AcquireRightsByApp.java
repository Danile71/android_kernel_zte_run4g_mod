package com.discretix.drmactions;

import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;

import android.content.Context;
import android.drm.DrmInfo;
import android.drm.DrmInfoRequest;
import android.drm.DrmManagerClient;
import android.os.StrictMode;
import android.os.StrictMode.ThreadPolicy;
import android.util.Log;

import com.discretix.drmactions.auxillary.Constants;
import com.discretix.drmactions.auxillary.Utilities;
import com.discretix.drmassist.DrmActionSync;

/**
 * Acquire rights for a encrypted file using file headers
 */
public class AcquireRightsByApp extends DrmActionSync {
	
	/*
	 * (non-Javadoc)
	 * @see com.discretix.drmassist.DrmActionBase#getName()
	 */
	public String getName() {
		return "Acquire Rights by Application";
	}
	
	/*
	 * (non-Javadoc)
	 * @see com.discretix.drmassist.DrmActionBase#isSupported(java.lang.String, java.lang.String)
	 */
	public boolean isSupported(String extension, String filePath) {
		return Utilities.isPlayreadyDrm(extension);
	}
	
	/*
	 * (non-Javadoc)
	 * @see com.discretix.drmassist.DrmActionAsync#perform(android.content.Context, java.lang.String, java.lang.String)
	 */
	protected String perform(Context context, final String filePath, final String extension) 
	{
		
		ThreadPolicy tp = ThreadPolicy.LAX;
		StrictMode.setThreadPolicy(tp);
	
	/*	if (!filePath.endsWith("SuperSpeedway_720_230.ismv") && !filePath.endsWith("bear_video_opls0.pye") &&  !filePath.endsWith("Taxi3_AACLC_154k.isma"))
		{
			return ("File Not Supported for this operation");
		} */
		
		DrmInfoRequest request = new DrmInfoRequest(DrmInfoRequest.TYPE_RIGHTS_ACQUISITION_INFO, Utilities.extension2mimeType(extension));
		DrmManagerClient drmClient = new DrmManagerClient(context);

		//prepare generate license challenge
/*		String ismaHeaderStr = "<WRMHEADER xmlns=\"http://schemas.microsoft.com/DRM/2007/03/PlayReadyHeader\" version=\"4.0.0.0\"><DATA><PROTECTINFO><KEYLEN>16</KEYLEN><ALGID>AESCTR</ALGID></PROTECTINFO><KID>k1zwDFBoWkyzrnlb+pfzsQ==</KID><CHECKSUM>3mgweWCEKNg=</CHECKSUM><CUSTOMATTRIBUTES><IIS_DRM_VERSION>7.1.1064.0</IIS_DRM_VERSION></CUSTOMATTRIBUTES><LA_URL>http://playready.directtaps.net/sl/rightsmanager.asmx</LA_URL></DATA></WRMHEADER>";
		String ismvHeaderStr = "<WRMHEADER xmlns=\"http://schemas.microsoft.com/DRM/2007/03/PlayReadyHeader\" version=\"4.0.0.0\"><DATA><PROTECTINFO><KEYLEN>16</KEYLEN><ALGID>AESCTR</ALGID></PROTECTINFO><KID>AmfjCTOPbEOl3WD/5mcecA==</KID><CHECKSUM>BGw1aYZ1YXM=</CHECKSUM><CUSTOMATTRIBUTES><IIS_DRM_VERSION>7.1.1064.0</IIS_DRM_VERSION></CUSTOMATTRIBUTES><LA_URL>http://playready.directtaps.net/pr/svc/rightsmanager.asmx</LA_URL><DS_ID>AH+03juKbUGbHl1V/QIwRA==</DS_ID></DATA></WRMHEADER>";
		String PyeHeaderStr  = "<WRMHEADER xmlns=\"http://schemas.microsoft.com/DRM/2007/03/PlayReadyHeader\" version=\"4.0.0.0\"><DATA><PROTECTINFO><KEYLEN>16</KEYLEN><ALGID>AESCTR</ALGID></PROTECTINFO><KID>tusYN3uoeU+zLAXCJuHQ0w==</KID><LA_URL>http://playready.directtaps.net/pr/svc/rightsmanager.asmx?contentid=tusYN3uoeU+zLAXCJuHQ0w==</LA_URL><DS_ID>AH+03juKbUGbHl1V/QIwRA==</DS_ID><CHECKSUM>3hNyF98QQko=</CHECKSUM></DATA></WRMHEADER>";
			//"<WRMHEADER xmlns=\"http://schemas.microsoft.com/DRM/2007/03/PlayReadyHeader\" version=\"4.0.0.0\"><DATA><PROTECTINFO><KEYLEN>16</KEYLEN><ALGID>AESCTR</ALGID></PROTECTINFO><KID>tusYN3uoeU+zLAXCJuHQ0w==</KID><LA_URL>http://playready.directtaps.net/pr/svc/rightsmanager.asmx?</LA_URL><LUI_URL>http://playready.directtaps.net/pr/svc/rightsmanager.asmx?</LUI_URL><DS_ID>AH+03juKbUGbHl1V/QIwRA==</DS_ID><CHECKSUM>3hNyF98QQko=</CHECKSUM></DATA></WRMHEADER>";
		*/
		
		request.put(Constants.DRM_ACTION, Constants.DRM_ACTION_GET_DRM_HEADER);
		request.put(Constants.DRM_DATA, filePath);		
			
	
			
		// get header
		DrmInfo response = drmClient.acquireDrmInfo(request);
				
		if ( null == response || !response.get(Constants.DRM_STATUS).equals("ok") ) 
		{
			Log.e("AcquireRights", "DrmManagerClient::AcquireRightsByApp failed");
			return("FAILURE");			
		}
		Log.d("AcquireRights", "DrmManagerClient::AcquireRightsByApp returned header :" + response.get(Constants.DRM_DATA) );

		
		 // generate challenge
		request.put(Constants.DRM_ACTION, Constants.DRM_OP_CODE_RIGHTS_GENERATE_LIC_CHALLENGE);
		request.put(Constants.DRM_HEADER,  response.get(Constants.DRM_DATA));
		//request.put(Constants.DRM_HEADER, response.get(Constants.DRM_DATA));
/*		
		if (filePath.endsWith("SuperSpeedway_720_230.ismv"))
		{
			request.put(Constants.DRM_HEADER, ismvHeaderStr);
		}
		if (filePath.endsWith("Taxi3_AACLC_154k.isma"))
		{
			request.put(Constants.DRM_HEADER, ismaHeaderStr);
		}
		else if (filePath.endsWith("bear_video_opls0.pye"))
		{
			request.put(Constants.DRM_HEADER, PyeHeaderStr);
		}
		*/
		response = drmClient.acquireDrmInfo(request);
		
		if ( null == response || !response.get(Constants.DRM_STATUS).equals("ok") )
		{
			Log.e("AcquireRightsByApp", "DrmManagerClient::AcquireRightsByApp failed");
			return("FAILURE");			
		}
		
		Log.i("AcquireRightsByApp", "acquireDrmInfo returned "+response.get(Constants.DRM_STATUS));
		
		String challenge = response.get(Constants.DRM_DATA).toString();
		String laUrl     = response.get(Constants.LA_URL).toString();
		
		if (laUrl.isEmpty() || challenge.isEmpty())
		{
			Log.e("AcquireRightsByApp", "DrmManagerClient::AcquireRightsByApp failed, laUrl or chanllenge are invalid");
			return("FAILURE");			
		}
		
		// send to server
		Log.i("AcquireRightsByApp"," LA_URL = "+laUrl);
				
		
		try 
		{
			URL obj;
			obj = new URL(laUrl);
			HttpURLConnection con = (HttpURLConnection) obj.openConnection();
			
			//add reuqest header
			con.setRequestMethod("POST");
			con.setRequestProperty("Content-Type", "text/xml; charset=utf-8");
			con.setRequestProperty("SOAPAction", "http://schemas.microsoft.com/DRM/2007/03/protocols/AcquireLicense");

			// Send post request
			con.setDoOutput(true);
			DataOutputStream wr = new DataOutputStream(con.getOutputStream());
			wr.writeBytes(challenge);
			wr.flush();
			wr.close();

	 		int responseCode = con.getResponseCode();
			System.out.println("\nSending 'POST' request to URL : " + laUrl);
			System.out.println("Response Code : " + responseCode);

			if (responseCode != 200)
			{
				return("Failure - server error "+ responseCode);
			}
			
			//process response
			BufferedReader in = new BufferedReader(
			        new InputStreamReader(con.getInputStream()));
			String inputLine;
			StringBuffer Httpresponse = new StringBuffer();
	 
			while ((inputLine = in.readLine()) != null) {
				Httpresponse.append(inputLine);
			}
			in.close();
			con.disconnect();
			
	 		//print result
			//System.out.println(Httpresponse.toString());
			request.put(Constants.DRM_ACTION, Constants.DRM_ACTION_PROCESS_LIC_RESPONSE);
			request.put(Constants.DRM_DATA, Httpresponse.toString());		
			
			response = drmClient.acquireDrmInfo(request);
			
			if ( null == response || !response.get(Constants.DRM_STATUS).equals("ok") )
			{
				Log.e("AcquireRightsByApp", "DrmManagerClient::AcquireRightsByApp process license response failed");
				return("FAILURE");			
			}
			
			//Log.i("AcquireRightsByApp", "acquireDrmInfo returned "+response.get(Constants.DRM_STATUS));
			
			// send Ack to server
			String ack = response.get(Constants.DRM_DATA).toString();
			
			if (ack.length() == 0)
			{
				return ("SUCCESS");
			}
			
			
			Log.i("AcquireRightsByApp", "acquireDrmInfo requires to send ack");	
			HttpURLConnection con2 = (HttpURLConnection) obj.openConnection();
			
			//add reuqest header
			con2.setRequestMethod("POST");
			con2.setRequestProperty("Content-Type", "text/xml; charset=utf-8");
				// Send post request
			con2.setDoOutput(true);

			DataOutputStream wr2 = new DataOutputStream(con2.getOutputStream());
			wr2.writeBytes(ack);
			wr2.flush();
			wr2.close();

			responseCode = con2.getResponseCode();
			System.out.println("\nSending 'POST' request to URL : " + laUrl);
			System.out.println("Response Code : " + responseCode);
			
			if (responseCode != 200)
			{
				return("Failure - server error "+ responseCode);
			}

			//process response
			BufferedReader in2 = new BufferedReader(
			        new InputStreamReader(con2.getInputStream()));
			String inputLine2;
			StringBuffer Httpresponse2 = new StringBuffer();
	 
			while ((inputLine2 = in2.readLine()) != null) {
				Httpresponse2.append(inputLine2);
			}
			in2.close();
			con2.disconnect();
			
			// process ack response
			request.put(Constants.DRM_ACTION, Constants.DRM_ACTION_PROCESS_LIC_ACK_RESPONSE);
			request.put(Constants.DRM_DATA, Httpresponse2.toString());		
			
			response = drmClient.acquireDrmInfo(request);
			
			if ( null == response || !response.get(Constants.DRM_STATUS).equals("ok") )
			{
				Log.e("AcquireRightsByApp", "DrmManagerClient::AcquireRightsByApp process ack response failed");
				return("FAILURE");			
			}
			
			Log.i("AcquireRightsByApp", "acquireDrmInfo returned "+response.get(Constants.DRM_STATUS));
			
		}
		catch (MalformedURLException e1) 
		{
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}
		catch (IOException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}
		

		return("SUCCESS");

	}

}
