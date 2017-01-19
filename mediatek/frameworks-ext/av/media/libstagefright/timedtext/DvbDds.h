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

#ifndef __DVB_DDS_H_
#define __DVB_DDS_H_

 /*-----------------------------------------------------------------------------
					  include files
   ----------------------------------------------------------------------------*/
  
 #include "DVBDataType.h"
  
  /*-----------------------------------------------------------------------------
					  macros, defines, typedefs, enums
   ----------------------------------------------------------------------------*/
  #define DVB_SBTL_DDS_MIN_LEN             ((UINT32)(5))
  
  /*-----------------------------------------------------------------------------
					  data declarations, extern, static, const
   ----------------------------------------------------------------------------*/
  
  /*-----------------------------------------------------------------------------
					  class declarations
   ----------------------------------------------------------------------------*/

class DvbDds
{
public:
	DvbDds(UINT16              ui2_pid);

	~DvbDds();

    static INT32 create(UINT8*              pui1_data, 
	                        UINT32              ui4_data_len, 
	                        UINT16              ui2_page_id,
	                        DvbDds**     ppt_this);
    
	INT32 judgeBoundary(
	    UINT16              ui24_width,
	    UINT16              ui2_height);

	INT32 adjustAddressbyBoundary(
	    UINT16*             pui2_width,
	    UINT16*             pui2_height);

	INT32 parseSegment(
	    UINT8*              pui1_data,
	    UINT32              ui4_data_len);

private:
    UINT16  ui2_same_dds_cnt;
    UINT16  ui2_page_id;

    UINT8   ui1_version_number;

    UINT8   ui1_display_window_flag;

    UINT16  ui2_display_width;
    UINT16  ui2_display_height;

    UINT16  ui2_display_window_hor_pos_min;
    UINT16  ui2_display_window_hor_pos_max;
    UINT16  ui2_display_window_ver_pos_min;
    UINT16  ui2_display_window_ver_pos_max;

	 INT32 parseSegment_impl(
	 UINT8* 			 pui1_data, 
	 UINT32 			 ui4_data_len,
	 bool				 is_new_rgn);
	
	static const UINT32 u4_dds_same_id_count;      /* Used to count the same dds, and relate to update_logi_size */
    friend class    DvbDdsMgr ;
} ;
#endif

