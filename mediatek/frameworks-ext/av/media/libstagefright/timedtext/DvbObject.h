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

#ifndef __DVB_OBJECT_H_
#define __DVB_OBJECT_H_

/*-----------------------------------------------------------------------------
				  include files
----------------------------------------------------------------------------*/

#include "DVBDataType.h"

/*-----------------------------------------------------------------------------
				  macros, defines, typedefs, enums
----------------------------------------------------------------------------*/
	/* Object Data Segment */
#define DVB_SBTL_ODS_MIN_LEN                                         ((UINT32) (4))
#define DVB_SBTL_ODS_PIXEL_MIN_LEN                                   ((UINT32) (7))
#define DVB_SBTL_ODS_CHAR_MIN_LEN                                    ((UINT32) (4))
	
#define GET_UINT16(pui1_data, ui2_result) \
		ui2_result = (UINT16) (((UINT8 *) (pui1_data))[0]);  \
		ui2_result = ui2_result << 8;					\
		ui2_result += (UINT16) (((UINT8 *) (pui1_data))[1]);
	
	
#define DVB_SBTL_ODS_GET_OBJ_ID(pui1_data, ui2_obj_id) \
		GET_UINT16(pui1_data, ui2_obj_id)
	
#define DVB_SBTL_ODS_GET_VERSION(pui1_data, ui1_version) \
		ui1_version = ( ((UINT8*) (pui1_data))[2] & 0xF0) >> 4
	
#define DVB_SBTL_ODS_GET_CODING_METHOD(pui1_data, ui1_coding_method) \
		ui1_coding_method = ( ((UINT8*) (pui1_data))[2] & 0x08) >> 2
	
#define DVB_SBTL_ODS_GET_NON_MODIFYING_COLOUR_FLAG(pui1_data, ui1_non_modifying_colour_flag) \
		ui1_non_modifying_colour_flag = (((UINT8*) (pui1_data))[2] & 0x02) >> 1
		
#define DVB_SBTL_ODS_GET_TOP_FIELD_DATA_BLOCK_LEN(pui1_data, ui2_top_field_data_block_len) \
		GET_UINT16( (pui1_data + 3),  ui2_top_field_data_block_len)
	
#define DVB_SBTL_ODS_GET_BOTTOM_FIELD_DATA_BLOCK_LEN(pui1_data, ui2_bottom_field_data_block_len) \
		GET_UINT16( (pui1_data + 5),  ui2_bottom_field_data_block_len)
	
#define DVB_SBTL_ODS_GET_NUM_OF_CODES(pui1_data, ui1_num_of_codes) \
		ui1_num_of_codes = ((UINT8*)(pui1_data))[3]
	

/* Object Relative */
#define DVB_SBTL_OBJ_CODING_METHOD_PIXEL    ((UINT8) 0)
#define DVB_SBTL_OBJ_CODING_METHOD_STRING   ((UINT8) 1)


/*-----------------------------------------------------------------------------
				  data declarations, extern, static, const
----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
				  class declarations
----------------------------------------------------------------------------*/
class DvbObject
{
public:
	DvbObject();
	~DvbObject();

	UINT16	getId(){return ui2_id;};
	
	static INT32 create(UINT8*              pui1_data, 
                        UINT32              ui4_data_len, 
                        DvbObject**     ppt_this);

	INT32 parseSegment(
			    UINT8*              pui1_data, 
			    UINT32              ui4_data_len);


	INT32 render(DVB_RENDER_INFO_T* pt_sbtl_render_info);
	
private:
	UINT16	ui2_id;
	
	UINT16	ui2_data_len;
	
	UINT8*	pui1_data; /* It's UINT8* for pixel object. 
						  It's UINT16* for string object */

	UINT8	ui1_version_number;

	UINT8	ui1_coding_method;

	UINT8	ui1_non_modifying_colour_flag;

	INT32 parseSegment_impl(
                 UINT8*              pui1_data, 
                 UINT32              ui4_data_len,
                 bool                is_new_obj);

	INT32 render(
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
			    UINT16*     pui2_obj_height);
	
} ;


#endif

