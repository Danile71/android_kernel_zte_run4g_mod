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

#ifdef MTK_SUBTITLE_SUPPORT

//#define LOG_NDEBUG 0
#define LOG_TAG "TimedTextDVBSource"
#include <utils/Log.h>

#include <binder/Parcel.h>
#include <media/stagefright/foundation/ADebug.h>  // CHECK_XX macro
#include <media/stagefright/foundation/ABitReader.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MediaDefs.h>  // for MEDIA_MIMETYPE_xxx
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MediaSource.h>
#include <media/stagefright/MetaData.h>
#include "TextDescriptions.h"


#include "TimedTextDVBSource.h"
#include "TextDescriptions.h"


namespace android
{

TimedTextDVBSource::TimedTextDVBSource(const sp<MediaSource>& mediaSource)
    : mSource(mediaSource)
{
    mDvbParser = NULL;
}

TimedTextDVBSource::~TimedTextDVBSource()
{
}

status_t TimedTextDVBSource::read(
    int64_t *startTimeUs, int64_t *endTimeUs, Parcel *parcel,
    const MediaSource::ReadOptions *options)
{
    MediaBuffer *textBuffer = NULL;
	int32_t                   i4_ret;
	int64_t		KeyTime;
    bool        fg_need_update_logical_sz = FALSE;
	
    status_t err = mSource->read(&textBuffer, options);
    if (err != OK)
    {
        ALOGE("mSource->read() failed, error code %d\n", err);
        return err;
    }	
	
    CHECK(textBuffer != NULL);
    textBuffer->meta_data()->findInt64(kKeyTime, &KeyTime);
	*startTimeUs = KeyTime;
	
	
    uint8_t* content = (uint8_t*)textBuffer->data();
    size_t size = textBuffer->size();

	ALOGI( "timeUS[%lld],size[%d] of this subtitle\n",KeyTime/1000, size);
	ALOGI("DVB subtitle content,%02x %02x %02x %02x %02x %02x %02x %02x\n", 
						content[0],content[1],content[2],content[3],
						content[4],content[5],content[6],content[7]);
    CHECK_GE(*startTimeUs, 0);
	
    //parse
	i4_ret = mDvbParser->parseSegment(content,size,fg_need_update_logical_sz);
	if (DVBR_OK != i4_ret)
    {
        ALOGE("mDvbParser->parseSegment fail i4_ret=:%d\n",i4_ret);
        //sm_sbtl_page_inst_delete(pt_page_inst);
#if 0
        if (DVBR_MEM_USAGE_CHANGED == i4_ret)
        {
            DVB_LOG( "reparse DVBR_MEM_USAGE_CHANGED\n",0 );

            sm_sbtl_rgn_mngr_free_all(pt_this->pt_rgn_mngr);
            sm_sbtl_clut_mngr_free_all(pt_this->pt_CLUT_mngr);
            sm_sbtl_obj_mngr_free_all(pt_this->pt_obj_mngr);
            #ifdef DVB_ENABLE_HD_SUBTITLE
            sm_sbtl_dds_mngr_free_all(pt_this->pt_dds_mngr);
            #endif
            
            i4_ret = _sm_sbtl_service_parse_segment(pt_this,
                                            pui1_data,
                                            ui4_idx,
                                            ui4_data_len,
                                            ui8_PTS,
                                            &fg_need_update_logical_sz,
                                            &pt_page_inst);
            if (DVBR_OK != i4_ret)
            {
                return;
            }
            
            #ifdef DVB_ENABLE_HD_SUBTITLE
            sm_sbtl_dds_mngr_reset_logi_size( pt_this->pt_dds_mngr, pt_this->t_base.t_scdb_rec.u.t_dvb_sbtl.ui2_comp_pg_id);
            #endif

        }
        else
#endif
        {
            return OK;
        }
    }

    ALOGE("mDvbParser->parseSegment fg_need_update_logical_sz=:%d\n",fg_need_update_logical_sz);
    if (fg_need_update_logical_sz)
    {
        #ifdef DVB_ENABLE_HD_SUBTITLE
        mDvbParser->requestUpdateLogicalSize();
        #endif
    }
    
	//paint obj
	i4_ret = mDvbParser->paintAllRegion();
    if (i4_ret != DVBR_OK)
    {
        ALOGI("ERROR: TimedTextDVBSource::read failed ret=%d\n", i4_ret);
        return UNKNOWN_ERROR;
    }
	
	//paint region
	mDvbParser->paintPage();

    //release page instance
    mDvbParser->ReleasePage();

	ALOGE("Call extractAndAppendLocalDescriptions, send data to \n");
    extractAndAppendLocalDescriptions(*startTimeUs, textBuffer, parcel);
		
    textBuffer->release();
    *endTimeUs = -1;
    ALOGE("read() finished\n");

    mDvbParser->incTmpFileIdx();
    return OK;
}

// Each text sample consists of a string of text, optionally with sample
// modifier description. The modifier description could specify a new
// text style for the string of text. These descriptions are present only
// if they are needed. This method is used to extract the modifier
// description and append it at the end of the text.
status_t TimedTextDVBSource::extractAndAppendLocalDescriptions(
    int64_t timeUs, const MediaBuffer *textBuffer, Parcel *parcel)
{
    const void *data;
    size_t size = 0;    

    const char *mime;
    CHECK(mSource->getFormat()->findCString(kKeyMIMEType, &mime));
    CHECK(strcasecmp(mime, MEDIA_MIMETYPE_TEXT_DVB) == 0);

    data = textBuffer->data();
    size = textBuffer->size();
    int fd = mDvbParser->getTmpFileIdx();
    int timeMs = timeUs/1000;
    int width = mDvbParser->mBitmapWidth;
    int height = mDvbParser->mBitmapHeight;

    int flag = TextDescriptions::LOCAL_DESCRIPTIONS |
               TextDescriptions::IN_BAND_TEXT_DVB;
    ALOGE("[DVB] here TimedTextDVBSource::extractAndAppendLocalDescriptions(%d) called\n",fd);

    if (size > 0)
    {
        parcel->freeData();
        flag |= TextDescriptions::IN_BAND_TEXT_DVB;
        return TextDescriptions::getParcelOfDescriptions(
                   fd, width, height, flag, timeMs, parcel);
    }

    return OK;
}

// To extract and send the global text descriptions for all the text samples
// in the text track or text file.
// TODO: send error message to application via notifyListener()...?
status_t TimedTextDVBSource::extractGlobalDescriptions(Parcel *parcel)
{
    const void *data;
    size_t size = 0;
    int32_t flag = TextDescriptions::GLOBAL_DESCRIPTIONS;


    ALOGE("[DVB] here TimedTextDVBSource::extractGlobalDescriptions() called\n");

    const char *mime;
    CHECK(mSource->getFormat()->findCString(kKeyMIMEType, &mime));
    CHECK(strcasecmp(mime, MEDIA_MIMETYPE_TEXT_DVB) == 0);

    uint32_t type;
    // get the 'tx3g' box content. This box contains the text descriptions
    // used to render the text track
    if (!mSource->getFormat()->findData(
                kKeyTextFormatData, &type, &data, &size))
    {
        return ERROR_MALFORMED;
    }

    if (size > 0)
    {
        flag |= TextDescriptions::IN_BAND_TEXT_DVB;
        return TextDescriptions::getParcelOfDescriptions(
                   (const uint8_t *)data, size, flag, 0, parcel);
    }
    return OK;
}

sp<MetaData> TimedTextDVBSource::getFormat()
{
    ALOGE("[DVB] here TimedTextDVBSource::getFormat() called\n");
    return mSource->getFormat();
}

status_t TimedTextDVBSource::start()
{
    ALOGE("[DVB] here TimedTextDVBSource::start() called\n");
    if (mDvbParser != NULL)
    {
        delete mDvbParser;
        mDvbParser = NULL;
    }
    
    mDvbParser = DVBParser::getInstance();
    mDvbParser->prepareBitmapBuffer();
    
    return mSource->start();
}

status_t TimedTextDVBSource::stop()
{
    ALOGE("[DVB] here TimedTextDVBSource::stop() called\n");

    if (mDvbParser != NULL)
    {
        mDvbParser->unmapBitmapBuffer();
        //mDvbParser->unInit();
        delete mDvbParser;
        mDvbParser = NULL;
    }
	
    return mSource->stop();
}


}  // namespace android

#endif

