package com.discretix.drmactions;

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
public class Performance_challenge extends DrmActionSync {
	
	/*
	 * (non-Javadoc)
	 * @see com.discretix.drmassist.DrmActionBase#getName()
	 */
	public String getName() {
		return "Performance - challenge";
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
	
		if (!filePath.endsWith("SuperSpeedway_720_230.ismv") && !filePath.endsWith("bear_video_opls0.pye") )
		{
			return ("File Not Supported for this operation");
		}
		
		DrmInfoRequest request = new DrmInfoRequest(DrmInfoRequest.TYPE_RIGHTS_ACQUISITION_INFO, Utilities.extension2mimeType(extension));
		DrmManagerClient drmClient = new DrmManagerClient(context);

		//prepare generate license challenge
		String PiffHeaderStr = "<WRMHEADER xmlns=\"http://schemas.microsoft.com/DRM/2007/03/PlayReadyHeader\" version=\"4.0.0.0\"><DATA><PROTECTINFO><KEYLEN>16</KEYLEN><ALGID>AESCTR</ALGID></PROTECTINFO><KID>AmfjCTOPbEOl3WD/5mcecA==</KID><CHECKSUM>BGw1aYZ1YXM=</CHECKSUM><CUSTOMATTRIBUTES><IIS_DRM_VERSION>7.1.1064.0</IIS_DRM_VERSION></CUSTOMATTRIBUTES><LA_URL>http://playready.directtaps.net/pr/svc/rightsmanager.asmx</LA_URL><DS_ID>AH+03juKbUGbHl1V/QIwRA==</DS_ID></DATA></WRMHEADER>";
		String PyeHeaderStr  = "<WRMHEADER xmlns=\"http://schemas.microsoft.com/DRM/2007/03/PlayReadyHeader\" version=\"4.0.0.0\"><DATA><PROTECTINFO><KEYLEN>16</KEYLEN><ALGID>AESCTR</ALGID></PROTECTINFO><KID>tusYN3uoeU+zLAXCJuHQ0w==</KID><LA_URL>http://playready.directtaps.net/pr/svc/rightsmanager.asmx?contentid=tusYN3uoeU+zLAXCJuHQ0w==</LA_URL><DS_ID>AH+03juKbUGbHl1V/QIwRA==</DS_ID><CHECKSUM>3hNyF98QQko=</CHECKSUM></DATA></WRMHEADER>";
			//"<WRMHEADER xmlns=\"http://schemas.microsoft.com/DRM/2007/03/PlayReadyHeader\" version=\"4.0.0.0\"><DATA><PROTECTINFO><KEYLEN>16</KEYLEN><ALGID>AESCTR</ALGID></PROTECTINFO><KID>tusYN3uoeU+zLAXCJuHQ0w==</KID><LA_URL>http://playready.directtaps.net/pr/svc/rightsmanager.asmx?</LA_URL><LUI_URL>http://playready.directtaps.net/pr/svc/rightsmanager.asmx?</LUI_URL><DS_ID>AH+03juKbUGbHl1V/QIwRA==</DS_ID><CHECKSUM>3hNyF98QQko=</CHECKSUM></DATA></WRMHEADER>";
		request.put(Constants.DRM_ACTION, Constants.DRM_OP_CODE_RIGHTS_GENERATE_LIC_CHALLENGE);
		//request.put(Constants.DRM_HEADER, response.get(Constants.DRM_DATA));
		
		if (filePath.endsWith("SuperSpeedway_720_230.ismv"))
		{
			request.put(Constants.DRM_HEADER, PiffHeaderStr);
		}
		else if (filePath.endsWith("bear_video_opls0.pye"))
		{
			request.put(Constants.DRM_HEADER, PyeHeaderStr);
		}
		
		
		for (;;) // until return
		{
			DrmInfo response = drmClient.acquireDrmInfo(request);
		
			if ( null == response || !response.get(Constants.DRM_STATUS).equals("ok") )
			{		
				Log.e("Performance_Challenge", "DrmManagerClient::Performance_Challenge failed");
				return("FAILURE");			
			}
			
				Log.i("Performance_Challenge", "acquireDrmInfo returned "+response.get(Constants.DRM_STATUS));
		}

			
		

	}

}
