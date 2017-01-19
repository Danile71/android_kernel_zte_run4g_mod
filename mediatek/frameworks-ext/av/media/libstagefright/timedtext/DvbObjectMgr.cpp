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
#define LOG_TAG "DVBObjectMgr"

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
#define _DVB_SBTL_OBJ_LST_SIZE   16

/*-----------------------------------------------------------------------------
                    data declarations, extern, static, const
 ----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                    functions declarations
 ----------------------------------------------------------------------------*/

DvbObjectMgr::DvbObjectMgr()
{
	this->ui2_obj_lst_size = _DVB_SBTL_OBJ_LST_SIZE;
    this->ui2_obj_lst_cnt = 0;
    this->apt_obj_lst = (DvbObject**) malloc(_DVB_SBTL_OBJ_LST_SIZE*sizeof(DvbObject*));
}


DvbObjectMgr::~DvbObjectMgr()
{
	if (NULL != this->apt_obj_lst)
    {
        //sm_sbtl_obj_mngr_free_all(pt_this);
        //sm_sbtl_service_mem_free(pt_this->pt_service, (VOID*) pt_this->apt_obj_lst);
        free(this->apt_obj_lst);
    }
}

VOID DvbObjectMgr::freeAll()
{
	INT32 i;
    DvbObject *pt_obj;
	DVB_LOG("DvbObjectMgr::freeAll()",0);
    if (NULL == this->apt_obj_lst)
    {
        return;
    }

    for (i = 0; i < (INT32)(this->ui2_obj_lst_cnt); i++)
    {
        pt_obj = this->apt_obj_lst[i];
        if (pt_obj != NULL)
        {
            free(pt_obj);
        }
    }

    this->ui2_obj_lst_cnt = 0;
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
INT32 DvbObjectMgr::parseSegment(
	    DvbRegionMgr&		t_rgn_mngr, 
	    UINT8*              pui1_data, 
	    UINT32              ui4_data_len)
{
    INT32 		i4_ret;
    DvbObject	*pt_obj;
    UINT16 		ui2_obj_id;
    
    if (DVB_SBTL_ODS_MIN_LEN > ui4_data_len)
    {
        return (DVBR_WRONG_SEGMENT_DATA);
    }

    DVB_SBTL_ODS_GET_OBJ_ID(pui1_data, ui2_obj_id);

    pt_obj = getObject(ui2_obj_id);

    if (NULL == pt_obj)
    {
        /* Create a new object and insert into list */
        i4_ret = DvbObject::create(pui1_data,
                           ui4_data_len,
                           &pt_obj);

        if (DVBR_OK != i4_ret)
        {
            return (i4_ret);
        }
        
        i4_ret = insertObject(pt_obj);

        if (DVBR_OK != i4_ret)
        {
            //sm_sbtl_obj_delete(pt_obj, pt_this->pt_service);
            delete pt_obj;
        }

#ifdef  DVB_SBTL_CRS_PAGE_OBJ_SUPPORT
        else
        {
            sm_sbtl_rgn_mngr_update_obj(pt_rgn_mngr, pt_obj);
        }
#endif
    }
    else
    {
        /* Parse the segment */
        i4_ret = pt_obj->parseSegment(pui1_data,ui4_data_len);

#ifdef  DVB_SBTL_CRS_PAGE_OBJ_SUPPORT
        if (i4_ret == DVBR_CONTENT_UPDATE)
        {
            sm_sbtl_rgn_mngr_update_obj(pt_rgn_mngr, pt_obj);
        }
#endif
    }

    return (i4_ret);
}


DvbObject*	DvbObjectMgr::getObject(UINT16    ui2_obj_id)

{
    DvbObject*  pt_obj = NULL;
    INT32           i;

    for (i = 0; i < (INT32)(this->ui2_obj_lst_cnt); i++)
    {
        pt_obj = this->apt_obj_lst[i];
        
        //if (pt_obj->ui2_id == ui2_obj_id)
        if (pt_obj->getId() == ui2_obj_id)
        {
            return (pt_obj);
        }
    }
    
    return (NULL);
}

INT32	DvbObjectMgr::insertObject(DvbObject*			 pt_obj)

{
    DvbObject** apt_obj_lst;
	DVB_LOG("DvbObjectMgr::insertObject() obj id=%d",pt_obj->getId());
    /* Buffer Full - Resize */
    if (this->ui2_obj_lst_cnt == this->ui2_obj_lst_size)
    {
        apt_obj_lst = (DvbObject**)malloc((this->ui2_obj_lst_size + _DVB_SBTL_OBJ_LST_SIZE)*sizeof(DvbObject*));

        if (NULL == apt_obj_lst)
        {
            return (DVBR_INSUFFICIENT_MEMORY);
        }

        /* copy memory */
        memcpy(apt_obj_lst, 
                 this->apt_obj_lst, 
                 sizeof(DvbObject*) * this->ui2_obj_lst_size);

        free(this->apt_obj_lst);
        this->apt_obj_lst = apt_obj_lst;
        this->ui2_obj_lst_size += _DVB_SBTL_OBJ_LST_SIZE;
    }

    this->apt_obj_lst[this->ui2_obj_lst_cnt] = pt_obj;
    this->ui2_obj_lst_cnt++;

    return (DVBR_OK);
}

INT32 DvbObjectMgr::renderObj(
		UINT16                  ui2_obj_id,
    	DVB_RENDER_INFO_T* pt_sbtl_render_info)
{
	INT32 i4_ret = DVBR_NOT_FOUND;
	DvbObject* pt_obj;

	pt_obj = getObject(ui2_obj_id);

	if (NULL == pt_obj)
	{
		DVB_LOG("DvbObjectMgr::renderObj() can not get obj id=%d",ui2_obj_id);
		return (i4_ret);
	}

	i4_ret = pt_obj->render(pt_sbtl_render_info);
	return (i4_ret);
}

INT32 DvbObjectMgr::renderRomObj(
	    UINT16                      ui2_obj_id, 
	    UINT8                       ui1_obj_type,
	    DVB_RENDER_INFO_T*     pt_sbtl_render_info)
{
	DVB_LOG("DvbObjectMgr::renderObj() render rom object id=%d",ui2_obj_id);
	return DVBR_OK;
}

#endif

