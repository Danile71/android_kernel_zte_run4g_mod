//#define LOG_NDEBUG 0
#define LOG_TAG "RepeaterSource"
#include <utils/Log.h>

#include "RepeaterSource.h"

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MetaData.h>

#ifndef ANDROID_DEFAULT_CODE

#include <cutils/properties.h>
#include "DataPathTrace.h"

#ifdef MTB_SUPPORT
#define ATRACE_TAG ATRACE_TAG_MTK_WFD
#include <utils/Trace.h>
#endif

#define WFD_LOGI(fmt,arg...) ALOGI(fmt,##arg)

#else
#define WFD_LOGI(fmt,arg...)
#endif

namespace android {

#ifndef ANDROID_DEFAULT_CODE	
void RepeaterSource::read_pro(int64_t timeUs){ 
#ifdef MTB_SUPPORT    
    ATRACE_ONESHOT(ATRACE_ONESHOT_VDATA, "Repeater, TS: %lld ms", timeUs/1000);
#endif

	int32_t usedTimes=0;
	if(mBuffer->meta_data()->findInt32('used', &usedTimes)){
		mBuffer->meta_data()->setInt32('used', usedTimes+1);
		mReadOutCountRpt++;

	}else{
#ifdef MTB_SUPPORT
        ATRACE_NAME_EXT("WFD_VFrame");
#endif
		mBuffer->meta_data()->setInt32('used', 1);
		mReadOutCountNew++;
	}

	int64_t gotTime,delayTime,readTime;
	sp<WfdDebugInfo> debugInfo= defaultWfdDebugInfo();
	if( mBuffer->meta_data()->findInt64('RpIn', &gotTime) ){
		int64_t nowUs = ALooper::GetNowUs();
		if(usedTimes > 0) {
			delayTime = 0; 
			gotTime = nowUs/1000;
			ALOGV("[WFDP]this buffer has beed used for %d times",usedTimes);
		}else{
			delayTime = (nowUs - gotTime*1000)/1000;
		}

	        debugInfo->addTimeInfoByKey(1, timeUs, "RpIn", gotTime);
	        debugInfo->addTimeInfoByKey(1, timeUs, "RpDisPlay", usedTimes);
	        debugInfo->addTimeInfoByKey(1, timeUs, "DeMs", delayTime);
	        debugInfo->addTimeInfoByKey(1, timeUs, "RpOt", nowUs/1000);
		 if( mBuffer->meta_data()->findInt64('RtMs', &readTime) ){
			debugInfo->addTimeInfoByKey(1, timeUs, "RpReadTimeMs", readTime);
		}

	}   
	int32_t latencyToken = 0;
	if(mBuffer->meta_data()->findInt32(kKeyWFDLatency, &latencyToken)){
		debugInfo->addTimeInfoByKey(1, timeUs, "LatencyToken", latencyToken);
#ifdef MTB_SUPPORT
		int32_t usedTimes=0;
		if(mBuffer->meta_data()->findInt32('used', &usedTimes) && usedTimes == 1){
		    ATRACE_ASYNC_END("STG-MPR", latencyToken);
		}
		ATRACE_ASYNC_BEGIN("MPR-CNV", latencyToken);
#endif
	}
	
	int64_t nowUs = ALooper::GetNowUs();
	int64_t totalTimeUs = nowUs - mStartCountTime ;
	if( (mReadOutCountNew+mReadOutCountRpt ) % 30 == 0){
		ALOGI("[WFD_P]:SMS in FPS %lld, read FPS %lld, repeat/new /total =  %d/%d/%d",
			mReadInCount*1000000ll / totalTimeUs,
			mReadOutCountNew*1000000ll / totalTimeUs,
			mReadOutCountRpt,mReadOutCountNew,mReadOutCountNew+mReadOutCountRpt);
		mReadInCount = 0;
		mReadOutCountRpt = 0;
		mReadOutCountNew = 0;	
		mStartCountTime = -1;

	}


    ALOGV("[WFDP][video]read one video buffer  framecount = %d, bufferTimeUs = %lld ms", mFrameCount, bufferTimeUs / 1000);

    //workaround for encoder init slow
    if(mFrameCount == 1)
    {
        mFrameCount = 6;
        ALOGI("read deley 5frames times");
    }
}


void RepeaterSource::read_fps(int64_t timeUs,int64_t readTimeUs){ 

	mBuffer->meta_data()->setInt64('RpIn', (mLastBufferUpdateUs / 1000));
	mBuffer->meta_data()->setInt64('RtMs', (readTimeUs / 1000));
	WFD_LOGI("[WFDP][video]read MediaBuffer %p,readtime=%lld ms",mBuffer, readTimeUs/1000);

	int32_t latencyToken = 0;
	if(mBuffer->meta_data()->findInt32(kKeyWFDLatency, &latencyToken)){
#ifdef MTB_SUPPORT
		ATRACE_ASYNC_BEGIN("STG-MPR", latencyToken);
#endif
	}

	
	if(mStartCountTime <  0){
		mStartCountTime = mLastBufferUpdateUs;
		mReadInCount = 0;
		mReadOutCountRpt = 0;
		mReadOutCountNew = 0;	
	}
  	mReadInCount++;
}



status_t RepeaterSource::stop_l() {
	CHECK(mStarted);
	mStopping = true;

	WFD_LOGI("stopping");

	if (mBuffer != NULL) {
		WFD_LOGI("releasing mbuf %p£¬ refcnt= %d ", mBuffer,mBuffer->refcount());
		mBuffer->release();
		mBuffer = NULL;
	}
	status_t err = mSource->stop();
	WFD_LOGI("stopped source ");


	if (mLooper != NULL) {
		mLooper->stop();
		mLooper.clear();
		mReflector.clear();
	}
	WFD_LOGI("stopped repeater looper ");
	mStarted = false;
	WFD_LOGI("stopped");
	return err;
}

#endif

RepeaterSource::RepeaterSource(const sp<MediaSource> &source, double rateHz)
    : mStarted(false),
      mSource(source),
      mRateHz(rateHz),
      mBuffer(NULL),
      mResult(OK),
      mLastBufferUpdateUs(-1ll),
      mStartTimeUs(-1ll),
      mFrameCount(0) {
      WFD_LOGI("FrameRate %.2f",mRateHz);  
#ifndef ANDROID_DEFAULT_CODE	
	mStartCountTime  = -1;
	mReadInCount   = 0;
	mReadOutCountNew  = 0;
	mReadOutCountRpt  = 0;
	mStopping = false;
	
#endif
}

RepeaterSource::~RepeaterSource() {
    CHECK(!mStarted);
}

double RepeaterSource::getFrameRate() const {
    return mRateHz;
}

void RepeaterSource::setFrameRate(double rateHz) {
    Mutex::Autolock autoLock(mLock);

    if (rateHz == mRateHz) {
        return;
    }

    if (mStartTimeUs >= 0ll) {
        int64_t nextTimeUs = mStartTimeUs + (mFrameCount * 1000000ll) / mRateHz;
        mStartTimeUs = nextTimeUs;
        mFrameCount = 0;
    }
    mRateHz = rateHz;
}

status_t RepeaterSource::start(MetaData *params) {
    CHECK(!mStarted);
#ifndef ANDROID_DEFAULT_CODE	
	mStopping = false;	
#endif	
#ifdef MTB_SUPPORT	
    ATRACE_CALL_EXT();
#endif
    WFD_LOGI("start++");

    status_t err = mSource->start(params);

    if (err != OK) {
    	 WFD_LOGI("surfaceMediaSource start err");
        return err;
    }

    mBuffer = NULL;
    mResult = OK;
    mStartTimeUs = -1ll;
    mFrameCount = 0;

    mLooper = new ALooper;
    mLooper->setName("repeater_looper");
    mLooper->start();

    mReflector = new AHandlerReflector<RepeaterSource>(this);
    mLooper->registerHandler(mReflector);

    postRead();

    mStarted = true;
    WFD_LOGI("start ---");
    return OK;
}

status_t RepeaterSource::stop() {

#ifndef ANDROID_DEFAULT_CODE	
	return stop_l();
#endif
 
    CHECK(mStarted);

    WFD_LOGI("stopping");

    if (mLooper != NULL) {
        mLooper->stop();
        mLooper.clear();

        mReflector.clear();
    }

    if (mBuffer != NULL) {
        WFD_LOGI("releasing mbuf %p", mBuffer);
        mBuffer->release();
        mBuffer = NULL;
    }

    status_t err = mSource->stop();

    WFD_LOGI("stopped");

    mStarted = false;

    return err;
}

sp<MetaData> RepeaterSource::getFormat() {
    return mSource->getFormat();
}

status_t RepeaterSource::read(
        MediaBuffer **buffer, const ReadOptions *options) {
    int64_t seekTimeUs;
    ReadOptions::SeekMode seekMode;
    CHECK(options == NULL || !options->getSeekTo(&seekTimeUs, &seekMode));

    for (;;) {
        int64_t bufferTimeUs = -1ll;

        if (mStartTimeUs < 0ll) {
            Mutex::Autolock autoLock(mLock);
            while ((mLastBufferUpdateUs < 0ll || mBuffer == NULL)
                    && mResult == OK) {
                mCondition.wait(mLock);
            }

            ALOGV("now resuming.");
            mStartTimeUs = ALooper::GetNowUs();
            bufferTimeUs = mStartTimeUs;
	     WFD_LOGI("now resuming.mStartTimeUs=%lld ms",mStartTimeUs/1000);
        } else {
            bufferTimeUs = mStartTimeUs + (mFrameCount * 1000000ll) / mRateHz;

            int64_t nowUs = ALooper::GetNowUs();
            int64_t delayUs = bufferTimeUs - nowUs;

            if (delayUs > 0ll) {
                usleep(delayUs);
            }
        }

        bool stale = false;

        {
	   
            Mutex::Autolock autoLock(mLock);
            if (mResult != OK) {
                CHECK(mBuffer == NULL);
		        WFD_LOGI("read return error %d",mResult);
                return mResult;
            }

#if SUSPEND_VIDEO_IF_IDLE
            int64_t nowUs = ALooper::GetNowUs();
            if (nowUs - mLastBufferUpdateUs > 1000000ll) {
                mLastBufferUpdateUs = -1ll;
                stale = true;
		        WFD_LOGI("[video buffer] has not  been updated than >1S");
            } else
#endif
            {
                mBuffer->add_ref();
                *buffer = mBuffer;
                (*buffer)->meta_data()->setInt64(kKeyTime, bufferTimeUs);
                ++mFrameCount;
#ifndef ANDROID_DEFAULT_CODE
	            read_pro(bufferTimeUs);
#endif
            }

        }

        if (!stale) {
            break;
        }

        mStartTimeUs = -1ll;
        mFrameCount = 0;
        ALOGI("now dormant");
    }

    return OK;
}

void RepeaterSource::postRead() {
#ifndef ANDROID_DEFAULT_CODE
	 if(mStopping){
		ALOGI("Stopping now, ingore read commad");
		return;
	 }
#endif	
    (new AMessage(kWhatRead, mReflector->id()))->post();
}

void RepeaterSource::onMessageReceived(const sp<AMessage> &msg) {
    switch (msg->what()) {
        case kWhatRead:
        {
            MediaBuffer *buffer;
#ifdef MTB_SUPPORT			
            ATRACE_BEGIN_EXT("Repeater, KWhatRead");
#endif
#ifndef ANDROID_DEFAULT_CODE
            int64_t startUs = ALooper::GetNowUs();
#endif
            status_t err = mSource->read(&buffer);
#ifndef ANDROID_DEFAULT_CODE
            if(err != OK){
                ALOGE("[SMS is ERROR EOS now!!!],should not err %d",err);
                return;
            }	else if(mStopping){
			if (buffer != NULL) {		           
				WFD_LOGI("read while stopping buffer=%p,refcnt= %d ", 
					buffer,static_cast<MediaBuffer *>(buffer)->refcount());
				buffer->release();
				buffer = NULL;
			}
			return;
	}
#endif
            ALOGV("read mbuf %p", buffer);


            Mutex::Autolock autoLock(mLock);
            if (mBuffer != NULL) {
	           
#ifndef ANDROID_DEFAULT_CODE	
            
            int32_t used=0;
		    if(!mBuffer->meta_data()->findInt32('used', &used) ){
#ifdef MTB_SUPPORT
                ATRACE_ONESHOT(ATRACE_ONESHOT_SPECIAL, "RptSrc_DropFrm"); 
#endif
                WFD_LOGI("[video buffer] mBuffer=%p is not used before release,used=%d,refcnt= %d ", mBuffer,used,mBuffer->refcount());
		    }
#endif			
				
                mBuffer->release();
                mBuffer = NULL;
            }
            mBuffer = buffer;
            mResult = err;
            mLastBufferUpdateUs = ALooper::GetNowUs();

#ifndef ANDROID_DEFAULT_CODE	
           read_fps(mLastBufferUpdateUs,mLastBufferUpdateUs-startUs);
#endif	

            mCondition.broadcast();

            if (err == OK) {
                postRead();
            }
#ifdef MTB_SUPPORT			
            ATRACE_END_EXT("Repeater, KWhatRead");
#endif
            break;
        }

        default:
            TRESPASS();
    }
}

void RepeaterSource::wakeUp() {
    ALOGV("wakeUp");
    Mutex::Autolock autoLock(mLock);
    if (mLastBufferUpdateUs < 0ll && mBuffer != NULL) {
        mLastBufferUpdateUs = ALooper::GetNowUs();
        mCondition.broadcast();
    }
}


}  // namespace android
