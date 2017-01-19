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
#define LOG_TAG "DVBRegion"

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

#define DVB_SBTL_2BIT_CLUT_SIZE      ((UINT32) 4)
#define DVB_SBTL_4BIT_CLUT_SIZE      ((UINT32) 16)

/*-----------------------------------------------------------------------------
                    data declarations, extern, static, const
 ----------------------------------------------------------------------------*/
const UINT8 aui1_2bit_byte_fill_tbl[DVB_SBTL_2BIT_CLUT_SIZE] = 
{
    0x00, 0x55, 0xAA, 0xFF
};

const UINT8 aui1_4bit_byte_fill_tbl[DVB_SBTL_4BIT_CLUT_SIZE] = 
{
    0x00, 0x11, 0x22, 0x33,
    0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xAA, 0xBB,
    0xCC, 0xDD, 0xEE, 0xFF
};


/*-----------------------------------------------------------------------------
                    functions declarations
 ----------------------------------------------------------------------------*/


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
DvbRegion::DvbRegion()
{
	pat_obj_list = NULL;
    pui1_rgn_buf = NULL;
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
DvbRegion::~DvbRegion()
{
    //if (NULL == pt_this)
    //{
    //    return;
    //}

    if (NULL != this->pat_obj_list)
    {
        free(this->pat_obj_list);
    }

    if (NULL != this->pui1_rgn_buf)
    {
        free(this->pui1_rgn_buf);
    }

    //sm_sbtl_service_mem_free(pt_service,
    //                         pt_this);

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
VOID DvbRegion::clear()
{
    //if (NULL == pt_this)
    //{
    //    return;
    //}
    
    memset(this->pui1_rgn_buf, 0, this->ui4_rgn_buf_len);
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
INT32 DvbRegion::create(
    DvbPage* 			pt_page_inst,
    UINT8*              pui1_data, 
    UINT32              ui4_data_len, 
    DvbRegion**     	ppt_this)
{
    INT32           i4_ret;
    DvbRegion* pt_this;

    *ppt_this = NULL;
    pt_this = new DvbRegion();
    
    if (NULL == pt_this)
    {
        return (DVBR_INSUFFICIENT_MEMORY);
    }

    pt_this->pat_obj_list = NULL;
    pt_this->pui1_rgn_buf = NULL;

    i4_ret = pt_this->parseSegment_impl(
                                        pt_page_inst,
                                        pui1_data, 
                                        ui4_data_len,
                                        TRUE);

    if (DVBR_OK > i4_ret)
    {
        /* segment data error */
        //sm_sbtl_rgn_delete(pt_this, pt_service);
        delete pt_this;
        return (i4_ret);
    }
    else
    {
        *ppt_this = pt_this;
        return (DVBR_OK);
    }
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
INT32 DvbRegion::parseSegment(
		DvbPage*			pt_page_inst,
		UINT8*				pui1_data, 
		UINT32				ui4_data_len)

{
    return (parseSegment_impl(pt_page_inst,
                               pui1_data, 
                               ui4_data_len,
                               FALSE));
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
VOID DvbRegion::updateClut(UINT8 ui1_CLUT_id)
{
    if (ui1_CLUT_id == this->ui1_CLUT_id)
    {
        this->ui2_mdf_top = 0;
        this->ui2_mdf_left = 0;
        this->ui2_mdf_bottom = this->ui2_height;
        this->ui2_mdf_right = this->ui2_width;
    }
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
VOID DvbRegion::updateObj(DvbObject*      pt_obj)
{
#if 0
    INT32                   i4_ret;
    DVB_SBTLH_RENDER_INFO_T  t_render_info;
    DVB_SBTL_RGN_OBJ_LST_T*  pt_rgn_obj_lst;
    UINT16                  ui2_obj_top;
    UINT16                  ui2_obj_left;
    UINT16                  ui2_obj_bottom;
    UINT16                  ui2_obj_right;
    INT32                   i;

    if (NULL != pt_this->pat_obj_list)
    {
        return;
    }

    for (i = 0; i < (INT32)(pt_this->ui2_obj_list_cnt); i++)
    {
        pt_rgn_obj_lst = &(pt_this->pat_obj_list[i]);

        if (NULL == pt_rgn_obj_lst)
        {
            continue;
        }

        if (pt_rgn_obj_lst->ui2_id != pt_obj->ui2_id)
        {
            continue;
        }
        
        t_render_info.ui2_obj_x_pos = pt_rgn_obj_lst->ui2_horizontal_position;
        t_render_info.ui2_obj_y_pos = pt_rgn_obj_lst->ui2_vertical_position;
        t_render_info.ui1_obj_fg_color = pt_rgn_obj_lst->ui1_fg_color;
        t_render_info.ui1_obj_bg_color = pt_rgn_obj_lst->ui1_bg_color;
        t_render_info.ui1_rgn_depth = pt_this->ui1_depth;
        t_render_info.ui2_rgn_width = pt_this->ui2_width;
        t_render_info.ui2_rgn_height = pt_this->ui2_height;
        t_render_info.pui1_rgn = pt_this->pui1_rgn_buf;

        i4_ret = sm_sbtl_obj_render(pt_obj, &t_render_info);

        pt_rgn_obj_lst->is_render = TRUE;
        if (DVBR_OK == i4_ret)
        {
            /* Take care zero base issue (offset one) */
            ui2_obj_top = pt_rgn_obj_lst->ui2_vertical_position;
            ui2_obj_left = pt_rgn_obj_lst->ui2_horizontal_position;
            ui2_obj_bottom = ui2_obj_top + t_render_info.ui2_obj_height;
            ui2_obj_right = ui2_obj_left + t_render_info.ui2_obj_width;

            if (pt_this->ui2_mdf_left > ui2_obj_left)
            {
                pt_this->ui2_mdf_left = ui2_obj_left;
            }

            if (pt_this->ui2_mdf_right < ui2_obj_right)
            {
                pt_this->ui2_mdf_right = ui2_obj_right;
            }

            if (pt_this->ui2_mdf_top > ui2_obj_top)
            {
                pt_this->ui2_mdf_top = ui2_obj_top;
            }

            if (pt_this->ui2_mdf_bottom < ui2_obj_bottom)
            {
                pt_this->ui2_mdf_bottom = ui2_obj_bottom;
            }
        }
    }
#endif
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
INT32 DvbRegion::linkSub(
		DvbClutMgr* 		   pt_CLUT_mngr,
		DVB_PAGE_INST_RGN_LST_T*			pt_page_inst_rgn_lst)
{
    UINT8*              pui1_rgn_buf;

    pui1_rgn_buf = (UINT8 *) malloc(ui4_rgn_buf_len);

    if (NULL == pui1_rgn_buf)
    {
        ALOGE("sm_sbtl_rgn_link_sub pui1_rgn_buf alloc fail\n");
        pt_page_inst_rgn_lst->pui1_rgn_buf = NULL;
        return (DVBR_INSUFFICIENT_MEMORY);
    }
    
    memset(pui1_rgn_buf, 0, ui4_rgn_buf_len);
    pt_page_inst_rgn_lst->pui1_rgn_buf = pui1_rgn_buf;

    memcpy(pt_page_inst_rgn_lst->pui1_rgn_buf,
             this->pui1_rgn_buf,
             this->ui4_rgn_buf_len);
	pt_page_inst_rgn_lst->ui4_rgn_lst_buf_len = this->ui4_rgn_buf_len;

    pt_page_inst_rgn_lst->ui1_depth = this->ui1_depth;
    pt_page_inst_rgn_lst->ui2_mdf_left = this->ui2_mdf_left;
    pt_page_inst_rgn_lst->ui2_mdf_top = this->ui2_mdf_top;
    pt_page_inst_rgn_lst->ui2_mdf_right = this->ui2_mdf_right;
    pt_page_inst_rgn_lst->ui2_mdf_bottom = this->ui2_mdf_bottom;

    this->ui2_mdf_left = this->ui2_width;
    this->ui2_mdf_top = this->ui2_height;
    this->ui2_mdf_right = 0;
    this->ui2_mdf_bottom = 0;

    pt_page_inst_rgn_lst->ui2_width = this->ui2_width;
    pt_page_inst_rgn_lst->ui2_height = this->ui2_height;

    pt_page_inst_rgn_lst->pt_nbit_CLUT = 
        pt_CLUT_mngr->Link_sub(this->ui1_CLUT_id, this->ui1_depth);

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
INT32 DvbRegion::parseSegment_impl(
			DvbPage*			pt_page_inst,
			UINT8*				pui1_data, 
			UINT32				ui4_data_len,
			bool				is_new_rgn)
{
	UINT8					ui1_version;
	UINT8					ui1_rgn_depth;
	UINT16					ui2_rgn_width;
	UINT16					ui2_rgn_height;
	UINT8					ui1_fill_byte;
	UINT8*					pui1_rgn_buf;
	DVB_RGN_OBJ_LST_T*		pt_rgn_obj_lst;
	UINT8*					pui1_data_idx;
	UINT32					ui4_remain_len;
	UINT8					ui1_obj_type;
	INT32					i;
	DVB_PAGE_INST_RGN_LST_T*	pt_page_inst_lst;
	UINT16							ui2_rgn_left;
	UINT16							ui2_rgn_top;
#ifdef MW_SBTL_FORCE_4BIT_RGN_DEPTH
	UINT8							ui1_rgn_level_of_compatibility;
#endif
#ifdef DVB_ENABLE_HD_SUBTITLE
	INT32							i4_ret;
#endif    

	if (DVB_SBTL_RCS_MIN_LEN > ui4_data_len)
	{
		return (DVBR_WRONG_SEGMENT_DATA);
	}

#if 0
	{
		UINT32			i;

		for (i = 0; i < ui4_data_len; i++)
		{
			if (0 == i%10)
			{
				x_dbg_stmt( "\n  " );
			}
			
			if (pui1_data[i] < 0x10)
			{
				x_dbg_stmt( " 0%x", pui1_data[i] );
			}
			else
			{
				x_dbg_stmt( " %x", pui1_data[i] );
			}
		}

		x_dbg_stmt( "\n" );
	}
#endif

	DVB_SBTL_RCS_GET_VERSION(pui1_data, ui1_version);

	if (FALSE == is_new_rgn)
	{
		if (this->ui1_version_number == ui1_version)
		{
			return (DVBR_OK);
		}
		
		/* Re-parse the region to verify region boundary */
		DVB_SBTL_RCS_GET_RGN_WIDTH(pui1_data, ui2_rgn_width);
		DVB_SBTL_RCS_GET_RGN_HEIGHT(pui1_data, ui2_rgn_height);

		if ((this->ui2_width != ui2_rgn_width) || (this->ui2_height != ui2_rgn_height))
		{
			return (DVBR_MEM_USAGE_CHANGED);
		}

		pt_page_inst_lst = pt_page_inst->find_rgn_lst(this->ui1_id);
		
		if (NULL==pt_page_inst_lst)
		{
			ui2_rgn_left = 0;
			ui2_rgn_top = 0;
		}
		else
		{
			ui2_rgn_left = pt_page_inst_lst->ui2_horizontal_addr;
			ui2_rgn_top = pt_page_inst_lst->ui2_vertical_addr;
		}

  #ifdef DVB_ENABLE_HD_SUBTITLE
		i4_ret = android::DVBParser::getInstance()->judgeBoundary(ui2_rgn_left+ui2_rgn_width-1,
												 ui2_rgn_top+ui2_rgn_height-1);    
		if (DVBR_OK!=i4_ret)
		{
			return (DVBR_WRONG_SEGMENT_DATA);
		}
  #else
		if ((ui2_rgn_width+ui2_rgn_left) > DvbPage::PageDefaultWidth)
		{
			return (DVBR_WRONG_SEGMENT_DATA);
		}
	
		if ((ui2_rgn_height+ui2_rgn_top) > DvbPage::PageDefaultHeight)
		{
			return (DVBR_WRONG_SEGMENT_DATA);
		}
  #endif
	}
	else /* New region */
	{
		DVB_SBTL_RCS_GET_RGN_ID(pui1_data, this->ui1_id);
		DVB_LOG("DvbRegionMgr::parseSegment_impl() new region id=",this->ui1_id);
		DVB_SBTL_RCS_GET_RGN_WIDTH(pui1_data, ui2_rgn_width);
		DVB_SBTL_RCS_GET_RGN_HEIGHT(pui1_data, ui2_rgn_height);
		DVB_LOG("DvbRegionMgr::parseSegment_impl() new region width=",ui2_rgn_width);
		DVB_LOG("DvbRegionMgr::parseSegment_impl() new region height=",ui2_rgn_height);
#ifdef MW_SBTL_FORCE_4BIT_RGN_DEPTH
		DVB_SBTL_RCS_GET_RGN_LEVEL_OF_COMPATIBILITY(pui1_data, ui1_rgn_level_of_compatibility);
#endif

		DVB_SBTL_RCS_GET_RGN_DEPTH(pui1_data, ui1_rgn_depth);
		
		DVB_SBTL_RCS_GET_RGN_CLUT_ID(pui1_data, this->ui1_CLUT_id);

		pt_page_inst_lst = pt_page_inst->find_rgn_lst(this->ui1_id);
		
		if (NULL==pt_page_inst_lst)
		{
			ui2_rgn_left = 0;
			ui2_rgn_top = 0;
		}
		else
		{
			ui2_rgn_left = pt_page_inst_lst->ui2_horizontal_addr;
			ui2_rgn_top = pt_page_inst_lst->ui2_vertical_addr;
		}

  #ifdef DVB_ENABLE_HD_SUBTITLE
		i4_ret = android::DVBParser::getInstance()->judgeBoundary(ui2_rgn_left+ui2_rgn_width-1,
												 ui2_rgn_top+ui2_rgn_height-1);
	
		if (DVBR_OK!=i4_ret)
		{
			return (DVBR_WRONG_SEGMENT_DATA);
		}
  #else
		if ((ui2_rgn_width+ui2_rgn_left) > DvbPage::PageDefaultWidth)
		{
			return (DVBR_WRONG_SEGMENT_DATA);
		}
		
		if ((ui2_rgn_height+ui2_rgn_top) > DvbPage::PageDefaultHeight)
		{
			return (DVBR_WRONG_SEGMENT_DATA);
		}
  #endif

		if (0 == ui2_rgn_width)
		{
			return (DVBR_WRONG_SEGMENT_DATA);
		}

		if (0 == ui2_rgn_height)
		{
			return (DVBR_WRONG_SEGMENT_DATA);
		}
		
		this->ui2_height = ui2_rgn_height;
		this->ui2_width = ui2_rgn_width;

		this->ui2_mdf_left = 0;
		this->ui2_mdf_top = 0;
		this->ui2_mdf_right = this->ui2_width;
		this->ui2_mdf_bottom = this->ui2_height;

		this->is_render = FALSE;
		
		/* pixel buffer memory, region_bits = region_width * region_height * region_depth/8 */
		switch (ui1_rgn_depth)
		{
			case 1: /* 2 bit */
				this->ui1_depth = DVB_SBTL_RGN_DEPTH_2BIT;
				if (0 == (ui2_rgn_width % 4))
				{
					this->ui4_rgn_buf_len = (UINT32)((ui2_rgn_width >> 2) * ui2_rgn_height);
				}
				else
				{
					this->ui4_rgn_buf_len = (UINT32)(((ui2_rgn_width >> 2) + 1) * ui2_rgn_height);
				}
				break;

			case 2: /* 4 bit */
				this->ui1_depth = DVB_SBTL_RGN_DEPTH_4BIT;
				
				if (0 == (ui2_rgn_width % 2))
				{
					this->ui4_rgn_buf_len = (UINT32)((ui2_rgn_width >> 1) * ui2_rgn_height);
				}
				else
				{
					this->ui4_rgn_buf_len = (UINT32)(((ui2_rgn_width >> 1) + 1) * ui2_rgn_height);
				}
				break;

			case 3: /* 8 bit */
            #ifdef MW_SBTL_FORCE_4BIT_RGN_DEPTH
				
				if ( (ui1_rgn_level_of_compatibility == 1) || /* 2 bits */
					 (ui1_rgn_level_of_compatibility == 2) )  /* 4 bits */
				{
					this->ui1_depth = DVB_SBTL_RGN_DEPTH_4BIT;
					this->ui4_rgn_buf_len = ((ui2_rgn_width >> 1) + 1) * ui2_rgn_height;
				}
				else
				{
					this->ui1_depth = DVB_SBTL_RGN_DEPTH_8BIT;
					this->ui4_rgn_buf_len = ui2_rgn_width * ui2_rgn_height;
				}
				
            #else
				
				this->ui1_depth = DVB_SBTL_RGN_DEPTH_8BIT;
				this->ui4_rgn_buf_len = ui2_rgn_width * ui2_rgn_height;
				
            #endif
				break;

			default:
				return (DVBR_WRONG_SEGMENT_DATA);
		}
		
		/* allocate memory */
		pui1_rgn_buf = (UINT8 *) malloc(this->ui4_rgn_buf_len);

		if (NULL == pui1_rgn_buf)
		{
			return (DVBR_INSUFFICIENT_MEMORY);
		}

		memset(pui1_rgn_buf, 0, this->ui4_rgn_buf_len);
		this->pui1_rgn_buf = pui1_rgn_buf;
	} /* new region */
	

	ALOGE( "RCS[ID:%d][CLUT_id:%d][W:%d][H:%d][depth:%d]\n", this->ui1_id, this->ui1_CLUT_id, this->ui2_width, this->ui2_height, this->ui1_depth );
	
	/* Set version number */
	this->ui1_version_number = ui1_version;

	/* Fill data */
	ui1_fill_byte = 0;
	switch (this->ui1_depth)
	{
		case DVB_SBTL_RGN_DEPTH_2BIT:
			DVB_SBTL_RCS_GET_RGN_2BIT_PIXEL_CODE(pui1_data, this->ui1_bg_color_code);
			ui1_fill_byte = aui1_2bit_byte_fill_tbl[this->ui1_bg_color_code];
			break;

		case DVB_SBTL_RGN_DEPTH_4BIT:
			DVB_SBTL_RCS_GET_RGN_4BIT_PIXEL_CODE(pui1_data, this->ui1_bg_color_code);
			ui1_fill_byte = aui1_4bit_byte_fill_tbl[this->ui1_bg_color_code];
			break;

		case DVB_SBTL_RGN_DEPTH_8BIT:
			DVB_SBTL_RCS_GET_RGN_8BIT_PIXEL_CODE(pui1_data, this->ui1_bg_color_code);
			ui1_fill_byte = this->ui1_bg_color_code;
			break;

		default: /* impossible case */
			break;
	}

	DVB_SBTL_RCS_GET_FILL_FLAG(pui1_data, this->ui1_fill_flag);
	DVB_LOG("DvbRegionMgr::parseSegment_impl() new region fill_flag=%d",this->ui1_fill_flag);
	
	/* Fill background color */
	if (1 == this->ui1_fill_flag)
	{
		memset(this->pui1_rgn_buf, ui1_fill_byte, this->ui4_rgn_buf_len);
		
		this->ui2_mdf_left = 0;
		this->ui2_mdf_top = 0;
		this->ui2_mdf_right = this->ui2_width;
		this->ui2_mdf_bottom = this->ui2_height;
	}

	pui1_data += DVB_SBTL_RCS_MIN_LEN;
	ui4_data_len -= DVB_SBTL_RCS_MIN_LEN;

	pui1_data_idx = pui1_data;
	ui4_remain_len = ui4_data_len;
	
	this->ui2_obj_list_cnt = 0;
	while (0 < ui4_remain_len)
	{
		DVB_SBTL_RCS_GET_OBJ_TYPE(pui1_data_idx, ui1_obj_type);
		
		switch (ui1_obj_type)
		{
			case 0: /* pixel object */
				if (DVB_SBTL_RCS_PIXEL_OBJ_LST_LEN > ui4_remain_len)
				{
					return (DVBR_WRONG_SEGMENT_DATA);
				}
				
				pui1_data_idx += DVB_SBTL_RCS_PIXEL_OBJ_LST_LEN;
				ui4_remain_len -= DVB_SBTL_RCS_PIXEL_OBJ_LST_LEN;

				break;

			case 1:
			case 2: /* character object */
				if (DVB_SBTL_RCS_CHAR_OBJ_LST_LEN > ui4_remain_len)
				{
					return (DVBR_WRONG_SEGMENT_DATA);
				}
				
				pui1_data_idx += DVB_SBTL_RCS_CHAR_OBJ_LST_LEN;
				ui4_remain_len -= DVB_SBTL_RCS_CHAR_OBJ_LST_LEN;
				break;

			default:
				return (DVBR_WRONG_SEGMENT_DATA);
		}

		this->ui2_obj_list_cnt++;
	}

	/* free obj list */
	if (NULL != this->pat_obj_list)
	{
		this->is_render = FALSE;
		free(this->pat_obj_list);
		this->pat_obj_list = NULL;
	}

	if (0 == this->ui2_obj_list_cnt)
	{
		return (DVBR_OK);
	}

	this->pat_obj_list = (DVB_RGN_OBJ_LST_T *) malloc((this->ui2_obj_list_cnt)*
													   sizeof (DVB_RGN_OBJ_LST_T));

	if ( (NULL == this->pat_obj_list) && (0 != this->ui2_obj_list_cnt) )
	{
		return (DVBR_INSUFFICIENT_MEMORY);
	}

	this->is_render = FALSE;
	for (i = 0; i < (INT32)(this->ui2_obj_list_cnt); i++)
	{
		pt_rgn_obj_lst = &(this->pat_obj_list[i]);
		
		DVB_SBTL_RCS_GET_OBJ_ID(pui1_data, pt_rgn_obj_lst->ui2_id);
		DVB_SBTL_RCS_GET_OBJ_TYPE(pui1_data, pt_rgn_obj_lst->ui1_type);
		DVB_SBTL_RCS_GET_OBJ_PROVIDER_FLAG(pui1_data, pt_rgn_obj_lst->ui1_provider_flag);
		DVB_SBTL_RCS_GET_OBJ_HORIZONTAL_POSITION(pui1_data, pt_rgn_obj_lst->ui2_horizontal_position);
		DVB_SBTL_RCS_GET_OBJ_VERTICAL_POSITION(pui1_data, pt_rgn_obj_lst->ui2_vertical_position);

		ALOGE( 
			"	[idx:%d][obj_id:%d][h_addr:%d][v_addr:%d]\n", 
			i, pt_rgn_obj_lst->ui2_id, 
			pt_rgn_obj_lst->ui2_horizontal_position, 
			pt_rgn_obj_lst->ui2_vertical_position );

		ALOGE( "add object[%d] to region[%d]\n", pt_rgn_obj_lst->ui2_id, this->ui1_id );

		pt_rgn_obj_lst->is_render = FALSE;
		if ( (1 == pt_rgn_obj_lst->ui1_type) && (2 == pt_rgn_obj_lst->ui1_type) ) /* character object */
		{
			DVB_SBTL_RCS_GET_OBJ_FOREGROUND_CODE(pui1_data, pt_rgn_obj_lst->ui1_fg_color);
			DVB_SBTL_RCS_GET_OBJ_BACKGROUND_CODE(pui1_data, pt_rgn_obj_lst->ui1_bg_color);

			pui1_data += DVB_SBTL_RCS_CHAR_OBJ_LST_LEN;
			ui4_data_len -= DVB_SBTL_RCS_CHAR_OBJ_LST_LEN;
		}
		else /* pixel object */
		{
			pui1_data += DVB_SBTL_RCS_PIXEL_OBJ_LST_LEN;
			ui4_data_len -= DVB_SBTL_RCS_PIXEL_OBJ_LST_LEN;
		}
		
		/* Boundary check */
		if ( (this->ui2_width<pt_rgn_obj_lst->ui2_horizontal_position)&&
			  (this->ui2_height<pt_rgn_obj_lst->ui2_vertical_position) )
		{
			pt_rgn_obj_lst->ui2_horizontal_position = 0;
			pt_rgn_obj_lst->ui2_vertical_position = 0;
			pt_rgn_obj_lst->is_render = TRUE;
			ALOGE("ERROR: Boundary check fail\n");
		}
	}

	return (DVBR_OK);
}

INT32 DvbRegion::paintObj(DvbObjectMgr&	objMgr)
{
	INT32					i;
	INT32					i4_ret;
	DVB_RGN_OBJ_LST_T*	pt_rgn_obj_lst;
	DVB_RENDER_INFO_T	t_render_info;
	UINT16					ui2_obj_top;
	UINT16					ui2_obj_left;
	UINT16					ui2_obj_bottom;
	UINT16					ui2_obj_right;

	if (TRUE == this->is_render)
	{
		return DVBR_OK;
	}
	
	ALOGI( "paint [%d]obj in region[%d]\n", this->ui2_obj_list_cnt, this->ui1_id );

	for (i = 0; i < (INT32)(this->ui2_obj_list_cnt); i++)
	{
		pt_rgn_obj_lst = &(this->pat_obj_list[i]);
		
		if (FALSE == pt_rgn_obj_lst->is_render)
		{
			/* Valid the object locate in the region */
			if (this->ui2_width <= pt_rgn_obj_lst->ui2_horizontal_position)
			{
				pt_rgn_obj_lst->is_render = TRUE;
				continue;
			}

			if (this->ui2_height <= pt_rgn_obj_lst->ui2_vertical_position)
			{
				pt_rgn_obj_lst->is_render = TRUE;
				continue;
			}

			t_render_info.ui2_obj_x_pos = pt_rgn_obj_lst->ui2_horizontal_position;
			t_render_info.ui2_obj_y_pos = pt_rgn_obj_lst->ui2_vertical_position;
			t_render_info.ui1_obj_fg_color = pt_rgn_obj_lst->ui1_fg_color;
			t_render_info.ui1_obj_bg_color = pt_rgn_obj_lst->ui1_bg_color;
			t_render_info.ui1_rgn_depth = this->ui1_depth;
			t_render_info.ui2_rgn_width = this->ui2_width;
			t_render_info.ui2_rgn_height = this->ui2_height;
			t_render_info.pui1_rgn = this->pui1_rgn_buf;

			if (0 == pt_rgn_obj_lst->ui1_provider_flag)
			{
				ALOGI( 
					"paint [%d]obj for region[%d], x[%d], y[%d]\n", 
					pt_rgn_obj_lst->ui2_id, 
					this->ui1_id,
					t_render_info.ui2_obj_x_pos,
					t_render_info.ui2_obj_y_pos );
				
				/* Provided in the subtitling stream */
				i4_ret = objMgr.renderObj(
												 pt_rgn_obj_lst->ui2_id,
												 &t_render_info);
			}
			else if (1 == pt_rgn_obj_lst->ui1_provider_flag)
			{
				/* provided by a ROM in the IRD */
				i4_ret = objMgr.renderRomObj(pt_rgn_obj_lst->ui2_id,
													 pt_rgn_obj_lst->ui1_type,
													 &t_render_info);
			}
			else
			{
				/* Unsupported object */
				pt_rgn_obj_lst->is_render = TRUE;
				continue;
			}

			pt_rgn_obj_lst->is_render = TRUE;
		
			if (DVBR_OK == i4_ret)
			{
				/* Take care zero base issue (offset one) */
				ui2_obj_top = pt_rgn_obj_lst->ui2_vertical_position;
				ui2_obj_left = pt_rgn_obj_lst->ui2_horizontal_position;
				ui2_obj_bottom = ui2_obj_top + t_render_info.ui2_obj_height;
				ui2_obj_right = ui2_obj_left + t_render_info.ui2_obj_width;

				if (this->ui2_mdf_left > ui2_obj_left)
				{
					this->ui2_mdf_left = ui2_obj_left;
				}

				if (this->ui2_mdf_right < ui2_obj_right)
				{
					this->ui2_mdf_right = ui2_obj_right;
				}

				if (this->ui2_mdf_top > ui2_obj_top)
				{
					this->ui2_mdf_top = ui2_obj_top;
				}

				if (this->ui2_mdf_bottom < ui2_obj_bottom)
				{
					this->ui2_mdf_bottom = ui2_obj_bottom;
				}
			}
			else
			{
				ALOGI("ERROR: render obj failed ret=%d rid=%d obj=%d \n", i4_ret, this->ui1_id, pt_rgn_obj_lst->ui2_id);
                return i4_ret;
			}
		
		}
	}

	this->is_render = TRUE;
    return DVBR_OK;
}

#endif

