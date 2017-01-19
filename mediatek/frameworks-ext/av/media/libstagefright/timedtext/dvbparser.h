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

#ifndef __DVB_PARSER_H_
#define __DVB_PARSER_H_

#include <core/SkBitmap.h>
//#include <images/SkImageEncoder.h>

#include "DvbClut.h"
#include "DvbPage.h"
#include "DvbRegion.h"
#include "DvbObject.h"

#ifdef DVB_ENABLE_HD_SUBTITLE 
#include "DvbDds.h"
#endif



#include "DvbClutMgr.h"
#include "DVbPageMgr.h"
#include "DvbRegionMgr.h"
#include "DvbObjectMgr.h"

#ifdef DVB_ENABLE_HD_SUBTITLE 
#include "DVBDdsMgr.h"
#endif

#define DVB_TMP_FILE_COUNT  4
#define DVB_TMP_FILE_MASK  (DVB_TMP_FILE_COUNT - 1)

namespace android
{

class DVBParser
{
    public:
        static DVBParser* getInstance() {
        DVBParser* instance = _sInstance;
        if (instance == 0) {
            instance = new DVBParser();
            _sInstance = instance;
        }
        return instance;
    }

    static bool hasInstance() {
        return _sInstance != 0;
    }
    
        ~DVBParser();
        int prepareBitmapBuffer();
        int unmapBitmapBuffer();
		int parseSegment(UINT8* pui1_data,size_t size,bool& fg_need_update_logical_sz);
        INT32 updateLogicalSize(UINT16  ui2_width, UINT16  ui2_height);
		INT32	 paintAllRegion();
		INT32	 paintPage();
		VOID	 ReleasePage();
        VOID     requestUpdateLogicalSize();
        INT32    judgeBoundary(UINT16 ui2_width, UINT16  ui2_height);
        INT32    adjustAddress(UINT16*   pui2_width, UINT16*  pui2_height);
        void    incTmpFileIdx() {mCurrTmpFileIdx++;};
        int     getTmpFileIdx() {return (mCurrTmpFileIdx & DVB_TMP_FILE_MASK);};
		int mBitmapWidth;
        int mBitmapHeight;


        int mFd[DVB_TMP_FILE_COUNT];
        void * mBitmapData[DVB_TMP_FILE_COUNT];
        unsigned int mCurrTmpFileIdx;
		DvbPage*	mCurrPage;	
		SkBitmap*  bitmap;
#if	DVB_SUBTITLE_SAVE_AS_YUV_FILE
		status_t prepareYuvFile();
		status_t stopRecordYuvFile();
		VOID	 Record2YuvFile(SkBitmap&  bitmap,unsigned int width,unsigned int height);
#endif

private:
        DVBParser();
        
        static DVBParser* _sInstance;
		DvbPageMgr		mPageMgr;
		UINT16			mPageId;
#ifdef DVB_ENABLE_HD_SUBTITLE 
		DvbDdsMgr		mDdsMgr;
#endif
		
		DvbRegionMgr	mRegionMgr;
		DvbObjectMgr	mObjectMgr;
		DvbClutMgr		mClutMgr;

		bool            mRecRefNewPage;

#if	DVB_SUBTITLE_SAVE_AS_YUV_FILE
		int mYuvFd;
#endif
};

}
#endif

