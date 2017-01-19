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


#ifndef __DVB_PAGE_MGR_H_
#define __DVB_PAGE_MGR_H_
 
 /*-----------------------------------------------------------------------------
					 include files
  ----------------------------------------------------------------------------*/
 
 #include "DVBDataType.h"
 
 /*-----------------------------------------------------------------------------
					 macros, defines, typedefs, enums
  ----------------------------------------------------------------------------*/
 
 
 /*-----------------------------------------------------------------------------
					 data declarations, extern, static, const
  ----------------------------------------------------------------------------*/
 
 /*-----------------------------------------------------------------------------
					 class declarations
  ----------------------------------------------------------------------------*/

 class DvbPageMgr
 {
 public:
 		DvbPageMgr();

		~DvbPageMgr();

	INT32 parseSegment(
	    //PTS_T                       ui8_PTS, 
	    UINT8*                      pui1_data, 
	    UINT16                      ui2_data_len, 
	    DvbPage**       			ppt_page_inst);

	INT32 linkPageInst(
	    DvbPage*        pt_page_inst, 
	    DvbRegionMgr*       pt_rgn_mngr, 
	    DvbClutMgr*        pt_CLUT_mngr);

	INT32 showPageInst(DvbPage*        pt_show_page_inst);

	//INT32 LockPageInst(DvbPage*        pt_page_inst);

	//INT32 unlockPageInst(DvbPage*        pt_page_inst);

	//VOID pageTimeout(DvbPage*        pt_page_inst); 

	INT32 freeAll();

	VOID forceFreeAll();
 private:
	 //UINT32								 ui4_timeout_timer;
 
	 //SM_SBTL_PAGE_INST_MNGR_TIMEOUT_TAG_T	 t_timeout_tag;
	 
	 //struct SM_SBTL_SERVICE*				 pt_service;
 
	 DvbPage*				 	pt_first_show_page_inst;
 
	 DvbPage*				 pt_last_show_page_inst;
 
	 UINT8					ui1_show_page_inst_cnt;
 };


#endif

