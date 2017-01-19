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

#ifndef __DVB_OBJECT_MGR_H_
#define __DVB_OBJECT_MGR_H_

/*-----------------------------------------------------------------------------
				  include files
----------------------------------------------------------------------------*/
#include <stdlib.h>

#include "DVBDataType.h"
#include "DvbRegionMgr.h"
#include "DvbObject.h"

/*-----------------------------------------------------------------------------
				  macros, defines, typedefs, enums
----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
				  data declarations, extern, static, const
----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
				  class declarations
----------------------------------------------------------------------------*/
class DvbObjectMgr
{
public:
	DvbObjectMgr();
	~DvbObjectMgr();

	VOID	freeAll();
	
	INT32 parseSegment(
	    DvbRegionMgr&		t_rgn_mngr, 
	    UINT8*              pui1_data, 
	    UINT32              ui4_data_len);

	INT32 renderObj(
		UINT16                  ui2_obj_id,
    	DVB_RENDER_INFO_T* pt_sbtl_render_info);

	INT32 renderRomObj(
	    UINT16                      ui2_obj_id, 
	    UINT8                       ui1_obj_type,
	    DVB_RENDER_INFO_T*     pt_sbtl_render_info);
	
private:
	UINT16					ui2_obj_lst_size;
	
	UINT16					ui2_obj_lst_cnt;

	//struct SM_SBTL_SERVICE* pt_service;

	DvbObject**			apt_obj_lst;


	DvbObject*			getObject(UINT16              ui2_obj_id);
	INT32				insertObject(DvbObject*			 pt_obj);
} ;


#endif

