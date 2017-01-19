/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __DX_DRM__PLUGIN_H__
#define __DX_DRM__PLUGIN_H__

#include <DrmEngineBase.h>
#include "SessionMap.h"

#include <IDxDrmCoreClient.h>
#include <DxImportStream.h>
#include <DxDrmStream.h>
#include <DxDrmFile.h>
#include <DxDrmDebugApi.h>
#include <DxLock.h>
#include <DxDrmFileUtils.h>
#include <DxDrmFrameworkPlugin_version.h>
namespace android {
#define	RENEW_RIGHTS			"RENEW_RIGHTS_DRM_CONTENT_PATH"
#define SUPER_DISTRIBUTION		"PREPARE_FOR_SUPERDISTRIBUTION_DRM_CONTENT_PATH"

#define START_SILENT 0xBA
#define START_PEEK   0xBB

#define STATUS_ERROR_MIME_TYPE_NOT_RECOGNIZED		3000
#define STATUS_ERROR_SECURE_STORAGE_IS_CORRUPTED	3001
#define STATUS_ERROR_BAD_ARGUMENTS			3002

// this is an array of mime types with extensions mapping
const char*  ALL_PLAYREADY_MIME_TYPES_ARR [] = {
	// WMDRM mime-types
//	"application/vnd.ms-wmdrm.lic-resp",
//	"application/vnd.ms-wmdrm.meter-chlg-req",
	// PlayReady mime-types
//	"audio/vnd.ms-playready.media.pya",
//	"video/vnd.ms-playready.media.pyv",
//	"video/vnd.ms-playready.media.eny",             // No mime-type is defined for envelope files so we use Samsung definition
	"application/vnd.ms-playready.initiator+xml",
	"application/vnd.ms-playready.lic-resp",        // No mime-type is defined for playReady license responses so we invented one
	// PIFF
	"audio/isma",
	"video/ismv",
	"SmoothStreaming/mp4",
	NULL // last entry
};


const char*  ALL_PLAYREADY_TRIGGER_MIME_TYPES_ARR [] = {
	 "application/vnd.ms-playready.initiator+xml", 
	 NULL // last entry
}; 

const char*  ALL_PLAYREADY_RO_MIME_TYPES_ARR [] = {
	 "application/vnd.ms-wmdrm.lic-resp", 
	 NULL // last entry
}; 

//@CR have one list per scheme!

// these are arrays of file extentions
const char* ALL_OMA_V1_FILE_EXTENTIONS_ARR[] = {
	".dcf",
	NULL // last entry
};

// this is an array of PR file extentions
const char* ALL_PLAYREADY_FILE_EXTENTIONS_ARR[] = {
//	".pya",
//	".pyv",
//	".eny",
//	".pye",
	".ismv",
	".isma",
	".ismc",
//	".cms",
	".playready",
	NULL  // last entry
};

const char* ALL_PLAYREADY_ASF_FILE_EXTENTIONS_ARR[] = {
	".pya",
	".pyv",
	NULL  // last entry
};
const char* ALL_PLAYREADY_ENV_FILE_EXTENTIONS_ARR[] = {
	".eny",
	".pye",
	NULL  // last entry
};
const char* ALL_TEXT_ATTR_ARR[] = {
	"DX_ATTR_CONTENT_ID",
	"DX_ATTR_CONTENT_MIME_TYPE",
	"DX_ATTR_FILE_MIME_TYPE",
	"DX_ATTR_TITLE",
	"DX_ATTR_DESCRIPTION",
	"DX_ATTR_AUTHOR",
	"DX_ATTR_RI_URL",
	"DX_ATTR_ICON_URI",
	"DX_ATTR_VENDOR",
	"DX_ATTR_ENCRYPTION_METHOD",
	//PlayReady
	"DX_ATTR_PRDY_DOMAIN_LIST",
	//TextualHeaders
	"DX_ATTR_INFO_URL",
	"DX_ATTR_COVER_URI",
	"DX_ATTR_LYRICS_URI",
	"DX_ATTR_SILENT_METHOD",
	"DX_ATTR_SILENT_URL",
	"DX_ATTR_PREVIEW_METHOD",
	"DX_ATTR_PREVIEW_URL",
	"DX_ATTR_CONTENT_URL",
	"DX_ATTR_CONTENT_VERSION",
	"DX_ATTR_CONTENT_LOCATION",
	"DX_ATTR_PROFILE_NAME",
	"DX_ATTR_GROUP_ID",
	"DX_ATTR_CLASSIFICATION_ENTITY",
	"DX_ATTR_CLASSIFICATION_TABLE",
	"DX_ATTR_CLASSIFICATION_INFO",
	"DX_ATTR_RECORDING_YEAR",
	"DX_ATTR_COPYRIGHT_NOTICE",
	"DX_ATTR_PERFORMER",
	"DX_ATTR_GENRE",
	"DX_ATTR_ALBUM_TITLE",
	"DX_ATTR_ALBUM_TRACK_NUM",
	"DX_ATTR_RATING_ENTITY",
	"DX_ATTR_RATING_CRITERIA",
	"DX_ATTR_RATING_INFO",
	"DX_ATTR_KEYWORDS",
	"DX_ATTR_LOCATION_INFO",
	"DX_ATTR_LOCATION_TYPE",
	"DX_ATTR_LOCATION_LONGITUDE",
	"DX_ATTR_LOCATION_LATITUDE",
	"DX_ATTR_LOCATION_ALTITUDE",
	"DX_ATTR_TRANSACTION_ID",
	"DX_ATTR_CUSTOM_DATA",
	"DX_ATTR_NUM_OF_ATTRIBUTES",
	NULL
};

class DxDrmPrRecommendedPlugin : public DrmEngineBase {

public:
    DxDrmPrRecommendedPlugin();
    virtual ~DxDrmPrRecommendedPlugin();

protected:
    DrmConstraints* onGetConstraints(int uniqueId, const String8* path, int action);

    DrmMetadata* onGetMetadata(int uniqueId, const String8* path);

    status_t onInitialize(int uniqueId);

    status_t onSetOnInfoListener(int uniqueId, const IDrmEngine::OnInfoListener* infoListener);

    status_t onTerminate(int uniqueId);

    bool onCanHandle(int uniqueId, const String8& path);

    DrmInfoStatus* onProcessDrmInfo(int uniqueId, const DrmInfo* drmInfo);

    status_t onSaveRights(int uniqueId, const DrmRights& drmRights,
            const String8& rightsPath, const String8& contentPath);

    DrmInfo* onAcquireDrmInfo(int uniqueId, const DrmInfoRequest* drmInfoRequest);

#ifdef DX_JELLYBEAN42
    String8 onGetOriginalMimeType(int uniqueId, const String8& path, int fd);
#else
    String8 onGetOriginalMimeType(int uniqueId, const String8& path);
#endif

    int onGetDrmObjectType(int uniqueId, const String8& path, const String8& mimeType);

    int onCheckRightsStatus(int uniqueId, const String8& path, int action);

    status_t onConsumeRights(int uniqueId, DecryptHandle* decryptHandle, int action, bool reserve);

    status_t onSetPlaybackStatus(
            int uniqueId, DecryptHandle* decryptHandle, int playbackStatus, int64_t position);

    bool onValidateAction(
            int uniqueId, const String8& path, int action, const ActionDescription& description);

    status_t onRemoveRights(int uniqueId, const String8& path);

    status_t onRemoveAllRights(int uniqueId);

    status_t onOpenConvertSession(int uniqueId, int convertId);

    DrmConvertedStatus* onConvertData(int uniqueId, int convertId, const DrmBuffer* inputData);

    DrmConvertedStatus* onCloseConvertSession(int uniqueId, int convertId);

    DrmSupportInfo* onGetSupportInfo(int uniqueId);

    status_t onOpenDecryptSession(
            int uniqueId, DecryptHandle* decryptHandle, int fd, off64_t offset, off64_t length);
#ifdef DX_JELLYBEAN
    status_t onOpenDecryptSession(int uniqueId, DecryptHandle* decryptHandle, int fd, off64_t offset, off64_t length, const char* mime); //Jellybean new API
#endif
    status_t onOpenDecryptSession(
            int uniqueId, DecryptHandle* decryptHandle, const char* uri);
#ifdef DX_JELLYBEAN
    status_t onOpenDecryptSession(int uniqueId, DecryptHandle* decryptHandle, const char* uri, const char* mime); //Jellybean new API
#endif
    status_t onCloseDecryptSession(int uniqueId, DecryptHandle* decryptHandle);

    status_t onInitializeDecryptUnit(int uniqueId, DecryptHandle* decryptHandle,
            int decryptUnitId, const DrmBuffer* headerInfo);

    status_t onDecrypt(int uniqueId, DecryptHandle* decryptHandle, int decryptUnitId,
            const DrmBuffer* encBuffer, DrmBuffer** decBuffer, DrmBuffer* IV);

    status_t onFinalizeDecryptUnit(int uniqueId, DecryptHandle* decryptHandle, int decryptUnitId);

    ssize_t onPread(int uniqueId, DecryptHandle* decryptHandle,
            void* buffer, ssize_t numBytes, off64_t offset);

private:

    class ConvertSession {
    public :
	
        ConvertSession(HDxImportStream newImportStream):importStream(newImportStream){};
        virtual ~ConvertSession() {};

	HDxImportStream importStream;
    };


    class DecodeSession {
    public :
	DecodeSession():drmStream(NULL),offset(0),decryptUnitId(-1),intentSet(false),sessionHeader(NULL),drmFile(NULL) {};
	DecodeSession(HDxDrmStream myDrmStream, HDxDrmFile myDrmFile, int myDecryptUnitId):drmStream(myDrmStream),drmFile(myDrmFile),offset(0),decryptUnitId(myDecryptUnitId),intentSet(false),sessionHeader(NULL),mimeType(String8::empty()){};
   	virtual ~DecodeSession(){
		if(NULL != drmStream){DxDrmStream_Close (&drmStream);}
	};

        //members
	HDxDrmFile	drmFile;
	HDxDrmStream 	drmStream;
	off64_t 	offset;            // used in case of file
	int     	decryptUnitId;     // used in case of stream
	bool    	intentSet;
	DrmBuffer*	sessionHeader;
	String8		mimeType;
    };

    class ClientSession {
    public :
	IDrmEngine::OnInfoListener* mInfoListener;
	HDxImportStream * HDxImportStreamInfo; 
	/**
 	* Session Map Tables for holding decoded sessions
	 */
	SessionMap<DecodeSession*> DecodeSessionMap;
	/**
 	* Session Map Tables for holding conversion sessions
	 */
	SessionMap<ConvertSession*> ConvertSessionMap;
        
        ClientSession() {mInfoListener = NULL;}
        virtual ~ClientSession() {/*in ICS destroyMap is private -> DecodeSessionMap.destroyMap();*/}
    };



    DecryptHandle* openDecryptSessionImpl();
    DxStatus lock(int uniqueId, const char * callerFuncName);
    void unlock(int uniqueId, const char * callerFuncName);
    status_t translateDxDrmStatus(EDxDrmStatus result, status_t defaultErrorRetVal, const char * callerFuncName);

    int isPartOfDomain(int uniqueId, String8 serviceId, String8 accountId, String8 revision);

    // use this method to get the client instance, it uses Lazy-initialization since the DxDrmServerIpc may not be available at startup
    status_t initClient_and_lock(int uniqueId, const char * callerFuncName);
    bool checkStatus 			(int uniqueId, EDxDrmStatus result);
    bool getClientSession 		(int uniqueId, ClientSession *&clientSession);
    bool getDecodeSession 		(int uniqueId, DecryptHandle* decryptHandle, DecodeSession *&decodeSession);
    bool getConvertSession 		(int uniqueId, int convertId, ConvertSession *&convertSession);
    String8 generateInitiatorXml 	(int infoType, String8 domainController, String8 serviceId, String8 accountId, String8 customData);
    void translateActionToFlags 	(int action, int& FlagsToCheck , EDxPermission& permission);
    void translateRightsStatus 		(int activeFlags, int& rightsStatus);
    void translateDxRightsStatus 	(EDxRightsStatus DxRights, int& rightsStatus);
    void translateIntent 		(int action, EDxIntent& intentVal);
    status_t translateDecryptResult 	(EDxDrmStatus result);
    String8 getMimeTypeFromExtension	(String8 extension);
    int getActionFromMimeType 	(String8 mimeType);
    EDxIntent getIntentFromMimeType(String8 mimeType);
    bool DxCanHandle (String8 path);
    bool isMimeTypeSupported(const char* mime);

	//Helper function for onAcquireDrmInfo():
	EDxDrmStatus onAcquireDrmInfo_openStream(const DrmInfoRequest* drmInfoRequest);
	EDxDrmStatus onAcquireDrmInfo_GenerateDomainChallenge(String8 drmAction, const DrmInfoRequest* drmInfoRequest, DrmInfo* drmInfo);
	EDxDrmStatus onAcquireDrmInfo_ProcessResponse(String8 drmAction, const DrmInfoRequest* drmInfoRequest);
	EDxDrmStatus onAcquireDrmInfo_HandleInitiator(const DrmInfoRequest* drmInfoRequest);
	EDxDrmStatus onAcquireDrmInfo_GenerateLicenseChallenge(const DrmInfoRequest* drmInfoRequest, DrmInfo* drmInfo);
	EDxDrmStatus onAcquireDrmInfo_ProcessLicenseResponse(const DrmInfoRequest* drmInfoRequest, DrmInfo* drmInfo);
	EDxDrmStatus onAcquireDrmInfo_GenerateMeterCertChallenge(const DrmInfoRequest* drmInfoRequest, DrmInfo* drmInfo);
	EDxDrmStatus onAcquireDrmInfo_GenerateMeterDataChallenge(const DrmInfoRequest* drmInfoRequest, DrmInfo* drmInfo);
	EDxDrmStatus onAcquireDrmInfo_GetDrmHeader(const DrmInfoRequest* drmInfoRequest, DrmInfo* drmInfo);
	EDxDrmStatus onAcquireDrmInfo_GetOplMiracast(int uniqueId, DrmInfo* drmInfo);

    IDxDrmCoreClient* 			mDrmClient; //@@CR - check if we need to destroy
    DxLock mLock;

    // keep stream handle for NPL support
    HDxDrmStream    mLastStreamHandle;

	DecryptHandle	mLastDecryptHandle; // This is used to get OPL & Miracast status for last decrypt handle.

    /**
     * Session Map Tables for holding client sessions
    */
    SessionMap<ClientSession*> 		ClientSessionMap;
    bool setClockOnce;
};

};

#endif /* __DX_DRM_PLUGIN_H__ */


