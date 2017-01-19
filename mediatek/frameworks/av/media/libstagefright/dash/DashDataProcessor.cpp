/*
 * Copyright (C) 2012 The Android Open Source Project
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

//#define LOG_NDEBUG 0
#define LOG_TAG "[Dash]DataProcessor"
#include <utils/Log.h>
#include <cutils/xlog.h>
#include "DashDataProcessor.h"

#include "include/avc_utils.h"
#include "include/ID3.h"
#include "AnotherPacketSource.h"
#include "DashFmp4Parser.h"

#include <media/stagefright/foundation/ABitReader.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/hexdump.h>
#include <media/stagefright/FileSource.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/Utils.h>
#include <utils/KeyedVector.h>
#include <cutils/properties.h>

#include <ctype.h>
#include <openssl/aes.h>
#include <openssl/md5.h>

namespace android {

const char* TrackName[3] ={"video","audio","subTitle"};
const char* LogTagName[8] ={"NULL",     "[V-DDP]",  "[A-DDP]","NULL",
                            "[S-DDP]",  "[VS-DDP]", "[AS-DDP]","[VAS-DDP]"};
DashDataProcessor::DashDataProcessor(
    const sp<AMessage> &notify, //used for feedback
    const sp<ALooper> & looper, 
    MPDPlaylists* playlist,
    const sp<AMessage> &DashDataInfo,
    int64_t basetimeUs)
    : mLooper ((looper == NULL)? (new ALooper): looper),
    mNotify(notify),
    mPlaylist(playlist),
    mBaseMediaTimeUs(basetimeUs),
    mTSParser(NULL),
    mFmp4Parser(NULL),
    mDrmHandler(NULL),
    mStreamTypeMask(0),
    mParserTypeMask(0),
    mIsEncryptData(false),    
    mIsEnableDequeueBufferNow(false),
    mMPDFileFormat(-1),
    mMPDDrmType(-1),
    mLogFile(NULL){

    ALOGI("DashDataInfo=%s,mBaseMediaTimeUs:%lld",DashDataInfo->debugString().c_str(),mBaseMediaTimeUs);
    CHECK(mNotify != NULL);
    //CHECK(mPlaylist != NULL);		

    mLooper->setName("dash_DDPLooper");		
    mLooper->start(false,false,PRIORITY_FOREGROUND);
    
	
	void* ptr;
	if(DashDataInfo->findPointer("audioSource", &ptr) && ptr != NULL){
		ALOGI("has audioSource %p",ptr);
		mStreamTypeMask |= keyTrack_Audio;
		mPacketSources.add(
            keyTrack_Audio,
            static_cast<AnotherPacketSource *>(ptr));
		
        TrackStatus mTs;
        mTs.mEOS = false;
        mTs.mSampleCount =0;
        mTrackStatus.add(keyTrack_Audio,mTs);
	};
	ptr = NULL;
    if(DashDataInfo->findPointer("videoSource", &ptr) && ptr != NULL){
        ALOGI("has videoSource %p",ptr);
        mStreamTypeMask |= keyTrack_Video;
        mPacketSources.add(
            keyTrack_Video,
            static_cast<AnotherPacketSource *>(ptr));

        TrackStatus mTs;
        mTs.mEOS = false;
        mTs.mSampleCount =0;
        mTrackStatus.add(keyTrack_Video,mTs);
    };
	 ptr = NULL;
       if(DashDataInfo->findPointer("subtitleSource", &ptr) && ptr != NULL){
	   	ALOGI("has subtitleSource %p",ptr);
		mStreamTypeMask |= keyTrack_SubTitle;
		mPacketSources.add(
                keyTrack_SubTitle,
                 static_cast<AnotherPacketSource *>(ptr));

        TrackStatus mTs;
        mTs.mEOS = false;
        mTs.mSampleCount =0;    
        mTrackStatus.add(keyTrack_SubTitle,mTs);
	};
	 ALOGI("source setting done");


	//2)container info //keyContariner_XXX
    ALOGI("%s Init :Find file-format",streamMask2LogTag());
    int32_t container;
    if(DashDataInfo->findInt32("file-Format", &container)){        
        mMPDFileFormat = container;
        CHECK(mMPDFileFormat == MPDPlaylists::MPD_MP4 || mMPDFileFormat == MPDPlaylists::MPD_TS);
    } else{        
        mMPDFileFormat =  mPlaylist->getMPDContainer();
        ALOGI("%s get container type %d from MPDPlaylists",streamMask2LogTag(),mMPDFileFormat); 
    }

	if(mMPDFileFormat  == MPDPlaylists::MPD_TS ){
		mTSParser = new ATSParser(ATSParser::TS_SOURCE_IS_STREAMING);
		ALOGI("%s new TS parser done",streamMask2LogTag());
	}else if(mMPDFileFormat == MPDPlaylists::MPD_MP4){	
		mFmp4Parser = new DashFmp4Parser();
	}

        //	3)drm info
	ALOGI("%s[dash drm] Find DRM",streamMask2LogTag());
    void *mDecryptHandle; //sp<DecryptHandle> mDecryptHandle;
    void *mDrmManagerClient;
	
	if(DashDataInfo->findInt32("Drm-type", &mMPDDrmType)){

        ALOGI("%s [dash drm] session setting:mDrmType %d",streamMask2LogTag(),mMPDDrmType);
        if( mMPDDrmType > MPDPlaylists::MPD_DRM_NONE ){
            mIsEncryptData = true;		
            if(DashDataInfo->findPointer("Drm-Handle", &mDecryptHandle) 
                && mDecryptHandle !=NULL
                && DashDataInfo->findPointer("Drm-Client", &mDrmManagerClient) 
                && mDrmManagerClient != NULL) {
                mDrmHandler = new DashDrmHandler(
                                    mMPDDrmType,
                                    static_cast<DecryptHandle *>(mDecryptHandle),
                                    (DrmManagerClient*)mDrmManagerClient);
                ALOGI("%s[dash drm] session setting:DecryptHandle %p mDrmManagerClient %p",
                    streamMask2LogTag(),mDecryptHandle,mDrmManagerClient);
            }else{
                ALOGE("%s [dash drm][error] session setting : no handle",streamMask2LogTag());
				CHECK(0);
            }
        }

	}else{
		//check from playlist		
        mMPDDrmType =  mPlaylist->getDrmType();
        if(mMPDDrmType > MPDPlaylists::MPD_DRM_NONE){
            mIsEncryptData = true;	
            mDrmHandler = new DashDrmHandler(mMPDDrmType,NULL,NULL);
        }     
        ALOGI("%s[dash drm]get drm type %d from MPDPlaylists",streamMask2LogTag(),mMPDDrmType);
	}
    ALOGI("%s[dash drm]get drm type %d mIsEncryptData %d",streamMask2LogTag(),mMPDDrmType,mIsEncryptData);

	
	char value[PROPERTY_VALUE_MAX];
    if (property_get("dash.dump.au.video", value, NULL) 
        && (!strcmp(value, "1") || !strcasecmp(value, "true"))) {
        ALOGI("open dump");
        mLogFile = fopen("/sdcard/log.bin", "wb");
	}     
	   
       
    ALOGI("%s new DashDataProcessor done ",streamMask2LogTag());
	
}

DashDataProcessor::~DashDataProcessor() {
    ALOGI("%s %p delate ddp++++++",streamMask2LogTag(),this);
	
    if (mLogFile != NULL) {
        fclose(mLogFile);
        mLogFile = NULL;
    }
    if (mLooper != NULL)
    {

        mLooper->stop();
        mLooper->unregisterHandler(id());
        ALOGI("%s %p stop ddp looper",streamMask2LogTag(),this);
    }
    
    ALOGI("%s %p delate ddp done-----",streamMask2LogTag(),this);
}



status_t DashDataProcessor::start(){

    ALOGI("%s %p start++++ [action]",streamMask2LogTag(),this);
    Mutex::Autolock autoLock(mLock);		

    mLooper->registerHandler(this); 
    sp<AMessage> notify = new AMessage(kWhatParserNotify, id());
    mFmp4Parser->setUserInfo( notify,mBaseMediaTimeUs,streamMask2LogTag());


    sp<AMessage> msg = new AMessage(kWhatStart, id());

    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    if (err != OK) {
        ALOGE("%s start fail 1: %d",streamMask2LogTag(), err);
        return err;
    }

    if (response->findInt32("err", &err) && err != OK) {
        ALOGI("%s start fail 2: %d  e", streamMask2LogTag(),err);
        return err;
    }
    ALOGI("%s %p start---- [action]",streamMask2LogTag(),this);
    return OK;

}

status_t DashDataProcessor::flush(sp<AMessage> &flushMsg){
	
    ALOGI("%s %p flush++++ [action]",streamMask2LogTag(), this);
    Mutex::Autolock autoLock(mLock);
    

    sp<AMessage> msg = new AMessage(kWhatFlush, id());
    msg->setMessage("flushMsg",flushMsg);
    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    if (err != OK) {
        ALOGE("%s flush fail 1: %d",streamMask2LogTag(), err);
        return err;
    }

    if (response->findInt32("err", &err) && err != OK) {
        ALOGI("%S flush fail 2: %d  e", streamMask2LogTag(),err);
        return err;
    }
    ALOGI("%s %p flush---- [action]",streamMask2LogTag(), this);

    return OK;

}

status_t DashDataProcessor::stop(){
	
	ALOGI("%s[%p] stop+++  ",streamMask2LogTag(), this);
    Mutex::Autolock autoLock(mLock);	

    //real stop
    sp<AMessage> msg = new AMessage(kWhatStop, id());
	sp<AMessage> response;
	status_t err = msg->postAndAwaitResponse(&response);

	if (err != OK) {
		ALOGE("%s stop fail 1: %d",streamMask2LogTag(), err);
		return err;
	}

	if (response->findInt32("err", &err) && err != OK) {
		ALOGI("%s stop fail 2: %d", streamMask2LogTag(),err);
		return err;
	}
  
   
    ALOGI("%s[%p] stop---- ",streamMask2LogTag(), this);
    return OK;
}


void DashDataProcessor::onMessageReceived(const sp<AMessage> &msg) {
    switch (msg->what()) { 
        case kWhatStart:
        {		

    		status_t err = onStart();

    		sp<AMessage> response = new AMessage;

    		response->setInt32("err", err);

    		uint32_t replyID;
    		CHECK(msg->senderAwaitsResponse(&replyID));

    		response->postReply(replyID);
            break;
        }

        case kWhatStop:
        {
            status_t err = onStop();

            sp<AMessage> response = new AMessage;

            response->setInt32("err", err);

            uint32_t replyID;
            CHECK(msg->senderAwaitsResponse(&replyID));

            response->postReply(replyID);
             break;
        }

        case kWhatFlush:
        {
            
            sp<AMessage> flushMsg =NULL;
            CHECK(msg->findMessage("flushMsg",&flushMsg) && flushMsg != NULL);
           
            status_t err = onFlush(flushMsg);

            sp<AMessage> response = new AMessage;

            response->setInt32("err", err);

            uint32_t replyID;
            CHECK(msg->senderAwaitsResponse(&replyID));

            response->postReply(replyID);
            break;
        }

    	case kWhatParserNotify :
    	{
    		int32_t what;
            CHECK(msg->findInt32("what", &what));
            if(what == DashFmp4Parser::kWhatError){


            }
    		else if(what == DashFmp4Parser::kWhatSidx){
    			Vector<DashFmp4Parser::SidxEntry>* ptSidx=NULL;
                if(msg->findPointer("video-sidx-vector", (void **)&ptSidx) && ptSidx != NULL){
                    ALOGI("%s[profile] parser notify the video sidx vector pointer back ",streamMask2LogTag());
    				    sp<AMessage> notify = mNotify->dup();
    				    notify->setInt32("what", kWhatSidx);
    				    notify->setPointer("video-sidx-vector", ptSidx);
    				    notify->post();

    			}
                if(msg->findPointer("audio-sidx-vector", (void **)&ptSidx) && ptSidx != NULL){
                    ALOGI("%s[profile] parser notify the audio sidx vector pointer back ",streamMask2LogTag());
    				    sp<AMessage> notify = mNotify->dup();
    				    notify->setInt32("what", kWhatSidx);
    				    notify->setPointer("audio-sidx-vector", ptSidx);
    				    notify->post();

    			} 
                if(msg->findPointer("SubTitle-sidx-vector", (void **)&ptSidx) && ptSidx != NULL){
                    ALOGI("%s[profile] parser notify the audio sidx vector pointer back ",streamMask2LogTag());
    				    sp<AMessage> notify = mNotify->dup();
    				    notify->setInt32("what", kWhatSidx);
    				    notify->setPointer("SubTitle-sidx-vector", ptSidx);
    				    notify->post();

    			} 
               // onSuspend();

    		}else if(what == DashFmp4Parser::kWhatFileMeta){
    		    sp<RefBase> obj;
                CHECK(msg->findObject("fileMeta", &obj));
                sp<MetaData> fileMeta = static_cast<MetaData *>(obj.get());
                
    			int32_t hasTrack=0; 
                status_t err =OK;
    			if(fileMeta->findInt32('hasV',&hasTrack)){
    				mParserTypeMask |= keyTrack_Video;
    			}
    			if(fileMeta->findInt32('hasA',&hasTrack)){
    				mParserTypeMask |= keyTrack_Audio;
    			}
    			if(fileMeta->findInt32('hasS',&hasTrack)){
    				mParserTypeMask |= keyTrack_SubTitle;
    			}
                //pssh: 16 systemID + 4byte size + size of data
                const void* pData= NULL;
                size_t  DataSize =0;
                uint32_t type;
                if(mIsEncryptData && fileMeta->findData(kKeyPssh,&type,&pData,&DataSize) && DataSize > 16 ){
                    ALOGI("%s[profile] kWhatFileMeta: has pssh size %d",streamMask2LogTag(),DataSize);
                    if(mDrmHandler != NULL){
                        sp<MetaData> pssh = new MetaData;
                        pssh->setData(kKeyPssh,'pssh',pData,DataSize);
                        mDrmHandler->setPSSH(pssh);
                    }
    			}
    			
    			ALOGI("%s[profile] mParserTypeMask = 0x%x",streamMask2LogTag(),mParserTypeMask);
    		}
    		break;
    	}
            default:
                TRESPASS();
    }
}
status_t DashDataProcessor::onStart() {
    if(mMPDFileFormat != MPDPlaylists::MPD_MP4  &&  mMPDFileFormat != MPDPlaylists::MPD_TS){
        ALOGE("%s[ERROR]stiil not valid fileformat ",streamMask2LogTag());
        return INVALID_OPERATION;
    }
    if(mIsEncryptData && mDrmHandler == NULL){
        ALOGE("%s[ERROR]stiil not setup DrmHandler with EncryptData",streamMask2LogTag());
        return INVALID_OPERATION;
    }

    return OK;
}

status_t DashDataProcessor::onFlush(sp<AMessage> &flushMsg) {
    ALOGI("%s onFLush now++++ [%p] flushMsg %s",streamMask2LogTag(),this,flushMsg->debugString().c_str());

    status_t err=OK;
    if(mMPDFileFormat == MPDPlaylists::MPD_MP4){
        CHECK( (err = mFmp4Parser->flush(flushMsg)) == OK) ; 
    }else if(mMPDFileFormat == MPDPlaylists::MPD_TS){
        //nothting to do for TS
    }
    return OK;
}


status_t DashDataProcessor::onStop() {
;    
    status_t err =OK ;
    for(size_t i =0 ;i < mTrackStatus.size(); i++){
        TrackStatus *mTs = &(mTrackStatus.editValueAt(i));
	    ALOGI("%s[profile]mTrackIndex %d has sampleCount %d,Eos %d",
                streamMask2LogTag(),
                mTrackStatus.keyAt(i),mTs->mSampleCount,mTs->mEOS);
   }   
	
    if(mMPDFileFormat == MPDPlaylists::MPD_MP4){
        err = mFmp4Parser->stop();
    }
    return OK;
}


void DashDataProcessor::notifyError(status_t err) {
    sp<AMessage> notify = mNotify->dup();
    notify->setInt32("what", kWhatError);
    notify->setInt32("err", err);
    notify->post();
}


status_t DashDataProcessor::feedMoreBuffer(sp<ABuffer> &buffer){
    SXLOGV("%s feedMoreBuffer:buffer data =%p size= %d [%p]",streamMask2LogTag(),buffer->data(),buffer->size(),this);

	status_t err =OK;

	if(mMPDFileFormat == MPDPlaylists::MPD_MP4 ){
		err = handleFmp4Buffer( buffer);

	}else if(mMPDFileFormat == MPDPlaylists::MPD_TS){
		err = handleTSBuffer( buffer);

	}else{
		ALOGE("%s unknow fileformat",streamMask2LogTag());
		err = UNKNOWN_ERROR; 
	}
	if(err != OK){
		ALOGE("%s feedMoreBuffer result in err %d",streamMask2LogTag(),err);
	}
	return  err ;   
}


status_t DashDataProcessor::handleFmp4Buffer(  sp<ABuffer> &buffer){

    status_t err = OK;
    err = mFmp4Parser->proceedBuffer(buffer);
    if(err == -EINVAL) {
        ALOGI("[error]%s %p parser proceedBuffer ERROR_MALFORMED !!!! ",streamMask2LogTag(),this);
        return ERROR_MALFORMED;
    }
        
    int32_t  trackIndex =0;
    int32_t  fmp4TrackType =0;
    if(!mIsEnableDequeueBufferNow){
        if(buffer->int32Data() == kBufferFlag_StartDequeueAU){
            mIsEnableDequeueBufferNow = true;
            ALOGI("%s %p dequeueAU mIsEnableDequeueBufferNow",streamMask2LogTag(),this);
        }
        
    }
    
    err = OK;
    int64_t starttimeUs = ALooper::GetNowUs();
    
    for (size_t i = mPacketSources.size(); i-- > 0;) {

        sp<AnotherPacketSource> packetSource = mPacketSources.valueAt(i);
        trackIndex = mPacketSources.keyAt(i);
        SXLOGD("%s dequeueAU trackIndex=%d++++++++++++",streamMask2LogTag(),trackIndex);
        switch (trackIndex) {
            case keyTrack_Video:{
                fmp4TrackType = DashFmp4Parser::Track_Video;
             break;
            }
            case keyTrack_Audio:{
                fmp4TrackType = DashFmp4Parser::Track_Audio;
                break;
            }
            case keyTrack_SubTitle:{
                fmp4TrackType = DashFmp4Parser::Track_SubTitles;
                break;
            }
            default:
                TRESPASS();
       }

        sp<ABuffer> accessUnit = NULL;
        //check format first
        if( mFormats.indexOfKey(trackIndex) < 0) {//has not beed getFormat before

            sp<AMessage> metaMsg = mFmp4Parser->getFormat(fmp4TrackType);
            if(metaMsg != NULL){
                ALOGI("%s [%p] getFormat from parser %s",streamMask2LogTag(),this,metaMsg->debugString().c_str());
                sp < MetaData >  meta  = new MetaData;
                convertMessageToMetaData(metaMsg, meta);

                mFormats.add(trackIndex,meta);  
            }      
        }
/*
        if(mFormats.indexOfKey(trackIndex) >= 0){

            if (packetSource->getFormat() == NULL) {
                ALOGI("%s [%p] setFormat",streamMask2LogTag(),this);    
                packetSource->setFormat(mFormats.editValueFor(trackIndex));
                //drm track info:tenc   
                if(mIsEncryptData && mDrmHandler != NULL){
                    //sp<RefBase> obj;
                    //if(metaMsg->findObject("tenc", &obj) && obj != NULL){
                       // sp<MetaData> meta = static_cast<MetaData *>(obj.get());
                       // mDrmHandler->setTenc(fmp4TrackType, meta);
                    //}
                }  
            }
        }
        
        if (packetSource->getFormat() == NULL) {
            ALOGD("%s setFormat track %d has not setformat yet, skip ",streamMask2LogTag(),trackIndex);
            continue;
        }
        //async to get data
 */           
        SXLOGD("%s DequeueAU trackIndex=%d enter while-loop",streamMask2LogTag(),trackIndex);

        while (mIsEnableDequeueBufferNow
            &&((err =mFmp4Parser->dequeueAccessUnit(fmp4TrackType,&accessUnit) )== OK)
            && accessUnit != NULL) {	

            if(trackIndex == keyTrack_Video){
                if (mLogFile != NULL) {
                    fwrite(accessUnit->data(), 1, accessUnit->size(), mLogFile);
                }
            }
            setDrmInfo(trackIndex,accessUnit);

            if(mFormats.indexOfKey(trackIndex) >= 0){
                accessUnit->meta()->setObject("format",mFormats.editValueFor(trackIndex));
            }

            packetSource->queueAccessUnit(accessUnit);
            int64_t timeUs =-1;
            int32_t isSyncFrame = -1;
            accessUnit->meta()->findInt64("timeUs",  &timeUs);
            accessUnit->meta()->findInt32("is-sync-frame", &isSyncFrame);
/*
            ALOGD("%s %p DequeueAU a buffer : timeUs=%lld ms,isSyncFrame=%d",
                    streamMask2LogTag(),this, timeUs/1000,isSyncFrame);
*/
            TrackStatus *mTs = &(mTrackStatus.editValueFor(trackIndex));
            mTs->mSampleCount =  mTs->mSampleCount + 1;
            if(mTs->mSampleCount == 1){
                ALOGI("%s[Profile] DequeueAU 1th AU done",streamMask2LogTag());
                mTs->mFirstAccessUnitValid = true;
            }
        }

    }
    int64_t endtimeUs = ALooper::GetNowUs();
    SXLOGD("%s DequeueAU err %d use %lld us(-11 EAGAIN -22 EINVAL -32 EPIPE -1007 ERROR_MALFORMED)----- ",streamMask2LogTag(),err,endtimeUs-starttimeUs);
    if(err == -EAGAIN) {
        return OK;
    }
    return err;
    
}

status_t DashDataProcessor::handleTSBuffer( sp<ABuffer> &buffer) {
    if (buffer->size() > 0 && buffer->data()[0] == 0x47) {
        // Let's assume this is an MPEG2 transport stream.

        if ((buffer->size() % 188) != 0) {
	            ALOGE("%s [Error]MPEG2 transport stream is not an even multiple of 188 "
	                  "bytes in length.",streamMask2LogTag());
	            return ERROR_MALFORMED;
        }
		
        size_t offset = 0;
        //while-loop feed data
        while (offset <= buffer->size()) 
	{  
		status_t err = mTSParser->feedTSPacket(buffer->data() + offset, 188);

		if (err != OK) {
			ALOGE("%s [Error]feedTSPacket err=%d",streamMask2LogTag(),err);
			return err;
		}

		offset += 188;
	}

        for (size_t i = mPacketSources.size(); i-- > 0;) {
            sp<AnotherPacketSource> packetSource = mPacketSources.valueAt(i);

            ATSParser::SourceType type;
            switch (mPacketSources.keyAt(i)) {
                case keyTrack_Video:
    	        {
                    type = ATSParser::VIDEO;
                    break;
                }
                case keyTrack_Audio:
                {
                    type  = ATSParser::AUDIO;
                    break;
    		    }
                case keyTrack_SubTitle:
                {
        			ALOGE("%s [Error]MPEG2 Transport streams do not contain subtitles.",streamMask2LogTag());
        			return ERROR_MALFORMED;
        			break;
                }

                default:
                    TRESPASS();
            }

            sp<AnotherPacketSource> source =
                static_cast<AnotherPacketSource *>(
                        mTSParser->getSource(type).get());

            if (source == NULL) {
                ALOGW("%s MPEG2 Transport stream does not contain %s data.",
                      streamMask2LogTag(),
                      type == ATSParser::VIDEO ? "video" : "audio");

                mStreamTypeMask &= ~mPacketSources.keyAt(i);
                mPacketSources.removeItemsAt(i);
                continue;
            }

            sp<ABuffer> accessUnit;
            status_t finalResult;
            while (source->hasBufferAvailable(&finalResult)
                    && source->dequeueAccessUnit(&accessUnit) == OK) {

                sp<MetaData> format = source->getFormat();
                if (format != NULL) {
                    accessUnit->meta()->setObject("format", format);
                }
                
                packetSource->queueAccessUnit(accessUnit);
            }
/*
            if (packetSource->getFormat() == NULL) {
                packetSource->setFormat(source->getFormat());
            }
*/
        }

        return OK;
    } 
    ALOGE("%s err:invalid ts data",streamMask2LogTag());
    return ERROR_MALFORMED;
}


const char* DashDataProcessor::streamMask2LogTag(){
    uint32_t streamType=0;
    if(mStreamTypeMask < 8){
        streamType = mStreamTypeMask;
    }  
    
	return LogTagName[streamType];
}



//drm related functions

void  DashDataProcessor::setDrmInfo(int32_t trackIndex, const sp<ABuffer> &buffer){
    
	if(mIsEncryptData && mDrmHandler != NULL){
		buffer->meta()->setObject("drm-handler", mDrmHandler);
		buffer->meta()->setInt32("trackIndex", trackIndex);
	}	
	
}

/*
// tenck 
meta->setInt32(kKeyCryptoMode, Tenc->mIsEncrypted);
meta->setInt32(kKeyCryptoDefaultIVSize, Tenc->mIV_Size);

meta->setData(kKeyCryptoKey, 'tenc',Tenc->mKID,sizeof(Tenc->mKID));//16 byte

// store clear/encrypted lengths in metadata//as we store then as Uint32_t type, so*4
meta->setData(kKeyPlainSizes, 0,
    sample.mBytesOfClearData.array(), sample.mBytesOfClearData.size() * 4);
meta->setData(kKeyEncryptedSizes, 0,
    sample.mBytesOfEncryptedData.array(), sample.mBytesOfEncryptedData.size() * 4);

meta->setData(kKeyCryptoIV, 0, sample.mIV->data(), sample.mIV->size());
(*accessUnit)->meta()->setObject("encrypt",meta);
*/
status_t  DashDataProcessor::decrypt(
    int32_t trackIndex, const sp<ABuffer> &encBuf,const sp<ABuffer> &decBuf){
    
	if(mIsEncryptData && mDrmHandler != NULL){
        sp<RefBase> obj =NULL;
        int32_t trackIndex =0;
		CHECK(encBuf->meta()->findObject("drm-handler", &obj));
		CHECK(encBuf->meta()->findInt32("trackIndex", &trackIndex));
        sp<DashDrmHandler> handler = static_cast<DashDrmHandler *>(obj.get());

        CHECK(encBuf->meta()->findObject("encrypt", &obj));
        sp<MetaData> encMeta = static_cast<MetaData *>(obj.get());

        uint32_t type;
        const void *data;
        size_t size;
        CHECK(encMeta->findData(kKeyCryptoIV, &type, &data, &size));

        //sp<ABuffer> IV =new ABuffer(size);  
        //memcpy();
        //status_t err = handler->decrypt(trackIndex, encBuf, decBuf, IV);
        return OK;
	}	
	return INVALID_OPERATION;
}

}


