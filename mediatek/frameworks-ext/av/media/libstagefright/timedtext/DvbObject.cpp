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

#define LOG_TAG "DVBObject"
 
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



/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/
struct DVB_OBJ_FILL_INFO_T
{
	UINT8	ui1_fill_mask;
	UINT8	ui1_bit_idx;
	UINT8	fill_pixel_cnt;
};

#define TWO_BIT_OBJ_INFO    4
#define FOUR_BIT_OBJ_INFO   2

/*-----------------------------------------------------------------------------
				 data declarations, extern, static, const
----------------------------------------------------------------------------*/

/* Fill Info Tables */
const DVB_OBJ_FILL_INFO_T at_2bit_obj_fill_info[TWO_BIT_OBJ_INFO][TWO_BIT_OBJ_INFO] =
{
	{ /* position = 0 */
		{0xFF, 0, 4}, /* length = 0(4) */
		{0xC0, 2, 1}, /* length = 1 */
		{0xF0, 4, 2}, /* length = 2 */
		{0xFC, 6, 3}, /* length = 3 */
	},

	{ /* position = 1(2) */
		{0x3F, 0, 3}, /* length = 0(4) */
		{0x30, 4, 1}, /* length = 1 */
		{0x3C, 6, 2}, /* length = 2 */
		{0x3F, 0, 3}, /* length = 3 */
	},

	{ /* position = 2(4) */
		{0x0F, 0, 2}, /* length = 0(4) */
		{0x0C, 6, 1}, /* length = 1 */
		{0x0F, 0, 2}, /* length = 2 */
		{0x0F, 0, 2}, /* length = 3 */
	},

	{ /* position = 3(6) */
		{0x03, 0, 1}, /* length = 0(4) */
		{0x03, 0, 1}, /* length = 1 */
		{0x03, 0, 1}, /* length = 2 */
		{0x03, 0, 1}, /* length = 3 */
	},
};

const DVB_OBJ_FILL_INFO_T at_2bit_obj_fill_info2[TWO_BIT_OBJ_INFO][TWO_BIT_OBJ_INFO] =
{
	{ /* position = 0 */
		{0xFF, 0, 4}, /* length = 0(4) */
		{0xFF, 0, 4}, /* length = 1(5) */
		{0xFF, 0, 4}, /* length = 2(6) */
		{0xFF, 0, 4}, /* length = 3(7) */
	},

	{ /* position = 1(2) */
		{0x3F, 0, 3}, /* length = 0(4) */
		{0x3F, 0, 3}, /* length = 1(5) */
		{0x3F, 0, 3}, /* length = 2(6) */
		{0x3F, 0, 3}, /* length = 3 */
	},

	{ /* position = 2(4) */
		{0x0F, 0, 2}, /* length = 0(4) */
		{0x0F, 0, 2}, /* length = 1(5) */
		{0x0F, 0, 2}, /* length = 2 */
		{0x0F, 0, 2}, /* length = 3 */
	},

	{ /* position = 3(6) */
		{0x03, 0, 1}, /* length = 0(4) */
		{0x03, 0, 1}, /* length = 1 */
		{0x03, 0, 1}, /* length = 2 */
		{0x03, 0, 1}, /* length = 3 */
	},
};

const DVB_OBJ_FILL_INFO_T at_4bit_obj_fill_info[FOUR_BIT_OBJ_INFO][FOUR_BIT_OBJ_INFO] = /* position, length */
{
	{ /* position = 0 */
		{0xFF, 0, 2}, /* length = 0(2) */
		{0xF0, 4, 1}, /* length = 1 */
	},

	{ /* position = 1(4) */
		{0x0F, 0, 1}, /* length = 0(2) */
		{0x0F, 0, 1}, /* length = 1 */
	},
};

/*-----------------------------------------------------------------------------
				 functions declarations
----------------------------------------------------------------------------*/
	
/* Map default tables*/
const UINT8 aui1_2_to_2_default_map_tbl[4] = {0x00, 0x55, 0xAA, 0xFF};
const UINT8 aui1_2_to_4_default_map_tbl[4] = {0x00, 0x77, 0x88, 0xFF};
const UINT8 aui1_2_to_8_default_map_tbl[4] = {0x00, 0x77, 0x88, 0xFF};

const UINT8 aui1_4_to_2_default_map_tbl[16] = 
{
	0x00, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
	0xAA, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
};

const UINT8 aui1_4_to_4_default_map_tbl[16] = 
{
	0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
	0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
};

const UINT8 aui1_4_to_8_default_map_tbl[] = 
{
	0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
	0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
};

const UINT8 aui1_8_to_2_default_map_tbl[256] = 
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
};

const UINT8 aui1_8_to_4_default_map_tbl[256] = 
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 
	0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 
	0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 
	0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 
	0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 
	0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
	0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 
	0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 
	0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 
	0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 
	0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 
	0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 
	0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 
	0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 
	0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 
	0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 
	0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 
	0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 
	0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 
	0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
};

const UINT8 aui1_8_to_8_default_map_tbl[256] = 
{
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 
	0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 
	0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 
	0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 
	0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 
	0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 
	0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 
	0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 
	0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F, 
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 
	0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 
	0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F, 
	0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 
	0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 
	0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 
	0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, 
	0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 
	0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 
	0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 
	0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF, 
	0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 
	0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF, 
	0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 
	0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF, 
};



DvbObject::DvbObject()
{
	
}

DvbObject::~DvbObject()
{
	//if (NULL == pt_this)
    //{
    //    return;
    //}

    if (NULL != this->pui1_data)
    {
        free(this->pui1_data);
    }

    //sm_sbtl_service_mem_free(pt_service,
    //                         pt_this);
}


INT32 DvbObject::create(UINT8*              pui1_data, 
                        UINT32              ui4_data_len, 
                        DvbObject**     ppt_this)
{
	INT32			i4_ret;
	DvbObject*	pt_this;

	*ppt_this = NULL;
	pt_this = new DvbObject();

	if (NULL == pt_this)
	{
		return(DVBR_INSUFFICIENT_MEMORY);
	}

	pt_this->pui1_data = NULL;

	/* Parse the object */
	i4_ret = pt_this->parseSegment_impl(
								pui1_data,
								ui4_data_len,
								TRUE);

	if (DVBR_OK > i4_ret)
	{
		/* segment data error */
		 delete pt_this ;
		return(i4_ret);
	}
	else
	{
		*ppt_this = pt_this;
		return(DVBR_OK);
	}
}


INT32 DvbObject::parseSegment(
			    UINT8*              pui1_data, 
			    UINT32              ui4_data_len)

{
    return (parseSegment_impl(
                              pui1_data,
                              ui4_data_len,
                              FALSE));
}

INT32 DvbObject::parseSegment_impl(
                                           UINT8*              pui1_data, 
                                           UINT32              ui4_data_len,
                                           bool                is_new_obj)
{
    UINT8       ui1_version;
    UINT8       ui1_num_of_codes;
    UINT16      ui2_top_field_len;
    UINT16      ui2_bottom_field_len;

    if (DVB_SBTL_ODS_MIN_LEN > ui4_data_len)
    {
        return(DVBR_WRONG_SEGMENT_DATA);
    }

    DVB_SBTL_ODS_GET_VERSION(pui1_data, ui1_version);
    if (FALSE == is_new_obj)
    {
        if (this->ui1_version_number == ui1_version)
        {
            return(DVBR_OK);
        }
    }
    else /* New object */
    {
        DVB_SBTL_ODS_GET_OBJ_ID(pui1_data, this->ui2_id);
		DVB_LOG("DvbObjectMgr::parseSegment_impl() new object id=%d",this->ui2_id);
    }

    this->ui1_version_number = ui1_version;

    DVB_SBTL_ODS_GET_CODING_METHOD(pui1_data, this->ui1_coding_method);
    DVB_SBTL_ODS_GET_NON_MODIFYING_COLOUR_FLAG(pui1_data, this->ui1_non_modifying_colour_flag);
	DVB_LOG("DvbObjectMgr::parseSegment_impl() new object coding method=%d(0->pixel,1->string)",this->ui1_coding_method);
    if ( (DVB_SBTL_OBJ_CODING_METHOD_PIXEL != this->ui1_coding_method) &&
         (DVB_SBTL_OBJ_CODING_METHOD_STRING != this->ui1_coding_method) )
    {
        return(DVBR_WRONG_SEGMENT_DATA);
    }

    /* check length */
    if (DVB_SBTL_OBJ_CODING_METHOD_PIXEL == this->ui1_coding_method)
    {
        if (DVB_SBTL_ODS_PIXEL_MIN_LEN > ui4_data_len)
        {
            return(DVBR_WRONG_SEGMENT_DATA);
        }

        DVB_SBTL_ODS_GET_TOP_FIELD_DATA_BLOCK_LEN(pui1_data, ui2_top_field_len);
        DVB_SBTL_ODS_GET_BOTTOM_FIELD_DATA_BLOCK_LEN(pui1_data, ui2_bottom_field_len);
		DVB_LOG("DvbObjectMgr::parseSegment_impl() new object top filed len=%d",ui2_top_field_len);
		DVB_LOG("DvbObjectMgr::parseSegment_impl() new object bottom filed len=%d",ui2_bottom_field_len);
        if ( (UINT32)(DVB_SBTL_ODS_PIXEL_MIN_LEN + ui2_top_field_len + ui2_bottom_field_len) > ui4_data_len)
        {
            return(DVBR_WRONG_SEGMENT_DATA);
        }
    }
    else /* DVB_SBTL_OBJ_CODING_METHOD_STRING == pt_this->ui1_coding_method */
    {
        DVB_SBTL_ODS_GET_NUM_OF_CODES(pui1_data, ui1_num_of_codes);

        if ( (UINT32) (DVB_SBTL_ODS_CHAR_MIN_LEN + ui1_num_of_codes) > ui4_data_len)
        {
            return(DVBR_WRONG_SEGMENT_DATA);
        }
    }

    if (NULL != this->pui1_data)
    {
        free((VOID*)this->pui1_data); 
    }

    this->pui1_data = (UINT8 *) malloc(ui4_data_len);
    if (NULL == this->pui1_data)
    {
        return(DVBR_INSUFFICIENT_MEMORY);
    }

    memcpy(this->pui1_data, pui1_data, ui4_data_len);
    this->ui2_data_len = (UINT16)ui4_data_len;

    return(DVBR_CONTENT_UPDATE);
}

INT32 DvbObject::render(DVB_RENDER_INFO_T* pt_sbtl_render_info)
{
	INT32		i4_ret;

	UINT16		ui2_top_field_data_block_len;
	UINT16		ui2_bottom_field_data_block_len;
	UINT16		ui2_rgn_line_byte_cnt;
	UINT16		ui2_rgn_obj_line_byte_idx;
	UINT8		ui1_rgn_obj_line_bit_idx;
	bool		is_dup_top_field;
	UINT8*		pui1_data_idx;
	UINT8*		pui1_rgn_cur;
	UINT16		ui2_obj_width;
	UINT16		ui2_obj_height;

	pt_sbtl_render_info->ui2_obj_height = 0;
	pt_sbtl_render_info->ui2_obj_width = 0;

	switch (pt_sbtl_render_info->ui1_rgn_depth)
	{
		case DVB_SBTL_RGN_DEPTH_2BIT:
			ui2_rgn_line_byte_cnt = pt_sbtl_render_info->ui2_rgn_width >> 2;
			if (0 != (pt_sbtl_render_info->ui2_rgn_width % 4))
			{
				ui2_rgn_line_byte_cnt++;
			}

			ui2_rgn_obj_line_byte_idx = pt_sbtl_render_info->ui2_obj_x_pos >> 2;
			ui1_rgn_obj_line_bit_idx = (UINT8)((pt_sbtl_render_info->ui2_obj_x_pos % 4) << 1);
			break;

		case DVB_SBTL_RGN_DEPTH_4BIT:
			ui2_rgn_line_byte_cnt = pt_sbtl_render_info->ui2_rgn_width >> 1;
			if (0 != (pt_sbtl_render_info->ui2_rgn_width % 2))
			{
				ui2_rgn_line_byte_cnt++;
			}

			ui2_rgn_obj_line_byte_idx = pt_sbtl_render_info->ui2_obj_x_pos >> 1;
			ui1_rgn_obj_line_bit_idx = (UINT8)((pt_sbtl_render_info->ui2_obj_x_pos % 2) << 2);
			break;

		case DVB_SBTL_RGN_DEPTH_8BIT:
			ui2_rgn_line_byte_cnt = pt_sbtl_render_info->ui2_rgn_width;

			ui2_rgn_obj_line_byte_idx = pt_sbtl_render_info->ui2_obj_x_pos;
			ui1_rgn_obj_line_bit_idx = 0;
			break;

		default:
			return(DVBR_INV_ARG);
	}

	if (DVB_SBTL_OBJ_CODING_METHOD_PIXEL == this->ui1_coding_method)
	{
		DVB_SBTL_ODS_GET_TOP_FIELD_DATA_BLOCK_LEN(this->pui1_data, ui2_top_field_data_block_len);
		DVB_SBTL_ODS_GET_BOTTOM_FIELD_DATA_BLOCK_LEN(this->pui1_data, ui2_bottom_field_data_block_len);

		if (0 == ui2_top_field_data_block_len)
		{
			return (DVBR_WRONG_SEGMENT_DATA);
		}

		/* parsing top field data block */
		pui1_data_idx = &(this->pui1_data[DVB_SBTL_ODS_PIXEL_MIN_LEN]);

		pui1_rgn_cur = pt_sbtl_render_info->pui1_rgn + 
					   (ui2_rgn_line_byte_cnt * pt_sbtl_render_info->ui2_obj_y_pos);

		if (0 == ui2_bottom_field_data_block_len)
		{
			is_dup_top_field = TRUE;
		}
		else
		{
			is_dup_top_field = FALSE;
		}

		if (0 < ui2_top_field_data_block_len)
		{
			i4_ret = render(
						 TRUE, /* is_top_field */
						 is_dup_top_field, /* is_dup_top_field */
						 this->ui1_non_modifying_colour_flag, /* non modifying flag */
						 pt_sbtl_render_info->ui1_rgn_depth, /* ui1_rgn_depth */
						 pui1_rgn_cur, /* pui1_rgn_buf */
						 ui2_rgn_line_byte_cnt, /* ui2_rgn_line_byte_cnt */
						 ui2_rgn_obj_line_byte_idx, /* ui2_rgn_obj_line_byte_idx */
						 ui1_rgn_obj_line_bit_idx, /* ui1_rgn_obj_line_bit_idx */					   

						 pt_sbtl_render_info->ui2_rgn_width - pt_sbtl_render_info->ui2_obj_x_pos, /* ui2_obj_max_width */
						 pt_sbtl_render_info->ui2_rgn_height - pt_sbtl_render_info->ui2_obj_y_pos, /* ui2_obj_max_height */

						 ((UINT32) ui2_top_field_data_block_len) << 3, /* ui2_field_data_block_len - bits */
						 pui1_data_idx, /* pui1_field_data */

						 &ui2_obj_width,
						 &ui2_obj_height);

			if (ui2_obj_height > 1)
			{
				if (FALSE == is_dup_top_field)
				{
					ui2_obj_height--;
				}
			}
			
			if (ui2_obj_height > pt_sbtl_render_info->ui2_rgn_height - pt_sbtl_render_info->ui2_obj_y_pos)
			{
				ui2_obj_height = pt_sbtl_render_info->ui2_rgn_height - pt_sbtl_render_info->ui2_obj_y_pos;
			}			 
			
			if (ui2_obj_height > pt_sbtl_render_info->ui2_obj_height)
			{
				pt_sbtl_render_info->ui2_obj_height = ui2_obj_height;
			}

			if (ui2_obj_width > pt_sbtl_render_info->ui2_obj_width)
			{
				pt_sbtl_render_info->ui2_obj_width = ui2_obj_width;
			}
		}

		/* inc data index */
		pui1_data_idx += ui2_top_field_data_block_len;
		
		/* parsing bottom field data block */
		if ((0 < ui2_bottom_field_data_block_len) && 
			((pt_sbtl_render_info->ui2_rgn_height - pt_sbtl_render_info->ui2_obj_y_pos) > 1)
		   )
		{
			i4_ret = render(
						 FALSE, /* is_top_field */
						 FALSE, /* is_dup_top_field */
						 this->ui1_non_modifying_colour_flag, /* non modifying flag */
						 pt_sbtl_render_info->ui1_rgn_depth, /* ui1_rgn_depth */
						 pui1_rgn_cur, /* pui1_rgn_buf */
						 ui2_rgn_line_byte_cnt, /* ui2_rgn_line_byte_cnt */
						 ui2_rgn_obj_line_byte_idx, /* ui2_rgn_obj_line_byte_idx */
						 ui1_rgn_obj_line_bit_idx, /* ui1_rgn_obj_line_bit_idx */

						 pt_sbtl_render_info->ui2_rgn_width - pt_sbtl_render_info->ui2_obj_x_pos, /* ui2_obj_max_width */
						 pt_sbtl_render_info->ui2_rgn_height - pt_sbtl_render_info->ui2_obj_y_pos, /* ui2_obj_max_height */

						 (ui2_bottom_field_data_block_len << 3), /* ui2_field_data_block_len */
						 pui1_data_idx, /* pui1_field_data */
						 
						 &ui2_obj_width,
						 &ui2_obj_height);
		
			if ( ui2_obj_height > (pt_sbtl_render_info->ui2_rgn_height - pt_sbtl_render_info->ui2_obj_y_pos) )
			{
				ui2_obj_height = pt_sbtl_render_info->ui2_rgn_height - pt_sbtl_render_info->ui2_obj_y_pos;
			}
			
			if (ui2_obj_height > pt_sbtl_render_info->ui2_obj_height)
			{
				pt_sbtl_render_info->ui2_obj_height = ui2_obj_height;
			}
		
			if (ui2_obj_width > pt_sbtl_render_info->ui2_obj_width)
			{
				pt_sbtl_render_info->ui2_obj_width = ui2_obj_width;
			}
		}

	}
	else /* string */
	{
#if 0
		i4_ret = sm_sbtl_hdlr_render_text_obj((UINT16 *) (this->pui1_data + 1),
												this->pui1_data[0],
												pt_sbtl_render_info);
#endif
		ALOGE("ERROR,do not support text  object\n");
	}

	return(i4_ret);
}


INT32 DvbObject::render(
			    bool        is_top_field,
			    bool        is_dup_top_field,
			    UINT8       ui1_non_modify_flag,
			    UINT8       ui1_rgn_depth,
			    UINT8*      pui1_rgn_buf,
			    UINT16      ui2_rgn_buf_line_byte_cnt,
			    UINT16      ui2_rgn_obj_line_byte_idx,
			    UINT8       ui1_rgn_obj_line_bit_idx,
			    
			    UINT16      ui2_obj_max_width,
			    UINT16      ui2_obj_max_height,
			    
			    UINT32      ui4_field_data_block_len,
			    UINT8*      pui1_field_data,
			    UINT16*     pui2_obj_width,
			    UINT16*     pui2_obj_height)
{
	UINT8						aui1_2_to_4_map_tbl[4];
	UINT8						aui1_2_to_8_map_tbl[4];
	UINT8						aui1_4_to_8_map_tbl[16];
	UINT8*						paui1_map_tbls[3][3] = 
								{
									{
										(UINT8*) aui1_2_to_8_default_map_tbl,
										(UINT8*) aui1_2_to_4_default_map_tbl,
										(UINT8*) aui1_2_to_2_default_map_tbl,
									},
									
									{
										(UINT8*) aui1_4_to_8_default_map_tbl,
										(UINT8*) aui1_4_to_4_default_map_tbl,
										(UINT8*) aui1_4_to_2_default_map_tbl,
									},
				
									{
										(UINT8*) aui1_8_to_8_default_map_tbl,
										(UINT8*) aui1_8_to_4_default_map_tbl,
										(UINT8*) aui1_8_to_2_default_map_tbl,
									},
								};

	UINT8						ui1_pixel_data_type;

	UINT8						ui1_field_data_bit_idx;
	UINT8						ui1_fill_byte;

	UINT32						ui4_data;
	UINT16						ui2_data;
	UINT8						ui1_used_data_bit_cnt;
	UINT16						ui2_pixel_cnt;
	UINT8						ui1_pixel_data;
	bool						is_end_of_string;

	UINT8*						pui1_rgn_line_start;
	UINT8*						pui1_rgn_cur;
	UINT8*						pui1_rgn_next_line_cur;
	UINT8						ui1_rgn_cur_bit_idx;

	DVB_OBJ_FILL_INFO_T 		t_obj_fill_info;
	UINT8						ui1_fill_mask;
	UINT8						ui1_fill_pixel_cnt;
	
	UINT8						ui1_tmp;
	UINT16						ui2_obj_line_width;


	/* initialize */
	is_end_of_string = TRUE;
	ui1_field_data_bit_idx = 0;
	*pui2_obj_height = 0;
	
	if (TRUE == is_top_field)
	{
		pui1_rgn_line_start = pui1_rgn_buf;
	}
	else
	{
		pui1_rgn_line_start = pui1_rgn_buf + ui2_rgn_buf_line_byte_cnt;
	}
	pui1_rgn_cur = pui1_rgn_line_start + ui2_rgn_obj_line_byte_idx;
	pui1_rgn_next_line_cur = pui1_rgn_cur + ui2_rgn_buf_line_byte_cnt;
	ui1_rgn_cur_bit_idx = ui1_rgn_obj_line_bit_idx;

	*pui2_obj_width = 0;
	ui2_obj_line_width = 0;

	ui1_pixel_data = 0;
	ui2_pixel_cnt = 0;
	ui1_pixel_data_type = 0xFF;

	/* Parsing Data */
	while (ui4_field_data_block_len > 0)
	{
		if (TRUE == is_end_of_string)
		{
			if ((0 != ui1_field_data_bit_idx) || (8 > ui4_field_data_block_len))
			{
				/* something wrong */
				return (DVBR_WRONG_SEGMENT_DATA);
			}

			if ( (is_dup_top_field==TRUE) &&
				 ((UINT16)((*pui2_obj_height)+1)>= ui2_obj_max_height) )
			{
				is_dup_top_field=FALSE;
			}
			
			ui1_pixel_data_type = pui1_field_data[0];
			is_end_of_string = FALSE;

			/* pixel-data_sub-block */
			ui4_field_data_block_len -= 8;
			pui1_field_data++;
		}

		/* Run length decoding */
		switch (ui1_pixel_data_type)
		{
			case 0x10: /* 2-bit */
				/* assert */
				if (0 != (ui1_field_data_bit_idx % 2))
				{
					return (DVBR_WRONG_SEGMENT_DATA);
				}
				
				/* get ui2_data */
				ui2_data = (((UINT16) (pui1_field_data[0] << ui1_field_data_bit_idx)) << 8) + 
					(((UINT16) pui1_field_data[1]) << ui1_field_data_bit_idx) + 
					(UINT16) (pui1_field_data[2] >> (8 - ui1_field_data_bit_idx));

				/* calcuate pixel */
				ui1_pixel_data = (UINT8)((ui2_data & 0xC000) >> 14);

				if (0 != ui1_pixel_data) /* nextbits() != '00' */
				{
					/* one pixel in colour 1, 2, 3 */
					ui1_used_data_bit_cnt = 2;
					ui2_pixel_cnt = 1;
				}
				else
				{
					if (0 != (ui2_data & 0x2000)) /* switch_1 */
					{
						/* '00 1L LL CC' - L pixels(3-10) in colour C */
						ui1_used_data_bit_cnt = 8;
						ui2_pixel_cnt = (UINT16)(((ui2_data & 0x1C00) >> 10) + 3);
						ui1_pixel_data = (UINT8)((ui2_data & 0x0300) >> 8);
					}
					else
					{
						if (0 == (ui2_data & 0x1000)) /* switch_2 */
						{
							switch (ui2_data & 0x0C00) /* switch_3 */
							{
								case 0x0400:
									/* '00 00 01' two pixel in colour 0 */ 
									ui2_pixel_cnt = 2;
									ui1_pixel_data = 0;
									ui1_used_data_bit_cnt = 6;
									break;

								case 0x0800: 
									/* '00 00 10 LL LL CC' - L pixels (12-27) in colour C */
									ui1_pixel_data = (UINT8)((ui2_data & 0x0030) >> 4);
									ui2_pixel_cnt = (UINT16)(((ui2_data & 0x03C0) >> 6) + 12);
									ui1_used_data_bit_cnt = 12;
									break;

								case 0x0C00: 
									/* '00 00 11 LL LL LL LL CC' - L pixels (29-284) in colour C */
									ui1_pixel_data = (UINT8)(ui2_data & 0x0003);
									ui2_pixel_cnt = (UINT16)(((ui2_data & 0x03FC) >> 2) + 29);
									ui1_used_data_bit_cnt = 16;
									break;

								default : /* 0x000000 */
									/* end of 2bit pixel code string */
									if (2 == ui1_field_data_bit_idx)
									{
										ui1_used_data_bit_cnt = 6;
									}
									else
									{
										ui1_used_data_bit_cnt = (UINT8)(14 - ((ui1_field_data_bit_idx + 6) % 8));
									}
									is_end_of_string = TRUE;
									break;
							}
						}
						else
						{
							/* one pixel in colour 0 */
							ui2_pixel_cnt = 1;
							ui1_pixel_data = 0;
							ui1_used_data_bit_cnt = 4;
						}
					}
				}

				break;

			case 0x11: /* 4-bit */
				/* assert */
				if (0 != (ui1_field_data_bit_idx % 4))
				{
					return (DVBR_WRONG_SEGMENT_DATA);
				}
				
				ui4_data = (((UINT32) (pui1_field_data[0] << ui1_field_data_bit_idx)) << 24) + \
					(((UINT32) pui1_field_data[1]) << (ui1_field_data_bit_idx + 16)) + \
					(((UINT32) pui1_field_data[2]) << (ui1_field_data_bit_idx + 8)) + \
					(((UINT32) pui1_field_data[3]) << ui1_field_data_bit_idx) + \
					(UINT32) (pui1_field_data[4] >> (8 - ui1_field_data_bit_idx));
				
				ui1_pixel_data = (UINT8)((ui4_data & 0xF0000000) >> 28);

				if (0 != ui1_pixel_data) /* nextbits() != '0000' */
				{
					/* one pixle in colour 1 - 15 */
					ui1_used_data_bit_cnt = 4;
					ui2_pixel_cnt = 1;
				}
				else
				{
					if (0 == (ui4_data & 0x08000000)) /* switch_1 */
					{
						ui2_pixel_cnt = (UINT16)((ui4_data & 0x07000000) >> 24);

						if (0 != ui2_pixel_cnt) /* nextbits() != '000' */
						{
							/* '0000 0LLL' - run length 3-9 in colour 0 */
							ui1_used_data_bit_cnt = 8;
							ui1_pixel_data = 0;
							ui2_pixel_cnt += 2;
						}
						else
						{
							/* '0000 0000' end of 4bit pixel code */
							if (0 == ui1_field_data_bit_idx)
							{
								ui1_used_data_bit_cnt = 8;
							}
							else
							{
								ui1_used_data_bit_cnt = 12;
							}
							is_end_of_string = TRUE;
						}
					}
					else
					{
						if (0 == (ui4_data & 0x04000000)) /* switch_2 */
						{
							/* '0000 10LL CCCC' - L pixels (4-7) in colour C */
							ui1_pixel_data = (UINT8)((ui4_data & 0x00F00000) >> 20);
							ui2_pixel_cnt = (UINT16)(((ui4_data & 0x03000000) >> 24) + 4);
							ui1_used_data_bit_cnt = 12;
						}
						else
						{
							switch (ui4_data & 0x03000000) /* switch_3 */
							{
								case 0x02000000:
									/* '0000 1110 LLLL CCCC' - L pixels (9-24) in colour C */
									ui1_pixel_data = (UINT8)((ui4_data & 0x000F0000) >> 16);
									ui2_pixel_cnt = (UINT16)(((ui4_data & 0x00F00000) >> 20) + 9);
									ui1_used_data_bit_cnt = 16;
									break;

								case 0x03000000:
									/* '0000 1111 LLLL LLLL CCCC' - L pixels (25-280) in colour C */
									ui1_pixel_data = (UINT8)((ui4_data & 0x0000F000) >> 12);
									ui2_pixel_cnt = (UINT16)(((ui4_data & 0x00FF0000) >> 16) + 25);
									ui1_used_data_bit_cnt = 20;
									break;

								case 0x01000000:
									/* '0000 1101' - two pixel in colour 0 */
									ui1_pixel_data = 0;
									ui2_pixel_cnt = 2;
									ui1_used_data_bit_cnt = 8;
									break;

								default : /* 0x00000000 */
									/* '0000 1100' - one pixel in colour 0 */
									ui1_pixel_data = 0;
									ui2_pixel_cnt = 1;
									ui1_used_data_bit_cnt = 8;
									break;
							}
						}
					}
				}

				break;

			case 0x12: /* 8-bit */
				
				/* assert */
				if (0 != ui1_field_data_bit_idx)
				{
					return (DVBR_WRONG_SEGMENT_DATA);
				}

				ui4_data = (((UINT32) (pui1_field_data[0] << ui1_field_data_bit_idx)) << 24) + \
					(((UINT32) pui1_field_data[1]) << (ui1_field_data_bit_idx + 16)) + \
					(((UINT32) pui1_field_data[2]) << (ui1_field_data_bit_idx + 8)) + \
					(((UINT32) pui1_field_data[3]) << ui1_field_data_bit_idx) + \
					(UINT32) (pui1_field_data[4] >> (8 - ui1_field_data_bit_idx));
				
				ui1_pixel_data = (UINT8)((ui4_data & 0xFF000000) >> 24);

				if (0 != ui1_pixel_data)
				{
					/* one pixel in colour 1-255 */
					ui1_used_data_bit_cnt = 8;
					ui2_pixel_cnt = 1;
				}
				else
				{
					ui2_pixel_cnt = (UINT16)((ui4_data & 0x007F0000) >> 16); 
					if (0 == (ui4_data & 0x00800000))
					{
						ui1_used_data_bit_cnt = 16;
						if (0 != ui2_pixel_cnt)
						{
							/* '00000000 0LLLLLLL' - L pixels(1-127) in colour 0, L > 0 */
							ui1_pixel_data = 0;
						}
						else
						{
							/* '00000000 00000000' - end of 8bit pixel code */
							is_end_of_string = TRUE;
						}
					}
					else
					{
						/* '00000000 1LLLLLLL CCCCCCCC' - L(3-127) pixel in colour C, L > 2 */
						ui1_used_data_bit_cnt = 24;
						ui1_pixel_data = (UINT8)((ui4_data & 0x0000FF00) >> 8);
					}
				}

				break;

			case 0x20: /* 2 to 4 bit map */
				/* assert */
				if (0 != ui1_field_data_bit_idx)
				{
					return (DVBR_WRONG_SEGMENT_DATA);
				}

				if (16 > ui4_field_data_block_len)
				{
					return (DVBR_WRONG_SEGMENT_DATA);
				}
				
				paui1_map_tbls[0][1] = aui1_2_to_4_map_tbl;

				ui1_tmp = (pui1_field_data[0] & 0xF0) >> 4;
				aui1_2_to_4_map_tbl[0] = (ui1_tmp << 4) + ui1_tmp;
				
				ui1_tmp = pui1_field_data[0] & 0x0F;
				aui1_2_to_4_map_tbl[1] = (ui1_tmp << 4) + ui1_tmp;
				
				ui1_tmp = (pui1_field_data[1] & 0xF0) >> 4;
				aui1_2_to_4_map_tbl[2] = (ui1_tmp << 4) + ui1_tmp;
				
				ui1_tmp = pui1_field_data[1] & 0x0F;
				aui1_2_to_4_map_tbl[3] = (ui1_tmp << 4) + ui1_tmp;

				ui1_used_data_bit_cnt = 16;
				is_end_of_string = TRUE;
				break;

			case 0x21: /* 2 to 8 bit map */
				/* assert */
				if (0 != ui1_field_data_bit_idx)
				{
					return (DVBR_WRONG_SEGMENT_DATA);
				}
				
				if (32 > ui4_field_data_block_len)
				{
					return (DVBR_WRONG_SEGMENT_DATA);
				}
				
				paui1_map_tbls[0][0] = aui1_2_to_8_map_tbl;
				aui1_2_to_8_map_tbl[0] = pui1_field_data[0];
				aui1_2_to_8_map_tbl[1] = pui1_field_data[1];
				aui1_2_to_8_map_tbl[2] = pui1_field_data[2];
				aui1_2_to_8_map_tbl[3] = pui1_field_data[3];

				ui1_used_data_bit_cnt = 32;
				is_end_of_string = TRUE;
				break;

			case 0x22: /* 4 to 8 bit map */
				/* assert */
				if (0 != ui1_field_data_bit_idx)
				{
					return (DVBR_WRONG_SEGMENT_DATA);
				}
				
				if (128 > ui4_field_data_block_len)
				{
					return (DVBR_WRONG_SEGMENT_DATA);
				}
				
				paui1_map_tbls[1][0] = aui1_4_to_8_map_tbl;
				memcpy(aui1_4_to_8_map_tbl, pui1_field_data, 16);

				ui1_used_data_bit_cnt = 128;
				is_end_of_string = TRUE;
				break;

			case 0xF0:
				pui1_rgn_line_start += (ui2_rgn_buf_line_byte_cnt << 1);

				pui1_rgn_cur = pui1_rgn_line_start + ui2_rgn_obj_line_byte_idx;
				pui1_rgn_next_line_cur = pui1_rgn_cur + ui2_rgn_buf_line_byte_cnt;
				ui1_rgn_cur_bit_idx = ui1_rgn_obj_line_bit_idx;

				if (ui2_obj_line_width <= ui2_obj_max_width)
				{
					if (ui2_obj_line_width > *pui2_obj_width)
					{
						*pui2_obj_width = ui2_obj_line_width;
					}
				}
				ui2_obj_line_width = 0;

				*pui2_obj_height = (UINT16)((*pui2_obj_height) + 2);

				ui1_used_data_bit_cnt = 0;
				is_end_of_string = TRUE;
				break;

			default:
				return (DVBR_WRONG_SEGMENT_DATA);
		}

		/* Check data block size, inc data block cursor */
		if (ui4_field_data_block_len < (UINT32) ui1_used_data_bit_cnt)
		{
			return (DVBR_WRONG_SEGMENT_DATA);
		}

		ui1_field_data_bit_idx += ui1_used_data_bit_cnt;
		pui1_field_data += (ui1_field_data_bit_idx >> 3);
		ui4_field_data_block_len -= ui1_used_data_bit_cnt;
		ui1_field_data_bit_idx = ui1_field_data_bit_idx % 8;

		if (TRUE == is_end_of_string)
		{
			continue;
		}

		ui2_obj_line_width += ui2_pixel_cnt;

		/* exceed boundary */
		if ( (ui2_obj_line_width > ui2_obj_max_width)||
			 (*pui2_obj_height>=ui2_obj_max_height) )
		{
			continue;
		}

		ui1_fill_byte = paui1_map_tbls[ui1_pixel_data_type & 0x03]
									  [ui1_rgn_depth >> 1]
									  [ui1_pixel_data];

		switch (ui1_rgn_depth)
		{
			case DVB_SBTL_RGN_DEPTH_2BIT:
			{
				UINT8	ui1_idx;
				UINT8	ui1_pix_cnt;

				/* fill first not-bytealigned region */
				if ((4 > ui2_pixel_cnt) || (0 != ui1_rgn_cur_bit_idx))
				{					 
					ui1_idx = (ui1_rgn_cur_bit_idx >> 1);
					ui1_pix_cnt = (UINT8)(ui2_pixel_cnt % 4);

					ui1_idx = (ui1_idx<TWO_BIT_OBJ_INFO)?ui1_idx:0;
					ui1_pix_cnt = (ui1_pix_cnt<TWO_BIT_OBJ_INFO)?ui1_pix_cnt:0;
				
					if (4 > ui2_pixel_cnt)
					{
						t_obj_fill_info = at_2bit_obj_fill_info[ui1_idx][ui1_pix_cnt];
					}
					else
					{
						t_obj_fill_info = at_2bit_obj_fill_info2[ui1_idx][ui1_pix_cnt];
					}
				
					ui1_fill_mask = t_obj_fill_info.ui1_fill_mask;
					ui1_fill_pixel_cnt = t_obj_fill_info.fill_pixel_cnt;
				
					/* Not transparent hole */
					if ((1 != ui1_non_modify_flag) || (1 != ui1_pixel_data))
					{
						pui1_rgn_cur[0] = (pui1_rgn_cur[0] & ~ui1_fill_mask) | 
							(ui1_fill_mask & ui1_fill_byte);
				
						if (TRUE == is_dup_top_field)
						{
							pui1_rgn_next_line_cur[0] = (pui1_rgn_next_line_cur[0] & ~ui1_fill_mask) | 
								(ui1_fill_mask & ui1_fill_byte);
						}
					}
				
					ui2_pixel_cnt -= ui1_fill_pixel_cnt;
				
					ui1_rgn_cur_bit_idx = t_obj_fill_info.ui1_bit_idx;
					if (0 == ui1_rgn_cur_bit_idx)
					{
						pui1_rgn_cur++;
						pui1_rgn_next_line_cur++;
					}
				}
			
				/* fill repeat bytes */
				if (ui2_pixel_cnt >= 4)
				{
					if ((1 != ui1_non_modify_flag) || (1 != ui1_pixel_data))
					{
						memset(pui1_rgn_cur, ui1_fill_byte, (ui2_pixel_cnt >> 2));
			
						if (TRUE == is_dup_top_field)
						{
							memset(pui1_rgn_next_line_cur, ui1_fill_byte, (ui2_pixel_cnt >> 2));
						}
					}
					pui1_rgn_cur += (ui2_pixel_cnt >> 2);
					pui1_rgn_next_line_cur += (ui2_pixel_cnt >> 2);
			
					ui2_pixel_cnt = ui2_pixel_cnt % 4;
				}
			
				/* fill left bits */
				if (0 < ui2_pixel_cnt)
				{
					ui1_idx = (ui1_rgn_cur_bit_idx >> 1);
					ui1_pix_cnt = (UINT8)(ui2_pixel_cnt % 4);

					ui1_idx = (ui1_idx<TWO_BIT_OBJ_INFO)?ui1_idx:0;
					ui1_pix_cnt = (ui1_pix_cnt<TWO_BIT_OBJ_INFO)?ui1_pix_cnt:0;

					t_obj_fill_info = at_2bit_obj_fill_info[ui1_idx][ui1_pix_cnt];
				
					ui1_fill_mask = t_obj_fill_info.ui1_fill_mask;
					ui1_fill_pixel_cnt = t_obj_fill_info.fill_pixel_cnt;
				
					/* Not transparent hole */
					if ((1 != ui1_non_modify_flag) || (1 != ui1_pixel_data))
					{
						pui1_rgn_cur[0] = (pui1_rgn_cur[0] & ~ui1_fill_mask) | 
							(ui1_fill_mask & ui1_fill_byte);
				
						if (TRUE == is_dup_top_field)
						{
							pui1_rgn_next_line_cur[0] = (pui1_rgn_next_line_cur[0] & ~ui1_fill_mask) | 
								(ui1_fill_mask & ui1_fill_byte);
						}
					}
				
					ui2_pixel_cnt -= ui1_fill_pixel_cnt;
				
					ui1_rgn_cur_bit_idx = t_obj_fill_info.ui1_bit_idx;
					if (0 == ui1_rgn_cur_bit_idx)
					{
						pui1_rgn_cur++;
						pui1_rgn_next_line_cur++;
					}
				}
			}
			break;

			case DVB_SBTL_RGN_DEPTH_4BIT:
			{
				UINT8	ui1_idx;
				UINT8	ui1_pix_cnt;

				/* fill first not-bytealigned region */
				if ((2 > ui2_pixel_cnt) || (0 != ui1_rgn_cur_bit_idx))
				{
					ui1_idx = (ui1_rgn_cur_bit_idx >> 2);
					ui1_pix_cnt = (UINT8)(ui2_pixel_cnt % 2);

					ui1_idx = (ui1_idx<FOUR_BIT_OBJ_INFO)?ui1_idx:0;
					ui1_pix_cnt = (ui1_pix_cnt<FOUR_BIT_OBJ_INFO)?ui1_pix_cnt:0;

					t_obj_fill_info = at_4bit_obj_fill_info[ui1_idx][ui1_pix_cnt];
			
					ui1_fill_mask = t_obj_fill_info.ui1_fill_mask;
					ui1_fill_pixel_cnt = t_obj_fill_info.fill_pixel_cnt;
			
					/* not transparent holes */
					if ((1 != ui1_non_modify_flag) || (1 != ui1_pixel_data))
					{
						pui1_rgn_cur[0] = (pui1_rgn_cur[0] & ~ui1_fill_mask) | 
							(ui1_fill_mask & ui1_fill_byte);
			
						if (TRUE == is_dup_top_field)
						{
							pui1_rgn_next_line_cur[0] = (pui1_rgn_next_line_cur[0] & ~ui1_fill_mask) | 
								(ui1_fill_mask & ui1_fill_byte);
						}
					}
			
					if (ui2_pixel_cnt < ui1_fill_pixel_cnt)
					{
						//DVB_SBTL_ASSERT(FALSE, DBG_CAT_INV_OP, DBG_ABRT_INV_ARG);
					}
					
					ui2_pixel_cnt -= ui1_fill_pixel_cnt;
			
					ui1_rgn_cur_bit_idx = t_obj_fill_info.ui1_bit_idx;
					if (0 == ui1_rgn_cur_bit_idx)
					{
						pui1_rgn_cur++;
						pui1_rgn_next_line_cur++;
					}
				}
			
				/* fill repeat bytes */
				if (ui2_pixel_cnt > 2)
				{
					if ((1 != ui1_non_modify_flag) || (1 != ui1_pixel_data))
					{
						memset(pui1_rgn_cur, ui1_fill_byte, (ui2_pixel_cnt >> 1));

						if (TRUE == is_dup_top_field)
						{
							memset(pui1_rgn_next_line_cur, ui1_fill_byte, (ui2_pixel_cnt >> 1));
						}
					}
					pui1_rgn_cur += (ui2_pixel_cnt >> 1);
					pui1_rgn_next_line_cur += (ui2_pixel_cnt >> 1);
			
					ui2_pixel_cnt = ui2_pixel_cnt % 2;
				}
			
				/* fill left bits */
				if (0 < ui2_pixel_cnt)
				{
					ui1_idx = (ui1_rgn_cur_bit_idx >> 2);
					ui1_pix_cnt = (UINT8)(ui2_pixel_cnt % 2);

					ui1_idx = (ui1_idx<FOUR_BIT_OBJ_INFO)?ui1_idx:0;
					ui1_pix_cnt = (ui1_pix_cnt<FOUR_BIT_OBJ_INFO)?ui1_pix_cnt:0;
					
					t_obj_fill_info = at_4bit_obj_fill_info[ui1_idx][ui1_pix_cnt];
			
					ui1_fill_mask = t_obj_fill_info.ui1_fill_mask;
					ui1_fill_pixel_cnt = t_obj_fill_info.fill_pixel_cnt;
			
					/* not transparent holes */
					if ((1 != ui1_non_modify_flag) || (1 != ui1_pixel_data))
					{
						pui1_rgn_cur[0] = (pui1_rgn_cur[0] & ~ui1_fill_mask) | 
							(ui1_fill_mask & ui1_fill_byte);
			
						if (TRUE == is_dup_top_field)
						{
							pui1_rgn_next_line_cur[0] = (pui1_rgn_next_line_cur[0] & ~ui1_fill_mask) | 
								(ui1_fill_mask & ui1_fill_byte);
						}
					}
			
					ui2_pixel_cnt -= ui1_fill_pixel_cnt;
			
					ui1_rgn_cur_bit_idx = t_obj_fill_info.ui1_bit_idx;
					if (0 == ui1_rgn_cur_bit_idx)
					{
						pui1_rgn_cur++;
						pui1_rgn_next_line_cur++;
					}
				}
			}
			break;

			case DVB_SBTL_RGN_DEPTH_8BIT:
			{
				/* assert */
				if (0 != (ui1_rgn_cur_bit_idx % 8))
				{
					return (DVBR_WRONG_SEGMENT_DATA);
				}

				/* not transparent holes */
				if ((1 != ui1_non_modify_flag) || (1 != ui1_pixel_data))
				{
					memset(pui1_rgn_cur, ui1_fill_byte, ui2_pixel_cnt);

					if (TRUE == is_dup_top_field)
					{
						memset(pui1_rgn_next_line_cur, ui1_fill_byte, ui2_pixel_cnt);
					}
				}
				
				pui1_rgn_cur += ui2_pixel_cnt;
				pui1_rgn_next_line_cur += ui2_pixel_cnt;
			}
			break;

			default: /* impossible case */
				return (DVBR_WRONG_SEGMENT_DATA);
		}
	}

	return(DVBR_OK);
}


#endif

