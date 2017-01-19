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
 /*-----------------------------------------------------------------------------
					 include files
  ----------------------------------------------------------------------------*/
#define LOG_TAG "DVBPAGE"

#include <stdlib.h>
#include <utils/Log.h>

#include "DvbClut.h"
#include "DvbPage.h"
#include "DvbRegion.h"
#include "DvbObject.h"


#include "DvbClutMgr.h"
#include "DVbPageMgr.h"
#include "DvbRegionMgr.h"
#include "DvbObjectMgr.h"
#include "dvbparser.h"


/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
				 data declarations, extern, static, const
----------------------------------------------------------------------------*/
const UINT16	DvbPage::PageDefaultWidth = 720;
const UINT16	DvbPage::PageDefaultHeight = 576;


/*-----------------------------------------------------------------------------
                    functions declarations
 ----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
 * Name: 
 *
 * Description: 
 *
 * Inputs:	this 	Pointer points to the 
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
DvbPage::DvbPage()
{
	
	pat_rgn_list = NULL;
	//this->ui8_PTS = ui8_PTS;
	i1_lock_pib_counter = 0;
	i1_unlock_pib_counter = 0;
	ui1_rgn_cnt  = 0;
}

DvbPage::~DvbPage()
{
	INT32							i;
	DVB_PAGE_INST_RGN_LST_T*		pt_page_inst_rgn_lst;

	if (NULL != this->pat_rgn_list)
    {
        for (i = 0; i < this->ui1_rgn_cnt; i++)
        {
            pt_page_inst_rgn_lst = &(this->pat_rgn_list[i]);
            
            if (FALSE == pt_page_inst_rgn_lst->is_exist)
            {
                continue;
            }

            if (NULL != pt_page_inst_rgn_lst->pt_nbit_CLUT)
            {
                DvbClut::release(pt_page_inst_rgn_lst->pt_nbit_CLUT,
                                 pt_page_inst_rgn_lst->ui1_depth);

                pt_page_inst_rgn_lst->pt_nbit_CLUT = NULL;
            }

            if (NULL != pt_page_inst_rgn_lst->pui1_rgn_buf)
            {
                free(pt_page_inst_rgn_lst->pui1_rgn_buf);

                pt_page_inst_rgn_lst->pui1_rgn_buf = NULL;
            }
        }

        //sm_sbtl_service_mem_free(this->pt_service,
        //                         this->pat_rgn_list);
    }
}



/*-----------------------------------------------------------------------------
 * Name: 
 *
 * Description: 
 *
 * Inputs:  this     Pointer points to the 
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
INT32 DvbPage::create(
			//PTS_T 				  ui8_PTS, 
			UINT8*					pui1_data, 
			UINT16					ui2_data_len, 
			DvbPage**	ppt_this)

{
    INT32       i4_ret;
    DvbPage*    pt_this;

    *ppt_this = NULL;
    pt_this = new DvbPage();
    
    if (NULL == pt_this)
    {
        return (DVBR_INSUFFICIENT_MEMORY);
    }

    i4_ret = pt_this->parseSegment_impl( 
                                      pui1_data, 
                                      ui2_data_len,
                                      TRUE);

    if (DVBR_OK > i4_ret)
    {
        /* segment data error */
        delete (pt_this);
        return (i4_ret);
    }

    *ppt_this = pt_this;
    return (DVBR_OK);

}

/*-----------------------------------------------------------------------------
 * Name: 
 *
 * Description: 
 *
 * Inputs:  pt_this     Pointer points to the 
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
INT32 DvbPage::dup(
			//PTS_T					ui8_PTS, 
			DvbPage*	pt_orig_page_inst, 
			DvbPage**	ppt_this)
{
    UINT8                           i;
    INT32                           i4_ret;
    DvbPage*            pt_this;
    DVB_PAGE_INST_RGN_LST_T*    pt_page_inst_rgn_lst;
    DVB_PAGE_INST_RGN_LST_T*    pt_old_page_inst_rgn_lst;

    *ppt_this = NULL;
    pt_this = new DvbPage();
    
    if (NULL == pt_this)
    {
        return (DVBR_INSUFFICIENT_MEMORY);
    }

    /* allocate region list */
    pt_this->pat_rgn_list = (DVB_PAGE_INST_RGN_LST_T*)malloc(pt_orig_page_inst->ui1_rgn_cnt*sizeof (DVB_PAGE_INST_RGN_LST_T));

    if ( (NULL == pt_this->pat_rgn_list) && (0 != pt_orig_page_inst->ui1_rgn_cnt) )
    {
        free(pt_this);
        return (DVBR_INSUFFICIENT_MEMORY);
    }

   
    //pt_this->ui8_PTS = ui8_PTS;

    pt_this->ui1_time_out = pt_orig_page_inst->ui1_time_out;
    pt_this->ui1_version_number = pt_orig_page_inst->ui1_version_number;
    pt_this->ui1_rgn_cnt = pt_orig_page_inst->ui1_rgn_cnt;

    pt_this->ui1_state = STATE_NORMAL;
    pt_this->ui1_status = STATUS_INIT;

    for (i = 0; i < pt_this->ui1_rgn_cnt; i++)
    {
        for (i = 0; i < pt_this->ui1_rgn_cnt; i++)
        {
            pt_page_inst_rgn_lst = &(pt_this->pat_rgn_list[i]);
            pt_old_page_inst_rgn_lst = &(pt_orig_page_inst->pat_rgn_list[i]);
            
            pt_page_inst_rgn_lst->is_exist = FALSE;
            
            pt_page_inst_rgn_lst->ui1_rid = pt_old_page_inst_rgn_lst->ui1_rid;
            pt_page_inst_rgn_lst->ui1_depth = pt_old_page_inst_rgn_lst->ui1_depth;
            
            pt_page_inst_rgn_lst->ui2_horizontal_addr =
                pt_old_page_inst_rgn_lst->ui2_horizontal_addr;

            pt_page_inst_rgn_lst->ui2_vertical_addr =
                pt_old_page_inst_rgn_lst->ui2_vertical_addr;

            pt_page_inst_rgn_lst->pt_nbit_CLUT = NULL;
            pt_page_inst_rgn_lst->pui1_rgn_buf = NULL;

            pt_page_inst_rgn_lst->ui2_mdf_left = 0;
            pt_page_inst_rgn_lst->ui2_mdf_top = 0;
            pt_page_inst_rgn_lst->ui2_mdf_right = 0;
            pt_page_inst_rgn_lst->ui2_mdf_bottom = 0;
        }
    }

    *ppt_this = pt_this;
    return (DVBR_OK);
}


/*-----------------------------------------------------------------------------
 * Name: 
 *
 * Description: 
 *
 * Inputs:  pt_this     Pointer points to the 
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
INT32 DvbPage::linkSub(
			DvbRegionMgr*	  pt_rgn_mngr, 
			DvbClutMgr* 	  pt_CLUT_mngr)
{
    INT32                           i;
    DVB_PAGE_INST_RGN_LST_T*    pt_page_inst_rgn_lst;
    
    if (NULL != this->pat_rgn_list)
    {
        for (i = 0; i < this->ui1_rgn_cnt; i++)
        {
            pt_page_inst_rgn_lst = &(this->pat_rgn_list[i]);

            pt_rgn_mngr->linkSub(pt_page_inst_rgn_lst->ui1_rid,
                                  pt_CLUT_mngr, 
                                  pt_page_inst_rgn_lst);
        }
    }

    this->ui1_status = STATUS_LINK;

    return (DVBR_OK);
}


/*-----------------------------------------------------------------------------
 * Name: sm_sbtl_page_inst_find_rgn_lst
 *
 * Description: Use ui1_rid to find the page_inst_rgn_lst, then get width and height
 *
 * Inputs: pt_this - page instatnce, ui1_rid - region id
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
DVB_PAGE_INST_RGN_LST_T* DvbPage::find_rgn_lst(
			UINT8					ui1_rid)

{
    INT32   i;
    DVB_PAGE_INST_RGN_LST_T*    pt_page_inst_rgn_lst;
    
    for(i = 0; i < this->ui1_rgn_cnt; i++)
    {
        pt_page_inst_rgn_lst = (DVB_PAGE_INST_RGN_LST_T*)&(this->pat_rgn_list[i]);
        
        if( (NULL!=pt_page_inst_rgn_lst) &&
            (ui1_rid==pt_page_inst_rgn_lst->ui1_rid) )
        {
            return pt_page_inst_rgn_lst;
        }
    }
    
    return NULL;
}

/*-----------------------------------------------------------------------------
 * Name: 
 *
 * Description: 
 *
 * Inputs:  pt_this     Pointer points to the 
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
INT32 DvbPage::parseSegment_impl(
		    UINT8*                  pui1_data, 
		    UINT32                  ui4_data_len,
		    bool                    is_new_page_inst)
{
	UINT8							ui1_version;
	INT32							i;
	DVB_PAGE_INST_RGN_LST_T*	pt_page_inst_rgn_lst;
#ifdef DVB_ENABLE_HD_SUBTITLE
	INT32							i4_ret;
#endif

	if (DVB_SBTL_PCS_MIN_LEN > ui4_data_len)
	{
		return (DVBR_WRONG_SEGMENT_DATA);
	}

	DVB_SBTL_PCS_GET_VERSION(pui1_data, ui1_version);
	DVB_LOG("page version = %d",ui1_version);
	
	if (FALSE == is_new_page_inst)
	{
		if (this->ui1_version_number == ui1_version)
		{
			return (DVBR_OK);
		}
	}
	else /* New PCS */
	{
		DVB_SBTL_PCS_GET_PAGE_TIMEOUT(pui1_data, this->ui1_time_out);
		DVB_LOG("page timeout = %d",this->ui1_time_out);

		
		DVB_SBTL_PCS_GET_STATE(pui1_data, this->ui1_state);

		 /* 00 - Normal case, page update
		  * 01 - Acquisition point, page refresh
		  * 10 - Mode change, new page
		  * 11 - REserved
		  */
	    DVB_LOG("page state = %d",this->ui1_state);
		if (0x11 == this->ui1_state)
		{
			return (DVBR_WRONG_SEGMENT_DATA);
		}

		this->ui1_status = STATUS_INIT;

		pui1_data += DVB_SBTL_PCS_MIN_LEN;
		ui4_data_len -= DVB_SBTL_PCS_MIN_LEN;
	}

	this->ui1_version_number = ui1_version;

	/* Check if the region list is aligned */
	if (0 != (ui4_data_len % DVB_SBTL_PCS_RGN_LST_LEN))
	{
		return (DVBR_WRONG_SEGMENT_DATA);
	}

	this->ui1_rgn_cnt = (UINT8)(ui4_data_len / DVB_SBTL_PCS_RGN_LST_LEN);
	DVB_LOG("RGN count in page = %d",this->ui1_rgn_cnt);
	
	/* free and allocate obj list */
	if (NULL != this->pat_rgn_list)
	{
		free(this->pat_rgn_list);
		this->pat_rgn_list = NULL;
	}

	this->pat_rgn_list = (DVB_PAGE_INST_RGN_LST_T*)malloc((this->ui1_rgn_cnt)*sizeof (DVB_PAGE_INST_RGN_LST_T));

	if ( (NULL == this->pat_rgn_list) && (0 != this->ui1_rgn_cnt) )
	{
		return (DVBR_INSUFFICIENT_MEMORY);
	}

	ALOGE( "[state:%d][rcs count:%d] --{0-Normal,1-Acquisition,2-Mode change}\n", this->ui1_state, this->ui1_rgn_cnt );

	for (i = 0; i < this->ui1_rgn_cnt; i++)
	{
		pt_page_inst_rgn_lst = &(this->pat_rgn_list[i]);

		DVB_SBTL_PCS_GET_RGN_ID(pui1_data, pt_page_inst_rgn_lst->ui1_rid);

		DVB_SBTL_PCS_GET_RGN_HORIZONTAL_ADDRESS(pui1_data,
											   pt_page_inst_rgn_lst->ui2_horizontal_addr);

		DVB_SBTL_PCS_GET_RGN_VERTICAL_ADDRESS(pui1_data,
											 pt_page_inst_rgn_lst->ui2_vertical_addr);

		ALOGE( "[rcs idx:%d][rcs id:%d][h_addr:%d][v_addr:%d]\n", i, pt_page_inst_rgn_lst->ui1_rid, pt_page_inst_rgn_lst->ui2_horizontal_addr, pt_page_inst_rgn_lst->ui2_vertical_addr );

  #ifdef DVB_ENABLE_HD_SUBTITLE
		i4_ret = android::DVBParser::getInstance()->judgeBoundary(pt_page_inst_rgn_lst->ui2_horizontal_addr,
										 pt_page_inst_rgn_lst->ui2_vertical_addr);
		if ( DVBR_OK!=i4_ret )
		{
			i4_ret = android::DVBParser::getInstance()->adjustAddress(&pt_page_inst_rgn_lst->ui2_horizontal_addr,
												 &pt_page_inst_rgn_lst->ui2_vertical_addr);
			
			if ( DVBR_OK!=i4_ret )
			{
				pt_page_inst_rgn_lst->ui2_horizontal_addr = 0;
				pt_page_inst_rgn_lst->ui2_vertical_addr = 0;
				return (DVBR_WRONG_SEGMENT_DATA);
			}
		}
  #endif
	  
		pt_page_inst_rgn_lst->is_exist = FALSE;

		pt_page_inst_rgn_lst->pt_nbit_CLUT = NULL;
		pt_page_inst_rgn_lst->pui1_rgn_buf = NULL;

		pt_page_inst_rgn_lst->ui2_mdf_left = 0;
		pt_page_inst_rgn_lst->ui2_mdf_top = 0;
		pt_page_inst_rgn_lst->ui2_mdf_right = 0;
		pt_page_inst_rgn_lst->ui2_mdf_bottom = 0;

		pt_page_inst_rgn_lst->ui1_depth = 0;

		pui1_data += DVB_SBTL_PCS_RGN_LST_LEN;
		ui4_data_len -= DVB_SBTL_PCS_RGN_LST_LEN;
	}

	return (DVBR_OK);
}

INT32 DvbPage::getRegionInfo(UINT16                  ui2_rgn_idx,
    			DVB_REGION_INFO*      pt_rgn)
{
	DVB_PAGE_INST_RGN_LST_T*	pt_page_inst_rgn_lst;
	
	

	if (ui2_rgn_idx < this->ui1_rgn_cnt)
	{
		pt_page_inst_rgn_lst = &(this->pat_rgn_list[ui2_rgn_idx]);

			/* fill data */
		if (FALSE == pt_page_inst_rgn_lst->is_exist)
		{
			return (DVBR_INV_ARG);
		}

		pt_rgn->ui4_rid = pt_page_inst_rgn_lst->ui1_rid;

		pt_rgn->i4_left = (INT32)pt_page_inst_rgn_lst->ui2_horizontal_addr;
		pt_rgn->i4_top = (INT32)pt_page_inst_rgn_lst->ui2_vertical_addr;
		pt_rgn->ui4_width = (UINT32)pt_page_inst_rgn_lst->ui2_width;
		pt_rgn->ui4_height = (UINT32)pt_page_inst_rgn_lst->ui2_height;

		pt_rgn->pv_bitmap = pt_page_inst_rgn_lst->pui1_rgn_buf;
		pt_rgn->ui4_bitmap_length = pt_page_inst_rgn_lst->ui4_rgn_lst_buf_len;
		pt_rgn->at_clut = pt_page_inst_rgn_lst->pt_nbit_CLUT->pat_color_entry;

#if 0
		if ( (pt_page_inst_rgn_lst->ui2_mdf_bottom > pt_page_inst_rgn_lst->ui2_mdf_top) ||
			 (pt_page_inst_rgn_lst->ui2_mdf_right > pt_page_inst_rgn_lst->ui2_mdf_left) )
		{
			pt_rgn->b_dirty = TRUE;
			pt_rgn->t_rc_dirty.i4_bottom = (INT32)pt_page_inst_rgn_lst->ui2_mdf_bottom;
			pt_rgn->t_rc_dirty.i4_top = (INT32)pt_page_inst_rgn_lst->ui2_mdf_top;
			pt_rgn->t_rc_dirty.i4_left = (INT32)pt_page_inst_rgn_lst->ui2_mdf_left;
			pt_rgn->t_rc_dirty.i4_right = (INT32)pt_page_inst_rgn_lst->ui2_mdf_right;
		}
		else
		{
			pt_rgn->b_dirty = FALSE;
		}
#endif

		switch (pt_page_inst_rgn_lst->ui1_depth)
		{
			case DVB_SBTL_RGN_DEPTH_2BIT:
				pt_rgn->e_clr_mode = DVB_COLOR_MODE_CLUT_2_BIT;
				break;

			case DVB_SBTL_RGN_DEPTH_4BIT:
				pt_rgn->e_clr_mode = DVB_COLOR_MODE_CLUT_4_BIT;
				break;

			case DVB_SBTL_RGN_DEPTH_8BIT:
				pt_rgn->e_clr_mode = DVB_COLOR_MODE_CLUT_8_BIT;
				break;

			default:
				return (DVBR_INV_ARG);
		}

		ALOGI( 
			"rgn_info [idx:%d][id:%d][left:%d][top:%d][width:%d][height:%d][mode:%d][bitmap_len=%d]\n", 
			ui2_rgn_idx, 
			pt_page_inst_rgn_lst->ui1_rid,
			pt_rgn->i4_left,
			pt_rgn->i4_top,
			pt_rgn->ui4_width,
			pt_rgn->ui4_height,
			pt_rgn->e_clr_mode,
			pt_rgn->ui4_bitmap_length);
	

		return (DVBR_OK);
	}

	return (DVBR_NOT_FOUND);
}

#endif

