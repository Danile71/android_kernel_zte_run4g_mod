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

#define LOG_TAG "[dash]fmp4Parser"

#include <utils/Log.h>

#include "include/avc_utils.h"
#include "include/ESDS.h"
#include "DashFmp4Parser.h"


#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/hexdump.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/Utils.h>


#include <media/stagefright/MetaData.h>
#include <cutils/xlog.h>



namespace android {

static const char *Fourcc2String(uint32_t fourcc) {
    static char buffer[5];
    buffer[4] = '\0';
    buffer[0] = fourcc >> 24;
    buffer[1] = (fourcc >> 16) & 0xff;
    buffer[2] = (fourcc >> 8) & 0xff;
    buffer[3] = fourcc & 0xff;

    return buffer;
}

static const char *IndentString(size_t n) {
    static const char kSpace[] = "                              ";
    return kSpace + sizeof(kSpace) - 2 * n - 1;
}

// static
const DashFmp4Parser::DispatchEntry DashFmp4Parser::kDispatchTable[] = {
    { FOURCC('m', 'o', 'o', 'v'), 0, NULL },
    { FOURCC('t', 'r', 'a', 'k'), FOURCC('m', 'o', 'o', 'v'), NULL },
    { FOURCC('u', 'd', 't', 'a'), FOURCC('t', 'r', 'a', 'k'), NULL },
    { FOURCC('u', 'd', 't', 'a'), FOURCC('m', 'o', 'o', 'v'), NULL },
    { FOURCC('m', 'e', 't', 'a'), FOURCC('u', 'd', 't', 'a'), NULL },
    { FOURCC('i', 'l', 's', 't'), FOURCC('m', 'e', 't', 'a'), NULL },

    { FOURCC('t', 'k', 'h', 'd'), FOURCC('t', 'r', 'a', 'k'),
        &DashFmp4Parser::parseTrackHeader
    },

    { FOURCC('m', 'v', 'e', 'x'), FOURCC('m', 'o', 'o', 'v'), NULL },

    { FOURCC('t', 'r', 'e', 'x'), FOURCC('m', 'v', 'e', 'x'),
        &DashFmp4Parser::parseTrackExtends
    },

    { FOURCC('e', 'd', 't', 's'), FOURCC('t', 'r', 'a', 'k'), NULL },
    { FOURCC('m', 'd', 'i', 'a'), FOURCC('t', 'r', 'a', 'k'), NULL },

    { FOURCC('m', 'd', 'h', 'd'), FOURCC('m', 'd', 'i', 'a'),
        &DashFmp4Parser::parseMediaHeader
    },

    { FOURCC('h', 'd', 'l', 'r'), FOURCC('m', 'd', 'i', 'a'),
        &DashFmp4Parser::parseMediaHandler
    },

    { FOURCC('m', 'i', 'n', 'f'), FOURCC('m', 'd', 'i', 'a'), NULL },
    { FOURCC('d', 'i', 'n', 'f'), FOURCC('m', 'i', 'n', 'f'), NULL },
    { FOURCC('s', 't', 'b', 'l'), FOURCC('m', 'i', 'n', 'f'), NULL },
    { FOURCC('s', 't', 's', 'd'), FOURCC('s', 't', 'b', 'l'), NULL },



    { FOURCC('a', 'v', 'c', 'C'), FOURCC('e', 'n', 'c', 'v'),
        &DashFmp4Parser::parseAVCCodecSpecificData },

    { FOURCC('e', 's', 'd', 's'), FOURCC('e', 'n', 'c', 'a'),
        &DashFmp4Parser::parseESDSCodecSpecificData },

    { FOURCC('e', 's', 'd', 's'), FOURCC('e', 'n', 'c', 'v'),
        &DashFmp4Parser::parseESDSCodecSpecificData },

    { FOURCC('s', 'i', 'n', 'f'), FOURCC('e', 'n', 'c', 'a'), NULL },
    { FOURCC('s', 'i', 'n', 'f'), FOURCC('e', 'n', 'c', 'v'), NULL },

    { FOURCC('f', 'r', 'm', 'a'), FOURCC('s' ,'i', 'n', 'f'),  
       &DashFmp4Parser::parseOriginalFormat },

    { FOURCC('s','c', 'h', 'm'),FOURCC('s', 'i', 'n', 'f'), 
        &DashFmp4Parser::parseSchemeType  },

    { FOURCC('s','c', 'h', 'i'), FOURCC('s', 'i', 'n', 'f'), NULL  },	

    { FOURCC('t','e', 'n', 'c'), FOURCC('s','c', 'h', 'i'), 
        &DashFmp4Parser::parseTrackEncryption   },		




    { FOURCC('s', 't', 's', 'z'), FOURCC('s', 't', 'b', 'l'),
        &DashFmp4Parser::parseSampleSizes },

    { FOURCC('s', 't', 'z', '2'), FOURCC('s', 't', 'b', 'l'),
        &DashFmp4Parser::parseCompactSampleSizes },

    { FOURCC('s', 't', 's', 'c'), FOURCC('s', 't', 'b', 'l'),
        &DashFmp4Parser::parseSampleToChunk },

    { FOURCC('s', 't', 'c', 'o'), FOURCC('s', 't', 'b', 'l'),
        &DashFmp4Parser::parseChunkOffsets },

    { FOURCC('c', 'o', '6', '4'), FOURCC('s', 't', 'b', 'l'),
        &DashFmp4Parser::parseChunkOffsets64 },

    { FOURCC('a', 'v', 'c', 'C'), FOURCC('a', 'v', 'c', '1'),
        &DashFmp4Parser::parseAVCCodecSpecificData },

    { FOURCC('e', 's', 'd', 's'), FOURCC('m', 'p', '4', 'a'),
        &DashFmp4Parser::parseESDSCodecSpecificData },

    { FOURCC('e', 's', 'd', 's'), FOURCC('m', 'p', '4', 'v'),
        &DashFmp4Parser::parseESDSCodecSpecificData },

    { FOURCC('m', 'd', 'a', 't'), 0, &DashFmp4Parser::parseMediaData },

    { FOURCC('m', 'o', 'o', 'f'), 0, NULL },
    { FOURCC('t', 'r', 'a', 'f'), FOURCC('m', 'o', 'o', 'f'), NULL },

    { FOURCC('t', 'f', 'h', 'd'), FOURCC('t', 'r', 'a', 'f'),
        &DashFmp4Parser::parseTrackFragmentHeader
    },
    { FOURCC('t', 'r', 'u', 'n'), FOURCC('t', 'r', 'a', 'f'),
        &DashFmp4Parser::parseTrackFragmentRun
    },


    { FOURCC('s', 'e', 'n', 'c'), FOURCC('t', 'r', 'a', 'f'),
        &DashFmp4Parser::parseSampleEncryption
    },
    { FOURCC('s', 'a', 'i', 'o'), FOURCC('t', 'r', 'a', 'f'),
        &DashFmp4Parser::parseSampleAuxiliaryInformationOffsets
    },
    { FOURCC('s', 'a', 'i', 'z'), FOURCC('t', 'r', 'a', 'f'),
        &DashFmp4Parser::parseSampleAuxiliaryInformationSize
    },


    { FOURCC('s', 'i', 'd', 'x'), 0, &DashFmp4Parser::parseSegmentIndex },
    

    { FOURCC('p', 's', 's', 'h'), FOURCC('m', 'o', 'o', 'v'), 
        &DashFmp4Parser::parseProtectionSystemSpecificHeader  },
    
    { FOURCC('s', 's', 'i', 'x'), 0, 
        &DashFmp4Parser::parseSubSegmentIndex },

    { FOURCC('m', 'f', 'r', 'a'), 0,  NULL},
    { FOURCC('t', 'f', 'r', 'a'),  FOURCC('m', 'f', 'r', 'a'),   
        &DashFmp4Parser::parseTrackFragmentRandomAccess },
     		
  

};




DashFmp4Parser::DashFmp4Parser()
    : mBufferPos(0),
    mSuspended(false),
    mDoneWithMoov(false),
    mFirstMoofOffset(0),
    mFinalResult(OK),
    mIsDrm(false),
    mHaveTrack(0),	
    mIsFlushing(false),
    mFileMeta(new MetaData),
    mObServerName(""),
    mBaseMediaTimeUs(0)
{

    mBuffer = new ABuffer(512 * 1024);
    mBuffer->setRange(0, 0);
    enter(0ll, 0, 0);  
    ALOGI("new DashFmp4Parser");

}



DashFmp4Parser::~DashFmp4Parser() {
}




void dumpBuffer(uint8_t* buffer, uint32_t size){

	uint32_t tmpsize =0;

    while(size - tmpsize >= 16 ){
		SXLOGV("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
  		buffer[tmpsize + 0],buffer[tmpsize + 1],buffer[tmpsize + 2],buffer[tmpsize + 3],buffer[tmpsize + 4],buffer[tmpsize + 5],buffer[tmpsize + 6],buffer[tmpsize + 7],
  		buffer[tmpsize + 8],buffer[tmpsize + 9],buffer[tmpsize + 10],buffer[tmpsize + 11],buffer[tmpsize + 12],buffer[tmpsize + 13],buffer[tmpsize + 14],buffer[tmpsize + 15]);
		tmpsize +=16;
	}
    
	while(size - tmpsize >= 8 ){
		SXLOGV(" %02x %02x %02x %02x %02x %02x %02x %02x",
  		buffer[tmpsize + 0],buffer[tmpsize + 1],buffer[tmpsize + 2],buffer[tmpsize + 3],buffer[tmpsize + 4],buffer[tmpsize + 5],buffer[tmpsize + 6],buffer[tmpsize + 7]);
		tmpsize +=8;
	}
	while(size - tmpsize >=4 ){
		SXLOGV(" %02x %02x %02x %02x  ",
  		buffer[tmpsize + 0],buffer[tmpsize + 1],buffer[tmpsize + 2],buffer[tmpsize + 3]);
		tmpsize +=4;
	}
	while(size - tmpsize >=2 ){
		SXLOGV(" %02x %02x  ", buffer[tmpsize + 0],buffer[tmpsize + 1]);
		tmpsize +=2;
	}

	while(size - tmpsize >  0 ){
		SXLOGV(" %02x  ", buffer[tmpsize]) ;
		tmpsize +=1;
	}
	
}

void DashFmp4Parser::setUserInfo(const sp<AMessage> &notify, int64_t baseMediaUs, const char* obServerName){
	
	mNotify = notify;
    mObServerName = obServerName;
    mBaseMediaTimeUs= baseMediaUs;
    
    ALOGI("setUserInfo %s  baseMediaUs =%lld",mObServerName,baseMediaUs);
}

status_t  DashFmp4Parser::proceedBuffer(sp<ABuffer> &buffer){
    Mutex::Autolock autoLock(mDataLock);
    SXLOGV("%s %p proceedBuffer mBufferPos %ld (0x%x) size %d offset %d feed %d ++++ ", 
       mObServerName,this,mBufferPos,mBufferPos,mBuffer->size(),mBuffer->offset(),buffer->size());

    dumpBuffer(mBuffer->data(), 8);
    dumpBuffer(buffer->data(), 8);
    
    memmove(mBuffer->base(), mBuffer->data(), mBuffer->size());
    mBuffer->setRange(0, mBuffer->size());
    dumpBuffer(mBuffer->data(), 8);

    size_t maxBytesToRead = mBuffer->capacity() - mBuffer->size();
    size_t needed = buffer->size();
    
    if (maxBytesToRead < needed) {
        ALOGI("%s %p resizing buffer.",mObServerName,this);

        sp<ABuffer> newBuffer =
            new ABuffer((mBuffer->size() + needed + 1023) & ~1023);
        memcpy(newBuffer->data(), mBuffer->data(), mBuffer->size());
        newBuffer->setRange(0, mBuffer->size());

        mBuffer = newBuffer;
    }

    maxBytesToRead = mBuffer->capacity() - mBuffer->size();

    CHECK_GE(maxBytesToRead, needed);    
    memcpy(mBuffer->data() + mBuffer->size(), buffer->data(), buffer->size());

    mBuffer->setRange(0, mBuffer->size() + buffer->size());

    dumpBuffer(mBuffer->data(), 8);
    
    status_t err =OK;
    
    SXLOGD("%s %p 2proceedBuffer mBufferPos %ld (0x%x) size %d offset %d feed %d ++++ ", 
       mObServerName,this,mBufferPos,mBufferPos,mBuffer->size(),mBuffer->offset(),buffer->size());
     
    int64_t starttimeUs = ALooper::GetNowUs();


    while(err == OK){
        err = onProceed();
    }

    int64_t endtimeUs = ALooper::GetNowUs();
    SXLOGD("%s %p proceedBuffer ---offset %d spend %lld us err %d ", mObServerName,this,mBuffer->offset(),endtimeUs-starttimeUs,err);

    return err;

}
status_t DashFmp4Parser::stop(){
	ALOGI("%s %p stop in++++ mHaveTrack=0x%x mDoneWithMoov %d, mFirstMoofOffset 0x%x",
            mObServerName,this,mHaveTrack,mDoneWithMoov,mFirstMoofOffset);
	
	ALOGI("%s[profile] mBufferPos %ld,mBuffer.Size %d,mMediaData size=%d",mObServerName,mBufferPos,mBuffer->size(),mMediaData.size());

	for(size_t i =0 ;i< mTracks.size() ; i++){

		TrackInfo *info = &mTracks.editValueAt(i);
		 ALOGI("%s[profile]  track %d  info->mFragments. size=%d",mObServerName, i,info->mFragments.size());	
	}
    ALOGI("%s[profile]all moof offset %d",mObServerName,mMoofOffsets.size() );
    
    for (int i = 0; i < mMoofOffsets.size(); i++) {
        SXLOGV("\t\t moof mOffset=0x%llx \t",mMoofOffsets.itemAt(i));
    }

    if(!mDoneWithMoov){
    	ALOGI("%s %p stop done- without profile---", mObServerName,this);
        return OK;

    }
    
    ssize_t trackIndex =-1;
    TrackInfo *info =NULL;
    if(mHaveTrack & Track_Audio){
        trackIndex = findTrack(Track_Audio);
        CHECK(trackIndex >= 0);  
        
        info = &mTracks.editValueAt(trackIndex);
        ALOGI("%s[profile]all Track_Audio sidx size %d",mObServerName,info->mSidx.size() );
        int numSidxEntries = info->mSidx.size();
        for (int i = 0; i < numSidxEntries; i++) {
            const SidxEntry *se = &(info->mSidx.itemAt(i));
            SXLOGD("\t\t sidx moof mOffset=0x%llx mSegmetnStartUs =%lldms mSsix %d",
                se->mOffset,se->mSegmentStartUs/1000ll,se->mSsix.size());
        }
    }
    if(mHaveTrack & Track_Video){
        trackIndex =-1;
        trackIndex = findTrack(Track_Video);
        CHECK(trackIndex >= 0);  
        
        info = &mTracks.editValueAt(trackIndex);
        ALOGI("%s[profile]all Track_Video sidx size %d",mObServerName,info->mSidx.size() );
        int numSidxEntries = info->mSidx.size();
        for (int i = 0; i < numSidxEntries; i++) {
            const SidxEntry *se = &(info->mSidx.itemAt(i));
            SXLOGD("\t\t sidx moof mOffset=0x%llx mSegmetnStartUs =%lldms mSsix %d",
                se->mOffset,se->mSegmentStartUs/1000ll,se->mSsix.size());
        }
    }
    if(mHaveTrack & Track_SubTitles){
        trackIndex =-1;
        trackIndex = findTrack(Track_SubTitles);
        if(mDoneWithMoov){
            CHECK(trackIndex >= 0 ); 
        }else{
            ALOGE("%s error not finish parse the moov header",mObServerName);
        }
        if(trackIndex > 0){        
            info = &mTracks.editValueAt(trackIndex);
            ALOGI("%s[profile]all Track_SubTitles sidx size %d",mObServerName,info->mSidx.size() );
            int numSidxEntries = info->mSidx.size();
            for (int i = 0; i < numSidxEntries; i++) {
                const SidxEntry *se = &(info->mSidx.itemAt(i));
                ALOGI("\t\t sidx moof mOffset=0x%llx mSegmetnStartUs =%lldms mSsix %d",
                    se->mOffset,se->mSegmentStartUs/1000ll,se->mSsix.size());
            }
        }
    }
    
	ALOGI("%s %p stop done----", mObServerName,this);
	return OK;
}


status_t DashFmp4Parser::flush(sp<AMessage> &flushMsg){
	ALOGI("%s %p flush in++++",mObServerName,this);
	Mutex::Autolock autoLock(mDataLock);
	
	status_t err=OK;

    //clear data 
    
    mStack.clear();// ro else fitContainer will error after flush
    enter(0ll, 0, 0);
    
	mMediaData.clear();

	for(uint32_t i =0 ;i< mTracks.size(); i++){
		TrackInfo *info = &mTracks.editValueAt(i);
	   	 info->mFragments.clear();
	}

	mBuffer->setRange(0,0);
	mBufferPos = 0;

    //reset info
    int32_t StartSegmentIndex =-1;
    
	if(flushMsg->findInt32("videoStartSegmentIndex",&StartSegmentIndex)
        && StartSegmentIndex > 0){        
        if((err = setTrackStartDecodeTime(Track_Video ,StartSegmentIndex))!= OK){
               return err; 
        }    
    }
    StartSegmentIndex =-1;
    
    if(flushMsg->findInt32("audioStartSegmentIndex",&StartSegmentIndex)
        && StartSegmentIndex > 0){        
        if((err = setTrackStartDecodeTime(Track_Audio,StartSegmentIndex))!= OK){
               return err; 
        }
    }
    StartSegmentIndex =-1;
    if(flushMsg->findInt32("subTitleStartSegmentIndex",&StartSegmentIndex)
        && StartSegmentIndex > 0){        
        if((err = setTrackStartDecodeTime(Track_SubTitles,StartSegmentIndex))!= OK){
               return err; 
        }
    }
    
	ALOGI("%s onflush done: mMediaData size=%d,mBuffer size=%d,mBufferPos=%ld(0x%lx)",
			 mObServerName,mMediaData.size(),mBuffer->size(),mBufferPos,mBufferPos);
	ALOGI("%s %p flush in-----",mObServerName,this);
	return OK;	 
}


status_t DashFmp4Parser::setTrackStartDecodeTime(
        uint32_t trackType,uint32_t 
        startDecoderSegmentIndex){

        ssize_t trackIndex =-1;
        TrackInfo *info =NULL; 
        int64_t totalTime =0;
        if(mHaveTrack & trackType){
            trackIndex = findTrack(trackType);
            CHECK(trackIndex >= 0);  
            
            info = &mTracks.editValueAt(trackIndex);        
            int numSidxEntries = info->mSidx.size();
            CHECK(startDecoderSegmentIndex < numSidxEntries);
            const SidxEntry *se = &info->mSidx[startDecoderSegmentIndex];
            info->mStartDecodeSegmentIndex = startDecoderSegmentIndex;
            info->mBaseMediaTimeUs = se->mSegmentStartUs;
            info->mDecodingTime = 0;
            ALOGI("%s setTrackStartDecode index =%d/%d base Time %lldus done", mObServerName,startDecoderSegmentIndex,numSidxEntries,info->mBaseMediaTimeUs);
            return OK;
        }
        ALOGE("%s setTrackStartDecodeTime error trackType %d  ", mObServerName,trackType);
        return INVALID_OPERATION;

}

bool DashFmp4Parser::hasAvaliBuffer(){

	bool hasBuffer = false;
	for(size_t i =0 ;i< mTracks.size() ; i++){

		TrackInfo *info = &mTracks.editValueAt(i);
		 if(!(info->mFragments.empty())){
			hasBuffer = true;
            ALOGI("%s[profile]parser track %d has mFragments %d",mObServerName,i,info->mFragments.size());
			break;
		 }	
	}

	ALOGI("%s [profile]parser has mediaData %d",mObServerName,mMediaData.size());
	return(( !mMediaData.empty()) || hasBuffer);
}



void DashFmp4Parser::notifyObserver(int32_t whatInfo){
    if(mNotify == NULL){

        ALOGI("%s not observer, not notify anyone",mObServerName);
        return;
    }
    ALOGI("%s notify observer, %s ",mObServerName,Fourcc2String(whatInfo));
    sp<AMessage> notify = mNotify->dup();
    switch(whatInfo){
		case kWhatSidx:
		{
			notify->setInt32("what", kWhatSidx);
			if(mHaveTrack & Track_Video){	
				ssize_t trackIndex = findTrack(Track_Video);
				CHECK_GE(trackIndex , 0) ;
				TrackInfo *info = &mTracks.editValueAt(trackIndex);
				if( info->mSidx.size() > 0 ){
					  ALOGI("%s kWhatSidx,add video",mObServerName);
					  notify->setPointer("video-sidx-vector", &(info->mSidx));	  
				}
			 }
			if(mHaveTrack & Track_Audio){	
				ssize_t trackIndex = findTrack(Track_Audio);
				CHECK_GE(trackIndex , 0) ;
				TrackInfo *info = &mTracks.editValueAt(trackIndex);
				if( info->mSidx.size() > 0 ){
					ALOGI("%s kWhatSidx,add audio",mObServerName);
					notify->setPointer("audio-sidx-vector", & (info->mSidx));	
				}
			 }
			if(mHaveTrack & Track_SubTitles){	
				ssize_t trackIndex = findTrack(Track_SubTitles);
				CHECK_GE(trackIndex , 0) ;
				TrackInfo *info = &mTracks.editValueAt(trackIndex);
				if( info->mSidx.size() > 0 ){
					ALOGI("%s kWhatSidx,add SubTitles",mObServerName);
					notify->setPointer("SubTitle-sidx-vector", & (info->mSidx));	
				}
			 }
			notify->post();
			break;
		}
		case kWhatFileMeta: 
		{
			 notify->setInt32("what", kWhatFileMeta);
			 notify->setObject("fileMeta",  mFileMeta);	  
			 notify->post();
			break;
		}
		case kWhatError: 
		{
			 notify->setInt32("what", kWhatError);
			 notify->setInt32("err",  mFinalResult);	  
			 notify->post();
			break;
		}
		default:
			TRESPASS();                                                                                                                                                                                                                                                                                                             
	}
	
}

status_t DashFmp4Parser::checkTrackValid(uint32_t trackType){
	status_t err = OK;
       if( !mDoneWithMoov){
		ALOGV("%s checkTrackValid err: moov is not parsed",mObServerName);
		return -EAGAIN;
	}else{
		if((trackType == Track_Audio && !(mHaveTrack & Track_Audio) )
	       || (trackType == Track_Video && !(mHaveTrack & Track_Video) )
	       || (trackType == Track_SubTitles && !(mHaveTrack & Track_SubTitles))){
			 err = -EINVAL;
			 ALOGD("[error]%s no %d track VS mHaveTrack 0x%x,checkTrackValid err %d",mObServerName,trackType,mHaveTrack,err);
	      }
	}
	return err;

}
sp<AMessage>  DashFmp4Parser::getFormat(uint32_t trackType){
	status_t err = OK;

    ALOGD("%s %p getFormat+++",mObServerName,this);
	if((err = checkTrackValid(trackType)) != OK){
		ALOGD("%s no %d track VS mHaveTrack 0x%x,getFormat fail err %d",mObServerName,trackType,mHaveTrack,err);
		return NULL;
	}

    int32_t trackIndex = findTrack(trackType);

    if (trackIndex < 0) {
        ALOGD("%s no %d track VS mHaveTrack 0x%x,getFormat findTrack err  ",mObServerName,trackType,mHaveTrack);
		
        return NULL;
    } 
    TrackInfo *info = &mTracks.editValueAt(trackIndex);

    sp<AMessage> format = info->mSampleDescs.itemAt(0).mFormat;

	ALOGD("%s %p getFormat:returning format %s",mObServerName, this,format->debugString().c_str());
	return format;

}


ssize_t DashFmp4Parser::findTrack(uint32_t trackType)  {
    CHECK(trackType == Track_Video || trackType == Track_Audio || trackType == Track_SubTitles);
	
    for (size_t i = 0; i < mTracks.size(); ++i) {
        const TrackInfo *info = &mTracks.valueAt(i);

        bool isAudio =
            info->mMediaHandlerType == FOURCC('s', 'o', 'u', 'n');

        bool isVideo =
            info->mMediaHandlerType == FOURCC('v', 'i', 'd', 'e');

	    bool isSubtile =
            info->mMediaHandlerType == FOURCC('s', 'u', 'b', 't');

	

        if (((trackType == Track_Audio) &&  isAudio)
	         ||((trackType == Track_Video) &&  isVideo)
	         ||((trackType == Track_SubTitles) &&  isSubtile)) {
            if (info->mSampleDescs.empty()) {
                break;
            }

            return i;
        }
    }	
    ALOGI("[error]%s %p findTrack %x track fail",mObServerName,this,trackType ); 
  
    return -EAGAIN;
}
status_t DashFmp4Parser::parseSubtitleSampleEntry(
        uint32_t type, size_t offset, uint64_t size) {
        
    if (offset +  8 > size) {			
	 ALOGD("%s %s err1  ",mObServerName,__FUNCTION__);
       	 return -EINVAL;
    }

    TrackInfo *trackInfo = editTrack(mCurrentTrackID);

    trackInfo->mSampleDescs.push();
    SampleDescription *sampleDesc =
        &trackInfo->mSampleDescs.editItemAt(
                trackInfo->mSampleDescs.size() - 1);

    sampleDesc->mType = type;
    sampleDesc->mDataRefIndex = readU16(offset + 6);

    sp<AMessage> format = new AMessage;

    switch (type) {
        case FOURCC('s', 't', 'p', 'p'):
	    ALOGI("%s parseSubtitleSampleEntry 'stpp' ",mObServerName);
            format->setString("mime", "image/png");//will modify
            break;
			
        default:
            format->setString("mime", "application/octet-stream");
            break;
    }

   // format->setInt32("width", readU16(offset + 8 + 16));
 //   format->setInt32("height", readU16(offset + 8 + 18));
     ALOGI("%s by pass subtile",mObServerName);

    sampleDesc->mFormat = format;

    return OK;
}
status_t DashFmp4Parser::parseTrackFragmentRandomAccess(
            uint32_t type, size_t offset, uint64_t size){
	ALOGD( "parseTrackFragmentRandomAccess");

    if (size < 12) {
	 ALOGE("%s err 1%s ",__FUNCTION__,mObServerName);
         return -EINVAL;
    }
    uint32_t flags;
    flags =  readU32(offset);
    uint32_t version = flags >> 24;
    flags &= 0xffffff;
    ALOGI("mfra version %d", version);

    //get track id
    uint32_t trackid = readU32(offset+4) ;	
    ALOGI("trackid :%d", trackid);

    // get length size
    uint32_t tmpLen = readU32(offset+8 );    
    uint8_t length_size_of_traf_num = (tmpLen & 0x3F) >> 4;
    uint8_t length_size_of_trun_num = (tmpLen & 0x0C) >> 2;
    uint8_t length_size_of_sample_num = (tmpLen & 0x03);

    // get entry
    uint32_t number_of_entry = readU32(offset+12 );
      
    offset += 16;
    ALOGI("mfra mumber_of_entry:%d", number_of_entry);
    for(size_t i=0; i<number_of_entry; i++) {
        MfraEntry me;
        me.mTrackID = trackid;
        uint32_t size = (length_size_of_traf_num+1) + (length_size_of_trun_num+1) + (length_size_of_sample_num+1); 
        uint8_t *data = NULL;
        uint8_t *buf = NULL;
        if(version == 1){
            me.mTime = readU64( offset);
            me.mMoofOffset =  readU64( offset + 8);
            offset += 8;
        } else {
            me.mTime = readU32( offset);
            me.mMoofOffset =  readU32( offset + 4);
            offset += 4;
        }

        me.mTrafNumber = readUInt(offset, length_size_of_traf_num+1);
        offset += (length_size_of_traf_num+1);
        me.mTrunNumber = readUInt(offset, (length_size_of_trun_num+1));
        offset += (length_size_of_trun_num+1);
        me.mSampleNumber = readUInt(offset, (length_size_of_sample_num+1));
        offset += (length_size_of_sample_num+1);

        ALOGI("mfra : trackid %d mTime %lld mMoofOffset 0x%llx  mumber_of_entry:%d,mTrafNumber %d mTrunNumber %d mSampleNumber %d ", 
        trackid,me.mTime,me.mMoofOffset,number_of_entry,me.mTrafNumber,me.mTrunNumber,me.mSampleNumber);

        mMfraEntries.add(me.mTrackID,me);
    }
    return OK;
}

bool DashFmp4Parser::isSeekable() const {
    return OK;
}

status_t DashFmp4Parser::seekTo(bool wantAudio, int64_t timeUs) {
    return OK;
}

status_t DashFmp4Parser::onProceed() {
    status_t err;    

    if ((err = need(8)) != OK) {
			
        if(err == -EINVAL) ALOGE("%s err1  %s ",__FUNCTION__,mObServerName);
		  	
        return err;
    }

    uint64_t size = readU32(0);
    uint32_t type = readU32(4);

	
	SXLOGD("%s onProcess:mBuffesPos=%ld (0x%lx), mBuffer:size=%d,  entering box size %lld , box of type '%s' ",
			mObServerName,mBufferPos,mBufferPos,mBuffer->size() ,size, Fourcc2String(type));
    dumpBuffer(mBuffer->data(), 8);


    size_t offset = 8;

    if (size == 1) {
        if ((err = need(16)) != OK) {
			
            if(err == -EINVAL) ALOGE("%s %s err2  ",mObServerName,__FUNCTION__);
			
            return err;
        }

        size = readU64(offset);
        offset += 8;
    }

    uint8_t userType[16];

    if (type == FOURCC('u', 'u', 'i', 'd')) {
        if ((err = need(offset + 16)) != OK) {
			
            if(err == -EINVAL) ALOGE("%s %s err3  ",mObServerName,__FUNCTION__);
			
            return err;
        }

        memcpy(userType, mBuffer->data() + offset, 16);
        offset += 16;
    }

    CHECK(!mStack.isEmpty());
    uint32_t ptype = mStack.itemAt(mStack.size() - 1).mType;

    static const size_t kNumDispatchers =
        sizeof(kDispatchTable) / sizeof(kDispatchTable[0]);

    size_t i  ;
    for (i = 0; i < kNumDispatchers; ++i) {
        if (kDispatchTable[i].mType == type
                && kDispatchTable[i].mParentType == ptype) {
            break;
        }
    }

    // SampleEntry boxes are container boxes that start with a variable
    // amount of data depending on the media handler type.
    // We don't look inside 'hint' type SampleEntry boxes.

    bool isSampleEntryBox =
        (ptype == FOURCC('s', 't', 's', 'd'))
        && editTrack(mCurrentTrackID)->mMediaHandlerType
        != FOURCC('h', 'i', 'n', 't');

    if ((i < kNumDispatchers && kDispatchTable[i].mHandler == 0)
            || isSampleEntryBox || ptype == FOURCC('i', 'l', 's', 't')) {
        // This is a container box.
        if (type == FOURCC('m', 'o', 'o', 'f')) {
            if (mFirstMoofOffset == 0) {               
                mFirstMoofOffset = mBufferPos + offset - 8; // point at the size
	                
                ALOGI("%s [profile]first moof @ 0x%lx",mObServerName, mFirstMoofOffset );
                notifyObserver(kWhatSidx);
	
            }

        }
        if (type == FOURCC('m', 'e', 't', 'a')) {
            if ((err = need(offset + 4)) < OK) {
			
                if(err == -EINVAL) ALOGE("%s %s err4  ",mObServerName,__FUNCTION__);
		
                return err;
            }

            if (readU32(offset) != 0) {
			
                ALOGE("%s %s err5  ",mObServerName,__FUNCTION__);

                return -EINVAL;
            }

            offset += 4;
        } else if (type == FOURCC('s', 't', 's', 'd')) {
            if ((err = need(offset + 8)) < OK) {
				
			
                if(err == -EINVAL) ALOGE("%s %s err6  %d  ",mObServerName,__FUNCTION__,err);
			
                return err;
            }

            if (readU32(offset) != 0) {
			
                ALOGE("%s %s err7 readU32 %d ",mObServerName,__FUNCTION__,readU32(offset));
				
                //return -EINVAL; //for ALPS01468037.remove this check, not influence the data 
            }

            if (readU32(offset + 4) == 0) {
                // We need at least some entries.
			
	    	    ALOGE("%s err8 no entry in stsd %s",__FUNCTION__,mObServerName);
      
                return -EINVAL;
            }

            offset += 8;
        } else if (isSampleEntryBox) {
            size_t headerSize;

            switch (editTrack(mCurrentTrackID)->mMediaHandlerType) {
                case FOURCC('v', 'i', 'd', 'e'):
                {
                    // 8 bytes SampleEntry + 70 bytes VisualSampleEntry
                    headerSize = 78;
                    break;
                }

                case FOURCC('s', 'o', 'u', 'n'):
                {
                    // 8 bytes SampleEntry + 20 bytes AudioSampleEntry
                    headerSize = 28;
                    break;
                }

                case FOURCC('m', 'e', 't', 'a'):
                {
                    headerSize = 8;  // 8 bytes SampleEntry
                    break;
                }
			
	    	 case FOURCC('s', 'u', 'b', 't'):
                {
                    headerSize = size -8;  //  size bytes SubtileSampleEntry  
                    break;
                }
   
                default:
                    TRESPASS();
            }

            if (offset + headerSize > size) {
			
	    	    SXLOGD("%s %s err9 offset %d(0x%x),headerSize %d ,size%lld",
	    	    	mObServerName,__FUNCTION__,offset,offset,headerSize,size);
			
                return -EINVAL;
            }

            if ((err = need(offset + headerSize)) != OK) {
			
	    	   if(err == -EINVAL) ALOGE("%s err9 %s",__FUNCTION__,mObServerName);
			
                return err;
            }
            switch (editTrack(mCurrentTrackID)->mMediaHandlerType) {
                case FOURCC('v', 'i', 'd', 'e'):
                {
                    err = parseVisualSampleEntry(
                            type, offset, offset + headerSize);
                    break;
                }
                case FOURCC('s', 'o', 'u', 'n'):
                {
                    err = parseAudioSampleEntry(
                            type, offset, offset + headerSize);
                    break;
                }
                case FOURCC('m', 'e', 't', 'a'):
                {
                    err = OK;
                    break;
                }

		  case FOURCC('s', 'u', 'b', 't'):
                {
                     err = parseSubtitleSampleEntry(
                            type, offset, offset + headerSize);
                    break;
                }

		

                default:
                    TRESPASS();
            }

            if (err != OK) {
			
	    	   SXLOGV("%s err10 %s",__FUNCTION__,mObServerName);
		
                return err;
            }

            offset += headerSize;
        }

        skip(offset);

	if(size - offset > 0){
		ALOGV("%sentering box of type '%s'",
	                IndentString(mStack.size()), Fourcc2String(type));

	        enter(mBufferPos - offset, type, size - offset);
	}
    } else {
        if (!fitsContainer(size)) {
			
	    	 ALOGE("%s err11 %s",__FUNCTION__,mObServerName);
	
            return -EINVAL;
        }

        if (i < kNumDispatchers && kDispatchTable[i].mHandler != 0) {
            // We have a handler for this box type.

            if ((err = need(size)) != OK) {
			
	    	   if(err == -EINVAL) ALOGD("%s err12  %s",__FUNCTION__,mObServerName);
			
                return err;
            }


            ALOGD("%sparsing box of type '%s @mBufferPos=%ld(0x%lx) ,offset =%d(0x%x), size=%lld' %s",
                    IndentString(mStack.size()), Fourcc2String(type),
                    mBufferPos,mBufferPos,offset, offset,size,mObServerName);

            if ((err = (this->*kDispatchTable[i].mHandler)(
                            type, offset, size)) != OK) {
			
	    	   ALOGD("%s err13  %s",__FUNCTION__,mObServerName);
                           
                return err;
            }
        } else {
            // Unknown box type

            ALOGD("%sskipping box of type '%s', size %llu,%s",
                    IndentString(mStack.size()),
                    Fourcc2String(type), size,mObServerName);

        }

        skip(size);
    }

    return OK;
}

// static,no relate a instance.save size?
int DashFmp4Parser::CompareSampleLocation(
        const SampleInfo &sample, const MediaDataInfo &mdatInfo) {
    if (sample.mOffset + sample.mSize < mdatInfo.mOffset) {
			
	     ALOGD("%s err1 ",__FUNCTION__);
		
        return -1;
    }

    if (sample.mOffset >= mdatInfo.mOffset + mdatInfo.mBuffer->size()) {
        return 1;
    }

    // Otherwise make sure the sample is completely contained within this
    // media data block.

    CHECK_GE(sample.mOffset, mdatInfo.mOffset);

    CHECK_LE(sample.mOffset + sample.mSize,
             mdatInfo.mOffset + mdatInfo.mBuffer->size());

    return 0;
}


status_t DashFmp4Parser::getSample(
        TrackInfo *info, sp<TrackFragment> *fragment, SampleInfo *sampleInfo) {
    for (;;) {
        if (info->mFragments.empty()) {
	   
            if (mFinalResult != OK) {
			
                ALOGD("%s %s err1  mFinalResult=%d",mObServerName,__FUNCTION__,mFinalResult);

                return mFinalResult;
            }
	
	
            SXLOGV("%s err2  EWOULDBLOCK %s",__FUNCTION__,mObServerName);

            

            return -EWOULDBLOCK;
        }

        *fragment = *info->mFragments.begin();

        status_t err = (*fragment)->getSample(sampleInfo);

        if (err == OK) {
            return OK;
        } else if (err != ERROR_END_OF_STREAM) {
			
            ALOGE("%s err3 %d %d ",__FUNCTION__,err,mObServerName);

            return err;
        }

        // Really, end of this fragment...

        info->mFragments.erase(info->mFragments.begin());
    }
}

status_t DashFmp4Parser::dequeueAccessUnit(
        size_t trackType, sp<ABuffer> *accessUnit) {


    status_t err = OK;   
    if((err = checkTrackValid(trackType)) != OK){
        ALOGD("%s no %d track VS mHaveTrack 0x%x,dequeueAccessUnit fail err %d",mObServerName,trackType,mHaveTrack,err);
        return err;
    }

    int32_t trackIndex = findTrack(trackType);

    if (trackIndex < 0) {
        ALOGD("%s dequeueAccessUnit fail trackIndex=%d",mObServerName,trackIndex);
        return -EAGAIN;
    } 


    err = OK;   
        
    TrackInfo *info = &mTracks.editValueAt(trackIndex);

    sp<TrackFragment> fragment;
    SampleInfo sampleInfo;
    err = getSample(info, &fragment, &sampleInfo);

    if (err != OK) {
        return err;
    }

    err = -EWOULDBLOCK;

    bool checkDroppable = false;

    for (size_t i = 0; i < mMediaData.size(); ++i) {
        const MediaDataInfo &mdatInfo = mMediaData.itemAt(i);

        int cmp = CompareSampleLocation(sampleInfo, mdatInfo);


        if (cmp < 0) 	
        {	
            ALOGE("[error]%s %s err1 %d ",mObServerName,__FUNCTION__,-EPIPE);
            
            return -EPIPE;
        } else if (cmp == 0) {
            if (i > 0) {
                checkDroppable = true;
            }

            err = makeAccessUnit(info, sampleInfo, mdatInfo, accessUnit);
            break;
        }
    }

    if (err != OK) {
			
	     SXLOGV("%s %s err2 %d ",mObServerName,__FUNCTION__,err);

        return err;
    }

    fragment->advance();

    if (!mMediaData.empty() && checkDroppable) {
        size_t numDroppable = 0;
        bool done = false;

        // XXX FIXME: if one of the tracks is not advanced (e.g. if you play an audio+video
        // file with sf2), then mMediaData will not be pruned and keeps growing
        for (size_t i = 0; !done && i < mMediaData.size(); ++i) {
            const MediaDataInfo &mdatInfo = mMediaData.itemAt(i);

            for (size_t j = 0; j < mTracks.size(); ++j) {
                TrackInfo *info = &mTracks.editValueAt(j);

                sp<TrackFragment> fragment;
                SampleInfo sampleInfo;
                err = getSample(info, &fragment, &sampleInfo);

                if (err != OK) {
                    done = true;
                    break;
                }

                int cmp = CompareSampleLocation(sampleInfo, mdatInfo);

                if (cmp <= 0) {
                    done = true;
                    break;
                }
            }

            if (!done) {
                ++numDroppable;
            }
        }

        if (numDroppable > 0) {
            mMediaData.removeItemsAt(0, numDroppable);

            if (mMediaData.size() < 5) {
               
            }
        }
    }

    return err;
}

static size_t parseNALSize(size_t nalLengthSize, const uint8_t *data) {
    switch (nalLengthSize) {
        case 1:
            return *data;
        case 2:
            return U16_AT(data);
        case 3:
            return ((size_t)data[0] << 16) | U16_AT(&data[1]);
        case 4:
            return U32_AT(data);
    }

    // This cannot happen, mNALLengthSize springs to life by adding 1 to
    // a 2-bit integer.
    TRESPASS();

    return 0;
}

status_t DashFmp4Parser::makeAccessUnit(
        TrackInfo *info,
        const SampleInfo &sample,
        const MediaDataInfo &mdatInfo,
        sp<ABuffer> *accessUnit) {
    if (sample.mSampleDescIndex < 1
            || sample.mSampleDescIndex > info->mSampleDescs.size()) {
			
		ALOGE("%s return err1 ERROR_MALFORMED %s",__FUNCTION__,mObServerName);	
	            
        return ERROR_MALFORMED;
    }

    int64_t presentationTimeUs =
        1000000ll * sample.mPresentationTime / info->mMediaTimeScale;

			
	presentationTimeUs += info->mBaseMediaTimeUs;
	    

    const SampleDescription &sampleDesc =
        info->mSampleDescs.itemAt(sample.mSampleDescIndex - 1);

    size_t nalLengthSize;
    if (!sampleDesc.mFormat->findSize("nal-length-size", &nalLengthSize)) {
        *accessUnit = new ABuffer(sample.mSize);

        memcpy((*accessUnit)->data(),
               mdatInfo.mBuffer->data() + (sample.mOffset - mdatInfo.mOffset),
               sample.mSize);

        (*accessUnit)->meta()->setInt64("timeUs", presentationTimeUs);
        if (IsIDR(*accessUnit)) {
            (*accessUnit)->meta()->setInt32("is-sync-frame", 1);
        }


        //add the encrypt info
        bool isEncrypted =info->mIsEncrpyted;
        if(isEncrypted){
            ALOGD("%s [dash drm]add sample drm meta info whithout NAL ", mObServerName);
            TencInfo* Tenc = &(info->mTenc);
            sp<MetaData> meta =new MetaData;
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
     }


        return OK;
    }

    const uint8_t *srcPtr =
        mdatInfo.mBuffer->data() + (sample.mOffset - mdatInfo.mOffset);
    //chage NAL_size+Nal data to 0000001+Nal data
    for (int i = 0; i < 2 ; ++i) {
        size_t srcOffset = 0;
        size_t dstOffset = 0;

        while (srcOffset < sample.mSize) {
            if (srcOffset + nalLengthSize > sample.mSize) {
			
                ALOGE("%s return err2 ERROR_MALFORMED %s",__FUNCTION__,mObServerName);
			
                return ERROR_MALFORMED;
            }

            size_t nalSize = parseNALSize(nalLengthSize, &srcPtr[srcOffset]);
            srcOffset += nalLengthSize;

            if (srcOffset + nalSize > sample.mSize) {
			
                ALOGE("%s return err3 ERROR_MALFORMED %s",__FUNCTION__,mObServerName);	
			
                return ERROR_MALFORMED;
            }

            if (i == 1) {
                memcpy((*accessUnit)->data() + dstOffset,
                       "\x00\x00\x00\x01",
                       4);

                memcpy((*accessUnit)->data() + dstOffset + 4,
                       srcPtr + srcOffset,
                       nalSize);
            }

            srcOffset += nalSize;
            dstOffset += nalSize + 4;
        }

        if (i == 0) {
            (*accessUnit) = new ABuffer(dstOffset);
            (*accessUnit)->meta()->setInt64(
                    "timeUs", presentationTimeUs);
        }
    }
    if (IsIDR(*accessUnit)) {
        (*accessUnit)->meta()->setInt32("is-sync-frame", 1);
    }

        //add the encrypt info
        bool isEncrypted =info->mIsEncrpyted;
        if(isEncrypted){
            ALOGD("%s[dash drm]add sample drm meta info whithout NAL",mObServerName);
            TencInfo* Tenc = &(info->mTenc);
            sp<MetaData> meta =new MetaData;
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
     }

    return OK;
}

status_t DashFmp4Parser::need(size_t size) {
    if (!fitsContainer(size)) {
			
		ALOGE("%s fitsContainer err %s",__FUNCTION__,mObServerName);	
	
        return -EINVAL;
    }

    if (size <= mBuffer->size()) {
        return OK;
    }
    return -EAGAIN;
}

void DashFmp4Parser::enter(off64_t offset, uint32_t type, uint64_t size) {
    Container container;
    container.mOffset = offset;
    container.mType = type;
    container.mExtendsToEOF = (size == 0);
    container.mBytesRemaining = size;
			
    SXLOGV("[box trace]entering box of type '%s',position=%lld(0x%llx),remain size=%lld",Fourcc2String(type),offset,offset,size);
	
    
    mStack.push(container);
}

bool DashFmp4Parser::fitsContainer(uint64_t size) const {
    CHECK(!mStack.isEmpty());
    const Container &container = mStack.itemAt(mStack.size() - 1);

			
    SXLOGV("%s fitsContainer box of type '%s'.EOF=%d,offset   %lld (0x%lx), size %lld , Remaining=%lld",
    mObServerName,Fourcc2String(container.mType),   container.mExtendsToEOF,container.mOffset, container.mOffset,   size ,container.mBytesRemaining);
  

    return container.mExtendsToEOF || size <= container.mBytesRemaining;
}

uint16_t DashFmp4Parser::readU16(size_t offset) {
    CHECK_LE(offset + 2, mBuffer->size());

    const uint8_t *ptr = mBuffer->data() + offset;
    return (ptr[0] << 8) | ptr[1];
}

uint32_t DashFmp4Parser::readU32(size_t offset) {
    CHECK_LE(offset + 4, mBuffer->size());

    const uint8_t *ptr = mBuffer->data() + offset;
    return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
}

uint64_t DashFmp4Parser::readU64(size_t offset) {
    return (((uint64_t)readU32(offset)) << 32) | readU32(offset + 4);
}

	

uint8_t DashFmp4Parser::readU8(size_t offset) {
    CHECK_LE(offset + 1, mBuffer->size());

    const uint8_t *ptr = mBuffer->data() + offset;
    return ptr[0];
}
uint32_t DashFmp4Parser::readU24(size_t offset) {
    CHECK_LE(offset + 3, mBuffer->size());

    const uint8_t *ptr = mBuffer->data() + offset;
    return (ptr[0] << 16) | (ptr[1] << 8) | (ptr[2])  ;
}
uint32_t DashFmp4Parser::readUInt(size_t offset, int size) {
    CHECK_LE(offset + size, mBuffer->size());
    switch (size) {
	case 1:
        return readU8(offset);         
	case 2:
        return readU16(offset); 
	case 3:
	    return readU24(offset); 
	case 4:
        return readU32(offset); 
	default:
	    CHECK(!"readUXX:Should not be here.");
	    break;
    }
    return 0;
}

status_t DashFmp4Parser::parseSubSegmentIndex(
            uint32_t type, size_t offset, uint64_t size){            

    ALOGI("%s ssix box type %s, offset %d, size %d",mObServerName,Fourcc2String(type), int(offset), int(size));
    if (offset + 8 > size) {		
        ALOGD("%s err1  %s ",__FUNCTION__,mObServerName);	
        return -EINVAL;
    }

    uint32_t flags = readU32(offset);
    uint32_t version = flags >> 24;
    flags &= 0xffffff;
    ALOGI("%s ssix version %d", mObServerName,version);

    uint32_t subsegment_count = readU32(offset + 4);
    ALOGI("%s ssix subsegment_count %d", mObServerName,subsegment_count);
    offset += 8 ;

    if (offset + subsegment_count * 4 > size) {
        ALOGD("%s %s err2  ",mObServerName,__FUNCTION__);
        return -EINVAL;
    }
    TrackInfo *info = editTrack(mCurrentTrackID);
    int numSidxEntries = info->mSidx.size();
    CHECK_LE(subsegment_count,numSidxEntries);
	XLOGD("%s [profile]current ssix from sidx index %d",mObServerName,numSidxEntries - subsegment_count);        
    for (size_t i = 0; i < subsegment_count; i++) {
        int32_t sidxIndex = numSidxEntries - subsegment_count + i;        
        SidxEntry *sidxE = &(info->mSidx.editItemAt(sidxIndex));
        
        uint32_t ranges_count = readU32(offset);
        offset += 4 ;	
    	if (offset + ranges_count * 4 > size) {
            ALOGD("%s err2  %s ",__FUNCTION__,mObServerName);
            return -EINVAL;
    	}
        for (size_t j = 0; j < ranges_count; j++) {
            uint8_t level = readU8(offset );//see `leva¡¯ assignment_type: Movie Extends Box (`mvex¡¯)
            uint32_t size = readU24(offset + 1);
            SXLOGV("ssix item [%d,%d],  %08x %08x", i, j, level, size);
            SsixEntry ssixE;
            ssixE.mLevel = level;
            ssixE.mRange_Size = size;
            sidxE->mSsix.add(ssixE);
            offset += 4;
        }
        
    }
    return OK;

}

status_t  DashFmp4Parser::parseProtectionSystemSpecificHeader(
	        uint32_t type, size_t offset, uint64_t size){
    if (offset + 24 > size) {	
        ALOGD("%s %s err1  ",mObServerName,__FUNCTION__);
        return -EINVAL;
    }
    PsshInfo mPssh;
    
    for(size_t i= 0; i< 16; i++){
        mPssh.mSystemId[i] = readU8( offset +4 + i);
    }
    ALOGD("%s [dash drm]PSSH INFO: systemID and data",mObServerName);    
    dumpBuffer(mPssh.mSystemId,  16);
    offset += 20;    
    mPssh.mDataSize = readU32( offset);
    offset += 4;  
    mPssh.mData =NULL;
    ALOGD("%s[dash drm]PSSH INFO: size =%d",mObServerName,mPssh.mDataSize);
    mPssh.mData = (uint8_t*)malloc(mPssh.mDataSize);
    CHECK(mPssh.mData != NULL);
    
    for(size_t i= 0; i< mPssh.mDataSize; i++){
        mPssh.mData[i] = readU8( offset + i);
    }
    dumpBuffer(mPssh.mData,  mPssh.mDataSize);

    if (mPssh.mDataSize > 0 ) {
        char *buf = (char*)malloc(mPssh.mDataSize + 20);
        char *ptr = buf;
        memcpy(ptr, mPssh.mSystemId, 16); // uuid 
        memcpy(ptr +16 , mPssh.mData, 4); //+ length
        memcpy(ptr + 20, mPssh.mData, mPssh.mDataSize);
        //16byte UUID+4 byte size+ size data
        mFileMeta->setData(kKeyPssh, 'pssh', buf, mPssh.mDataSize + 20);
        free(buf);
    }
    free(mPssh.mData);

    return OK;

}
status_t  DashFmp4Parser::parseOriginalFormat(
    uint32_t type, size_t offset, uint64_t size){
    
    if (offset + 4 > size) {	
        ALOGD("%s err1  %s ",__FUNCTION__,mObServerName);
        return -EINVAL;
    }
    TrackInfo *trackInfo = editTrack(mCurrentTrackID);

    SampleDescription *sampleDesc =
        &trackInfo->mSampleDescs.editItemAt(trackInfo->mSampleDescs.size()- 1);
   
    sampleDesc->mType = readU32(offset );

    ALOGI("%s [dash drm]parseOriginalFormat '%s'",mObServerName,Fourcc2String( sampleDesc->mType));

    sp<AMessage> format = sampleDesc->mFormat;
	
   
    switch (sampleDesc->mType) {
        case FOURCC('a', 'v', 'c', '1'):
            format->setString("mime", MEDIA_MIMETYPE_VIDEO_AVC);
            break;
        case FOURCC('m', 'p', '4', 'v'):
            format->setString("mime", MEDIA_MIMETYPE_VIDEO_MPEG4);
            break;
        case FOURCC('s', '2', '6', '3'):
        case FOURCC('h', '2', '6', '3'):
        case FOURCC('H', '2', '6', '3'):
            format->setString("mime", MEDIA_MIMETYPE_VIDEO_H263);
            break;
	 case FOURCC('m', 'p', '4', 'a'):
            format->setString("mime", MEDIA_MIMETYPE_AUDIO_AAC);
            break;

        case FOURCC('s', 'a', 'm', 'r'):
            format->setString("mime", MEDIA_MIMETYPE_AUDIO_AMR_NB);
            format->setInt32("channel-count", 1);
            format->setInt32("sample-rate", 8000);
            break;

        case FOURCC('s', 'a', 'w', 'b'):
            format->setString("mime", MEDIA_MIMETYPE_AUDIO_AMR_WB);
            format->setInt32("channel-count", 1);
            format->setInt32("sample-rate", 16000);
            break;
        default:
            format->setString("mime", "application/octet-stream");
            break;
    }
		
    ALOGD("%s [profile]:format update %s ",__FUNCTION__,mObServerName);
   
	return OK;
}
status_t  DashFmp4Parser::parseSchemeType(
            uint32_t type, size_t offset, uint64_t size){			
    if (offset + 12 > size) {	
        ALOGD("%s err1  %s ",__FUNCTION__,mObServerName);
        return -EINVAL;
    }

    uint32_t scheme_type   = readU32(offset + 4 );
    uint32_t scheme_version   = readU32(offset + 8 );

    ALOGI("%s [dash drm]parseSchemeType mCurrentTrackID %d  scheme_type '%s',scheme_version %d ", 
    	mObServerName,mCurrentTrackID,Fourcc2String(scheme_type),scheme_version);
    return OK;

}
status_t  DashFmp4Parser::parseSchemeInformation(
    uint32_t type, size_t offset, uint64_t size){

    return OK;

}
status_t  DashFmp4Parser::parseTrackEncryption(
            uint32_t type, size_t offset, uint64_t size){
    if (offset + 24 > size) {	
	     ALOGD("%s err1  %s ",__FUNCTION__,mObServerName);
        return -EINVAL;
    }

    TrackInfo *trackInfo = editTrack(mCurrentTrackID);

    TencInfo* tencInfo = &(trackInfo->mTenc);

    tencInfo->mIsEncrypted = readU24( offset +4 );
    tencInfo->mIV_Size   = readU8( offset +4 + 3 );

    ALOGI("%s [dash drm]parse TENC trackID %d,mIsEncrypted 0x%x(0:no,1:AES128:other reser),iv size %d ",
    	mObServerName,mCurrentTrackID, tencInfo->mIsEncrypted , tencInfo->mIV_Size);


    for(size_t i= 0; i< 16; i++){
        tencInfo->mKID[i] = readU8( offset +4 + 3 + 1+ i);
    }
    //check format
    if ((tencInfo->mIsEncrypted == 0 && tencInfo->mIV_Size != 0) ||
    (tencInfo->mIsEncrypted != 0 && tencInfo->mIV_Size == 0)) {
        ALOGE("%s [dash drm][error]unencrypted data must have 0 IV size",mObServerName);
        return ERROR_MALFORMED;
    } else if (tencInfo->mIV_Size != 0 &&
                tencInfo->mIV_Size != 8 &&
                tencInfo->mIV_Size != 16) {
        ALOGE("%s [dash drm][error]encrypted IV size is not in 0/8/16",mObServerName);
        return ERROR_MALFORMED; 
    }
    
    //set format
    
    SampleDescription *sampleDesc =
        &trackInfo->mSampleDescs.editItemAt(trackInfo->mSampleDescs.size()- 1);
    sp<AMessage> format = sampleDesc->mFormat;
    //tenc 
    TencInfo* Tenc = &(trackInfo->mTenc);
    sp<MetaData> meta =new MetaData;  
    
    meta->setInt32(kKeyCryptoMode, tencInfo->mIsEncrypted);
    meta->setInt32(kKeyCryptoDefaultIVSize, Tenc->mIV_Size);    
    meta->setData(kKeyCryptoKey, 'tenc',Tenc->mKID,sizeof(Tenc->mKID));//16 byte    
    format->setObject("tenc", meta);
    //log them
    ALOGD("%s[dash drm]TENC:mCurrentTrackID %d ",mObServerName,mCurrentTrackID);
    dumpBuffer(tencInfo->mKID,16);
    return OK;

}


status_t  DashFmp4Parser::parseSampleEncryption(
            uint32_t type, size_t offset, uint64_t size){

    if (offset + 4 > size) {	
        ALOGD("%s %s err1  ",mObServerName,__FUNCTION__);
        return -EINVAL;
    }
    uint32_t flags = readU32(offset);
    uint32_t version = flags >> 24;
    flags &= 0xffffff;
    bool hasSub = ((flags & 0x000002) == 0x000002)?true:false;
    ALOGD("%s [dash drm]hasSubSampe %d  ",mObServerName,hasSub);
    uint32_t sampleCount = readU32(offset + 4);
    offset += 8;	 

    TrackInfo *info = editTrack(mTrackFragmentHeaderInfo.mTrackID);	
    const sp<TrackFragment> &fragment = *--info->mFragments.end();
    
	uint8_t IvSize= info->mTenc.mIV_Size;	
    for(size_t i =0 ;i < sampleCount;i++){
        SampleInfo sampleInfo;
        status_t err = static_cast<DynamicTrackFragment *>(
                fragment.get())->getSampleByIndex(i,&sampleInfo);
        
		sp<ABuffer> mIV =new ABuffer(IvSize) ; 
		uint8_t tmpdata =0;
        
        for(size_t j =0 ;j < IvSize;j++){
            tmpdata=readU8(offset + j);
            memcpy(mIV->data()+j ,&tmpdata, 1);		
        }
        offset += IvSize;

        dumpBuffer(mIV->data(),IvSize);
        uint16_t mSubsample_count = 0;
            
        if(hasSub){
            mSubsample_count = readU16(offset); 
            offset += 2;
            for(size_t k =0 ;k < mSubsample_count;k++){
                sampleInfo.mBytesOfClearData.push_back(readU16(offset));
                offset += 2;
                sampleInfo.mBytesOfEncryptedData.push_back(readU32(offset));
                offset += 4;
                ALOGV("%s [dash drm]SENC:mCurrentTrackID %d,sub mSubsample_count%2d,c:0x%02x e:0x%04x",
                mObServerName,mTrackFragmentHeaderInfo.mTrackID,mSubsample_count,
                sampleInfo.mBytesOfClearData.editItemAt(k),sampleInfo.mBytesOfEncryptedData.editItemAt(k));
            }
            
        }else{
            sampleInfo.mBytesOfClearData.push_back(0); 
            sampleInfo.mBytesOfEncryptedData.push_back(0);       
        }

        ALOGD("%s[dash drm]SENC:mCurrentTrackID %d, dump IV sampleCount %d IvSize %d subCount %d",
            mObServerName,mTrackFragmentHeaderInfo.mTrackID,sampleCount,IvSize,mSubsample_count);
        sampleInfo.mIV =mIV;
        sampleInfo.mSubsample_count = mSubsample_count;
    }
	
		
    return OK;
}
   
status_t  DashFmp4Parser::parseSampleAuxiliaryInformationOffsets(
        uint32_t type, size_t offset, uint64_t size){
    return OK;
}
status_t  DashFmp4Parser::parseSampleAuxiliaryInformationSize(
        uint32_t type, size_t offset, uint64_t size){
    return OK;
}	
 
void DashFmp4Parser::skip(off_t distance) {
    CHECK(!mStack.isEmpty());
    for (size_t i = mStack.size(); i-- > 0;) {
        Container *container = &mStack.editItemAt(i);
        if (!container->mExtendsToEOF) {
            CHECK_LE(distance, (off_t)container->mBytesRemaining);

            container->mBytesRemaining -= distance;

            if (container->mBytesRemaining == 0) {
                ALOGD("[box trace]%sleaving box of type '%s'",
                        IndentString(mStack.size() - 1),
                        Fourcc2String(container->mType));

#if 0
                if (container->mType == FOURCC('s', 't', 's', 'd')) {
                    TrackInfo *trackInfo = editTrack(mCurrentTrackID);
                    for (size_t i = 0;
                            i < trackInfo->mSampleDescs.size(); ++i) {
                        ALOGI("format #%d: %s",
                              i,
                              trackInfo->mSampleDescs.itemAt(i)
                                .mFormat->debugString().c_str());
                    }
                }

#endif

                if (container->mType == FOURCC('s', 't', 'b', 'l')) {
                    TrackInfo *trackInfo = editTrack(mCurrentTrackID);

                    trackInfo->mStaticFragment->signalCompletion();

                    CHECK(trackInfo->mFragments.empty());
                    trackInfo->mFragments.push_back(trackInfo->mStaticFragment);
                    trackInfo->mStaticFragment.clear();
                } else if (container->mType == FOURCC('t', 'r', 'a', 'f')) {
                    TrackInfo *trackInfo =
                        editTrack(mTrackFragmentHeaderInfo.mTrackID);

                    const sp<TrackFragment> &fragment =
                        *--trackInfo->mFragments.end();

                    static_cast<DynamicTrackFragment *>(
                            fragment.get())->signalCompletion();
                } else if (container->mType == FOURCC('m', 'o', 'o', 'v')) {
                    mDoneWithMoov = true;

                    ALOGI("%s[profile]moov is parsed done,mHasTrack=%d",mObServerName,mHaveTrack);
                    notifyObserver(kWhatFileMeta);

                }

                else if(container->mType == FOURCC('m', 'o', 'o', 'f')) {
                    mMoofOffsets.add(container->mOffset);  
                }


                container = NULL;
                mStack.removeItemsAt(i);
            }
        }
    }


    if (distance < (off_t)mBuffer->size()) {
        mBuffer->setRange(mBuffer->offset() + distance, mBuffer->size() - distance);
		
        mBufferPos += distance;
        
        ALOGD("skip:mBufferPos %ld (0x%x),mBuffer->size()=%d, distance=%d",mBufferPos,mBufferPos,mBuffer->size(),distance);
    
	
        return;
    }

    mBuffer->setRange(0, 0);
    mBufferPos += distance;
    ALOGD("skip:mBufferPos %ld (0x%x),mBuffer->size()=%d, distance=%d",mBufferPos,mBufferPos,mBuffer->size(),distance);
	
}

status_t DashFmp4Parser::parseTrackHeader(
        uint32_t type, size_t offset, uint64_t size) {
    if (offset + 4 > size) {
			
	     ALOGE("%s err1 %s ",__FUNCTION__,mObServerName);
		
        return -EINVAL;
    }

    uint32_t flags = readU32(offset);

    uint32_t version = flags >> 24;
    flags &= 0xffffff;

    uint32_t trackID;
    uint64_t duration;

    if (version == 1) {
        if (offset + 36 > size) {
			
            ALOGE("%s err2  %s ",__FUNCTION__,mObServerName);
			
            return -EINVAL;
        }

        trackID = readU32(offset + 20);
        duration = readU64(offset + 28);

        offset += 36;
    } else if (version == 0) {
        if (offset + 24 > size) {
			
            ALOGE("%s err3 %s ",__FUNCTION__,mObServerName);
			
            return -EINVAL;
        }

        trackID = readU32(offset + 12);
        duration = readU32(offset + 20);

        offset += 24;
    } else {
			
        ALOGE("%s err4 %s ",__FUNCTION__,mObServerName);

        return -EINVAL;
    }

    TrackInfo *info = editTrack(trackID, true /* createIfNecessary */);
    info->mFlags = flags;
    info->mDuration = duration;
    if (info->mDuration == 0xffffffff) {
        // ffmpeg sets this to -1, which is incorrect.
        info->mDuration = 0;
    }

    info->mStaticFragment = new StaticTrackFragment;

    mCurrentTrackID = trackID;

    return OK;
}

status_t DashFmp4Parser::parseMediaHeader(
        uint32_t type, size_t offset, uint64_t size) {
    if (offset + 4 > size) {
			
	     ALOGE("%s err1  %s ",__FUNCTION__,mObServerName);
		
        return -EINVAL;
    }

    uint32_t versionAndFlags = readU32(offset);

    if (versionAndFlags & 0xffffff) {
			
	     ALOGE("%s err2  %s ",__FUNCTION__,mObServerName);
	
        return ERROR_MALFORMED;
    }

    uint32_t version = versionAndFlags >> 24;

    TrackInfo *info = editTrack(mCurrentTrackID);

    if (version == 1) {
        if (offset + 4 + 32 > size) {
			
            ALOGE("%s err3  %s ",__FUNCTION__,mObServerName);
		
            return -EINVAL;
        }
        info->mMediaTimeScale = U32_AT(mBuffer->data() + offset + 20);
    } else if (version == 0) {
        if (offset + 4 + 20 > size) {
			
            ALOGD("%s err4  %s ",__FUNCTION__,mObServerName);
		
            return -EINVAL;
        }
        info->mMediaTimeScale = U32_AT(mBuffer->data() + offset + 12);
    } else {
			
        ALOGE("%s err5  %s ",__FUNCTION__,mObServerName);
    
        return ERROR_MALFORMED;
    }

    return OK;
}

status_t DashFmp4Parser::parseMediaHandler(
        uint32_t type, size_t offset, uint64_t size) {
    if (offset + 12 > size) {
			
        ALOGE("%s err1 %s ",__FUNCTION__,mObServerName);
		
        return -EINVAL;
    }

    if (readU32(offset) != 0) {
			
        ALOGE("%s err2 %s ",__FUNCTION__,mObServerName);
		
        return -EINVAL;
    }

    uint32_t handlerType = readU32(offset + 8);

    switch (handlerType) {
        case FOURCC('v', 'i', 'd', 'e'):

    	{
    		mHaveTrack |= Track_Video ;  
    		mFileMeta->setInt32('hasV', 1);
    		mTrackIndexAndTrackIDMaps.add(Track_Video, mCurrentTrackID);
    		break;
    	}

        case FOURCC('s', 'o', 'u', 'n'):

        {
        	mHaveTrack |= Track_Audio ;
        	mFileMeta->setInt32('hasA', 1);
        	mTrackIndexAndTrackIDMaps.add(Track_Audio, mCurrentTrackID);
        	break;
        }
        case FOURCC('s', 'u', 'b', 't'):

    	{
    		mHaveTrack |= Track_SubTitles;
    		mFileMeta->setInt32('hasS', 1);
    		mTrackIndexAndTrackIDMaps.add(Track_SubTitles, mCurrentTrackID);
    		break;
    	}
	
        case FOURCC('h', 'i', 'n', 't'):
        case FOURCC('m', 'e', 't', 'a'):
            break;

        default:
            return -EINVAL;
    }
			
        ALOGI("%s[profile]mCurrentTrackID=%d,mMediaHandlerType=%s",mObServerName,mCurrentTrackID,Fourcc2String(handlerType));
	
    
    editTrack(mCurrentTrackID)->mMediaHandlerType = handlerType;

    return OK;
}

status_t DashFmp4Parser::parseVisualSampleEntry(
        uint32_t type, size_t offset, uint64_t size) {
    if (offset + 78 > size) {
			
        ALOGE("%s err1 %s ",__FUNCTION__,mObServerName);
		
        return -EINVAL;
    }

    TrackInfo *trackInfo = editTrack(mCurrentTrackID);

    trackInfo->mSampleDescs.push();
    SampleDescription *sampleDesc =
        &trackInfo->mSampleDescs.editItemAt(
                trackInfo->mSampleDescs.size() - 1);

    sampleDesc->mType = type;
    sampleDesc->mDataRefIndex = readU16(offset + 6);

    sp<AMessage> format = new AMessage;

    switch (type) {
        case FOURCC('a', 'v', 'c', '1'):
            format->setString("mime", MEDIA_MIMETYPE_VIDEO_AVC);
            break;
        case FOURCC('m', 'p', '4', 'v'):
            format->setString("mime", MEDIA_MIMETYPE_VIDEO_MPEG4);
            break;
        case FOURCC('s', '2', '6', '3'):
        case FOURCC('h', '2', '6', '3'):
        case FOURCC('H', '2', '6', '3'):
            format->setString("mime", MEDIA_MIMETYPE_VIDEO_H263);
            break;
	
        case  FOURCC('e', 'n', 'c', 'v'):{
            ALOGI("%s parseAudioSampleEntry:'encv'",mObServerName);
            trackInfo->mIsEncrpyted = true;
            break;
        }


	   default:
            format->setString("mime", "application/octet-stream");
            break;
    }

    format->setInt32("width", readU16(offset + 8 + 16));
    format->setInt32("height", readU16(offset + 8 + 18));

    sampleDesc->mFormat = format;
			
    ALOGD("%s %s [profile]:format get now ",mObServerName,__FUNCTION__);
 
    return OK;
}

status_t DashFmp4Parser::parseAudioSampleEntry(
        uint32_t type, size_t offset, uint64_t size) {
    if (offset + 28 > size) {
			
        ALOGE("%s err1 %s ",__FUNCTION__,mObServerName);
		
        return -EINVAL;
    }

    TrackInfo *trackInfo = editTrack(mCurrentTrackID);

    trackInfo->mSampleDescs.push();
    SampleDescription *sampleDesc =
        &trackInfo->mSampleDescs.editItemAt(
                trackInfo->mSampleDescs.size() - 1);

    sampleDesc->mType = type;
    sampleDesc->mDataRefIndex = readU16(offset + 6);

    sp<AMessage> format = new AMessage;

    format->setInt32("channel-count", readU16(offset + 8 + 8));
    format->setInt32("sample-size", readU16(offset + 8 + 10));
    format->setInt32("sample-rate", readU32(offset + 8 + 16) / 65536.0f);

    switch (type) {
        case FOURCC('m', 'p', '4', 'a'):
            format->setString("mime", MEDIA_MIMETYPE_AUDIO_AAC);
            break;

        case FOURCC('s', 'a', 'm', 'r'):
            format->setString("mime", MEDIA_MIMETYPE_AUDIO_AMR_NB);
            format->setInt32("channel-count", 1);
            format->setInt32("sample-rate", 8000);
            break;

        case FOURCC('s', 'a', 'w', 'b'):
            format->setString("mime", MEDIA_MIMETYPE_AUDIO_AMR_WB);
            format->setInt32("channel-count", 1);
            format->setInt32("sample-rate", 16000);
            break;
	
        case  FOURCC('e', 'n', 'c', 'a'):{
            ALOGI("%s parseAudioSampleEntry:'enca'",mObServerName);
            trackInfo->mIsEncrpyted = true;
            break;	
        }
			
        default:
            format->setString("mime", "application/octet-stream");
            break;
    }

    sampleDesc->mFormat = format;
			
    ALOGD("%s [profile]:format get %s ",__FUNCTION__,mObServerName);
    

    return OK;
}

static void addCodecSpecificData(
        const sp<AMessage> &format, int32_t index,
        const void *data, size_t size,
        bool insertStartCode = false) {
    sp<ABuffer> csd = new ABuffer(insertStartCode ? size + 4 : size);

    memcpy(csd->data() + (insertStartCode ? 4 : 0), data, size);

    if (insertStartCode) {
        memcpy(csd->data(), "\x00\x00\x00\x01", 4);
    }

    csd->meta()->setInt32("csd", true);
    csd->meta()->setInt64("timeUs", 0ll);

    format->setBuffer(StringPrintf("csd-%d", index).c_str(), csd);
}

status_t DashFmp4Parser::parseSampleSizes(
        uint32_t type, size_t offset, uint64_t size) {
    return editTrack(mCurrentTrackID)->mStaticFragment->parseSampleSizes(
            this, type, offset, size);
}

status_t DashFmp4Parser::parseCompactSampleSizes(
        uint32_t type, size_t offset, uint64_t size) {
    return editTrack(mCurrentTrackID)->mStaticFragment->parseCompactSampleSizes(
            this, type, offset, size);
}

status_t DashFmp4Parser::parseSampleToChunk(
        uint32_t type, size_t offset, uint64_t size) {
    return editTrack(mCurrentTrackID)->mStaticFragment->parseSampleToChunk(
            this, type, offset, size);
}

status_t DashFmp4Parser::parseChunkOffsets(
        uint32_t type, size_t offset, uint64_t size) {
    return editTrack(mCurrentTrackID)->mStaticFragment->parseChunkOffsets(
            this, type, offset, size);
}

status_t DashFmp4Parser::parseChunkOffsets64(
        uint32_t type, size_t offset, uint64_t size) {
    return editTrack(mCurrentTrackID)->mStaticFragment->parseChunkOffsets64(
            this, type, offset, size);
}

status_t DashFmp4Parser::parseAVCCodecSpecificData(
        uint32_t type, size_t offset, uint64_t size) {
    TrackInfo *trackInfo = editTrack(mCurrentTrackID);

    SampleDescription *sampleDesc =
        &trackInfo->mSampleDescs.editItemAt(
                trackInfo->mSampleDescs.size() - 1);

    if (sampleDesc->mType != FOURCC('a', 'v', 'c', '1')
			
        && sampleDesc->mType !=  FOURCC('e', 'n', 'c', 'v')

    ) {
			
        ALOGE("%s err1 %s ",__FUNCTION__,mObServerName);

        return -EINVAL;
    }

    const uint8_t *ptr = mBuffer->data() + offset;

    size -= offset;
    offset = 0;

    if (size < 7 || ptr[0] != 0x01) {
			
	     ALOGE("%s err2 %s ",__FUNCTION__,mObServerName);
	
        return ERROR_MALFORMED;
    }

    sampleDesc->mFormat->setSize("nal-length-size", 1 + (ptr[4] & 3));
			
	     ALOGD("%s %s nal-length-size %d",mObServerName,__FUNCTION__,1 + (ptr[4] & 3));
	
    size_t numSPS = ptr[5] & 31;

    ptr += 6;
    size -= 6;

    for (size_t i = 0; i < numSPS; ++i) {
        if (size < 2) {
			
            ALOGE("%s err3 %s ",__FUNCTION__,mObServerName);
	
            return ERROR_MALFORMED;
        }

        size_t length = U16_AT(ptr);

        ptr += 2;
        size -= 2;

        if (size < length) {
			
            ALOGE("%s err4 %s ",__FUNCTION__,mObServerName);
		
            return ERROR_MALFORMED;
        }

        addCodecSpecificData(
                sampleDesc->mFormat, i, ptr, length,
                true /* insertStartCode */);

        ptr += length;
        size -= length;
    }

    if (size < 1) {
			
        ALOGE("%s err5 %s ",__FUNCTION__,mObServerName);
	
        return ERROR_MALFORMED;
    }

    size_t numPPS = *ptr;
    ++ptr;
    --size;

    for (size_t i = 0; i < numPPS; ++i) {
        if (size < 2) {
			
            ALOGE("%s err6 %s ",__FUNCTION__,mObServerName);
		
            return ERROR_MALFORMED;
        }

        size_t length = U16_AT(ptr);

        ptr += 2;
        size -= 2;

        if (size < length) {
			
            ALOGE("%s err7 %s ",__FUNCTION__,mObServerName);
			
            return ERROR_MALFORMED;
        }

        addCodecSpecificData(
                sampleDesc->mFormat, numSPS + i, ptr, length,
                true /* insertStartCode */);

        ptr += length;
        size -= length;
    }

    return OK;
}

status_t DashFmp4Parser::parseESDSCodecSpecificData(
        uint32_t type, size_t offset, uint64_t size) {
    TrackInfo *trackInfo = editTrack(mCurrentTrackID);

    SampleDescription *sampleDesc =
        &trackInfo->mSampleDescs.editItemAt(
                trackInfo->mSampleDescs.size() - 1);

    if (sampleDesc->mType != FOURCC('m', 'p', '4', 'a')
            && sampleDesc->mType != FOURCC('m', 'p', '4', 'v')
	
	  && sampleDesc->mType != FOURCC('e', 'n', 'c', 'a')


	) {
			
	     ALOGE("%s err1 %s ",__FUNCTION__,mObServerName);
       
        return -EINVAL;
    }

    const uint8_t *ptr = mBuffer->data() + offset;

    size -= offset;
    offset = 0;

    if (size < 4) {
			
        ALOGE("%s err2  %s ",__FUNCTION__,mObServerName);
	
        return -EINVAL;
    }

    if (U32_AT(ptr) != 0) {
			
        ALOGE("%s err3 %s ",__FUNCTION__,mObServerName);
		
        return -EINVAL;
    }

    ptr += 4;
    size -=4;

    ESDS esds(ptr, size);

    uint8_t objectTypeIndication;
    if (esds.getObjectTypeIndication(&objectTypeIndication) != OK) {
			
	     ALOGE("%s err4 %s ",__FUNCTION__,mObServerName);
	
        return ERROR_MALFORMED;
    }

    const uint8_t *csd;
    size_t csd_size;
    if (esds.getCodecSpecificInfo(
                (const void **)&csd, &csd_size) != OK) {
			
        ALOGE("%s err5%s ",__FUNCTION__,mObServerName);
                
        return ERROR_MALFORMED;
    }

    addCodecSpecificData(sampleDesc->mFormat, 0, csd, csd_size);

    if (sampleDesc->mType != FOURCC('m', 'p', '4', 'a')) {
        return OK;
    }

    if (csd_size == 0) {
        // There's no further information, i.e. no codec specific data
        // Let's assume that the information provided in the mpeg4 headers
        // is accurate and hope for the best.

        return OK;
    }

    if (csd_size < 2) {
			
	     ALOGE("%s err6 %s ",__FUNCTION__,mObServerName);

        return ERROR_MALFORMED;
    }

    uint32_t objectType = csd[0] >> 3;

    if (objectType == 31) {
			
	     ALOGE("%s err7 %s ",__FUNCTION__,mObServerName);
	
        return ERROR_UNSUPPORTED;
    }

    uint32_t freqIndex = (csd[0] & 7) << 1 | (csd[1] >> 7);
    int32_t sampleRate = 0;
    int32_t numChannels = 0;
    if (freqIndex == 15) {
        if (csd_size < 5) {
			
            ALOGE("%s err8%s ",__FUNCTION__,mObServerName);
		
            return ERROR_MALFORMED;
        }

        sampleRate = (csd[1] & 0x7f) << 17
                        | csd[2] << 9
                        | csd[3] << 1
                        | (csd[4] >> 7);

        numChannels = (csd[4] >> 3) & 15;
    } else {
        static uint32_t kSamplingRate[] = {
            96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050,
            16000, 12000, 11025, 8000, 7350
        };

        if (freqIndex == 13 || freqIndex == 14) {
			
            ALOGE("%s err9  %s ",__FUNCTION__,mObServerName);
		
            //return ERROR_MALFORMED;
        }else{
            sampleRate = kSamplingRate[freqIndex];
            sampleDesc->mFormat->setInt32("sample-rate", sampleRate);
        }        
        numChannels = (csd[1] >> 3) & 15;
    }

    if (numChannels == 0) {
			
        ALOGE("%s err10  %s ",__FUNCTION__,mObServerName);

        //return ERROR_UNSUPPORTED;//remove
    }else{
        sampleDesc->mFormat->setInt32("channel-count", numChannels);
    }
    
    
    

    return OK;
}

status_t DashFmp4Parser::parseMediaData(
        uint32_t type, size_t offset, uint64_t size) {
    
// ALOGI("parseMediaData box type %d, offset %d, size %d", type, int(offset), int(size));
    sp<ABuffer> buffer = new ABuffer(size - offset);
    memcpy(buffer->data(), mBuffer->data() + offset, size - offset);

    mMediaData.push();
    MediaDataInfo *info = &mMediaData.editItemAt(mMediaData.size() - 1);
    info->mBuffer = buffer;
    info->mOffset = mBufferPos + offset;

//dash not need suspend 
    if (mMediaData.size() > 10) { 
        ALOGI("suspending for now.");
        mSuspended = true;
    }
    return OK;
}

status_t DashFmp4Parser::parseSegmentIndex(
        uint32_t type, size_t offset, uint64_t size) {
        ALOGI("sidx box type %s, offset %d, size %d", Fourcc2String(type), int(offset), int(size));
//    AString sidxstr;
//    hexdump(mBuffer->data() + offset, size, 0 /* indent */, &sidxstr);
//    ALOGI("raw sidx:");
//    ALOGI("%s", sidxstr.c_str());
    if (offset + 12 > size) {
			
	     ALOGE("%s err1  %s ",__FUNCTION__,mObServerName);

	
        return -EINVAL;
    }

    uint32_t flags = readU32(offset);

    uint32_t version = flags >> 24;
    flags &= 0xffffff;

    ALOGI("sidx version %d", version);

    uint32_t referenceId = readU32(offset + 4);
    uint32_t timeScale = readU32(offset + 8);
    ALOGI("sidx refid/timescale: %d/%d", referenceId, timeScale);

    uint64_t earliestPresentationTime;
    uint64_t firstOffset;

    offset += 12;

    if (version == 0) {
        if (offset + 8 > size) {
			
            ALOGE("%s err3  %s ",__FUNCTION__,mObServerName);
			
            return -EINVAL;
        }
        earliestPresentationTime = readU32(offset);
        firstOffset = readU32(offset + 4);
        offset += 8;
    } else {
        if (offset + 16 > size) {
			
            ALOGE("%s err3  %s ",__FUNCTION__,mObServerName);
			
            return -EINVAL;
        }
        earliestPresentationTime = readU64(offset);
        firstOffset = readU64(offset + 8);
        //is the distance in bytes, in the file containing media, 
        //from the anchor point(In the file containing the Segment Index box, 
        //the anchor point for a Segment Index box is the first byte after
        //that box.), to the first byte of the indexed material;
        offset += 16;
    }
    
    if (offset + 4 > size) {
			
	     ALOGE("%s err4%s ",__FUNCTION__,mObServerName);
	
        return -EINVAL;
    }
    if (readU16(offset) != 0) { // reserved
			
	     ALOGE("%s err4.1%s ",__FUNCTION__,mObServerName);
    
        return -EINVAL;
    }
    int32_t referenceCount = readU16(offset + 2);
    offset += 4;
    
    ALOGI("%s [profile]sidx early pts/firstoff/refcount/timescale: %lld/0x%llx/%d/%d", 
          mObServerName,earliestPresentationTime, firstOffset,referenceCount,timeScale);


    if (offset + referenceCount * 12 > size) {
			
        ALOGE("%s err5  %s ",__FUNCTION__,mObServerName);
	
        return -EINVAL;
    }

    TrackInfo *info = editTrack(referenceId);
    CHECK(info != NULL);
    uint64_t indexMediaOffset  = mBufferPos + size + firstOffset;
    uint64_t indexMediaTimeUs = earliestPresentationTime;
	
    uint64_t total_duration = 0;
	
    
    for (int i = 0; i < referenceCount; i++) {
        uint32_t d1 = readU32(offset);
        uint32_t d2 = readU32(offset + 4);
        uint32_t d3 = readU32(offset + 8);
        //reference_type:
        //:  1 are inthe index segment, 
        //:  0 are in the media file
        
        if (d1 & 0x80000000) {
            ALOGW("this is the index for sub-sidx!!!");
        }
        bool sap = d3 & 0x80000000;

        uint8_t saptype = d3 >> 28;
        saptype  = saptype & 0x03;

        if (!sap || saptype > 2) {
            ALOGW("not a stream access point, or unsupported type");
        }
        total_duration += d2;
        offset += 12;        
        SidxEntry se;
        se.mSize = d1 & 0x7fffffff;
        se.mDurationUs = 1000000LL * d2 / timeScale;   
        
        SXLOGV(" sidx item %d, 0x%08x %d %d", i, d1, d2, se.mDurationUs);


	
        se.mEarliest_PTS = earliestPresentationTime;
        se.mFirst_Offset = firstOffset;
        se.mReference_ID = referenceId;
        se.mStarts_with_SAP = sap;
        se.mSAP_Type = saptype;
        se.mReference_Type = (d1 >> 31) & 0x01;
        se.mTimeScale = timeScale;
        if(se.mReference_Type == 0){
            se.mOffset = indexMediaOffset;
            se.mSegmentStartUs = 1000000LL * indexMediaTimeUs / timeScale + mBaseMediaTimeUs;      
            indexMediaOffset += se.mSize;
            indexMediaTimeUs += d2;
            info->mSidx.add(se);
        }
   
    }

    info->mSidxDuration = total_duration * 1000000 / timeScale;
       
    ALOGI("%s duration: %lld", mObServerName,info->mSidxDuration);

    return OK;
}

status_t DashFmp4Parser::parseTrackExtends(
        uint32_t type, size_t offset, uint64_t size) {
    if (offset + 24 > size) {
			
	     ALOGE("%s err1  %s ",__FUNCTION__,mObServerName);
	
        return -EINVAL;
    }

    if (readU32(offset) != 0) {
			
	     ALOGE("%s err2 %s ",__FUNCTION__,mObServerName);
	
        return -EINVAL;
    }

    uint32_t trackID = readU32(offset + 4);

    TrackInfo *info = editTrack(trackID, true /* createIfNecessary */);
    info->mDefaultSampleDescriptionIndex = readU32(offset + 8);
    info->mDefaultSampleDuration = readU32(offset + 12);
    info->mDefaultSampleSize = readU32(offset + 16);
    info->mDefaultSampleFlags = readU32(offset + 20);

    return OK;
}

DashFmp4Parser::TrackInfo *DashFmp4Parser::editTrack(
        uint32_t trackID, bool createIfNecessary) {
    ssize_t i = mTracks.indexOfKey(trackID);

    if (i >= 0) {
        return &mTracks.editValueAt(i);
    }

    if (!createIfNecessary) {
        return NULL;
    }

    TrackInfo info;
    info.mTrackID = trackID;
    info.mFlags = 0;
    info.mDuration = 0xffffffff;
    info.mSidxDuration = 0;
    info.mMediaTimeScale = 0;
    info.mMediaHandlerType = 0;
    info.mDefaultSampleDescriptionIndex = 0;
    info.mDefaultSampleDuration = 0;
    info.mDefaultSampleSize = 0;
    info.mDefaultSampleFlags = 0;

    info.mDecodingTime = 0;
	
    info.mBaseMediaTimeUs = mBaseMediaTimeUs;
    info.mIsEncrpyted =false;
    info.mTenc.mIsEncrypted =false;
    info.mTenc.mIV_Size =0;
    memset(info.mTenc.mKID,0,sizeof(info.mTenc.mKID));
    info.mStartDecodeSegmentIndex =0;
		


        
    mTracks.add(trackID, info);
    return &mTracks.editValueAt(mTracks.indexOfKey(trackID));
}

status_t DashFmp4Parser::parseTrackFragmentHeader(
        uint32_t type, size_t offset, uint64_t size) {
    if (offset + 8 > size) {
			
        ALOGE("%s err1  %s ",__FUNCTION__,mObServerName);
	
        return -EINVAL;
    }

    uint32_t flags = readU32(offset);

    if (flags & 0xff000000) {
			
	     ALOGE("%s err2  %s ",__FUNCTION__,mObServerName);

        return -EINVAL;
    }

    mTrackFragmentHeaderInfo.mFlags = flags;

    mTrackFragmentHeaderInfo.mTrackID = readU32(offset + 4);
    offset += 8;

    if (flags & TrackFragmentHeaderInfo::kBaseDataOffsetPresent) {
        if (offset + 8 > size) {
			
            ALOGE("%s err3 %s ",__FUNCTION__,mObServerName);
			
            return -EINVAL;
        }

        mTrackFragmentHeaderInfo.mBaseDataOffset = readU64(offset);
        offset += 8;
    }

    if (flags & TrackFragmentHeaderInfo::kSampleDescriptionIndexPresent) {
        if (offset + 4 > size) {
			
            ALOGE("%s err4 %s ",__FUNCTION__,mObServerName);
		
            return -EINVAL;
        }

        mTrackFragmentHeaderInfo.mSampleDescriptionIndex = readU32(offset);
        offset += 4;
    }

    if (flags & TrackFragmentHeaderInfo::kDefaultSampleDurationPresent) {
        if (offset + 4 > size) {
			
            ALOGE("%s err5 %s ",__FUNCTION__,mObServerName);
		
            return -EINVAL;
        }

        mTrackFragmentHeaderInfo.mDefaultSampleDuration = readU32(offset);
        offset += 4;
    }

    if (flags & TrackFragmentHeaderInfo::kDefaultSampleSizePresent) {
        if (offset + 4 > size) {
			
            ALOGE("%s err6%s ",__FUNCTION__,mObServerName);
		
            return -EINVAL;
        }

        mTrackFragmentHeaderInfo.mDefaultSampleSize = readU32(offset);
        offset += 4;
    }

    if (flags & TrackFragmentHeaderInfo::kDefaultSampleFlagsPresent) {
        if (offset + 4 > size) {
			
            ALOGE("%s err7 %s ",__FUNCTION__,mObServerName);
		
            return -EINVAL;
        }

        mTrackFragmentHeaderInfo.mDefaultSampleFlags = readU32(offset);
        offset += 4;
    }

    if (!(flags & TrackFragmentHeaderInfo::kBaseDataOffsetPresent)) {
        // This should point to the position of the first byte of the
        // enclosing 'moof' container for the first track and
        // the end of the data of the preceding fragment for subsequent
        // tracks.

        CHECK_GE(mStack.size(), 2u);

        mTrackFragmentHeaderInfo.mBaseDataOffset =
            mStack.itemAt(mStack.size() - 2).mOffset;

        // XXX TODO: This does not do the right thing for the 2nd and
        // subsequent tracks yet.
    }

    mTrackFragmentHeaderInfo.mDataOffset =
        mTrackFragmentHeaderInfo.mBaseDataOffset;

    TrackInfo *trackInfo = editTrack(mTrackFragmentHeaderInfo.mTrackID);

    if (trackInfo->mFragments.empty()
            || (*trackInfo->mFragments.begin())->complete()) {
        trackInfo->mFragments.push_back(new DynamicTrackFragment);
    }

    return OK;
}

status_t DashFmp4Parser::parseTrackFragmentRun(
        uint32_t type, size_t offset, uint64_t size) {
    if (offset + 8 > size) {
			
	     ALOGE("%s err1  %s ",__FUNCTION__,mObServerName);
		
        return -EINVAL;
    }

    enum {
        kDataOffsetPresent                  = 0x01,
        kFirstSampleFlagsPresent            = 0x04,
        kSampleDurationPresent              = 0x100,
        kSampleSizePresent                  = 0x200,
        kSampleFlagsPresent                 = 0x400,
        kSampleCompositionTimeOffsetPresent = 0x800,
    };

    uint32_t flags = readU32(offset);

    if (flags & 0xff000000) {
			
	     ALOGE("%s err2 %s ",__FUNCTION__,mObServerName);
	
        return -EINVAL;
    }

    if ((flags & kFirstSampleFlagsPresent) && (flags & kSampleFlagsPresent)) {
        // These two shall not be used together.
			
	     ALOGE("%s err3 %s ",__FUNCTION__,mObServerName);
        
        return -EINVAL;
    }

    uint32_t sampleCount = readU32(offset + 4);
    offset += 8;

    uint64_t dataOffset = mTrackFragmentHeaderInfo.mDataOffset;

    uint32_t firstSampleFlags = 0;

    if (flags & kDataOffsetPresent) {
        if (offset + 4 > size) {
			
            ALOGE("%s err4 %s ",__FUNCTION__,mObServerName);
		
            return -EINVAL;
        }

        int32_t dataOffsetDelta = (int32_t)readU32(offset);

        dataOffset = mTrackFragmentHeaderInfo.mBaseDataOffset + dataOffsetDelta;

        offset += 4;
	    ALOGD(" Info.mBaseDataOffset =%lld,Info.mDataOffse =%lld,dataOffsetDelta=%d",
   	            mTrackFragmentHeaderInfo.mBaseDataOffset , mTrackFragmentHeaderInfo.mDataOffset,dataOffsetDelta);
    }

   

    if (flags & kFirstSampleFlagsPresent) {
        if (offset + 4 > size) {
			
            ALOGE("%s err5 %s ",__FUNCTION__,mObServerName);
		
            return -EINVAL;
        }

        firstSampleFlags = readU32(offset);
        offset += 4;
    }

    TrackInfo *info = editTrack(mTrackFragmentHeaderInfo.mTrackID);

    if (info == NULL) {
			
	     ALOGE("%s err6 %s ",__FUNCTION__,mObServerName);
	
        return -EINVAL;
    }

    uint32_t sampleDuration = 0, sampleSize = 0, sampleFlags = 0,
             sampleCtsOffset = 0;

    size_t bytesPerSample = 0;
    if (flags & kSampleDurationPresent) {
        bytesPerSample += 4;
    } else if (mTrackFragmentHeaderInfo.mFlags
            & TrackFragmentHeaderInfo::kDefaultSampleDurationPresent) {
        sampleDuration = mTrackFragmentHeaderInfo.mDefaultSampleDuration;
    } else {
        sampleDuration = info->mDefaultSampleDuration;
    }

    if (flags & kSampleSizePresent) {
        bytesPerSample += 4;
    } else if (mTrackFragmentHeaderInfo.mFlags
            & TrackFragmentHeaderInfo::kDefaultSampleSizePresent) {
        sampleSize = mTrackFragmentHeaderInfo.mDefaultSampleSize;
    } else {
        sampleSize = info->mDefaultSampleSize;
    }

    if (flags & kSampleFlagsPresent) {
        bytesPerSample += 4;
    } else if (mTrackFragmentHeaderInfo.mFlags
            & TrackFragmentHeaderInfo::kDefaultSampleFlagsPresent) {
        sampleFlags = mTrackFragmentHeaderInfo.mDefaultSampleFlags;
    } else {
        sampleFlags = info->mDefaultSampleFlags;
    }

    if (flags & kSampleCompositionTimeOffsetPresent) {
        bytesPerSample += 4;
    } else {
        sampleCtsOffset = 0;
    }

    if (offset + sampleCount * bytesPerSample > size) {
			
	     ALOGE("%s err7  %s ",__FUNCTION__,mObServerName);
	
        return -EINVAL;
    }

    uint32_t sampleDescIndex =
        (mTrackFragmentHeaderInfo.mFlags
            & TrackFragmentHeaderInfo::kSampleDescriptionIndexPresent)
            ? mTrackFragmentHeaderInfo.mSampleDescriptionIndex
            : info->mDefaultSampleDescriptionIndex;

    for (uint32_t i = 0; i < sampleCount; ++i) {
        if (flags & kSampleDurationPresent) {
            sampleDuration = readU32(offset);
            offset += 4;
        }

        if (flags & kSampleSizePresent) {
            sampleSize = readU32(offset);
            offset += 4;
        }

        if (flags & kSampleFlagsPresent) {
            sampleFlags = readU32(offset);
            offset += 4;
        }

        if (flags & kSampleCompositionTimeOffsetPresent) {
            sampleCtsOffset = readU32(offset);
            offset += 4;
        }



        const sp<TrackFragment> &fragment = *--info->mFragments.end();

        uint32_t decodingTime = info->mDecodingTime;
        info->mDecodingTime += sampleDuration;
        uint32_t presentationTime = decodingTime + sampleCtsOffset;

        SXLOGV("%s adding sample at offset 0x%08llx, size %u, duration %u, "
              "sampleDescIndex=%u, flags 0x%08x,presentationTime=%d",
                mObServerName,dataOffset, sampleSize, sampleDuration,
                sampleDescIndex,
                (flags & kFirstSampleFlagsPresent) && i == 0
                    ? firstSampleFlags : sampleFlags,presentationTime);

        static_cast<DynamicTrackFragment *>(
                fragment.get())->addSample(
                    dataOffset,
                    sampleSize,
                    presentationTime,
                    sampleDescIndex,
                    ((flags & kFirstSampleFlagsPresent) && i == 0)
                        ? firstSampleFlags : sampleFlags);

        dataOffset += sampleSize;
    }

    mTrackFragmentHeaderInfo.mDataOffset = dataOffset;

    return OK;
}

void DashFmp4Parser::copyBuffer(
        sp<ABuffer> *dst, size_t offset, uint64_t size) const {
    sp<ABuffer> buf = new ABuffer(size);
    memcpy(buf->data(), mBuffer->data() + offset, size);

    *dst = buf;
}


//////////////////////////////////////////////



DashFmp4Parser::DynamicTrackFragment::DynamicTrackFragment()
    : mComplete(false),
      mSampleIndex(0) {
      
    ALOGI("DynamicTrackFragment[%s]",__FUNCTION__);
    mSampleCount = 0;


}

DashFmp4Parser::DynamicTrackFragment::~DynamicTrackFragment() {
      
    ALOGI("DynamicTrackFragment[%s][profile]:mSampleIndex %d VS mSamples.size() %d",
            __FUNCTION__,mSampleIndex,mSamples.size());

    for(size_t i=0;i<mSamples.size();i++){
        SampleInfo *sampleInfo = &mSamples.editItemAt(i);
        sampleInfo->mIV.clear();
        sampleInfo->mBytesOfClearData.clear();
        sampleInfo->mBytesOfEncryptedData.clear();
    }

}

status_t DashFmp4Parser::DynamicTrackFragment::getSample(SampleInfo *info) {
    if (mSampleIndex >= mSamples.size()) {
        return mComplete ? ERROR_END_OF_STREAM : -EWOULDBLOCK;
    }

    *info = mSamples.itemAt(mSampleIndex);

    return OK;
}

void DashFmp4Parser::DynamicTrackFragment::advance() {
    ++mSampleIndex;
}

void DashFmp4Parser::DynamicTrackFragment::addSample(
        off64_t dataOffset, size_t sampleSize,
        uint32_t presentationTime,
        size_t sampleDescIndex,
        uint32_t flags) {
      
    SXLOGV("DynamicTrackFragment[%s]dataOffset=%d,presentationTime=%d,flags=0x%x",__FUNCTION__,dataOffset,presentationTime,flags);
    mSampleCount++;

    mSamples.push();
    SampleInfo *sampleInfo = &mSamples.editItemAt(mSamples.size() - 1);

    sampleInfo->mOffset = dataOffset;
    sampleInfo->mSize = sampleSize;
    sampleInfo->mPresentationTime = presentationTime;
    sampleInfo->mSampleDescIndex = sampleDescIndex;
    sampleInfo->mFlags = flags;
}

status_t DashFmp4Parser::DynamicTrackFragment::signalCompletion() {
    	
	ALOGD("[profile]DynamicTrackFragment[%s] mSampleCount=%d",__FUNCTION__,mSampleCount);


    mComplete = true;

    return OK;
}

bool DashFmp4Parser::DynamicTrackFragment::complete() const {
    	
	ALOGD("DynamicTrackFragment[%s]",__FUNCTION__);

    return mComplete;
}
   

status_t DashFmp4Parser::DynamicTrackFragment::getSampleByIndex(
    uint32_t sampleIndex,SampleInfo *info) {
    SXLOGD("sampleIndex %d VS mSamples.size %d",sampleIndex,mSamples.size());
    CHECK(sampleIndex < mSamples.size());//0~size-1
    
    *info = mSamples.itemAt(sampleIndex);

    return OK;
}

////////////////////////////////////////////////////////////////////////////////

DashFmp4Parser::StaticTrackFragment::StaticTrackFragment()
    : mSampleIndex(0),
      mSampleCount(0),
      mChunkIndex(0),
      mSampleToChunkIndex(-1),
      mSampleToChunkRemaining(0),
      mPrevChunkIndex(0xffffffff),
      mNextSampleOffset(0) {

    	
	ALOGD("StaticTrackFragment[%s]",__FUNCTION__);

}

DashFmp4Parser::StaticTrackFragment::~StaticTrackFragment() {
    	
	ALOGD("StaticTrackFragment[%s]",__FUNCTION__);

}

status_t DashFmp4Parser::StaticTrackFragment::getSample(SampleInfo *info) {
    if (mSampleIndex >= mSampleCount) {
    	
	ALOGD("StaticTrackFragment[%s]",__FUNCTION__);

        return ERROR_END_OF_STREAM;
    }

    *info = mSampleInfo;

    ALOGI("StaticTrackFragment::getSample:returning sample %d at [0x%08llx, 0x%08llx)",
          mSampleIndex,
          info->mOffset, info->mOffset + info->mSize);

    return OK;
}

void DashFmp4Parser::StaticTrackFragment::updateSampleInfo() {
    if (mSampleIndex >= mSampleCount) {
        return;
    }

    if (mSampleSizes != NULL) {
        uint32_t defaultSampleSize = U32_AT(mSampleSizes->data() + 4);
        if (defaultSampleSize > 0) {
            mSampleInfo.mSize = defaultSampleSize;
        } else {
            mSampleInfo.mSize= U32_AT(mSampleSizes->data() + 12 + 4 * mSampleIndex);
        }
    } else {
        CHECK(mCompactSampleSizes != NULL);

        uint32_t fieldSize = U32_AT(mCompactSampleSizes->data() + 4);

        switch (fieldSize) {
            case 4:
            {
                unsigned byte = mCompactSampleSizes->data()[12 + mSampleIndex / 2];
                mSampleInfo.mSize = (mSampleIndex & 1) ? byte & 0x0f : byte >> 4;
                break;
            }

            case 8:
            {
                mSampleInfo.mSize = mCompactSampleSizes->data()[12 + mSampleIndex];
                break;
            }

            default:
            {
                CHECK_EQ(fieldSize, 16);
                mSampleInfo.mSize =
                    U16_AT(mCompactSampleSizes->data() + 12 + mSampleIndex * 2);
                break;
            }
        }
    }

    CHECK_GT(mSampleToChunkRemaining, 0);

    // The sample desc index is 1-based... XXX
    mSampleInfo.mSampleDescIndex =
        U32_AT(mSampleToChunk->data() + 8 + 12 * mSampleToChunkIndex + 8);

    if (mChunkIndex != mPrevChunkIndex) {
        mPrevChunkIndex = mChunkIndex;

        if (mChunkOffsets != NULL) {
            uint32_t entryCount = U32_AT(mChunkOffsets->data() + 4);

            if (mChunkIndex >= entryCount) {
                mSampleIndex = mSampleCount;
                return;
            }

            mNextSampleOffset =
                U32_AT(mChunkOffsets->data() + 8 + 4 * mChunkIndex);
        } else {
            CHECK(mChunkOffsets64 != NULL);

            uint32_t entryCount = U32_AT(mChunkOffsets64->data() + 4);

            if (mChunkIndex >= entryCount) {
                mSampleIndex = mSampleCount;
                return;
            }

            mNextSampleOffset =
                U64_AT(mChunkOffsets64->data() + 8 + 8 * mChunkIndex);
        }
    }

    mSampleInfo.mOffset = mNextSampleOffset;

    mSampleInfo.mPresentationTime = 0;
    mSampleInfo.mFlags = 0;
}

void DashFmp4Parser::StaticTrackFragment::advance() {
    ALOGI("[%s]",__FUNCTION__);
    mNextSampleOffset += mSampleInfo.mSize;

    ++mSampleIndex;
    if (--mSampleToChunkRemaining == 0) {
        ++mChunkIndex;

        uint32_t entryCount = U32_AT(mSampleToChunk->data() + 4);

        // If this is the last entry in the sample to chunk table, we will
        // stay on this entry.
        if ((uint32_t)(mSampleToChunkIndex + 1) < entryCount) {
            uint32_t nextChunkIndex =
                U32_AT(mSampleToChunk->data() + 8 + 12 * (mSampleToChunkIndex + 1));

            CHECK_GE(nextChunkIndex, 1u);
            --nextChunkIndex;

            if (mChunkIndex >= nextChunkIndex) {
                CHECK_EQ(mChunkIndex, nextChunkIndex);
                ++mSampleToChunkIndex;
            }
        }

        mSampleToChunkRemaining =
            U32_AT(mSampleToChunk->data() + 8 + 12 * mSampleToChunkIndex + 4);
    }

    updateSampleInfo();
}

static void setU32At(uint8_t *ptr, uint32_t x) {
    ptr[0] = x >> 24;
    ptr[1] = (x >> 16) & 0xff;
    ptr[2] = (x >> 8) & 0xff;
    ptr[3] = x & 0xff;
}

status_t DashFmp4Parser::StaticTrackFragment::signalCompletion() {
    mSampleToChunkIndex = 0;
    	
	ALOGD("StaticTrackFragment[%s]",__FUNCTION__);

    mSampleToChunkRemaining =
        (mSampleToChunk == NULL)
            ? 0
            : U32_AT(mSampleToChunk->data() + 8 + 12 * mSampleToChunkIndex + 4);

    updateSampleInfo();

    return OK;
}

bool DashFmp4Parser::StaticTrackFragment::complete() const {
    	
	ALOGD("StaticTrackFragment[%s]",__FUNCTION__);

    return true;
}

status_t DashFmp4Parser::StaticTrackFragment::parseSampleSizes(
        DashFmp4Parser *parser, uint32_t type, size_t offset, uint64_t size) {
    	
	ALOGD("StaticTrackFragment[%s]",__FUNCTION__);

    if (offset + 12 > size) {
        return ERROR_MALFORMED;
    }

    if (parser->readU32(offset) != 0) {
        return ERROR_MALFORMED;
    }

    uint32_t sampleSize = parser->readU32(offset + 4);
    uint32_t sampleCount = parser->readU32(offset + 8);

    if (sampleSize == 0 && offset + 12 + sampleCount * 4 != size) {
        return ERROR_MALFORMED;
    }

    parser->copyBuffer(&mSampleSizes, offset, size);

    mSampleCount = sampleCount;

    return OK;
}

status_t DashFmp4Parser::StaticTrackFragment::parseCompactSampleSizes(
        DashFmp4Parser *parser, uint32_t type, size_t offset, uint64_t size) {
    	
	ALOGD("StaticTrackFragment[%s]",__FUNCTION__);

    if (offset + 12 > size) {
        return ERROR_MALFORMED;
    }

    if (parser->readU32(offset) != 0) {
        return ERROR_MALFORMED;
    }

    uint32_t fieldSize = parser->readU32(offset + 4);

    if (fieldSize != 4 && fieldSize != 8 && fieldSize != 16) {
        return ERROR_MALFORMED;
    }

    uint32_t sampleCount = parser->readU32(offset + 8);

    if (offset + 12 + (sampleCount * fieldSize + 4) / 8 != size) {
        return ERROR_MALFORMED;
    }

    parser->copyBuffer(&mCompactSampleSizes, offset, size);

    mSampleCount = sampleCount;

    return OK;
}

status_t DashFmp4Parser::StaticTrackFragment::parseSampleToChunk(
        DashFmp4Parser *parser, uint32_t type, size_t offset, uint64_t size) {
    	
	ALOGD("StaticTrackFragment[%s]",__FUNCTION__);

    if (offset + 8 > size) {
        return ERROR_MALFORMED;
    }

    if (parser->readU32(offset) != 0) {
        return ERROR_MALFORMED;
    }

    uint32_t entryCount = parser->readU32(offset + 4);

    if (entryCount == 0) {
        return OK;
    }

    if (offset + 8 + entryCount * 12 != size) {
        return ERROR_MALFORMED;
    }

    parser->copyBuffer(&mSampleToChunk, offset, size);

    return OK;
}

status_t DashFmp4Parser::StaticTrackFragment::parseChunkOffsets(
        DashFmp4Parser *parser, uint32_t type, size_t offset, uint64_t size) {
    	
	ALOGD("StaticTrackFragment[%s]",__FUNCTION__);

    if (offset + 8 > size) {
        return ERROR_MALFORMED;
    }

    if (parser->readU32(offset) != 0) {
        return ERROR_MALFORMED;
    }

    uint32_t entryCount = parser->readU32(offset + 4);

    if (offset + 8 + entryCount * 4 != size) {
        return ERROR_MALFORMED;
    }

    parser->copyBuffer(&mChunkOffsets, offset, size);

    return OK;
}

status_t DashFmp4Parser::StaticTrackFragment::parseChunkOffsets64(
        DashFmp4Parser *parser, uint32_t type, size_t offset, uint64_t size) {
    	
	ALOGD("StaticTrackFragment[%s]",__FUNCTION__);

    if (offset + 8 > size) {
        return ERROR_MALFORMED;
    }

    if (parser->readU32(offset) != 0) {
        return ERROR_MALFORMED;
    }

    uint32_t entryCount = parser->readU32(offset + 4);

    if (offset + 8 + entryCount * 8 != size) {
        return ERROR_MALFORMED;
    }

    parser->copyBuffer(&mChunkOffsets64, offset, size);

    return OK;
}



}  // namespace android




