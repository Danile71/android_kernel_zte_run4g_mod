package com.discretix.drmactions.auxillary;

/**
 * Constants that will be used in DRM actions
 */
public class Constants {
	public static final String INITIATOR_MIME_TYPE                       = "application/vnd.ms-playready.initiator+xml";
	public static final String DRM_OP_CODE                               = "drm_op_code";									
	
	public static final String DRM_OP_CODE_RIGHTS_ACQUISITION_FILE_PATH	 = "drm_op_code_rights_acquisition_file_path";
	public static final String DRM_OP_CODE_RIGHTS_ACQUISITION_FULL		 = "AcquireLicenseFull";
	public static final String DRM_OP_CODE_RIGHTS_GENERATE_LIC_CHALLENGE = "GenerateLicChallenge";
	public static final String DRM_OP_CODE_RIGHTS_ACQUISITION_INITIATOR	 = "drm_op_code_rights_acquisition_initiator";		
	public static final String DRM_OP_CODE_JOIN_DOMAIN					 = "drm_op_code_join_domain";			
	public static final String DRM_OP_CODE_LEAVE_DOMAIN				     = "drm_op_code_leave_domain";
	public static final String DRM_FILE_PATH							 = "drm_file_path";
	public static final String DRM_CUSTOM_DATA							 = "drm_custom_data";
	public static final String DRM_INITIATOR							 = "drm_initiator";
	public static final String DRM_ACTION_REMOVE_ALL_LICENSES			 = "RemoveAllLicenses";
	public static final String DRM_ACTION_IS_REMOVE_ALL_LICENSES_EXIST	 = "isRemoveAllLicenses";
			
	
	
	public static final String DRM_DOMAIN_CONTROLLER		             = "drm_domain_controller";
	public static final String DRM_SERVICE_ID				             = "drm_service_id";
	public static final String DRM_ACCOUNT_ID				             = "drm_account_id";
	public static final String DRM_EXTENDED_DATA			             = "drm_extended_data";
	
	
	
	
	public static final String MIME_TYPE_ENY			             = "video/vnd.ms-playready.media.eny";//i'm not sure it is correct but 
	public static final String MIME_TYPE_PYV			             = "video/vnd.ms-playready.media.pyv";
	public static final String MIME_TYPE_PYA			             = "audio/vnd.ms-playready.media.pya";
	public static final String MIME_TYPE_ISMV			             = "video/ismv";
	public static final String MIME_TYPE_ISMA			             = "audio/isma";
	
	public static final String[] PRD_EXT_AUDIO = {"pya", "isma"};
	public static final String[] PRD_EXT_VIDEO = {"pyv", "ismv"};
	public static final String[] PRD_EXT_ENVELOPE = {"eny", "pye"};
	public static final String[] PRD_EXT_INITIATOR = {"cms"};
	public static final String[] CLR_EXT_MEDIA = {"mp4", "wma", "wmv", "mp3"};
	
	
	public static final String DRM_DATA									 = "Data";
	public static final String LA_URL									 = "LA_URL";
	public static final String DRM_HEADER								 = "Header";
	public static final String DRM_ACTION                                = "Action";
	public static final String DRM_STATUS                                = "Status";
	public static final String DRM_ACTION_GET_DRM_HEADER				 = "GetDrmHeader";
	public static final String DRM_ACTION_PROCESS_LIC_RESPONSE			 = "ProcessLicResponse";	
	public static final String DRM_ACTION_PROCESS_LIC_ACK_RESPONSE		 = "ProcessLicAckResponse";
	public static final String DRM_ACTION_JOIN_DOMAIN_INITIATOR			 = "ProcessJoinDomInitiator";
	public static final String DRM_ACTION_LEAVE_DOMAIN_INITIATOR		 = "ProcessLeaveDomInitiator";
	public static final String DRM_MIRACAST                              = "MIRACAST";
	public static final String DRM_MIRACAST_CONNECTED                    = "Connected";
	public static final String DRM_MIRACAST_DISCONNECTED                 = "Disconnected";

}
