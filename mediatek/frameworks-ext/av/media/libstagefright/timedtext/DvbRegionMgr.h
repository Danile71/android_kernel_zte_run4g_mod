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

#ifndef __DVB_REGION_MGR_H_
#define __DVB_REGION_MGR_H_

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
class DvbRegionMgr
{
public:
	DvbRegionMgr();
	~DvbRegionMgr();
	
	VOID freeAll();
	
	INT32 parseSegment(
	    DvbPage*			pt_page_inst,
	    UINT8*              pui1_data, 
	    UINT32              ui4_data_len);

	VOID updateClut(
    	UINT8               ui1_CLUT_id);

	VOID updateObj(
	    DvbObject*      pt_obj);

	INT32 linkSub(
	    UINT8                           ui1_rgn_id, 
	    DvbClutMgr*			            pt_CLUT_mngr,
	    DVB_PAGE_INST_RGN_LST_T*    	pt_page_inst_rgn_lst);
	
    INT32 paintAllRegion(
		DvbObjectMgr&	objMgr);
private:
	DvbRegion**    apt_rgn_lst;
    
    //struct SM_SBTL_SERVICE* pt_service;

    UINT8                   ui1_rgn_lst_size;

    UINT8                   ui1_rgn_lst_cnt;

	
	DvbRegion* getRegion(
		UINT8				ui1_rid);
	
	INT32 insertRegion(
		DvbRegion*		pt_rgn);
} ;


#endif

