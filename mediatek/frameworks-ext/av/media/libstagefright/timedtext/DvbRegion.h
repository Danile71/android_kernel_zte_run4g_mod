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

#ifndef __DVB_REGION_H_
#define __DVB_REGION_H_

 /*-----------------------------------------------------------------------------
					  include files
   ----------------------------------------------------------------------------*/
  
 #include "DVBDataType.h"

/*-----------------------------------------------------------------------------
				  macros, defines, typedefs, enums
----------------------------------------------------------------------------*/
/* Region Composition Segment */
#define DVB_SBTL_RCS_MIN_LEN                                     ((UINT32) (10))
#define DVB_SBTL_RCS_PIXEL_OBJ_LST_LEN                           ((UINT32) (6))
#define DVB_SBTL_RCS_CHAR_OBJ_LST_LEN                            ((UINT32) (8))
	 
#define DVB_SBTL_RCS_GET_RGN_ID(pui1_data, ui1_rid) \
		 ui1_rid = (((UINT8*) (pui1_data))[0])
	 
#define DVB_SBTL_RCS_GET_VERSION(pui1_data, ui1_version) \
		 ui1_version = (((UINT8*) (pui1_data))[1] & 0xF0) >> 4
	 
#define DVB_SBTL_RCS_GET_FILL_FLAG(pui1_data, ui1_fill_flag) \
		 ui1_fill_flag = (((UINT8*) (pui1_data))[1] & 0x08) >> 3
	 
#define DVB_SBTL_RCS_GET_RGN_WIDTH(pui1_data, ui2_rgn_width) \
		 GET_UINT16(pui1_data + 2, ui2_rgn_width)
	 
#define DVB_SBTL_RCS_GET_RGN_HEIGHT(pui1_data, ui2_rgn_height) \
		 GET_UINT16(pui1_data + 4, ui2_rgn_height)
	 
#define DVB_SBTL_RCS_GET_RGN_LEVEL_OF_COMPATIBILITY(pui1_data, ui1_rgn_level_of_compatibility) \
		 ui1_rgn_level_of_compatibility = (((UINT8 *) (pui1_data))[6] & 0xE0) >> 5
	 
#define DVB_SBTL_RCS_GET_RGN_DEPTH(pui1_data, ui1_rgn_depth) \
		 ui1_rgn_depth = (((UINT8 *) (pui1_data))[6] & 0x1C) >> 2
	 
#define DVB_SBTL_RCS_GET_RGN_CLUT_ID(pui1_data, ui1_CLUT_id) \
		 ui1_CLUT_id = ((UINT8 *) (pui1_data))[7]
	 
#define DVB_SBTL_RCS_GET_RGN_8BIT_PIXEL_CODE(pui1_data, ui1_8bit_pixel_code) \
		 ui1_8bit_pixel_code = ((UINT8 *) (pui1_data))[8]
	 
#define DVB_SBTL_RCS_GET_RGN_4BIT_PIXEL_CODE(pui1_data, ui1_4bit_pixel_code) \
		 ui1_4bit_pixel_code = (((UINT8 *) (pui1_data))[9] & 0xF0) >> 4
	 
#define DVB_SBTL_RCS_GET_RGN_2BIT_PIXEL_CODE(pui1_data, ui1_2bit_pixel_code) \
		 ui1_2bit_pixel_code = (((UINT8 *) (pui1_data))[9] & 0x0C) >> 2
	 
#define DVB_SBTL_RCS_GET_OBJ_ID(pui1_data, ui2_obj_id) \
		 GET_UINT16(pui1_data, ui2_obj_id)
	 
#define DVB_SBTL_RCS_GET_OBJ_TYPE(pui1_data, ui1_obj_type) \
		 ui1_obj_type = (((UINT8 *) (pui1_data))[2] & 0xC0) >> 6
	 
#define DVB_SBTL_RCS_GET_OBJ_PROVIDER_FLAG(pui1_data, ui1_obj_provider_flag) \
		 ui1_obj_provider_flag = (((UINT8 *) (pui1_data))[2] & 0x30) >> 4
	 
#define DVB_SBTL_RCS_GET_OBJ_HORIZONTAL_POSITION(pui1_data, ui2_obj_horizontal_position) \
		 ui2_obj_horizontal_position = (((UINT8 *) (pui1_data))[2] & 0x0F) << 8; \
		 ui2_obj_horizontal_position += ((UINT8 *) (pui1_data))[3]
	 
#define DVB_SBTL_RCS_GET_OBJ_VERTICAL_POSITION(pui1_data, ui2_obj_vertical_position) \
		 ui2_obj_vertical_position = (((UINT8 *) (pui1_data))[4] & 0x0F) << 8; \
		 ui2_obj_vertical_position += ((UINT8 *) (pui1_data))[5]
	 
#define DVB_SBTL_RCS_GET_OBJ_FOREGROUND_CODE(pui1_data, ui1_obj_fg_code) \
		 ui1_obj_fg_code = ((UINT8 *) (pui1_data))[6]
	 
#define DVB_SBTL_RCS_GET_OBJ_BACKGROUND_CODE(pui1_data, ui1_obj_bg_code) \
		 ui1_obj_bg_code = ((UINT8 *) (pui1_data))[7]
  
  
  /*-----------------------------------------------------------------------------
					  data declarations, extern, static, const
   ----------------------------------------------------------------------------*/
  
  /*-----------------------------------------------------------------------------
					  class declarations
   ----------------------------------------------------------------------------*/

 
struct DVB_RGN_OBJ_LST_T
{
	UINT16  ui2_id;

	UINT8	 ui1_type;

	UINT8	 ui1_provider_flag;

	UINT16  ui2_horizontal_position;

	UINT16  ui2_vertical_position;

	UINT8	 ui1_fg_color;

	UINT8	 ui1_bg_color;

	bool	 is_render;
};




struct DVB_RENDER_INFO_T
{
	UINT16 ui2_obj_x_pos;
	UINT16 ui2_obj_y_pos;
	UINT8	ui1_obj_fg_color;		 /* Only used as a text object */
	UINT8	ui1_obj_bg_color;		 /* Only used as a text object */
	UINT8	ui1_rgn_depth;
	UINT16 ui2_rgn_width;
	UINT16 ui2_rgn_height;
	UINT8* pui1_rgn;

	UINT16 ui2_obj_width;		 /* Output */
	UINT16 ui2_obj_height; 	 /* Output */
};
 
class DvbPage;
class DvbClutMgr;
class DvbObject;
class DvbObjectMgr;

class DvbRegion
{
public:
	DvbRegion();
	~DvbRegion();

	UINT8 getId(){return ui1_id;};
	
	static INT32 create(
	    DvbPage* 			pt_page_inst,
	    UINT8*              pui1_data, 
	    UINT32              ui4_data_len, 
	    DvbRegion**     	ppt_this);

	VOID clear();


	INT32 parseSegment(
	    DvbPage* 			pt_page_inst,
	    UINT8*              pui1_data, 
	    UINT32              ui4_data_len);

	VOID updateClut(UINT8               ui1_CLUT_id);

	VOID updateObj(DvbObject*      pt_obj);

	//VOID sm_sbtl_rgn_paint(
	//    SM_SBTL_RGN_T*      pt_this,
	//    SM_SBTL_OBJ_MNGR_T* pt_obj_mngr);

	INT32 linkSub(
	    DvbClutMgr*            		pt_CLUT_mngr,
	    DVB_PAGE_INST_RGN_LST_T*    pt_page_inst_rgn_lst);

	INT32 paintObj(DvbObjectMgr&	objMgr);
	
 private:
	 UINT8						 ui1_id;
 
	 UINT8						 ui1_version_number;
 
	 UINT8						 ui1_fill_flag;
 
	 UINT8						 ui1_CLUT_id;
 
	 UINT16 					 ui2_width;
 
	 UINT16 					 ui2_height;
 
	 UINT32 					 ui4_rgn_buf_len;
 
	 UINT8* 					 pui1_rgn_buf;
 
	 UINT16 					 ui2_mdf_left;
 
	 UINT16 					 ui2_mdf_top;
 
	 UINT16 					 ui2_mdf_right;
 
	 UINT16 					 ui2_mdf_bottom;
 
	 DVB_RGN_OBJ_LST_T* 		 pat_obj_list;
	 
	 UINT16 					 ui2_obj_list_cnt;
	 
	 UINT8						 ui1_depth;
 
	 UINT8						 ui1_bg_color_code;
	 
	 bool						 is_render;

	 INT32 parseSegment_impl(
		    DvbPage* 			pt_page_inst,
		    UINT8*              pui1_data, 
		    UINT32              ui4_data_len,
		    bool                is_new_rgn);
};

#endif

