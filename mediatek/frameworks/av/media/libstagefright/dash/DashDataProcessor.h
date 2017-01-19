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

#ifndef _DASH_PROCESSOR_H_

#define _DASH_PROCESSOR_H_

#include <media/stagefright/foundation/AHandler.h>

#include <media/stagefright/MediaDefs.h>

#include "ATSParser.h"
#include "DashDrmHandler.h"
#include "DashFileFetcher.h"
#include "MPDPlaylists.h"

using namespace android_dash;


namespace android {


struct ABuffer;
struct String8;
struct AnotherPacketSource;
struct DashFmp4Parser;
struct DashDrmHandler; 
 
/*

normal case: start--feedMoreBuffer--get sidx(check startTimeUs mapped segment index ==0) --feedMoreBuffer(kBufferFlag_StartDequeueAU)--stop
seek case: start- feedMoreBuffer--get sidx(check startTimeUs mapped segment index !=0)--flush --feedMoreBuffer(kBufferFlag_StartDequeueAU)--stop
BWS: feedMoreBuffer-need bws-stop-start new ddp --feedMoreBuffer--get sidx(check startTimeUs mapped segment index !=0)--flush --feedMoreBuffer(kBufferFlag_StartDequeueAU)--sto

*/



//for fmp4 parser, we should run in a single looper thread with parser.
struct DashDataProcessor : public AHandler {
	enum {
	    kWhatStarted,
	    kWhatFlushed,
	    kWhatStopped,	//in Async mode, check msg->findInt32("err",&err) for the result 	 
	    kWhatError,
	    kWhatPicture,
	    kWhatSidx,//post the sidx table
	};
	enum { 
		kBufferFlag_StartDequeueAU   = 1,			
    };

	enum{
		keyTrack_InvalidMin =0,
		keyTrack_Video = 1,
		keyTrack_Audio = 2,	
		keyTrack_SubTitle   = 4,	
		keyTrack_InvalidMax = 5,
	};
 
/*    
      what is in DashDataInfo?
      [must] : must be set  [may]:  set or not set is OK 

	1)source info [must]
	
	the feeded data by feedMoreBuffer container the corresponding track if XXXSource is set
	
	const sp<AnotherPacketSource>audioSource/videoSource/subtitleSource
	dataInfo->setPointer("audioSource", &audioSource.get());  // data has audio track
	dataInfo->setPointer("videoSource", &videoSource.get());  // data has video track
	dataInfo->setPointer("subtitleSource", &subtitleSource.get());// data has subtitle track

	2)container info[must]
	int32_t container;
	dataInfo->setInt32("file-Format", &container);  //keyContariner_XXX

	3)drm info[must]

        sp<DecryptHandle> mDecryptHandle;
        DrmManagerClient *mDrmManagerClient;
	int32_t drmType;
	
	dataInfo->setInt32("Drm-type", &drmType);//KeyDRM_XXX
	dataInfo->setPointer("Drm-Handle", &mDecryptHandle.get());
	dataInfo->setPointer("Drm-Client", &mDrmManagerClient);
	
*/  
	DashDataProcessor(
			const sp<AMessage> &notify, //used for feedback
			const sp<ALooper> & looper, // if set == NULL, we will new a looper inside for processor
			//const sp<MPDPlaylists> &playlist,//used for get drm info,container type 
			MPDPlaylists* playlist,
			const sp<AMessage> &DashDataInfo,
            int64_t basetimeUs
		);

	// should set the right offset and range-length 
	//if TS data : 
			//->will check whether the size is 188*n  and start code is 0x47
			//->will parser this buffer directly in the feedMoreBuffer function of caller thread
	//if Fmp4: 
		//parse full box data, lefe data will be parse in next feedMoreBuffer

	//put the extra message in meta when need: such as KeyID 
	//if start to dequeue AU from this buffer, please setInt32Data(1);
	status_t  feedMoreBuffer( sp<ABuffer> &buffer);

	status_t  start(); 

	//if flush is called, 
	//-->Processor will stop receiving/handle  Abuffer passed in by feedMoreBuffer
	//-->flush parser  and sent parser the start parsing segment index

	//if flush is finished
	//-->Processor will start receiving/handle Abuffer passed in by feedMoreBuffer


	//1) for start(BWS) : new ddp - start -flush;after get sidx will flush and start dl with 64K buffer
	//2) for stop£» will flush info
	//3) ts will ingore this parameter

	//4)msg->setInt32("videoStartSegmentIndex",XXX)
	//	msg->setInt32("audioStartSegmentIndex",XXX)
	//  msg->setInt32("subTitleStartSegmentIndex",XXX)
	
	status_t  flush(sp<AMessage> &flushMsg);  


	status_t  stop(); 


	//maybe used
	status_t  decrypt(
		int32_t trackIndex, const sp<ABuffer> &encBuf,const sp<ABuffer> &decBuf);
    
protected:
    virtual ~DashDataProcessor();
    virtual void onMessageReceived(const sp<AMessage> &msg);

private:
	
    enum { 
		kWhatStart          = 'strt' ,
		kWhatFlush        = 'flsh' ,
		kWhatStop           = 'stop',
		kWhatStartSync    = 'strs' ,
		kWhatParserNotify = 'pnty',

    };

	struct TrackStatus{
		int32_t mSampleCount;
		bool mFirstAccessUnitValid;
		bool mEOS;	
	};
	sp<ALooper> mLooper;
	sp<AMessage> mNotify;
	MPDPlaylists* mPlaylist;
	KeyedVector<int32_t, sp<AnotherPacketSource> >  mPacketSources ;
	KeyedVector<int32_t, sp<MetaData> >  mFormats ;
	KeyedVector<int32_t,TrackStatus >  mTrackStatus ;
		
    sp<ATSParser> mTSParser;
    sp<DashFmp4Parser> mFmp4Parser;
	
    sp<DashDrmHandler> mDrmHandler;
	Mutex mLock;
	
    uint32_t mStreamTypeMask;
    uint32_t mParserTypeMask;
    int64_t mBaseMediaTimeUs;
    bool mIsEncryptData;
	bool mIsEnableDequeueBufferNow;
	
    int32_t mMPDFileFormat;//container tyep 
    int32_t mMPDDrmType;
    FILE *mLogFile;
	
	

    status_t onStart();
    status_t onStop();
    status_t onFlush(sp<AMessage> &flushMsg);
    void setDrmInfo(int32_t trackIndex, const sp<ABuffer> &buffer);
    status_t handleTSBuffer(  sp<ABuffer> &buffer);
    status_t handleFmp4Buffer(  sp<ABuffer> &buffer);

    void notifyError(status_t err);
	//misc
	const char* trackIndex2Name(uint32_t trackIndex);
	const char* streamMask2LogTag();
	
	

    DISALLOW_EVIL_CONSTRUCTORS(DashDataProcessor);
};

}  // namespace android

#endif  // _DASH_PROCESSOR_H_

