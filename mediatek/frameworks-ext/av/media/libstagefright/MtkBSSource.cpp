
#define LOG_TAG "MtkBSSource"
#include <utils/Log.h>
#include <cutils/xlog.h>
#include <utils/Condition.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/CameraSource.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaDefs.h>
#include <media/mediarecorder.h>
#include <cutils/properties.h>

//#include <media/MediaProfiles.h>

#include <MtkBSSource.h>

#define ALOGV(fmt, arg...)       XLOGV("[%s] "fmt, __FUNCTION__, ##arg)
#define ALOGD(fmt, arg...)       XLOGD("[%s] "fmt, __FUNCTION__, ##arg)
#define ALOGI(fmt, arg...)       XLOGI("[%s] "fmt, __FUNCTION__, ##arg)
#define ALOGW(fmt, arg...)       XLOGW("[%s] "fmt, __FUNCTION__, ##arg)
#define ALOGE(fmt, arg...)       XLOGE("[%s] "fmt, __FUNCTION__, ##arg)

namespace android {



/******************************************************************************
*
*******************************************************************************/
sp<MediaSource> MtkBSSource::Create(const sp<MediaSource> &source, const sp<MetaData> &meta) {
    return new MtkBSSource(source, meta);
}


/******************************************************************************
*
*******************************************************************************/
MtkBSSource::MtkBSSource(const sp<MediaSource> &source, const sp<MetaData> &meta)
	:mSource(source)
	,mLock()
	,mStarted(false)
	,mCodecConfigReceived(false)
	,mNeedDropFrame(true)
	,mOutputFormat(meta)
	{
	ALOGD("+");

	if (OK != setEncParam(meta))
		CHECK(!"set encoder parameter for direct link failed!");

	ALOGD("-");
}

/******************************************************************************
*
*******************************************************************************/
MtkBSSource::~MtkBSSource() {
	ALOGD("+");

	stop();

	if(mSource != NULL)
		mSource.clear();

	ALOGD("-");
}

/******************************************************************************
*
*******************************************************************************/
status_t MtkBSSource::setEncParam( const sp<MetaData> &meta) {
	ALOGD("+");

	int32_t videoBitRate = 0;
	int32_t videoEncoder = 0;
	const char *mime;
	int32_t IFramesIntervalSec = 1;

    if(!meta->findInt32(kKeyBitRate, &videoBitRate)) {
		ALOGE("not set video bit rate");
		return UNKNOWN_ERROR;
	}
	
	if(!meta->findCString(kKeyMIMEType, &mime)) {
		ALOGE("not set video mime type");
		return UNKNOWN_ERROR;
	}

	if (!strcasecmp(MEDIA_MIMETYPE_VIDEO_AVC, mime)) {
		videoEncoder = VIDEO_ENCODER_H264;
#ifdef MTK_VIDEO_HEVC_SUPPORT
	} else if (!strcasecmp(MEDIA_MIMETYPE_VIDEO_HEVC, mime)) {
		videoEncoder = VIDEO_ENCODER_HEVC;
#endif
	} else if (!strcasecmp(MEDIA_MIMETYPE_VIDEO_MPEG4, mime)) {
		videoEncoder = VIDEO_ENCODER_MPEG_4_SP;
	} else if (!strcasecmp(MEDIA_MIMETYPE_VIDEO_H263, mime)) {
		videoEncoder = VIDEO_ENCODER_H263;
	} else {
		ALOGE("Not a supported video mime type: %s", mime);
		CHECK(!"Should not be here. Not a supported video mime type.");
	}
	
    if(!meta->findInt32(kKeyIFramesInterval, &IFramesIntervalSec)) {
   	
		ALOGE("not set I frames interval");
	   	return UNKNOWN_ERROR;
   	}


	char param[PROPERTY_VALUE_MAX];
    sprintf(param, "%d", videoBitRate);
	ALOGD("property_set video bit rate %s", param);
    property_set("dl.vr.set.bit.rate", param);

    sprintf(param, "%d", videoEncoder);
	ALOGD("property_set video mime type %s", param);
    property_set("dl.vr.set.encoder", param);

    sprintf(param, "%d", IFramesIntervalSec);
	ALOGD("property_set I frames interval %s", param);
    property_set("dl.vr.set.iframes.interval", param);
	
	ALOGD("-");
	return OK;
}


/******************************************************************************
*
*******************************************************************************/
status_t MtkBSSource::start(MetaData *params) {
	ALOGD("+");
	Mutex::Autolock _lock(mLock);
	
	if(mSource == NULL)	{
		ALOGE("Failed: mSource is NULL");
		return UNKNOWN_ERROR;
	}
	status_t err = mSource->start(params);
	if (err != OK) {
		ALOGE("Failed: source start err(%d)", err);
		return err;
	}

	mStarted = true;
	
	ALOGD("-");
	return err;
}
/******************************************************************************
*
*******************************************************************************/
status_t MtkBSSource::stop() 
{
	ALOGD("+");
    Mutex::Autolock _lock(mLock);

	if(!mStarted)
		return OK;
	
	mStarted = false;
		
	status_t err = OK;
	if(mSource != NULL)	{
		ALOGD("mSource stop()");
		err = mSource->stop();
	}

	ALOGD("-");
	return err; 
}

/******************************************************************************
*
*******************************************************************************/
status_t MtkBSSource::read(MediaBuffer **buffer, const ReadOptions *options) {
	ALOGD("+");

    *buffer = NULL;
	if(options != NULL) {
		ALOGE("Failed: MtkBSSource dose not support read options");
		return ERROR_UNSUPPORTED;
	}
	
	{
		Mutex::Autolock _lock(mLock);

		if(!mStarted || mSource == NULL)
			return UNKNOWN_ERROR;

		status_t err = OK;
		if(mCodecConfigReceived && mNeedDropFrame) {
			err = dropFrame(buffer);
			mNeedDropFrame = false;
		}
		else {
			status_t err = mSource->read(buffer); 
		}
		
		if(err != OK)
			return err;

        if (!mCodecConfigReceived)
        {
			ALOGD("got codec config data, size=%d", (*buffer)->range_length());
            (*buffer)->meta_data()->setInt32(kKeyIsCodecConfig, true);

			uint32_t frameSize = *((uint32_t*)((uint8_t*)((*buffer)->data())+(*buffer)->range_length()-4));
			ALOGD("frame size is %d", frameSize);
			(*buffer)->set_range(0, frameSize);
			
            mCodecConfigReceived = true;
        }
		else {
			ALOGV("got bitstream, size=%d", (*buffer)->range_length());
			ALOGV("last 4 byte =%x", *((uint32_t*)((uint8_t*)((*buffer)->data())+(*buffer)->range_length()-4)));
			bool bitstreamState = *((uint8_t*)((*buffer)->data())+(*buffer)->range_length()-6);

			if(!bitstreamState) {
				ALOGE("get failed bitstream, return UNKNOWN_ERROR");
				(*buffer)->release();
				(*buffer) = NULL;
				return UNKNOWN_ERROR;
			}
			bool isSyncFrame = *((uint8_t*)((*buffer)->data())+(*buffer)->range_length()-5);
			if (isSyncFrame) {
				ALOGD("Got an I frame");
				(*buffer)->meta_data()->setInt32(kKeyIsSyncFrame, true);
			}

			uint32_t frameSize = *((uint32_t*)((uint8_t*)((*buffer)->data())+(*buffer)->range_length()-4));
			ALOGD("frame size is %d", frameSize);
			(*buffer)->set_range(0, frameSize);
			
		}
		ALOGD("-");
		return OK;
	}
}

/******************************************************************************
*
*******************************************************************************/
sp<MetaData> MtkBSSource::getFormat() {
	return mOutputFormat;
}

/******************************************************************************
*
*******************************************************************************/
status_t MtkBSSource::dropFrame(MediaBuffer **buffer) { // drop frame before 2nd I Frame
	int32_t iFframeCnt = 0;

	while(1) {
		status_t err = mSource->read(buffer); 
		if(err != OK)
			return err;
		
		bool isSyncFrame = *((uint8_t*)((*buffer)->data())+(*buffer)->range_length()-5);
		if (isSyncFrame) {
			ALOGD("Got an I frame");
			iFframeCnt++;
		}

		if(iFframeCnt < 1) {
			ALOGD("drop the frame before 2nd I Frame");
			(*buffer)->release();
			(*buffer) = NULL;
		}
		else
			break;
	}

	return OK;
}

}

