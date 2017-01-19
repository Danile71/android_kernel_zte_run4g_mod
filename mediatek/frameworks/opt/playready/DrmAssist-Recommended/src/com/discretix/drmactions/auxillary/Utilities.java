package com.discretix.drmactions.auxillary;


import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.Arrays;

import android.drm.DrmManagerClient;
import android.drm.DrmStore.RightsStatus;

/**
 * Auxilary functions that will be used as part of the DRM actions
 */
public class Utilities {
	/**
	 * convert return value of {@link DrmManagerClient} methods to string
	 * @param drmClientStatus return value of {@link DrmManagerClient} method
	 * @return string representing return value of {@link DrmManagerClient} method
	 */
	public static String drmClientStatus2Str(int drmClientStatus){
		switch (drmClientStatus) {
			case DrmManagerClient.ERROR_NONE:    return "ERROR_NONE"; 
			case DrmManagerClient.ERROR_UNKNOWN: return "ERROR_UNKNOWN";
			default:
				return "INVALID INT VALUE!!!";
		}
	}
	
	/**
	 * convert {@link RightsStatus} to string
	 * @param rightsStatus integer representing rights status
	 * @return corresponding rights status string
	 */
	public static String rightsStatus2Str(int rightsStatus){
		switch (rightsStatus) {
			case RightsStatus.RIGHTS_VALID:        return "RIGHTS_VALID"; 
			case RightsStatus.RIGHTS_EXPIRED:      return "RIGHTS_EXPIRED";
			case RightsStatus.RIGHTS_INVALID:      return "RIGHTS_INVALID";
			case RightsStatus.RIGHTS_NOT_ACQUIRED: return "RIGHTS_NOT_ACQUIRED";
			default:
				return "QA INVALID INT VALUE!!!";
		}
	}
	
	/**
	 * load text file in to memory
	 * @param path path to text file
	 * @return string representation of a log file
	 * @throws IOException
	 */
	public static String loadTextFile(String path) throws IOException {
		FileInputStream inputStream = new FileInputStream(new File(path));
		ByteArrayOutputStream outputStream  = new ByteArrayOutputStream();
	    
	    try{
	    	byte[] data = new byte[4096];
		    int count = inputStream.read(data);
		    while(count != -1) {
		    	outputStream.write(data, 0, count);
		        count = inputStream.read(data);
		    }
	    } finally {
	    	inputStream.close();
	    }
		

	    return outputStream .toString();
	}
	
	/**
	 * test whether current file extension belongs to one of PlayReady Video file extensions 
	 * @param extension current file extension
	 * @return true belongs, false otherwise
	 */
	public static boolean isPlayreadyVideo(String extension){
		return Arrays.asList(Constants.PRD_EXT_VIDEO).contains(extension);
	}
	
	/**
	 * test whether current file extension belongs to one of PlayReady Audio file extensions 
	 * @param extension current file extension
	 * @return true belongs, false otherwise
	 */
	public static boolean isPlayreadyAudio(String extension){
		return Arrays.asList(Constants.PRD_EXT_AUDIO).contains(extension);
	}
	
	/**
	 * test whether current file extension belongs to one of PlayReady Envelope file extensions 
	 * @param extension current file extension
	 * @return true belongs, false otherwise
	 */
	public static boolean isPlayreadyEnvelope(String extension){
		return Arrays.asList(Constants.PRD_EXT_ENVELOPE).contains(extension);
	}
	
	/**
	 * test whether current file extension belongs to one of PlayReady Initiator file extensions 
	 * @param extension current file extension
	 * @return true belongs, false otherwise
	 */
	public static boolean isPlayreadyInitiator(String extension){
		return Arrays.asList(Constants.PRD_EXT_INITIATOR).contains(extension);
	}

	/**
	 * test whether current file extension belongs to one of PlayReady DRM protected file extensions 
	 * @param extension current file extension
	 * @return true belongs, false otherwise
	 */
	public static boolean isPlayreadyDrm(String extension){
		return isPlayreadyEnvelope(extension) ||isPlayreadyVideo(extension) || isPlayreadyAudio(extension);
	}
	
	/**
	 * test whether current file extension belongs to one of supported clear media file extensions 
	 * @param extension current file extension
	 * @return true belongs, false otherwise
	 */
	public static boolean isMediaFile(String extension){
		return Arrays.asList(Constants.CLR_EXT_MEDIA).contains(extension);
	}
	
	/**
	 * get MIME type string for current file extension
	 * @param extension file extension string
	 * @return MIME type string
	 */
	public static String extension2mimeType(String extension){
		if (isPlayreadyEnvelope(extension)){
			return Constants.MIME_TYPE_ENY;
		} else if (extension.equals("pya")){
			return Constants.MIME_TYPE_PYA;
		} else if (extension.equals("pyv")){
			return Constants.MIME_TYPE_PYV;
		} else if (extension.equals("ismv")){
			return Constants.MIME_TYPE_ISMV;
		}  else if (extension.equals("isma")){
		   return Constants.MIME_TYPE_ISMA;
	    } else if (extension.equals("cms")){
			return Constants.INITIATOR_MIME_TYPE;

		}
		return null;
	}
	
}
