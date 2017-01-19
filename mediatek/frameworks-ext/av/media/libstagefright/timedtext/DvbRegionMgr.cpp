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
#define LOG_TAG "DVBRegionMgr"

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
#define _DVB_SBTL_RGN_LST_SIZE 8

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
 * Inputs:  pt_this     Pointer points to the 
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
DvbRegionMgr::DvbRegionMgr()
{

    this->ui1_rgn_lst_size = _DVB_SBTL_RGN_LST_SIZE;
    this->ui1_rgn_lst_cnt = 0;
    this->apt_rgn_lst = (DvbRegion**)malloc(_DVB_SBTL_RGN_LST_SIZE*sizeof(DvbRegion*));
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
DvbRegionMgr::~DvbRegionMgr()
{   
    if (NULL != this->apt_rgn_lst)
    {
        freeAll();
        free(this->apt_rgn_lst);
    }
    
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
VOID DvbRegionMgr::freeAll()

{
    INT32 i;
    DvbRegion *pt_rgn;
	DVB_LOG("DvbRegionMgr::freeAll()",0);
    if (NULL == this->apt_rgn_lst)
    {
        return;
    }

    for (i = 0; i < this->ui1_rgn_lst_cnt; i++)
    {
        pt_rgn = this->apt_rgn_lst[i];
        if (pt_rgn != NULL)
        {
            delete pt_rgn;
        }
    }

    this->ui1_rgn_lst_cnt = 0;
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
INT32 DvbRegionMgr::parseSegment(
		DvbPage*			pt_page_inst,
		UINT8*				pui1_data, 
		UINT32				ui4_data_len)

{
    INT32 i4_ret;
    DvbRegion* pt_rgn;
    UINT8 ui1_rgn_id;
    
    if (DVB_SBTL_RCS_MIN_LEN > ui4_data_len)
    {
        return (DVBR_WRONG_SEGMENT_DATA);
    }
    
    DVB_SBTL_RCS_GET_RGN_ID(pui1_data, ui1_rgn_id);

    pt_rgn = getRegion(ui1_rgn_id);

    if (NULL == pt_rgn)
    {
        /* Create a new object and insert into list */
        i4_ret = DvbRegion::create(
                                    pt_page_inst,
                                    pui1_data,
                                    ui4_data_len,
                                    &pt_rgn);

        if (DVBR_OK != i4_ret)
        {
            return (i4_ret);
        }
        
        i4_ret = insertRegion(pt_rgn);

        if (DVBR_OK != i4_ret)
        {
            delete pt_rgn;
        }
    }
    else
    {
        /* Parse the segment */
        i4_ret = pt_rgn->parseSegment(pt_page_inst,
                                       pui1_data,
                                       ui4_data_len);
        if (DVBR_OK != i4_ret)
        {
            ALOGE("ERROR: DvbRegionMgr::parseSegment fail\n");
        }
    }

    return (i4_ret);
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
VOID DvbRegionMgr::updateClut(
		UINT8				ui1_CLUT_id)

{
    INT32 i;
    DvbRegion *pt_rgn;

    for (i = 0; i < this->ui1_rgn_lst_cnt; i++)
    {
        pt_rgn = this->apt_rgn_lst[i];
        if (pt_rgn != NULL)
        {
            pt_rgn->updateClut(ui1_CLUT_id);
        }
    }
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
VOID DvbRegionMgr::updateObj(
		DvbObject*		pt_obj)

{
    INT32 i;
    DvbRegion *pt_rgn;

    for (i = 0; i < this->ui1_rgn_lst_cnt; i++)
    {
        pt_rgn = this->apt_rgn_lst[i];
        if (pt_rgn != NULL)
        {
            pt_rgn->updateObj(pt_obj);
        }
    }
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
INT32 DvbRegionMgr::linkSub(
    UINT8                           ui1_rgn_id, 
    DvbClutMgr*			            pt_CLUT_mngr,
    DVB_PAGE_INST_RGN_LST_T*    	pt_page_inst_rgn_lst)
{
    INT32           i4_ret;
    DvbRegion*  	pt_rgn;

    pt_rgn = getRegion(ui1_rgn_id);

    if (NULL != pt_rgn)
    {
        i4_ret = pt_rgn->linkSub(pt_CLUT_mngr,
                                  pt_page_inst_rgn_lst);

        if (DVBR_OK == i4_ret)
        {
            pt_page_inst_rgn_lst->is_exist = TRUE;
        }
    }
    else
    {
        i4_ret = DVBR_NOT_FOUND;
    }

    return (i4_ret);
}


DvbRegion* DvbRegionMgr::getRegion(
		UINT8				ui1_rid)
{
	INT32			i;
	DvbRegion*	pt_rgn;

	for (i = 0; i < this->ui1_rgn_lst_cnt; i++)
	{
		pt_rgn = this->apt_rgn_lst[i];
		
		//if (pt_rgn->ui1_id == ui1_rid)
		if (pt_rgn->getId() == ui1_rid)
		{
			return (pt_rgn);
		}
	}
			
	return (NULL);
}


INT32 DvbRegionMgr::insertRegion(
		DvbRegion*		pt_rgn)
{
	DvbRegion** 	apt_rgn_lst;
	DVB_LOG("DvbRegionMgr::insertRegion() region id=",pt_rgn->getId());
	/* Buffer Full - Resize */
	if (this->ui1_rgn_lst_cnt == this->ui1_rgn_lst_size)
	{
		apt_rgn_lst = (DvbRegion**) malloc(
						(this->ui1_rgn_lst_size + _DVB_SBTL_RGN_LST_SIZE)*sizeof(DvbRegion*));

		if (NULL == apt_rgn_lst)
		{
			return (DVBR_INSUFFICIENT_MEMORY);
		}

		/* copy memory */
		memcpy(apt_rgn_lst, 
				 this->apt_rgn_lst, 
				 sizeof(DvbRegion*) * this->ui1_rgn_lst_cnt);

		free(this->apt_rgn_lst);
		this->apt_rgn_lst = apt_rgn_lst;
		this->ui1_rgn_lst_size += _DVB_SBTL_RGN_LST_SIZE;
	}

	this->apt_rgn_lst[this->ui1_rgn_lst_cnt] = pt_rgn;
	this->ui1_rgn_lst_cnt++;
	
	return (DVBR_OK);
}

INT32 DvbRegionMgr::paintAllRegion(
		DvbObjectMgr&	objMgr)
{
	INT32 i;
    INT32		i4_ret;
    DvbRegion *pt_rgn;
	DVB_LOG("DvbRegionMgr::paintAllRegion()",0);
    for (i = 0; i < this->ui1_rgn_lst_cnt; i++)
    {
        pt_rgn = this->apt_rgn_lst[i];
        if (pt_rgn != NULL)
        {
            i4_ret = pt_rgn->paintObj(objMgr);
            if (DVBR_OK !=  i4_ret)
            {
                ALOGI("ERROR: DvbRegionMgr::paintAllRegion failed ret=%d\n", i4_ret);
                return i4_ret;
            }
        }
    }
    
    return DVBR_OK;
}

#endif

