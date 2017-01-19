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
#define LOG_TAG "DVBPageMgr"

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


/*-----------------------------------------------------------------------------
				 data declarations, extern, static, const
----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
                    functions declarations
 ----------------------------------------------------------------------------*/
 
 /*-----------------------------------------------------------------------------
  * Name: 
  *
  * Description: 
  *
  * Inputs:  pt_this	 Pointer points to the 
  *
  * Outputs: -
  *
  * Returns: -
  ----------------------------------------------------------------------------*/
DvbPageMgr::DvbPageMgr()
{

	pt_first_show_page_inst = NULL;
	pt_last_show_page_inst = NULL;

	ui1_show_page_inst_cnt = 0;

}

 /*-----------------------------------------------------------------------------
  * Name: 
  *
  * Description: 
  *
  * Inputs:  pt_this	 Pointer points to the 
  *
  * Outputs: -
  *
  * Returns: -
  ----------------------------------------------------------------------------*/
 DvbPageMgr::~DvbPageMgr()
 {
	 
	 ALOGE("\n[SBTL]DvbPageMgr::~DvbPageMgr()\n");
 }


 /*-----------------------------------------------------------------------------
  * Name: 
  *
  * Description: 
  *
  * Inputs:  pt_this	 Pointer points to the 
  *
  * Outputs: -
  *
  * Returns: -
  ----------------------------------------------------------------------------*/
 INT32 DvbPageMgr::parseSegment(
	    //PTS_T                       ui8_PTS, 
	    UINT8*                      pui1_data, 
	    UINT16                      ui2_data_len, 
	    DvbPage**       			ppt_page_inst)
 {
	 INT32					 i4_ret;
	 DvbPage*				 pt_orig_page_inst;
 
	 if (0 != ui2_data_len)
	 {
		 i4_ret = DvbPage::create(
										   //ui8_PTS,
										   pui1_data,
										   ui2_data_len,
										   ppt_page_inst);
		 
		 if (DVBR_OK != i4_ret)
		 {
			 return (i4_ret);
		 }
	 }
	 else
	 {
		 pt_orig_page_inst = NULL;
		 
		 if (2 == this->ui1_show_page_inst_cnt)
		 {
			 pt_orig_page_inst = this->pt_last_show_page_inst;
		 }
		 
		 if (1 == this->ui1_show_page_inst_cnt)
		 {
			 pt_orig_page_inst = this->pt_first_show_page_inst;
		 }
 
		 if (NULL == pt_orig_page_inst)
		 {
			 return (DVBR_OK);
		 }
 
		 i4_ret = DvbPage::dup(
								   //ui8_PTS, 
								   pt_orig_page_inst,
								   ppt_page_inst);
 
		 if (DVBR_OK != i4_ret)
		 {
			 return (i4_ret);
		 }
	 }
 
	 return (DVBR_OK);
 }

 /*-----------------------------------------------------------------------------
  * Name: 
  *
  * Description: 
  *
  * Inputs:  pt_this	 Pointer points to the 
  *
  * Outputs: -
  *
  * Returns: -
  ----------------------------------------------------------------------------*/
 INT32 DvbPageMgr::linkPageInst(
		 DvbPage*		 pt_page_inst, 
		 DvbRegionMgr*       pt_rgn_mngr, 
		 DvbClutMgr*		pt_CLUT_mngr)

 {
	 INT32 i4_ret = DVBR_OK;
 
	 i4_ret = pt_page_inst->linkSub(pt_rgn_mngr,
								 pt_CLUT_mngr);
 
	 if (DVBR_OK != i4_ret)
	 {
		 return (i4_ret);
	 }
 
	 return (i4_ret);
 }
 
 /*-----------------------------------------------------------------------------
  * Name: 
  *
  * Description: 
  *
  * Inputs:  pt_this	 Pointer points to the 
  *
  * Outputs: -
  *
  * Returns: -
  ----------------------------------------------------------------------------*/
 INT32 DvbPageMgr::showPageInst(DvbPage*        pt_show_page_inst)
 {

	 INT32					 i4_ret = 0;
  #if 0
	 if (1 < pt_this->ui1_show_page_inst_cnt)
	 {
		 /* Impossible, Error handle */
		 DBG_SBTL_LOG("ERROR: Show page as two more pages are showed\n");
		 return (DVBR_NOT_FOUND);
	 }
 
	 DBG_SBTL_LOG("Before Show page handle=%x page cnt=%d, page_inst_mngr:0x%x\n", 
			   pt_show_page_inst->h_handle,
			   pt_this->ui1_show_page_inst_cnt,
			   (UINT32)pt_this);
 
	 
	 if (0 == pt_this->ui1_show_page_inst_cnt)
	 {
		 pt_this->pt_first_show_page_inst = pt_show_page_inst;
	 }
	 else
	 {
		 pt_this->pt_last_show_page_inst = pt_show_page_inst;
	 }
 
	 pt_this->ui1_show_page_inst_cnt++;
 
	 i4_ret = sm_sbtl_page_inst_show(pt_show_page_inst);
 
	 if (DVBR_OK != i4_ret)
	 {
		 /* error handle */
	 }
 
	 /* Show page inst count is 2, suspend data queue */
	 if (2 == pt_this->ui1_show_page_inst_cnt)
	 {
		 _sm_sbtl_service_data_que_postpond_parse(pt_this->pt_service);
	 }
 
	 DBG_SBTL_LOG("After Show page handle=%x page cnt=%d, page_inst_mngr:0x%x\n", 
			   pt_show_page_inst->h_handle,
			   pt_this->ui1_show_page_inst_cnt,
			   (UINT32)pt_this);
#endif 
	 return (i4_ret);
 }


 INT32 DvbPageMgr::freeAll()
 {
	 INT32					 i4_ret = 0;
 
	 ALOGE("\n page inst mngr free all show page cnt=%d\n", this->ui1_show_page_inst_cnt);
 #if 0
	 if (0 < pt_this->ui1_show_page_inst_cnt)
	 {
		 if (1 == pt_this->ui1_show_page_inst_cnt)
		 {
			 i4_ret = sm_sbtl_service_wgl_clear_all(pt_this->pt_service, 
													pt_this->pt_first_show_page_inst->ui8_PTS);
		 }
		 else
		 {
			 i4_ret = sm_sbtl_service_wgl_clear_all(pt_this->pt_service, 
													pt_this->pt_last_show_page_inst->ui8_PTS);
		 }
 
		 if (DVBR_OK != i4_ret)
		 {
			 return (i4_ret);
		 }
	 }
	 else
	 {
		 _sm_sbtl_service_state_page_inst_free(pt_this->pt_service);
	 }
 #endif
	 return (DVBR_OK);
 }
 
 /*-----------------------------------------------------------------------------
  * Name: 
  *
  * Description: 
  *
  * Inputs:  pt_this	 Pointer points to the 
  *
  * Outputs: -
  *
  * Returns: -
  ----------------------------------------------------------------------------*/
 VOID DvbPageMgr::forceFreeAll()
 {
	 if (NULL != this->pt_first_show_page_inst)
	 {
		 delete (this->pt_first_show_page_inst);
	 }
	 
	 if (NULL != this->pt_last_show_page_inst)
	 {
		 delete (this->pt_last_show_page_inst);
	 }
	 
	 this->ui1_show_page_inst_cnt = 0;
	 this->pt_first_show_page_inst = NULL;
	 this->pt_last_show_page_inst = NULL;
}
#endif

