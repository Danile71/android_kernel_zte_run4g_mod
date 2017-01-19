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
#define LOG_TAG "DVBClutMgr"
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

#ifdef MTK_SUBTITLE_SUPPORT

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/
#define _DVB_SBTL_CLUT_LST_SIZE   8


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
DvbClutMgr::DvbClutMgr()
{
    this->ui1_CLUT_lst_size = _DVB_SBTL_CLUT_LST_SIZE;
    this->ui1_CLUT_lst_cnt = 0;
    this->apt_CLUT_lst = (DvbClut**) malloc(_DVB_SBTL_CLUT_LST_SIZE*sizeof(DvbClut*));

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
DvbClutMgr::~DvbClutMgr()
{
    
    if (NULL != this->apt_CLUT_lst)
    {
        freeAll();
        free(this->apt_CLUT_lst);
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
VOID DvbClutMgr::freeAll()
{
    INT32 i;
    DvbClut	*pt_clut;
	DVB_LOG("DvbClutMgr::freeAll()",0);
    if (NULL == this->apt_CLUT_lst)
    {
        return;
    }

    for (i = 0; i < this->ui1_CLUT_lst_cnt; i++)
    {
        pt_clut = this->apt_CLUT_lst[i];
        if (pt_clut != NULL)
        {
            delete pt_clut;
        }
    }

    this->ui1_CLUT_lst_cnt = 0;
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
INT32 DvbClutMgr::parseSegment(
    DvbRegionMgr&    		pt_rgn_mngr, 
    UINT8*                  pui1_data, 
    UINT32                  ui4_data_len)
{
    INT32 i4_ret;
    DvbClut *pt_CLUT;
    UINT8 ui1_CLUT_id;

    if (DVB_SBTL_CLUTDS_MIN_LEN > ui4_data_len)
    {
        return (DVBR_WRONG_SEGMENT_DATA);
    }

    DVB_SBTL_CLUTDS_GET_CLUT_ID(pui1_data, ui1_CLUT_id);

    pt_CLUT = getClut(ui1_CLUT_id);

    if (NULL == pt_CLUT)
    {
        /* Create a new object and insert into list */
        i4_ret = DvbClut::create(pui1_data,
	                           ui4_data_len,
	                           &pt_CLUT);

        if (DVBR_OK != i4_ret)
        {
            return (i4_ret);
        }
        
        i4_ret = insertClut(pt_CLUT);

        if (DVBR_OK != i4_ret)
        {
            delete pt_CLUT;
        }
        else
        {
            //sm_sbtl_rgn_mngr_update_clut(pt_rgn_mngr, ui1_CLUT_id);
        }
    }
    else
    {
        /* Parse the segment */
        i4_ret = pt_CLUT->parseSegment(pui1_data,
                                       ui4_data_len);

        if (DVBR_CONTENT_UPDATE == i4_ret)
        {
            //sm_sbtl_rgn_mngr_update_clut(pt_rgn_mngr, ui1_CLUT_id);
            i4_ret = DVBR_OK;
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
DVB_NBIT_CLUT_T* DvbClutMgr::Link_sub(
    UINT8                   ui1_CLUT_id,
    UINT8                   ui1_color_depth)
{
    DvbClut*         pt_CLUT;

    pt_CLUT = getClut(ui1_CLUT_id);

    if (NULL == pt_CLUT)
    {
        return ((DVB_NBIT_CLUT_T *) DvbClut::getDefaultClut(ui1_color_depth));
    }

    return (pt_CLUT->linkSub(ui1_color_depth));
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
 DvbClut* DvbClutMgr::getClut(
    UINT8                ui1_CLUT_id)
{
    INT32           i;
    DvbClut* pt_CLUT;

    for (i = 0; i < this->ui1_CLUT_lst_cnt; i++)
    {
        pt_CLUT = this->apt_CLUT_lst[i];
        
        //if (pt_CLUT->ui1_id == ui1_CLUT_id)
        if (pt_CLUT->getId() == ui1_CLUT_id)
        {
            return (pt_CLUT);
        }
    }
    
    return (NULL);
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
INT32 DvbClutMgr::insertClut(
    DvbClut*      pt_CLUT)
{
    DvbClut**    apt_CLUT_lst;

	DVB_LOG("DvbClutMgr::insertClut() clut id = %d",pt_CLUT->getId());
	
    /* Buffer Full - Resize */
    if (this->ui1_CLUT_lst_cnt == this->ui1_CLUT_lst_size)
    {
        apt_CLUT_lst = (DvbClut**) malloc(
                        (this->ui1_CLUT_lst_size + _DVB_SBTL_CLUT_LST_SIZE)*sizeof(DvbClut*));

        if (NULL == apt_CLUT_lst)
        {
            return (DVBR_INSUFFICIENT_MEMORY);
        }

        /* copy memory */
        memcpy(apt_CLUT_lst, 
                 this->apt_CLUT_lst, 
                 sizeof(DvbClut*) * this->ui1_CLUT_lst_cnt);

        free(this->apt_CLUT_lst);
        this->apt_CLUT_lst = apt_CLUT_lst;
        this->ui1_CLUT_lst_size += _DVB_SBTL_CLUT_LST_SIZE;
    }

    this->apt_CLUT_lst[this->ui1_CLUT_lst_cnt] = pt_CLUT;
    this->ui1_CLUT_lst_cnt++;

    return (DVBR_OK);
}
#endif

