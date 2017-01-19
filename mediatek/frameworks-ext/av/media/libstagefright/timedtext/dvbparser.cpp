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
#include <sys/mman.h>
#include <fcntl.h>


#define LOG_TAG "DVBParser"
#include <utils/Log.h>

#define DVB_WITH_NEON   0

#if DVB_WITH_NEON
#include <arm_neon.h>
#endif


#include <binder/Parcel.h>
#include <media/stagefright/foundation/ADebug.h>  // CHECK_XX macro
#include <media/stagefright/foundation/ABitReader.h>


#include "dvbparser.h"

namespace android
{

/*buffer size for HD */
#define DVB_FILE_BUFFER_SIZE (1280 * 720 * 4)

#define DVB_SUBTITLE_SAVE_AS_YUV_FILE	0
#define DVB_PAINT_BITMAPDATA_DIRECTLY	1
#if DVB_SUBTITLE_SAVE_AS_YUV_FILE
const char g_yuv_file_path[] = "/sdcard/dvb_subtitle.yuv";
#endif

DVBParser* DVBParser::_sInstance = NULL;

DVBParser::DVBParser()
{
	for (int i=0;i < DVB_TMP_FILE_COUNT; i++)mFd[i] = -1;
    mCurrTmpFileIdx = 0;
    for (int i=0;i < DVB_TMP_FILE_COUNT; i++)mBitmapData[i] = NULL;
    mCurrPage = NULL;
	mBitmapWidth = DvbPage::PageDefaultWidth;
	mBitmapHeight = DvbPage::PageDefaultHeight;
	
	bitmap = new SkBitmap;

	ALOGI("alloc a skia bitmap whick width=%d,height = %d\n",mBitmapWidth,mBitmapHeight);
	bitmap->setConfig(SkBitmap::kARGB_8888_Config, mBitmapWidth, mBitmapHeight);
    bitmap->allocPixels();
	
#if DVB_SUBTITLE_SAVE_AS_YUV_FILE
	mYuvFd = -1;
#endif

#if DVB_SUBTITLE_SAVE_AS_YUV_FILE
	unlink(g_yuv_file_path);
	prepareYuvFile();
#endif
}


DVBParser::~DVBParser()
{
#if DVB_SUBTITLE_SAVE_AS_YUV_FILE
     stopRecordYuvFile();
#endif

	delete bitmap;
    bitmap = NULL;

    mRegionMgr.freeAll();
	mClutMgr.freeAll();
	mObjectMgr.freeAll();
    ReleasePage();
    _sInstance = NULL;
}

INT32 DVBParser::updateLogicalSize(UINT16  ui2_width, UINT16  ui2_height)
{
    INT32           i4_ret;
    i4_ret = DVBR_OK;
    
    ALOGE("[RY] mBitmapHeight %d, mBitmapWidth %d\n", mBitmapHeight, mBitmapWidth);
    ALOGE("[RY] ui2_height %d, ui2_width %d\n", ui2_height, ui2_width);
    if ((mBitmapHeight == ui2_height ) && (ui2_width == mBitmapWidth))
    {
        //same ,return
        return DVBR_OK;
    }

    unmapBitmapBuffer();

    mBitmapWidth = ui2_width;
    mBitmapHeight = ui2_height;
    prepareBitmapBuffer();
    return i4_ret;
}

int DVBParser::
prepareBitmapBuffer()
{
    char filepath[256];
    //const char *filepath = "/sdcard/IdxSubBitmapBuffer.tmp";
    ALOGE("[RY] mBitmapHeight %d, mBitmapWidth %d\n", mBitmapHeight, mBitmapWidth);
    int bufSize = DVB_FILE_BUFFER_SIZE; //mBitmapHeight * mBitmapWidth;
    if ((mBitmapHeight * mBitmapWidth *4) > DVB_FILE_BUFFER_SIZE)
    {
        bufSize = mBitmapHeight * mBitmapWidth * 4;
    }
    
    ALOGE("[RY] bufSize is %d\n", bufSize);

    for (int i=0;i < DVB_TMP_FILE_COUNT; i++)
    {
        sprintf(filepath,"/sdcard/IdxSubBitmapBuffer_%d.tmp",i);
        mFd[i] = open(filepath, O_CREAT | O_RDWR | O_TRUNC, 0x777);
        
        if (-1 == mFd[i])
        {
            ALOGE("open fd:[%s], errno=%d:%s.", filepath, errno, strerror(errno));
            //reset();
            return -1;
        }
        ALOGD("[MY] --- start, open fd = 0x%x", mFd);
    	
#if 1
    	lseek(mFd[i], bufSize, SEEK_SET);
        write(mFd[i], "", 1);
#else    
    	/*if extst alreay ,set size to zero*/
    	if(-1 == ftruncate(mFd[i],0))
    	{
    		ALOGD(" ftruncate IdxSubBitmapBuffer.tmp size to zero");
    	}
#endif
#if 1
        //mBitmapData = mmap(0, bufSize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        mBitmapData[i] = mmap(0, bufSize, PROT_READ | PROT_WRITE, MAP_SHARED , mFd[i], 0);
        if (MAP_FAILED == mBitmapData[i])
        {
            ALOGE("start, mmap failed, errno=%d:%s.", errno, strerror(errno));
            //reset();
            return -1;
        }
    }
    ALOGD("prepareMapMemory done");
#endif
    return OK;
}

INT32 DVBParser::judgeBoundary(UINT16 ui2_width, UINT16  ui2_height)
{
    INT32           i4_ret;
    DvbDds*    pt_dds;
    
    i4_ret = DVBR_OK;     
    

    /*retrieve the match-id dds*/
    pt_dds = mDdsMgr.getDds(mPageId);
    
    if (NULL != pt_dds)
    {
        i4_ret = pt_dds->judgeBoundary(ui2_width, ui2_height);
    }
    else
    {
        if ( (ui2_width > (UINT16)(DvbPage::PageDefaultWidth))||
             (ui2_height > (UINT16)(DvbPage::PageDefaultHeight)) )
        {
            return DVBR_WRONG_SEGMENT_DATA;
        }
    }

    return i4_ret;
}

INT32 DVBParser::adjustAddress(UINT16*   pui2_width, UINT16*  pui2_height)
{
    INT32           i4_ret;
    DvbDds*         pt_dds;
    
    i4_ret = DVBR_OK;     
    
    /*retrieve the match-id dds*/
    pt_dds = mDdsMgr.getDds(mPageId);
    
    if (NULL != pt_dds)
    {
        i4_ret = pt_dds->adjustAddressbyBoundary(pui2_width, pui2_height);
    }
    else
    {
        if ( (*pui2_width  > (UINT16)(DvbPage::PageDefaultWidth))||
             (*pui2_height > (UINT16)(DvbPage::PageDefaultHeight)) )
        {
            return DVBR_WRONG_SEGMENT_DATA;
        }
    }
    
    return i4_ret;
}

VOID DVBParser::requestUpdateLogicalSize()
{
    mDdsMgr.updateLogiSize(mPageId);
}

int DVBParser::unmapBitmapBuffer()
{
    for (int i=0;i < DVB_TMP_FILE_COUNT; i++)
    {
        if (mBitmapData[i] > 0)
        {
            munmap(mBitmapData, DVB_FILE_BUFFER_SIZE);
            mBitmapData[i] = NULL;
        }
        
        if (mFd[i] >= 0)
        {
            close(mFd[i]);
            mFd[i] = -1;
        }
    }
    return OK;
}

//void DVBParser::unInit()
//{
    

//}

#if	DVB_SUBTITLE_SAVE_AS_YUV_FILE
int DVBParser::prepareYuvFile()
{
	mYuvFd = open(g_yuv_file_path, O_CREAT | O_RDWR | O_TRUNC, 0x777);
	if (-1 == mYuvFd)
    {
        ALOGE("open fd:[%s], errno=%d:%s.", g_yuv_file_path, errno, strerror(errno));
        //reset();
        return -1;
    }
	return OK;
}
status_t DVBParser::stopRecordYuvFile()
{
	if (mYuvFd > 0)
	{
		close(mYuvFd);
	}
	return OK;
}

VOID	 DVBParser::Record2YuvFile(SkBitmap&  bitmap,unsigned int width,unsigned int height)
{
	uint32_t u4RGB_value;
	uint8_t u1Y, u1Cb, u1Cr;
  	uint32_t u4R, u4G, u4B;
  	ALOGI("Record2YuvFile luma width = %d,htight = %d\n",width,height);
	/*save luma */
	for (unsigned int y = 0; y < height;y++)
	{
		uint8_t aui_y[width];
		for (unsigned int x = 0; x < width;x++)
		{
			u4RGB_value = *(bitmap.getAddr32(x,y));
			u4RGB_value = u4RGB_value & 0x00ffffff; /*ignore a*/
			u4R = (u4RGB_value>>16) & 0xff;
			u4G = (u4RGB_value>>8) & 0xff;
			u4B = u4RGB_value & 0xff;
			u1Y = (16 + (((u4R * 66) + (u4G * 129) + (u4B * 25)) / 255)) & 0xFF;
			aui_y[x] = u1Y;
		}
		write(mYuvFd,aui_y,width);
	}

	ALOGI("Record2YuvFile Cb width = %d,htight = %d\n",width/2,height/2);
	for (unsigned int y = 0; y < height/2;y++)
	{
		uint8_t aui_Cb[width];
		for (unsigned int x = 0; x < width/2;x++)
		{
			u4RGB_value = *(bitmap.getAddr32((x << 1),(y << 1)));
			u4RGB_value = u4RGB_value & 0x00ffffff; /*ignore a*/
			u4R = (u4RGB_value>>16) & 0xff;
			u4G = (u4RGB_value>>8) & 0xff;
			u4B = u4RGB_value & 0xff;
			u1Cb = ((((128 * 255) + (u4B * 112)) - ((u4R * 38) + (u4G * 74))) / 255) & 0xFF;
			aui_Cb[x] = u1Cb;
		}
		write(mYuvFd,aui_Cb,width/2);
	}

	ALOGI("Record2YuvFile Cr width = %d,htight = %d\n",width/2,height/2);
	for (unsigned int y = 0; y < height/2;y++)
	{
		uint8_t aui_Cr[width];
		for (unsigned int x = 0; x < width/2;x++)
		{
			u4RGB_value = *(bitmap.getAddr32((x << 1),(y << 1)));
			u4RGB_value = u4RGB_value & 0x00ffffff; /*ignore a*/
			u4R = (u4RGB_value>>16) & 0xff;
			u4G = (u4RGB_value>>8) & 0xff;
			u4B = u4RGB_value & 0xff;
			u1Cr = ((((128 * 255) + (u4R * 112)) - ((u4G * 94) + (u4B * 18))) / 255) & 0xFF;
			aui_Cr[x] = u1Cr;
		}
		write(mYuvFd,aui_Cr,width/2);
	}
}
#endif


status_t DVBParser::parseSegment(UINT8* pui1_data, size_t size, bool& fg_need_update_logical_sz)
{
	INT32					i4_ret;
	UINT16					ui2_page_id;
	UINT16					ui2_segment_len;
	size_t                  ui4_idx;
#ifdef DVB_ENABLE_HD_SUBTITLE
	bool					fg_has_dds = false;
#endif
	DvbPage*				pt_page;	
	ui4_idx = 0;
	
	//*ppt_page_inst = NULL;
	fg_need_update_logical_sz = FALSE;

	/* Parse DDS and PCS*/
	while (ui4_idx < size)
	{
		if (0x0F == pui1_data[ui4_idx]) /* Sync_byte is '0x0F' */
		{
			ui2_page_id = pui1_data[ui4_idx + 2];
			ui2_page_id = ui2_page_id << 8;
			ui2_page_id += pui1_data[ui4_idx + 3];
			DVB_LOG("the page id = %d",ui2_page_id);
			mPageId = ui2_page_id;
            
			ui2_segment_len = pui1_data[ui4_idx + 4];
			ui2_segment_len = ui2_segment_len << 8;
			ui2_segment_len += pui1_data[ui4_idx + 5];
			DVB_LOG("ui2_segment_len = %d",ui2_segment_len);

			DVB_LOG("segment type = %d",pui1_data[ui4_idx + 1]);
			/* PCS */
			if (0x10 == pui1_data[ui4_idx + 1])
			{
				/* Verify Page ID */
				//if ((ui2_page_id == pt_this->t_base.t_scdb_rec.u.t_dvb_sbtl.ui2_comp_pg_id) ||
				//	(ui2_page_id == pt_this->t_base.t_scdb_rec.u.t_dvb_sbtl.ui2_anci_pg_id))
				{
					/* Has PCS */
					i4_ret = mPageMgr.parseSegment(//ui8_PTS,
												  &(pui1_data[ui4_idx + 6]),
												  ui2_segment_len,
												  &pt_page);

					if (DVBR_OK > i4_ret)
					{
						return (i4_ret);
					}
					ui4_idx += (ui2_segment_len + 6);
					break;
				}
				ui4_idx += (ui2_segment_len + 6);
			}
			else if (0x14==pui1_data[ui4_idx + 1])	/*DDS*/
			{
          #ifdef DVB_ENABLE_HD_SUBTITLE
				/* Verify Page ID */
				//if ( (ui2_page_id == pt_this->t_base.t_scdb_rec.u.t_dvb_sbtl.ui2_comp_pg_id) ||
				//	 (ui2_page_id == pt_this->t_base.t_scdb_rec.u.t_dvb_sbtl.ui2_anci_pg_id) )
				{
					ALOGE( "DDS[-------------------]\n" );
					
					/* Has DDS */
					i4_ret = mDdsMgr.parseSegment(&(pui1_data[ui4_idx + 6]),
															ui2_segment_len,
															ui2_page_id);

					if (DVBR_OK > i4_ret)
					{
						ALOGE("ERROR: DDS parse fail\n");
						return (i4_ret);
					}

					if (DVBR_CONTENT_UPDATE == i4_ret)
					{
						fg_need_update_logical_sz = TRUE;
					}

					fg_has_dds = TRUE;

				}
          #endif /* DVB_ENABLE_HD_SUBTITLE */
					ui4_idx += (ui2_segment_len + 6);
			}
			else
			{
				break;
			}
		}
		else
		{
			ALOGE("ERROR: unknown expect first segment %d\n",  pui1_data[ui4_idx]);
			return (DVBR_WRONG_SEGMENT_DATA);
		}
	}
	DVB_LOG("parse segment %d",__LINE__);
	/* This page instance can not be handled */
	if (NULL == pt_page)
	{
		return (DVBR_WRONG_SEGMENT_DATA);
	}
	DVB_LOG("parse segment %d",__LINE__);
	mCurrPage = pt_page;
	DVB_LOG("parse segment %d",__LINE__);
	switch (pt_page->getState())
	{
	case DvbPage::STATE_MODE_CHANGE:
		mRegionMgr.freeAll();
		mClutMgr.freeAll();
		mObjectMgr.freeAll();

    #ifdef DVB_ENABLE_HD_SUBTITLE
		if (!fg_has_dds)
		{
			mDdsMgr.freeAll();
			fg_need_update_logical_sz = TRUE;
		}
    #endif

		//pt_this->b_is_rec_ref_new_page = TRUE;
		mRecRefNewPage = TRUE;
		break;

	case DvbPage::STATE_ACQUISITION_POINT:
		//sm_sbtl_obj_mngr_free_all(pt_this->pt_obj_mngr);
		mObjectMgr.freeAll();
		//pt_this->b_is_rec_ref_new_page = TRUE;
		mRecRefNewPage = TRUE;
		break;

	case DvbPage::STATE_NORMAL:
#ifndef SM_SBTL_CRS_PAGE_OBJ_SUPPORT
/*		  sm_sbtl_obj_mngr_free_all(pt_this->pt_obj_mngr); */
#endif
		break;

	default:
		break;
	}
    DVB_LOG("parse segment %d",__LINE__);
	/* Parse Data Segment */
	while (ui4_idx < size)
	{
		if (0x0F == pui1_data[ui4_idx])
		{
			ui2_page_id = pui1_data[ui4_idx + 2];
			ui2_page_id = ui2_page_id << 8;
			ui2_page_id += pui1_data[ui4_idx + 3];
			DVB_LOG("parse data segment ,the page id = %d",ui2_page_id);
			
			ui2_segment_len = pui1_data[ui4_idx + 4];
			ui2_segment_len = ui2_segment_len << 8;
			ui2_segment_len += pui1_data[ui4_idx + 5];
			DVB_LOG("parse data segment ,ui2_segment_len = %d",ui2_segment_len);
			
			/* check page id */
			DVB_LOG("parse data segment ,segment type = %d",pui1_data[ui4_idx + 1]);
			switch (pui1_data[ui4_idx + 1])
			{
			case 0x11: /* RCS */
				/* Verify Page ID */
				//if ( (ui2_page_id == pt_this->t_base.t_scdb_rec.u.t_dvb_sbtl.ui2_comp_pg_id) ||
				//	 (ui2_page_id == pt_this->t_base.t_scdb_rec.u.t_dvb_sbtl.ui2_anci_pg_id) )
				{
					i4_ret = mRegionMgr.parseSegment(pt_page,
												  &(pui1_data[ui4_idx + 6]),
												  ui2_segment_len);
					if (DVBR_OK > i4_ret)
					{
						ALOGE("ERROR: wrong RCS\n");
					
						/* CR146043 & CR146242: in the stream the state is acquisition point, but the memory usage changed */
						if ((DVBR_MEM_USAGE_CHANGED == i4_ret) && (DvbPage::STATE_ACQUISITION_POINT == pt_page->getState()))
						{
							return (DVBR_MEM_USAGE_CHANGED);
						}
						
						break;
					}

				}
				break;

			case 0x12: /* CLUT */
				/* Verify Page ID */
				//if ( (ui2_page_id == pt_this->t_base.t_scdb_rec.u.t_dvb_sbtl.ui2_comp_pg_id) ||
				//	 (ui2_page_id == pt_this->t_base.t_scdb_rec.u.t_dvb_sbtl.ui2_anci_pg_id) )
				{
					if (DVBR_OK > mClutMgr.parseSegment(mRegionMgr,
														   &(pui1_data[ui4_idx + 6]),
														   ui2_segment_len))
					{
						ALOGE("ERROR: wrong CLUT segment\n");
						break;
					}
				}
				break;

			case 0x13: /* obj */
				/* Verify Page ID */
				//if ( (ui2_page_id == pt_this->t_base.t_scdb_rec.u.t_dvb_sbtl.ui2_comp_pg_id) ||
				//	 (ui2_page_id == pt_this->t_base.t_scdb_rec.u.t_dvb_sbtl.ui2_anci_pg_id) )
				{
					if (DVBR_OK > mObjectMgr.parseSegment(mRegionMgr,
														  &(pui1_data[ui4_idx + 6]),
														  ui2_segment_len))
					{
						ALOGE("ERROR: wrong object segment\n");
						break;
					}
				}
				break;

			case 0x80: /* end of display set segment */
				//if ( (ui2_page_id == pt_this->t_base.t_scdb_rec.u.t_dvb_sbtl.ui2_comp_pg_id) ||
				//	 (ui2_page_id == pt_this->t_base.t_scdb_rec.u.t_dvb_sbtl.ui2_anci_pg_id) )
				{
					return (DVBR_OK);
				}
				break;
			
			case 0x10: /* PCS */
				break;

			default:
				/* Unexpected segment data */
				break;
			}
			
			ui4_idx += (ui2_segment_len + 6);
		}
		else if (0xFF == pui1_data[ui4_idx])
		{
			/* End of PES data*/
			ui4_idx++;
			break;
		}
		else
		{
			/* Unknown PES data field marker */
			ALOGE("ERROR: Unknown PES data field marker\n");
			return (DVBR_WRONG_SEGMENT_DATA);
		}
	}

	return (DVBR_OK);
}

INT32 DVBParser::paintAllRegion()
{
	return mRegionMgr.paintAllRegion(mObjectMgr);
}

INT32 DVBParser::paintPage()
{
	DVB_REGION_INFO t_region_info;
    void*   ptrBitmapData = mBitmapData[getTmpFileIdx()];
	UINT16	  ui2_rgn_idx;
#if	DVB_PAINT_BITMAPDATA_DIRECTLY
	UINT32    ui4RowBytes = mBitmapWidth<< 2;
#endif
	DVB_LOG("DvbRegionMgr::paintPage() start",0);
	mCurrPage->linkSub(&mRegionMgr,&mClutMgr);

#if DVB_WITH_NEON
    DVB_LOG("DvbRegionMgr::paintPage() transparent neon start",0);  
    uint32x4_t  zero = vdupq_n_u32(0);
    for (int y = 0; y < mBitmapHeight;y++)
	{	
		UINT32* 	pui4_pos = (UINT32*)((char*)ptrBitmapData + y * ui4RowBytes);
        uint32_t*   pui4_pos_n = (uint32_t*)pui4_pos;
		for (int x = 0; x < mBitmapWidth/4;x++)
		{
		    vst1q_u32(pui4_pos_n,zero);
            pui4_pos_n += 4;
		}
	}
#else
	DVB_LOG("DvbRegionMgr::paintPage() transparent start",0);    
	/* transparent */
	for (int y = 0; y < mBitmapHeight;y++)
	{	
#if	DVB_PAINT_BITMAPDATA_DIRECTLY
		UINT32* 	pui4_pos = (UINT32*)((char*)ptrBitmapData + y * ui4RowBytes);
#endif
		for (int x = 0; x < mBitmapWidth;x++,pui4_pos++)
		{
#if	DVB_PAINT_BITMAPDATA_DIRECTLY
			*pui4_pos = 0x00000000;
            //*(UINT32*)((char*)mBitmapData + y * ui4RowBytes + (x << 2)) = 0x00000000;
#else		
            *(bitmap->getAddr32(x,y)) = 0x00000000;
#endif
		}
	}

    
#endif
    DVB_LOG("DvbRegionMgr::paintPage() transparent end",0); 

	DVB_LOG("DvbRegionMgr::paintPage() paint start",0);
	/*walk through each region*/
	ui2_rgn_idx = 0;
	while(DVBR_OK == mCurrPage->getRegionInfo(ui2_rgn_idx,&t_region_info))
	{
		/*paint this one*/
		ABitReader br((const uint8_t *)t_region_info.pv_bitmap, t_region_info.ui4_bitmap_length);
		size_t	nbit_each_time = 0;

		/*loop next*/
		ui2_rgn_idx++;
		
		switch(t_region_info.e_clr_mode)
		{
			case DVB_COLOR_MODE_CLUT_2_BIT:
				nbit_each_time = 2;
				break;
					
		    case DVB_COLOR_MODE_CLUT_4_BIT:
				nbit_each_time = 4;
				break;
		    case DVB_COLOR_MODE_CLUT_8_BIT:
				nbit_each_time = 8;
				break;
			default:
				ALOGE("invalid color mode = %d\n",t_region_info.e_clr_mode);
				continue;
		}
		
		for (int y = 0; y < t_region_info.ui4_height;y++)
		{
#if	DVB_PAINT_BITMAPDATA_DIRECTLY
			UINT32*		pui4_pos = (UINT32*)((char*)ptrBitmapData + (y + t_region_info.i4_top) * ui4RowBytes + ((t_region_info.i4_left) << 2));
#endif
			for (int x = 0; x < t_region_info.ui4_width;pui4_pos++,x++)
			{
				int index  = br.getBits(nbit_each_time);
				GL_COLOR_T gl_color = t_region_info.at_clut[index];
#if	DVB_PAINT_BITMAPDATA_DIRECTLY
                *pui4_pos = 
                        (gl_color.a << 24) +
						(gl_color.u1.r << 16) +
						(gl_color.u2.g << 8) +
						(gl_color.u3.b);
#else
				*(bitmap->getAddr32(x+t_region_info.i4_left, y + t_region_info.i4_top)) = 
						(gl_color.a << 24) +
						(gl_color.u1.r << 16) +
						(gl_color.u2.g << 8) +
						(gl_color.u3.b);
#endif
			}
		}

		
	}
    DVB_LOG("DvbRegionMgr::paintPage() paint end",0);
#if 0
#if	DVB_PAINT_BITMAPDATA_DIRECTLY
    /*write to bitmap*/
    DVB_LOG("DvbRegionMgr::paintPage() copy data to skbitmap ,bitmap size = %d\n",bitmap->getSize());
	memcpy(bitmap->getPixels(),mBitmapData,bitmap->getSize());
#else
	/*write into tmp file*/
	DVB_LOG("DvbRegionMgr::paintPage() copy skbitmap to data,bitmap size = %d\n",bitmap->getSize());
	memcpy(mBitmapData,bitmap->getPixels(),bitmap->getSize());
#endif
#endif

	
#if 0
	static int file_seq = 0;
	char filename[64];
	sprintf(filename,"/sdcard/dvb_sub_%03d.png",file_seq++);
    ALOGI("DvbRegionMgr::paintPage() save to %s",filename);
	SkImageEncoder::EncodeFile(filename,*bitmap,SkImageEncoder::kPNG_Type,100);
#endif
#if 0
        char filename[255];
        static unsigned char index = 0;
        sprintf(filename,"/sdcard/%d.argb",index);
        ALOGD("DvbRegionMgr::paintPage() save to file :%s",filename);
        FILE* fp = fopen(filename,"ab");
        if(fp)
        {
            fwrite((void*)mBitmapData,1,mBitmapWidth*mBitmapHeight*4,fp);
            fclose(fp);
        }        
        index++;
#endif

#if	DVB_SUBTITLE_SAVE_AS_YUV_FILE
	Record2YuvFile(*bitmap,mBitmapWidth,mBitmapHeight);
#endif
   
    
	DVB_LOG("DvbRegionMgr::paintPage() end",0);
	return (DVBR_OK);
}


VOID DVBParser::ReleasePage()
{
    if (mCurrPage)
    {
        delete mCurrPage;
        mCurrPage = NULL;
    }
}

}
#endif

