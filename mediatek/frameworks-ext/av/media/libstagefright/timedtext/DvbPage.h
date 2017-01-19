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

#ifndef __DVB_PAGE_H_
#define __DVB_PAGE_H_

/*-----------------------------------------------------------------------------
				  include files
----------------------------------------------------------------------------*/
  
#include "DVBDataType.h"
  
/*-----------------------------------------------------------------------------
				  macros, defines, typedefs, enums
----------------------------------------------------------------------------*/
#include <sys/time.h>
#define DVB_LOG(x, y) \
		do { struct timeval sysTime = {0};unsigned tmp = y; gettimeofday(&sysTime,NULL);	\
		ALOGI("[%d:%d]"x,sysTime.tv_sec,sysTime.tv_usec / 1000,tmp); } while (0)

		
#define DVB_SBTL_PCS_MIN_LEN                                     ((UINT32) (2))
#define DVB_SBTL_PCS_RGN_LST_LEN                                 ((UINT32) (6))
 
#define DVB_SBTL_PCS_GET_PAGE_TIMEOUT(pui1_data, ui1_page_timeout) \
	 ui1_page_timeout = (((UINT8*) (pui1_data))[0])
 
#define DVB_SBTL_PCS_GET_VERSION(pui1_data, ui1_version) \
	 ui1_version = (((UINT8*) (pui1_data))[1] & 0xF0) >> 4
 
#define DVB_SBTL_PCS_GET_STATE(pui1_data, ui1_state) \
	 ui1_state = (((UINT8*) (pui1_data))[1] & 0x0C) >> 2
 
#define DVB_SBTL_PCS_GET_RGN_ID(pui1_data, ui1_rid) \
	 ui1_rid = (((UINT8*) (pui1_data))[0])
 
#define DVB_SBTL_PCS_GET_RGN_HORIZONTAL_ADDRESS(pui1_data, ui2_rgn_horizontal_address) \
	 GET_UINT16(pui1_data + 2, ui2_rgn_horizontal_address)
 
#define DVB_SBTL_PCS_GET_RGN_VERTICAL_ADDRESS(pui1_data, ui2_rgn_vertical_address) \
	 GET_UINT16(pui1_data + 4, ui2_rgn_vertical_address)
  
  /*-----------------------------------------------------------------------------
					  data declarations, extern, static, const
   ----------------------------------------------------------------------------*/
  
  /*-----------------------------------------------------------------------------
					  class declarations
   ----------------------------------------------------------------------------*/
struct DVB_PAGE_INST_RGN_LST_T
{
	DVB_NBIT_CLUT_T*	  pt_nbit_CLUT;

	UINT8*					  pui1_rgn_buf;

	UINT32					ui4_rgn_lst_buf_len;

	UINT16					  ui2_mdf_left;

	UINT16					  ui2_mdf_top;

	UINT16					  ui2_mdf_right;

	UINT16					  ui2_mdf_bottom;

	UINT16					  ui2_horizontal_addr;

	UINT16					  ui2_vertical_addr;

	UINT16					  ui2_width;

	UINT16					  ui2_height;

	UINT8 					  ui1_rid;

	UINT8 					  ui1_depth;

	bool					 is_exist;

} ;

 enum DVB_COLOR_MODE_T
{
    DVB_COLOR_MODE_CLUT_2_BIT = 0,
    DVB_COLOR_MODE_CLUT_4_BIT,
    DVB_COLOR_MODE_CLUT_8_BIT
};   

 
 struct DVB_REGION_INFO
 {
	 UINT32 					 ui4_rid;
	 //bool 					   b_dirty;
	 INT32						 i4_left;
	 INT32						 i4_top;
	 UINT32 					 ui4_width;
	 UINT32 					 ui4_height;
	 DVB_COLOR_MODE_T			 e_clr_mode;
	 VOID						 *pv_bitmap;
	 UINT32						 ui4_bitmap_length;
	 GL_COLOR_T 				 *at_clut;
	 //GL_RECT_T				   t_rc_dirty;
 };    
 
  

class DvbRegionMgr;
class DvbClutMgr;

 class DvbPage
 {
public:
	enum DvbPageState{
			STATE_NORMAL = 0,
			STATE_ACQUISITION_POINT = 1,
			STATE_MODE_CHANGE = 2,
		};
	
	enum  DvbPageStatus{
		STATUS_INIT = 0,
		STATUS_LINK,
		STATUS_ENABLE_PTS,
		STATUS_SHOW,    
		STATUS_SHOWLOCK,   
		};

	static const UINT16	PageDefaultWidth;
	static const UINT16	PageDefaultHeight;
	
	DvbPage();
	~DvbPage();

	static INT32 create(
		    //PTS_T                   ui8_PTS, 
		    UINT8*                  pui1_data, 
		    UINT16                  ui2_data_len, 
		    DvbPage**   ppt_this);

	
	static INT32 dup(
			//PTS_T					ui8_PTS, 
			DvbPage*	pt_orig_page_inst, 
			DvbPage**	ppt_this);

	INT32 linkSub(
		    DvbRegionMgr*     pt_rgn_mngr, 
		    DvbClutMgr*	      pt_CLUT_mngr);

	DVB_PAGE_INST_RGN_LST_T* find_rgn_lst(
		    UINT8                   ui1_rid);

	
	UINT8 getState()  {return ui1_state;};

	INT32 getRegionInfo(
				UINT16                  ui2_rgn_idx,
    			DVB_REGION_INFO*      pt_rgn);
	
private:
	 DVB_PAGE_INST_RGN_LST_T*	 		pat_rgn_list;
 
	 
	 //PTS_T								 ui8_PTS;
 
	 UINT8								 ui1_time_out;
 
	 UINT8								 ui1_version_number;
 
	 UINT8								 ui1_state;
 
	 UINT8								 ui1_status;
 
	 INT8								 i1_lock_pib_counter;
 
	 INT8								 i1_unlock_pib_counter;
 
	 UINT8								 ui1_rgn_cnt;

	 INT32 parseSegment_impl(
		    UINT8*                  pui1_data, 
		    UINT32                  ui4_data_len,
		    bool                    is_new_page_inst);
 };

#endif

