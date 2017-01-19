#include <string.h>
#include <jni.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <DrmManagerClient.h>
#include <DrmInfoRequest.h>
#include <DrmInfo.h>
#include <drm_framework_common.h>
#include <DxDrmPluginDefines.h>
#include <utils/String8.h>


// size for specific file (jazz.wmv)
#define PACKET_OVERHEAD  13 
#define PAYLOAD_OVERHEAD 23
#define MAX_PACKET_SIZE  20000
#define PACKET_SIZE	5982	


#define DELETE_PTR(_ptr) \
	if (NULL != (_ptr)) { \
		delete(_ptr); \
		_ptr = NULL; \
	}

namespace android {

class DrmManagerClient;


extern "C" {

void log_print_params(android_LogPriority priority, const char *sTag, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	__android_log_vprint(priority, sTag, fmt, ap);
	va_end(ap);
}


// Super_speedway frame sample
// ===========================
#define BEAR_FILE_NAME "bear_video_opls0.pye"

#define SUPER_SPEEDWAY_720_230_FILE_NAME "SuperSpeedway_720_230.ismv"
					  
#define SUPER_SPEEDWAY_720_230_FILE_HEADER		"<WRMHEADER xmlns=\"http://schemas.microsoft.com/DRM/2007/03/PlayReadyHeader\" version=\"4.0.0.0\"><DATA><PROTECTINFO><KEYLEN>16</KEYLEN><ALGID>AESCTR</ALGID></PROTECTINFO><KID>AmfjCTOPbEOl3WD/5mcecA==</KID><CHECKSUM>BGw1aYZ1YXM=</CHECKSUM><CUSTOMATTRIBUTES><IIS_DRM_VERSION>7.1.1064.0</IIS_DRM_VERSION></CUSTOMATTRIBUTES><LA_URL>http://playready.directtaps.net/pr/svc/rightsmanager.asmx</LA_URL><DS_ID>AH+03juKbUGbHl1V/QIwRA==</DS_ID></DATA></WRMHEADER>"
#define MAX_SAMPLE_DATA_LEN		T0_SAMPLE_DATA_LEN

#define SIZE_OF_DRM_HEADER		10
#define DECRYPT_UNIT_ID_VIDEO 	0
#define DECRYPT_UNIT_ID_AUDIO 	1
ssize_t DRM_BUFFER_SIZE	 =	128;

char NON_ENC_BUFFER[] = {
    0x30, 0x26, 0xb2, 0x75, 0x8e, 0x66, 0xcf, 0x11, 0xa6, 0xd9, 0x00, 0xaa, 0x00, 0x62, 0xce, 0x6c, 0x94, 0x15, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x02, 0x40, 0xa4, 0xd0, 0xd2, 0x07, 0xe3, 0xd2, 0x11, 0x97, 0xf0, 0x00, 0xa0, 0xc9, 0x5e, 
    0xa8, 0x50, 0xa4, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x24, 0x00, 0x57, 0x00, 0x4d, 0x00, 0x2f, 0x00, 0x50, 0x00, 
    0x61, 0x00, 0x72, 0x00, 0x65, 0x00, 0x6e, 0x00, 0x74, 0x00, 0x61, 0x00, 0x6c, 0x00, 0x52, 0x00, 0x61, 0x00, 0x74, 0x00, 0x69, 0x00, 
    0x6e, 0x00, 0x67, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x57, 0x00, 0x4d, 0x00, 0x46, 0x00, 0x53, 0x00, 
    0x44, 0x00, 0x4b, 0x00, 0x56, 0x00, 0x65, 0x00, 0x72, 0x00, 0x73, 0x00, 0x69, 0x00, 0x6f, 0x00, 0x6e, 0x00};

// test vector index 0
DxSubSample t0_subSampleArr[2] = {{5,300}, {10,100}};
#define T0_SAMPLE_DATA_LEN 415
char t0_encryptedData[] = {
    0x00, 0x00, 0x01, 0x2D, 0x41, 0xAB, 0x8C, 0x50, 0xDB, 0xB8, 0xCE, 0xD0, 0x3B, 0xB1, 0x2D, 0x60, 0x46, 0x8C, 0xBF, 0xE8, 0x52, 0x5C,
    0xDB, 0x78, 0x97, 0x70, 0xBF, 0x12, 0x91, 0x09, 0x3A, 0xDA, 0xC1, 0xB6, 0x40, 0x6B, 0x16, 0xBB, 0x13, 0x54, 0x40, 0x22, 0x3A, 0xB3,
    0x14, 0xD2, 0x14, 0x7E, 0xFD, 0x57, 0x93, 0x59, 0x59, 0x06, 0x8B, 0x58, 0x6A, 0x61, 0x48, 0x8A, 0x45, 0x54, 0x0E, 0x28, 0x9B, 0x5B,
    0x11, 0x40, 0x69, 0x30, 0x8F, 0x60, 0xB5, 0xA8, 0x3A, 0x43, 0x38, 0xC1, 0x48, 0x5D, 0x5D, 0x64, 0x18, 0x01, 0xEC, 0xB1, 0x1C, 0x75,
    0xFA, 0x74, 0x91, 0x0A, 0xE4, 0x17, 0xC1, 0x73, 0x1A, 0xA2, 0xB4, 0x2C, 0x49, 0xE2, 0x8E, 0xE1, 0x47, 0xF6, 0x71, 0x12, 0x2D, 0xE7,
    0x72, 0x8D, 0xCE, 0xE6, 0x58, 0x2D, 0x59, 0x7F, 0x3D, 0x25, 0x73, 0x4A, 0x1C, 0xED, 0x60, 0x3F, 0xB0, 0xF7, 0xA4, 0xCB, 0xDD, 0x3C,
    0xB6, 0x44, 0x05, 0xB8, 0x08, 0xF4, 0x36, 0x30, 0xAA, 0x24, 0x83, 0x38, 0x44, 0x5F, 0xE9, 0xFB, 0xA7, 0x7D, 0x0F, 0x53, 0xD7, 0xC6,
    0xD4, 0x63, 0x78, 0xB0, 0xBC, 0x57, 0x4C, 0x9D, 0xE2, 0x7B, 0xFA, 0xFB, 0xAC, 0xA3, 0x26, 0x0D, 0x28, 0xA1, 0x11, 0x61, 0x19, 0x6B,
    0x30, 0xD0, 0x52, 0xE3, 0x17, 0xCE, 0x77, 0x9C, 0x1A, 0xEF, 0x52, 0xD4, 0xF3, 0x22, 0x38, 0xD9, 0x19, 0x99, 0xA8, 0x98, 0x29, 0x11,
    0xFE, 0xC1, 0xD9, 0x20, 0x26, 0x3B, 0x7A, 0x87, 0x3B, 0x77, 0x1F, 0x8D, 0x9A, 0xEE, 0x46, 0x54, 0x1B, 0x3B, 0xB0, 0x7E, 0xC2, 0xD2,
    0xB8, 0xA3, 0x33, 0xEF, 0x8F, 0x3D, 0x29, 0xDA, 0x8B, 0x15, 0xEA, 0x5F, 0xAF, 0x0F, 0x0C, 0x26, 0xCD, 0x64, 0xD5, 0xE9, 0x69, 0x8C,
    0xCE, 0xB3, 0xB7, 0x16, 0xF7, 0x6B, 0x8D, 0xAC, 0xB3, 0x2A, 0x3C, 0x25, 0x8C, 0x02, 0xD9, 0xD0, 0xDF, 0xCE, 0xDD, 0xE8, 0xB5, 0x1D,
    0x08, 0xC1, 0xB6, 0xE0, 0xFC, 0x65, 0x4D, 0x40, 0x7D, 0xD6, 0xC2, 0xA9, 0x78, 0xD9, 0x3A, 0x2B, 0xCD, 0xB9, 0x97, 0x33, 0x09, 0xB3,
    0x00, 0xBD, 0x2D, 0xA2, 0x8A, 0x92, 0x46, 0x68, 0xD4, 0x72, 0x8D, 0x7A, 0x37, 0xDF, 0xD4, 0x30, 0x7A, 0xCA, 0x47, 0xF0, 0x12, 0x34,
    0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x0F, 0xAF, 0xEF, 0xE0, 0x74, 0xA6, 0x31, 0xB2, 0x2C, 0x27, 0xAD, 0x5E, 0xD7, 0x16, 0xCC, 0xC2,
    0x14, 0x99, 0x7E, 0x3D, 0x3D, 0x70, 0xAE, 0xA5, 0xDA, 0x4E, 0x53, 0x43, 0xDE, 0xA2, 0x98, 0x13, 0x9F, 0x31, 0xCF, 0x1B, 0x4A, 0xC2,
    0x9C, 0x46, 0x80, 0x24, 0xA2, 0xAB, 0xC1, 0x09, 0xA4, 0xEE, 0x18, 0x51, 0x5A, 0x1F, 0x99, 0x0C, 0x61, 0xD7, 0x00, 0x49, 0x05, 0x25,
    0x52, 0xED, 0xD3, 0x8A, 0x8F, 0x33, 0x7C, 0xED, 0xC2, 0x45, 0x41, 0x15, 0x1F, 0x2C, 0xD5, 0x4E, 0x96, 0x76, 0x94, 0x86, 0x44, 0x2B,
    0xC6, 0xB4, 0xE6, 0xBC, 0x87, 0xAB, 0xBD, 0xC3, 0x1C, 0x9B, 0x89, 0xFA, 0x05, 0xCF, 0x0C, 0xD7, 0x06, 0x08, 0x1E};

char t0_clearData[] = { 
    0x00, 0x00, 0x01, 0x2D, 0x41, 0x9E, 0x34, 0x65, 0xB2, 0x09, 0x22, 0xC5, 0xFF, 0xFB, 0x7E, 0xA2, 0xC4, 0x31, 0x5E, 0x04, 0x0D, 0x98,
    0x6F, 0x81, 0xE2, 0x1E, 0x33, 0x98, 0xB0, 0xEE, 0x99, 0x10, 0xE9, 0xFB, 0x1C, 0xE8, 0x66, 0x7E, 0x92, 0x90, 0xC5, 0x33, 0xA8, 0x05,
    0xFA, 0x0F, 0x70, 0xF1, 0x33, 0xDB, 0xC4, 0x8A, 0x69, 0x2B, 0x84, 0xC9, 0x73, 0x87, 0x0D, 0x2F, 0xE3, 0x33, 0xCE, 0x43, 0x0F, 0x91,
    0x1E, 0x6C, 0x95, 0xB7, 0x97, 0xDC, 0xD0, 0x76, 0x12, 0xD9, 0xD1, 0x57, 0x2F, 0x08, 0xD8, 0x33, 0xA3, 0xCC, 0x4F, 0x11, 0x97, 0x2C,
    0x99, 0xA1, 0xF6, 0x60, 0xD3, 0x01, 0xAC, 0x82, 0xF5, 0xB4, 0x37, 0x47, 0xFB, 0xA9, 0x65, 0x1C, 0x3A, 0xB5, 0x91, 0x54, 0x06, 0xA6,
    0x47, 0x50, 0x09, 0x5B, 0x09, 0x06, 0xE7, 0x41, 0xD7, 0x9E, 0xC8, 0xDB, 0xD8, 0x5A, 0x34, 0x50, 0x0B, 0x7A, 0xB4, 0x9F, 0x73, 0x51,
    0x45, 0x51, 0x30, 0xD7, 0xD8, 0xA1, 0xCD, 0xF8, 0xA7, 0xC9, 0x27, 0x1D, 0x34, 0x33, 0xEB, 0xE2, 0x47, 0x5A, 0x44, 0x26, 0xEB, 0xA7,
    0x49, 0x15, 0x7E, 0x51, 0xD6, 0xE5, 0x3C, 0x1B, 0x49, 0xC4, 0x9F, 0x86, 0x46, 0x63, 0x45, 0x33, 0x54, 0x1D, 0xF2, 0x8A, 0xC5, 0xE9,
    0x79, 0xB0, 0xDA, 0x7D, 0x11, 0x67, 0xFD, 0x2F, 0xD6, 0x19, 0x34, 0x39, 0x35, 0x21, 0x12, 0x79, 0xA6, 0xE2, 0xD1, 0xDA, 0x19, 0xDB,
    0x35, 0xC8, 0xEF, 0xE0, 0xFC, 0x39, 0xC4, 0x91, 0x73, 0xA3, 0xF5, 0x1F, 0xAB, 0x48, 0x1E, 0x9F, 0xAD, 0x5E, 0x6F, 0x13, 0x95, 0xF8,
    0xEA, 0x3C, 0xE2, 0xDE, 0x75, 0x07, 0x7C, 0x8D, 0x8F, 0x34, 0x71, 0x29, 0x4B, 0x9D, 0x23, 0xA4, 0xE4, 0xAD, 0x88, 0xD6, 0x39, 0xA7,
    0xC3, 0xCD, 0x7F, 0x43, 0x49, 0x8A, 0x81, 0x4A, 0x4B, 0xA1, 0xF9, 0x7C, 0xB7, 0x1A, 0x0F, 0x30, 0x35, 0x0B, 0xC2, 0x2F, 0xA5, 0x36,
    0x4C, 0xDD, 0x0C, 0x4D, 0x02, 0x5B, 0xE8, 0x9F, 0x01, 0xD0, 0x48, 0x65, 0xF1, 0xC8, 0xE9, 0xEB, 0x56, 0x8A, 0x41, 0xF8, 0xA0, 0x25,
    0x41, 0x98, 0x8C, 0x60, 0x98, 0xC0, 0x8C, 0x84, 0x29, 0x14, 0x75, 0xFF, 0xEB, 0xA6, 0xFD, 0xF2, 0x9C, 0xDA, 0x34, 0xF0, 0x12, 0x34,
    0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x0F, 0x18, 0x24, 0xB2, 0x29, 0xB1, 0x65, 0xB6, 0xA1, 0xE7, 0x19, 0xC0, 0x28, 0x94, 0xDD, 0x74,
    0x55, 0xB6, 0xEE, 0x34, 0x5C, 0x0A, 0x8C, 0xF7, 0x8C, 0xF9, 0x0C, 0xBB, 0xF4, 0xA1, 0x03, 0xF3, 0x68, 0x9E, 0xEA, 0xF0, 0x2C, 0x1C,
    0x6A, 0x75, 0x0D, 0x22, 0x1E, 0xC4, 0xA0, 0xFA, 0x03, 0x02, 0xAD, 0xDD, 0xF2, 0xA6, 0xBF, 0x10, 0xFE, 0x69, 0x5C, 0x28, 0xBD, 0xB8,
    0xAC, 0x6D, 0x3E, 0x9B, 0x35, 0x0B, 0x5F, 0x6F, 0x98, 0x21, 0x67, 0x27, 0xC6, 0xBE, 0xCD, 0xF7, 0x02, 0xC5, 0xA2, 0x48, 0xF1, 0x62,
    0x8D, 0x23, 0x84, 0x35, 0xC0, 0x17, 0xF1, 0x49, 0x0D, 0x77, 0x92, 0x75, 0x77, 0xF0, 0x70, 0xE9, 0xA4, 0x99, 0x7B};


typedef struct 
{
	DxMultiSampleHeader	sampleHeader	;
	DxSubSample *subSampleArr		;
	char *encData				;
	char *clearData				;
	uint32_t dataLength			;
} testData;

testData testDataArr[] = 
{
	// index 0
	{
		// DxMultiSampleHeader sampleHeader;
		{
			0xF4B31077753F6A34,	// uint64_t	 qwInitializationVector:	The IV from the PIFF file, used for the decryption of all NAL units in this call
			0,					// uint32_t	 dwOutBufferOffset: 		Offset into the output buffer (FD)
			0,					// uint32_t	 dwMediaOffset: 			Will be always zero. Will be non zero in case NAL units in the same frame will be passed in different calls.
			2,					// uint32_t	 dwSubSamplesNum:			Number of structures of type DxSubSample present
		},
		t0_subSampleArr, 	// DxSubSample *subSampleArr;
		t0_encryptedData,	// char *encData;
		t0_clearData,		// char *clearData;
		T0_SAMPLE_DATA_LEN,	// uint32_t dataLength;
	}
	// index n
};

bool ConsumeFile (DrmManagerClient *mDrmManagerClient, String8 filename, char *strRes);
bool ConsumeStream (DrmManagerClient *mDrmManagerClient, char* strRes);
bool GetOplMiracast_File_inner (DrmManagerClient *mDrmManagerClient, String8 filename, char* strRes);


jstring
Java_com_discretix_drmactions_DecryptSession_Decrypt( JNIEnv* env, jobject thiz, jint jTestNum, jstring filename)
{

	DrmManagerClient *mDrmManagerClient;
	bool result = false;
	bool result2 = false;
	int  testnum = (int)jTestNum;
	String8 filenameStr((char*)env->GetStringUTFChars(filename, NULL));
	char strRes[1000] = {0};
   
	__android_log_write (ANDROID_LOG_INFO,"DrmAssistJNI","allocate DrmManagerClient");
	mDrmManagerClient = new DrmManagerClient();

	if (mDrmManagerClient == NULL) {
		__android_log_write (ANDROID_LOG_INFO,"DrmAssistJNI","failed to allocate DrmManagerClient, exit...");
		goto end;
	}

	switch (testnum)
	{
		case 1:
			result = ConsumeFile (mDrmManagerClient, filenameStr, strRes);
			break;
		case 2:
			result = ConsumeStream (mDrmManagerClient, strRes);
			break;
		case 3:
			result = ConsumeFile (mDrmManagerClient, filenameStr, strRes);
			if (filenameStr.find (SUPER_SPEEDWAY_720_230_FILE_NAME) > 0)
			{
				result2 = ConsumeStream (mDrmManagerClient, strRes);	
				result = result && result2;			
			}
			break;
		case 4:
			__android_log_write (ANDROID_LOG_INFO,"DrmAssistJNI","call GetOplMiracast_File_inner");
			result = GetOplMiracast_File_inner (mDrmManagerClient, filenameStr, strRes);
			break;
	}
	end:
	if (mDrmManagerClient != NULL) delete mDrmManagerClient;

	if (result == true)
	{
		strcat (strRes, "\ntest completed successfully !");
		return env->NewStringUTF(strRes);
	}
	else
	{
		strcat (strRes, "\ntest failed..");
		return env->NewStringUTF(strRes);
	}
}

/*
jstring
Java_com_discretix_drmactions_GetMiracastOpl_GetOplMiracastFromFile( JNIEnv* env, jobject thiz, jint jTestNum, jstring filename)
{

	DrmManagerClient *mDrmManagerClient;
	bool result = false;
	bool result2 = false;
	int  testnum = (int)jTestNum;
	String8 filenameStr((char*)env->GetStringUTFChars(filename, NULL));
	char strRes[1000] = {0};

	__android_log_write (ANDROID_LOG_INFO,"DrmAssistJNI","allocate DrmManagerClient");
	mDrmManagerClient = new DrmManagerClient();

	if (mDrmManagerClient == NULL) {
		__android_log_write (ANDROID_LOG_INFO,"DrmAssistJNI","failed to allocate DrmManagerClient, exit...");
		goto end;
	}

	result = GetOplMiracastFromFile_inner(mDrmManagerClient, filenameStr, strRes);

end:

if (mDrmManagerClient != NULL) delete mDrmManagerClient;

	if (result == true)
	{
		strcat (strRes, "\ntest completed successfully !");
		return env->NewStringUTF(strRes);
	}
	else
	{
		strcat (strRes, "\ntest failed..");
		return env->NewStringUTF(strRes);
	}
}

*/

void addErrorString(status_t status, char * str)
{
	switch (status)
	{
		case DRM_ERROR_UNKNOWN: 						
			strcat (str, "DRM_ERROR_UNKNOWN");
			break;
		case DRM_ERROR_NO_LICENSE:
			strcat (str, "DRM_ERROR_NO_LICENSE");
			break;
		case DRM_ERROR_LICENSE_EXPIRED:
			strcat (str, "DRM_ERROR_LICENSE_EXPIRED");
			break;
		case DRM_ERROR_SESSION_NOT_OPENED:
			strcat (str, "DRM_ERROR_SESSION_NOT_OPENED");
			break;
		case DRM_ERROR_DECRYPT_UNIT_NOT_INITIALIZED:
			strcat (str, "DRM_ERROR_DECRYPT_UNIT_NOT_INITIALIZED");
			break;
		case DRM_ERROR_DECRYPT:
			strcat (str, "DRM_ERROR_DECRYPT");
			break;
		case DRM_ERROR_CANNOT_HANDLE: 	 
			strcat (str, "DRM_ERROR_CANNOT_HANDLE");
			break;
		case DRM_ERROR_TAMPER_DETECTED: 			 
			strcat (str, "DRM_ERROR_TAMPER_DETECTED");
			break;
		case DRM_ERROR_NO_PERMISSION: 				 
			strcat (str, "DRM_ERROR_NO_PERMISSION");
			break;
		case DRM_NO_ERROR: 			 
			strcat (str, "DRM_NO_ERROR");
			break;
		default:
			sprintf(str, "%d", status);
			break;
	}
}

void printSubSampleArr(const char *str, uint32_t dwSubSamplesNum, DxSubSample *subSampleArr)
{
	for (uint i = 0 ; i < dwSubSamplesNum ; i ++)
	{
		log_print_params (ANDROID_LOG_INFO,"DrmAssistJNI", "%s[%u]: dwClearDataSize: %u, dwEncryptedDataSize: %u",
				str, i, subSampleArr[i].dwClearDataSize, subSampleArr[i].dwEncryptedDataSize);
	}
}

/*!< DxMultiSampleHeader *header:       a pointer to DxMultiSampleHeader struct containing the data to fill in the IV header */
/*!< DxSubSample 		 *subSampleArr:	a pointer to the DxSubSample array */
DrmBuffer *IVFactory(DxMultiSampleHeader *header, DxSubSample *subSampleArr)
{
	uint32_t IVBufferLen = 0;
	char *IVBufferData = NULL;
	DxMultiSampleHeader *f_pheader = NULL;
	DxSubSample *f_pSubSampleArr = NULL;
	DxSubSample *pSubSample = NULL;
	DrmBuffer *buf = NULL;

	if (NULL == header)
	{
		__android_log_write (ANDROID_LOG_ERROR,"DrmAssistJNI","IVFactory - header is NULL, return NULL.");
		return NULL;
	}

	if (NULL == subSampleArr)
	{
		__android_log_write (ANDROID_LOG_ERROR,"DrmAssistJNI","IVFactory - subSampleArr is NULL, return NULL.");
		return NULL;
	}
	
	// Printing received data
    log_print_params (ANDROID_LOG_INFO,"DrmAssistJNI",
        "IVFactory - dwOutBufferOffset: %u, qwInitializationVector: 0x%X,%X, dwMediaOffset: %u, dwSubSamplesNum: %u, subSampleArr: 0x%08X, calculated IVBufferLen: %u",
        header->dwOutBufferOffset, header->qwInitializationVector, header->dwMediaOffset, header->dwSubSamplesNum, subSampleArr, IVBufferLen);
    printSubSampleArr("IVFactory - SubSample", header->dwSubSamplesNum, subSampleArr);

    // IVBufferData construction
	IVBufferLen = DX_GET_IV_BUFFER_ACTUAL_LEN(header->dwSubSamplesNum);
    log_print_params (ANDROID_LOG_INFO,"DrmAssistJNI", "calculated IVBufferLen: %u", IVBufferLen);
	
	IVBufferData = new char[IVBufferLen]();
    if (NULL == IVBufferData)
    {
    	log_print_params (ANDROID_LOG_ERROR,"DrmAssistJNI", "IVFactory - Failed allocation of %u bytes for IVBufferData", IVBufferLen);
    	return NULL;
    }
    
	// Copying the header data
	f_pheader = (DxMultiSampleHeader *)IVBufferData;
	memcpy(f_pheader, header, sizeof(DxMultiSampleHeader));

	// copying the subSampleArr
	f_pSubSampleArr = DX_GET_SAMPLE_ARR_PTR(IVBufferData);
	memcpy(f_pSubSampleArr, subSampleArr, header->dwSubSamplesNum * sizeof(DxSubSample));

    buf = new DrmBuffer(IVBufferData, IVBufferLen);
    if (NULL == buf)
    {
    	log_print_params (ANDROID_LOG_ERROR,"DrmAssistJNI", "IVFactory - Failed allocation of %u bytes for DrmBuffer", sizeof(DrmBuffer));
    	DELETE_PTR(IVBufferData);
    	return NULL;
    }

	return buf;
}

#define DELETE_IV_Buf(_ivBuf) \
	if (NULL != (_ivBuf)) \
	{ \
		DELETE_PTR((_ivBuf)->data); \
		DELETE_PTR(_ivBuf); \
	} 
bool ConsumeFile (DrmManagerClient *mDrmManagerClient,String8 filename, char* strRes)
{
	int fd = 0;
	sp<DecryptHandle> mDecryptHandle(NULL);
	sp<DecryptHandle> mDecryptHandle2(NULL);
	bool result = false;
	status_t status = DRM_NO_ERROR;
	DrmBuffer *drmHeader = NULL;
	DrmBuffer *encBuf = NULL;
	DrmBuffer *decBuf = NULL;
	char decBufferData[MAX_SAMPLE_DATA_LEN] = {0};
	bool bTest1 = true;
	DrmBuffer *IVBuf = NULL;
	uint32_t idx = 0;
	int bytesRead = 0;

	
	if (filename.find(BEAR_FILE_NAME) >= 0)
	{
		strcat (strRes, "\ntestings local file with <fd> and pRead:");
		// test case 1 : PYV file, use openSession with fd
		fd = open (filename, O_RDONLY);
	
		if (fd == -1)	
		{
			__android_log_write (ANDROID_LOG_ERROR,"DrmAssistJNI","ConsumeFile- failed to open content file, exit...");
			 bTest1 = false;
			goto end;
		}
	
		__android_log_write (ANDROID_LOG_INFO,"DrmAssistJNI","ConsumeFile - calling openDecryptSession() with fd");
		mDecryptHandle = mDrmManagerClient->openDecryptSession (fd, 0, 0, NULL);
		
		if (mDecryptHandle == NULL)
		{
			__android_log_write (ANDROID_LOG_ERROR,"DrmAssistJNI","ConsumeFile - failed in openSession, exit...");
			strcat (strRes, "\nopenDecryptSession(fd) failed..");
			bTest1 = false;
			goto TestCase2;

		}	
	
		strcat (strRes, "\nopenDecryptSession(fd) success!");

		status = mDrmManagerClient->consumeRights(mDecryptHandle, Action::PLAY, false);
		if ( status != DRM_NO_ERROR)
		{
			__android_log_write (ANDROID_LOG_ERROR,"DrmAssistJNI","ConsumeFile - failed in consumeRights, exit...");
			strcat (strRes, "\nconsumeRights failed with error: ");
			addErrorString(status, strRes);
			bTest1 = false;
			goto TestCase2;
		}

		strcat (strRes, "\nconsumeRights success!");
		// read 128 bytes and compare them to the non-encrypted file
		bytesRead = mDrmManagerClient->pread (mDecryptHandle, decBufferData, DRM_BUFFER_SIZE, 0 );
 	
	       if (bytesRead == -1)
	       {
        	  __android_log_write (ANDROID_LOG_ERROR,"DrmAssistJNI","ConsumeFile - failed to read file");
		   strcat (strRes, "\npread failed..");
	           bTest1 = false;
        	   goto TestCase2;
	       }

		strcat (strRes, "\npread success!");
		log_print_params (ANDROID_LOG_INFO,"DrmAssistJNI", "ConsumeFile - performing memory comparison on pRead content size=%d");

		if (memcmp (NON_ENC_BUFFER , decBufferData, bytesRead) != 0)
		{
			log_print_params (ANDROID_LOG_ERROR,"DrmAssistJNI","ConsumeFile - failed memory comparison on (pRead) decrypted content");
			strcat (strRes, "\nmemcmp failed..");
			status = DRM_ERROR_UNKNOWN;
			 bTest1 = false;
			goto TestCase2;
		}
		log_print_params (ANDROID_LOG_INFO,"DrmAssistJNI", "ConsumeFile - memory comparison on (pRead) decrypted content is OK !! :)"); 
		strcat (strRes, "\nmemcmp ok!");
	
		log_print_params (ANDROID_LOG_INFO,"DrmAssistJNI","ConsumeFile - calling closeDecryptSession()");
		status = mDrmManagerClient->closeDecryptSession (mDecryptHandle);
		if (status != DRM_NO_ERROR)
		{
			log_print_params (ANDROID_LOG_ERROR,"DrmAssistJNI",
				"ConsumeFile - failed to close Decrypt session, exit...");
			strcat (strRes, "\ncloseDecryptSession failed with error: ");
			addErrorString(status, strRes);
			 bTest1 = false;
			goto end;
		}
		strcat (strRes, "\ncloseDecryptSession ok!");
	}
TestCase2:
	if (filename.find(SUPER_SPEEDWAY_720_230_FILE_NAME) >= 0)
	{
		strcat (strRes, "\ntestings local file with <uri> and decrypt:");
		// test case 2: ISMV file, use openSession with uri
		__android_log_write (ANDROID_LOG_INFO,"DrmAssistJNI","ConsumeFile - calling openDecryptSession() with uri");
		mDecryptHandle2 = mDrmManagerClient->openDecryptSession (filename, NULL);
	
		if (mDecryptHandle2 == NULL)
		{
			__android_log_write (ANDROID_LOG_ERROR,"DrmAssistJNI","ConsumeFile - failed to openDecryptSession uri, exit...");
			strcat (strRes, "\nopenDecryptSession(uri) failed..");
			status = DRM_ERROR_UNKNOWN;
			goto end;
		}
		strcat (strRes, "\nopenDecryptSession(uri) success!");
	
		status = mDrmManagerClient->consumeRights(mDecryptHandle2,  Action::PLAY, false);
		if ( status != DRM_NO_ERROR)
		{
			__android_log_write (ANDROID_LOG_ERROR,"DrmAssistJNI","ConsumeFile - failed in consumeRights, exit...");
			strcat (strRes, "\nconsumeRights failed with error: ");
			addErrorString(status, strRes);
			status = DRM_ERROR_UNKNOWN;
			goto end;
		}
	
		// Decrypt the first sample (testDataArr[0])
		idx = 0;
		strcat (strRes, "\nconsumeRights success!");
	
		// Preparing the encBuf and decBuf
		encBuf = new DrmBuffer(testDataArr[idx].encData, testDataArr[idx].dataLength);
		if (NULL == encBuf)
		{
			log_print_params (ANDROID_LOG_ERROR, "DrmAssistJNI","ConsumeFile[%u] - failed to allocate encBuf (size: %u), exit...", idx, sizeof(DrmBuffer));
			status = DRM_ERROR_UNKNOWN;
			goto end;
		}
		
		decBuf = new DrmBuffer(decBufferData, testDataArr[idx].dataLength);
		if (NULL == encBuf)
		{
			log_print_params (ANDROID_LOG_ERROR, "DrmAssistJNI","ConsumeFile[%u] - failed to allocate encBuf (size: %u), exit...", idx, sizeof(DrmBuffer));
			status = DRM_ERROR_UNKNOWN;
			goto end;
		}
	
		IVBuf = IVFactory(&testDataArr[idx].sampleHeader, testDataArr[idx].subSampleArr);
		if (NULL == IVBuf)
		{
			log_print_params (ANDROID_LOG_ERROR, "DrmAssistJNI","ConsumeFile[%u] - IVFactory returned NULL IVBuf, exit...", idx);
			status = DRM_ERROR_UNKNOWN;
			goto end;
		}
	
		log_print_params (ANDROID_LOG_INFO,"DrmAssistJNI", "ConsumeFile[%u] - calling decrypt", idx);
		status = mDrmManagerClient->decrypt (mDecryptHandle2, DECRYPT_UNIT_ID_AUDIO, encBuf, &decBuf, IVBuf);
		if (status != DRM_NO_ERROR)
		{
			log_print_params (ANDROID_LOG_ERROR,"DrmAssistJNI","ConsumeFile[%u] - failed to decrypt buffer,  exit...", idx);
			strcat (strRes, "\ndecrypt failed with error: ");
			addErrorString(status, strRes);
			status = DRM_ERROR_UNKNOWN;
			goto end;
		}
		strcat (strRes, "\ndecrypt success!");
	
		// compare decrypted buffer payload and clear buffer
		log_print_params (ANDROID_LOG_INFO,"DrmAssistJNI", "ConsumeFile[%u] - performing memory comparison on decrypted content", idx);
		if (memcmp (decBuf->data, testDataArr[idx].clearData, testDataArr[idx].dataLength) != 0)
		{
			log_print_params (ANDROID_LOG_ERROR,"DrmAssistJNI","ConsumeFile[%u] - failed memory comparison on decrypted content", idx);
			strcat (strRes, "\nmemcmp failed..");
			status = DRM_ERROR_UNKNOWN;
			goto end;
		}
		strcat (strRes, "\nmemcmp success!");
		log_print_params (ANDROID_LOG_INFO,"DrmAssistJNI", "ConsumeFile[%u] - memory comparison on decrypted content is OK !! :)", idx);
		
		log_print_params (ANDROID_LOG_INFO,"DrmAssistJNI","ConsumeFile[%u] - calling closeDecryptSession()", idx);
		status = mDrmManagerClient->closeDecryptSession (mDecryptHandle2);
		if (status != DRM_NO_ERROR)
		{
			log_print_params (ANDROID_LOG_ERROR,"DrmAssistJNI","ConsumeFile[%u] - failed to close Decrypt session, exit...", idx);
			strcat (strRes, "\ncloseDecryptSession failed with error: ");
			addErrorString(status, strRes);
			status = DRM_ERROR_UNKNOWN;
			goto end;
		}
		strcat (strRes, "\ncloseDecryptSession success!");
	
	}
end:	
	DELETE_PTR(encBuf);
	DELETE_PTR(decBuf);
	DELETE_IV_Buf(IVBuf);

	return (status == DRM_NO_ERROR && bTest1 );
}

bool GetOplMiracast_File_inner (DrmManagerClient *mDrmManagerClient, String8 filename, char* strRes)
{
	bool bTest1 = true;
	int fd = 0;
	sp<DecryptHandle> mDecryptHandle(NULL);
	bool result = false;
	status_t status = DRM_NO_ERROR;
	DrmInfo *drmInfo = NULL;
	String8 value;

	__android_log_write (ANDROID_LOG_INFO,"DrmAssistJNI","GetOplMiracast_File_inner() start");
	DrmInfoRequest * drmInfoRequest = new DrmInfoRequest(DrmInfoRequest::TYPE_RIGHTS_ACQUISITION_INFO, String8("video/ismv"));

	mDecryptHandle = mDrmManagerClient->openDecryptSession (filename, NULL);

	if (mDecryptHandle == NULL)
	{
		__android_log_write (ANDROID_LOG_ERROR,"DrmAssistJNI","ConsumeFile - failed to openDecryptSession uri, exit...");
		strcat (strRes, "\nopenDecryptSession(uri) failed..");
		status = DRM_ERROR_UNKNOWN;
		goto end;
	}

	/*strcat (strRes, "\nGetOplMiracast_File - testings local file with <fd>:");
	// 1. Open Decrypt Session:
	fd = open (filename, O_RDONLY);

	if (fd == -1)
	{
		__android_log_write (ANDROID_LOG_ERROR,"DrmAssistJNI","GetOplMiracast_File- failed to open content file, exit...");
		 bTest1 = false;
		goto end;
	}

	__android_log_write (ANDROID_LOG_INFO,"DrmAssistJNI","GetOplMiracast_File - calling openDecryptSession() with fd");
	mDecryptHandle = mDrmManagerClient->openDecryptSession (fd, 0, 0, NULL);

	if (mDecryptHandle == NULL)
	{
		__android_log_write (ANDROID_LOG_ERROR,"DrmAssistJNI","GetOplMiracast_File - failed in openSession, exit...");
		strcat (strRes, "\nopenDecryptSession(uri) failed..");
		bTest1 = false;
		goto end;
	}
	*/

	strcat (strRes, "\nopenDecryptSession(uri) success!");

	// 2. Consume Rights:
	status = mDrmManagerClient->consumeRights(mDecryptHandle, Action::PLAY, false);
	if ( status != DRM_NO_ERROR)
	{
		__android_log_write (ANDROID_LOG_ERROR,"DrmAssistJNI","GetOplMiracast_File - failed in consumeRights, exit...");
		strcat (strRes, "\nconsumeRights failed..");
		bTest1 = false;
		goto end;
	}

	strcat (strRes, "\nconsumeRights success!");

	//3. Acquire Drm Info
	drmInfoRequest->put(String8("Action"), String8("Get_OPL_Miracast_Values"));
	drmInfo = mDrmManagerClient->acquireDrmInfo(drmInfoRequest);
	if (NULL == drmInfo || drmInfo->get(String8("Status")) != String8("ok"))
	{
		__android_log_write (ANDROID_LOG_ERROR,"DrmAssistJNI","GetOplMiracast_File - failed in acquireDrmInfo, exit...");
		strcat (strRes, "\nacquireDrmInfo failed..");
		bTest1 = false;
		goto end;
	}

	strcat (strRes, "\nacquireDrmInfo ok! ");
	value = drmInfo->get(String8("OPL_AUDIO_COMPRESSED_LEVEL"));
	strcat (strRes, "\naudioCompressedLevel:");
	strcat (strRes, value.string());
	value = drmInfo->get(String8("OPL_AUDIO_UNCOMPRESSED_LEVEL"));
	strcat (strRes, "\naudioUncompressedLevel:");
	strcat (strRes, value.string());
	value = drmInfo->get(String8("OPL_VIDEO_COMPRESSED_LEVEL"));
	strcat (strRes, "\nvideoCompressedLevel:");
	strcat (strRes, value.string());
	value = drmInfo->get(String8("OPL_VIDEO_UNCOMPRESSED_LEVEL"));
	strcat (strRes, "\nvideoUncompressedLevel:");
	strcat (strRes, value.string());
	value = drmInfo->get(String8("OPL_VIDEO_ANALOG_LEVEL"));
	strcat (strRes, "\nvideoAnalogLevel:");
	strcat (strRes, value.string());
	value = drmInfo->get(String8("DRM_MIRACAST_ENABLER"));
	strcat (strRes, "\nmiracastEnabler:");
	strcat (strRes, value.string());

	// 4. Close Decrypt Session:
	log_print_params (ANDROID_LOG_INFO,"DrmAssistJNI","GetOplMiracast_File - calling closeDecryptSession()");
	status = mDrmManagerClient->closeDecryptSession (mDecryptHandle);
	if (status != DRM_NO_ERROR)
	{
		log_print_params (ANDROID_LOG_ERROR,"DrmAssistJNI",
			"GetOplMiracast_File - failed to close Decrypt session, exit...");
		strcat (strRes, "\ncloseDecryptSession failed...");
		bTest1 = false;
		goto end;
	}
	strcat (strRes, "\ncloseDecryptSession ok!");

end:

	if(drmInfoRequest != NULL)
		delete drmInfoRequest;
	return (status == DRM_NO_ERROR && bTest1 );
}



bool ConsumeStream (DrmManagerClient *mDrmManagerClient, char* strRes)
{
	sp<DecryptHandle> mDecryptHandle(NULL);
	bool result = false;
	status_t status = DRM_NO_ERROR;

	typedef struct
	{
		int length			__attribute__((packed));
		short recordCount	__attribute__((packed));
		short recordType	__attribute__((packed));
		short recordLength	__attribute__((packed));
	} drmHeader_t;

	struct
	{
		drmHeader_t _header ;
		char headerBuf [500];
	} headerStruct;

	DrmBuffer *drmHeader = NULL;
	DrmBuffer *encBuf = NULL;
	DrmBuffer *decBuf = NULL;
	char decBufferData[MAX_SAMPLE_DATA_LEN] = {0};
	DrmBuffer *IVBuf = NULL;
	uint32_t idx = 0;

	/* open Decrypt Session */
	strcat (strRes, "\ntest Streaming content(uri):");
	__android_log_write (ANDROID_LOG_INFO,"DrmAssistJNI","ConsumeStream - calling openDecryptSession() with uri");
	mDecryptHandle = mDrmManagerClient->openDecryptSession ("http-stream-download", "video/pyv");
	if (mDecryptHandle == NULL)
	{
		__android_log_write (ANDROID_LOG_ERROR,"DrmAssistJNI","ConsumeStream - failed to openDecryptSession, exit...");
		strcat (strRes, "\nopenDecryptSession(uri) failed..");
		goto end;
	}
	strcat (strRes, "\nopenDecryptSession(uri) success!");

	// Sanity: make sure there is no packing issue with the drmHeader_t
	if (sizeof(drmHeader_t) != SIZE_OF_DRM_HEADER)
	{
		__android_log_write (ANDROID_LOG_ERROR,"DrmAssistJNI","ConsumeStream - Packing error for drmHeader_t struct. exit ...");
		goto end;
	}

	headerStruct._header.length = strlen(SUPER_SPEEDWAY_720_230_FILE_HEADER) + sizeof(drmHeader_t);
	headerStruct._header.recordCount = 1;
	headerStruct._header.recordType  = 1;
	headerStruct._header.recordLength = strlen(SUPER_SPEEDWAY_720_230_FILE_HEADER);
	strcpy(headerStruct.headerBuf, SUPER_SPEEDWAY_720_230_FILE_HEADER );

	drmHeader = new DrmBuffer ((char*)&headerStruct, headerStruct._header.length);
	if (drmHeader == NULL)
	{
		log_print_params (ANDROID_LOG_ERROR, "DrmAssistJNI","ConsumeStream - failed to allocate header buffer (size: %u), exit...", headerStruct._header.length);
		goto end;
	}

	// initialize decrypt unit, unitId is the track id
	__android_log_write (ANDROID_LOG_INFO,"DrmAssistJNI","ConsumeStream - calling initializeDecryptUnit()");
	status = mDrmManagerClient->initializeDecryptUnit (mDecryptHandle , DECRYPT_UNIT_ID_VIDEO, drmHeader);
	if (status != DRM_NO_ERROR)
	{
		__android_log_write (ANDROID_LOG_ERROR,"DrmAssistJNI","ConsumeStream - failed to initiazlize Decrypt unit, exit...");
		strcat (strRes, "\ninitializeDecryptUnit failed with error: ");
		addErrorString(status, strRes);
		goto end;
	}
	strcat (strRes, "\ninitializeDecryptUnit success!");

	// consume rights
	__android_log_write (ANDROID_LOG_INFO,"DrmAssistJNI","ConsumeStream - calling consumeRights()");
	status = mDrmManagerClient->consumeRights(mDecryptHandle,  Action::PLAY, false);
	if ( status != DRM_NO_ERROR)
	{
		strcat (strRes, "\nconsumeRights failed with error: ");
		addErrorString(status, strRes);
		__android_log_write (ANDROID_LOG_ERROR,"DrmAssistJNI","ConsumeStream - failed in comsumeRights, exit...");
		goto end;
	}
	strcat (strRes, "\nconsumeRights success!");

	// Decrypt the first sample (testDataArr[0])
	idx = 0;

	// Preparing the encBuf and decBuf
	encBuf = new DrmBuffer(testDataArr[idx].encData, testDataArr[idx].dataLength);
	if (NULL == encBuf)
	{
		log_print_params (ANDROID_LOG_ERROR, "DrmAssistJNI",
			"ConsumeStream[%u] - failed to allocate encBuf (size: %u), exit...", idx, sizeof(DrmBuffer));
		goto end;
	}
	
	decBuf = new DrmBuffer(decBufferData, testDataArr[idx].dataLength);
	if (NULL == encBuf)
	{
		log_print_params (ANDROID_LOG_ERROR, "DrmAssistJNI",
			"ConsumeStream[%u] - failed to allocate encBuf (size: %u), exit...", idx, sizeof(DrmBuffer));
		goto end;
	}

	IVBuf = IVFactory(&testDataArr[idx].sampleHeader, testDataArr[idx].subSampleArr);
	if (NULL == IVBuf)
	{
		log_print_params (ANDROID_LOG_ERROR, "DrmAssistJNI",
			"ConsumeStream[%u] - IVFactory returned NULL IVBuf, exit...", idx);
		goto end;
	}

	log_print_params (ANDROID_LOG_INFO,"DrmAssistJNI", "ConsumeStream[%u] - calling decrypt", idx);
	status = mDrmManagerClient->decrypt (mDecryptHandle, DECRYPT_UNIT_ID_AUDIO, encBuf, &decBuf, IVBuf);
	if (status != DRM_NO_ERROR)
	{
		log_print_params (ANDROID_LOG_ERROR,"DrmAssistJNI",
			"ConsumeStream[%u] - failed to decrypt buffer,  exit...", idx);
		strcat (strRes, "\ndecrypt failed with error: ");
		addErrorString(status, strRes);
		goto end;
	}
	strcat (strRes, "\ndecrypt success!");

	// compare decrypted buffer payload and clear buffer
	log_print_params (ANDROID_LOG_INFO,"DrmAssistJNI", "ConsumeStream[%u] - performing memory comparison on decrypted content", idx);
	if (memcmp (decBuf->data, testDataArr[idx].clearData, testDataArr[idx].dataLength) != 0)
	{
		log_print_params (ANDROID_LOG_ERROR,"DrmAssistJNI",
			"ConsumeStream[%u] - failed memory comparison on decrypted content", idx);
		strcat (strRes, "\nmemcmp failed..");
		goto end;
	}
	strcat (strRes, "\nmemcmp success!");
	log_print_params (ANDROID_LOG_INFO,"DrmAssistJNI", "ConsumeStream[%u] - memory comparison decrypted content is OK !! :)", idx); 
	// finalize decrypt unit, unitId is the track id
	log_print_params (ANDROID_LOG_INFO,"DrmAssistJNI","ConsumeStream[%u] - calling finalizeDecryptUnit()", idx);
	status = mDrmManagerClient->finalizeDecryptUnit (mDecryptHandle, DECRYPT_UNIT_ID_VIDEO);
	if (status != DRM_NO_ERROR)
	{
		log_print_params (ANDROID_LOG_ERROR,"DrmAssistJNI",
			"ConsumeStream[%u] - failed to finalize Decrypt unit, exit...", idx);
		strcat (strRes, "\nfinalizeDecryptUnit failed with error: ");
		addErrorString(status, strRes);
		goto end;
	}
	strcat (strRes, "\nfinalizeDecryptUnit success!");

	/* close the session */
	log_print_params (ANDROID_LOG_INFO,"DrmAssistJNI","ConsumeStream[%u] - calling closeDecryptSession()", idx);
	status = mDrmManagerClient->closeDecryptSession (mDecryptHandle);
	if (status != DRM_NO_ERROR)
	{
		log_print_params (ANDROID_LOG_ERROR,"DrmAssistJNI",
			"ConsumeStream[%u] - failed to close Decrypt session, exit...", idx);
		strcat (strRes, "\ncloseDecryptSession failed with error: ");
		addErrorString(status, strRes);
		goto end;
	}

	strcat (strRes, "\ncloseDecryptSession success!");
	log_print_params (ANDROID_LOG_INFO,"DrmAssistJNI","ConsumeStream[%u] finished successfully!", idx);
	result = true;
	
end:
	DELETE_PTR(drmHeader);
	DELETE_PTR(encBuf);
	DELETE_PTR(decBuf);
	DELETE_IV_Buf(IVBuf);
	
	return result;
}

} // extern
}// namespace
