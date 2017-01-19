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

#define LOG_TAG "DVBDDS"

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
#include "dvbparser.h"

#include "DvbDds.h"
#include "DVBDdsMgr.h"
#ifdef DVB_ENABLE_HD_SUBTITLE 

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/
#define _DVB_SBTL_DDS_LST_SIZE        2

/*-----------------------------------------------------------------------------
                    data declarations, extern, static, const
 ----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                    functions declarations
 ----------------------------------------------------------------------------*/

DvbDdsMgr::DvbDdsMgr()
{
	this->ui1_dds_lst_size = _DVB_SBTL_DDS_LST_SIZE;
	this->ui1_dds_lst_cnt = 0;
	this->apt_dds_lst = (DvbDds**)malloc(_DVB_SBTL_DDS_LST_SIZE*sizeof(DvbDds*));
	
}

DvbDdsMgr::~DvbDdsMgr()
{
	freeAll();
}


VOID DvbDdsMgr::freeAll()
{
	INT32			i;
	DvbDds*	pt_dds;
	DVB_LOG("DvbObjectMgr::freeAll()",0);	
	if (NULL==this)
	{
		return;
	}

	for (i=0; i<this->ui1_dds_lst_cnt; i++)
	{
		pt_dds = (DvbDds*)this->apt_dds_lst[i];
		
		if (NULL!=pt_dds)
		{
			//sm_sbtl_dds_delete(pt_dds, pt_this->pt_service);
			delete pt_dds;
		}
	}
	
	this->ui1_dds_lst_cnt = 0;
}


INT32 DvbDdsMgr::Judgeboundary(
    UINT16                  ui2_width,
    UINT16                  ui2_height)
{
    INT32           i4_ret;
#if 0
    DvbDds*    pt_dds;
#endif
    i4_ret = DVBR_OK;     
#if 0
    if (NULL!=pt_service->pt_dds_mngr)
    {
        /*retrieve the match-id dds*/
        pt_dds = _sm_sbtl_dds_mngr_get_dds(pt_service->pt_dds_mngr, 
                                           pt_service->t_base.t_scdb_rec.u.t_dvb_sbtl.ui2_comp_pg_id);
        
        if (NULL!=pt_dds)
        {
            i4_ret = pt_dds->Judgeboundary(ui2_width, ui2_height);
        }
        else
        {
            if ( (ui2_width > DvbPage::PageDefaultWidth)||
                 (ui2_height > DvbPage::PageDefaultHeight)) )
            {
                return (DVBR_WRONG_SEGMENT_DATA);
            }
        }
    }
    else
    {
        if ( (ui2_width > DvbPage::PageDefaultWidth)||
             (ui2_height > DvbPage::PageDefaultHeight) )
        {
            return (DVBR_WRONG_SEGMENT_DATA);
        }
    }
#endif
    return i4_ret;
}



INT32 DvbDdsMgr::adjust_address(
    UINT16*                 pui2_width,
    UINT16*                 pui2_height)
{
    INT32           i4_ret;    
#if 0
    DvbDds*  pt_dds;
#endif   
    i4_ret = DVBR_OK;     
#if 0
    if (NULL!=pt_service->pt_dds_mngr)
    {
        /*retrieve the match-id dds*/
        pt_dds = getDds(pt_service->t_base.t_scdb_rec.u.t_dvb_sbtl.ui2_comp_pg_id);
        
        if (NULL!=pt_dds)
        {
            i4_ret = pt_dds->adjust_address_by_boundary(pui2_width, pui2_height);
        }
        else
        {
            if ( (*pui2_width  > DvbPage::PageDefaultWidth))||
                 (*pui2_height >  DvbPage::PageDefaultHeight)) )
            {
                return (DVBR_WRONG_SEGMENT_DATA);
            }
        }
    }
    else
    {
        if ( (*pui2_width  > DvbPage::PageDefaultWidth)||
             (*pui2_height > DvbPage::PageDefaultHeight) )
        {
            return (DVBR_WRONG_SEGMENT_DATA);
        }
    }
#endif   

    return i4_ret;
}


INT32 DvbDdsMgr::parseSegment(
    UINT8*              pui1_data, 
    UINT32              ui4_data_len,
    UINT16              ui2_page_id)
{
    INT32            i4_ret;
    DvbDds*    pt_dds;
    
    if (DVB_SBTL_DDS_MIN_LEN > ui4_data_len)
    {
        return (DVBR_WRONG_SEGMENT_DATA);
    }
    
    /*retrieve the match-id dds*/
    pt_dds = getDds(ui2_page_id);
    
    if (NULL==pt_dds)
    {
        INT32       i4_ret_insert;
        
        /*create a new dds*/
        i4_ret = DvbDds::create(   pui1_data,
                                    ui4_data_len,
                                    ui2_page_id,
                                    &pt_dds);
        if (DVBR_OK>i4_ret)
        {
            return i4_ret;
        }
        
        /*insert to apt_dds_lst*/
        i4_ret_insert = insertDds(pt_dds);
        
        if (DVBR_OK>i4_ret_insert)
        {
            delete pt_dds;
            i4_ret = i4_ret_insert;
        }
    }
    else
    {
        /*reuse the previous one*/
        i4_ret = pt_dds->parseSegment(pui1_data,
                                      ui4_data_len);
    }

#ifdef CLI_LVL_ALL
    {
        DVB_SBTL_STRM_TYPE_T e_prev_type;
        UINT16              ui2_display_width;
        UINT16              ui2_display_height;
                
        e_prev_type         = t_sbtl_hdlr.e_sbtl_type;
        ui2_display_width   = t_sbtl_hdlr.ui2_display_width;
        ui2_display_height  = t_sbtl_hdlr.ui2_display_height; 
        
        t_sbtl_hdlr.e_sbtl_type = DVB_SBTL_STRM_HD;
        t_sbtl_hdlr.ui2_display_width = pt_dds->ui2_display_width;
        t_sbtl_hdlr.ui2_display_height = pt_dds->ui2_display_height;
  
        if ( (e_prev_type!=t_sbtl_hdlr.e_sbtl_type) ||
             (ui2_display_width!=t_sbtl_hdlr.ui2_display_width) ||
             (ui2_display_height!=t_sbtl_hdlr.ui2_display_height) )
        {
            DBG_SBTL_LOG("Update Subtitle Type\r\n");
            sm_sbtl_dump_sub_info();
        }   
    }
#endif /* CLI_LVL_ALL */
    
#if 0
    /* Update logical display region */
    if (DVBR_CONTENT_UPDATE==i4_ret)
    {
        /* Compare the assumed widht and height, if not equal need to update the logi_window to wgl_sbtl for scaling */
        i4_ret = sm_sbtl_service_update_logi_size(pt_this->pt_service, pt_dds->ui2_display_width, pt_dds->ui2_display_height);
    }
#endif
    
    return (i4_ret);
}



INT32 DvbDdsMgr::updateLogiSize(
    UINT16              ui2_page_id)
{
    INT32               i4_ret = DVBR_OK;
    DvbDds*      pt_dds;

    pt_dds = getDds(ui2_page_id);
    if (NULL != pt_dds)
    {
        i4_ret = android::DVBParser::getInstance()->updateLogicalSize(pt_dds->ui2_display_width, 
                                            pt_dds->ui2_display_height);
    }
    else
    {
        i4_ret = android::DVBParser::getInstance()->updateLogicalSize(720, 576);
    }

    return i4_ret;
}


INT32 DvbDdsMgr::resetLogiSize(
    UINT16              ui2_page_id)
{
    INT32               i4_ret = DVBR_OK;
    DvbDds*      pt_dds;

    pt_dds = getDds(ui2_page_id);
    if (NULL == pt_dds)
    {
        i4_ret = android::DVBParser::getInstance()->updateLogicalSize( 720, 576);
    }

    return i4_ret;
}


/*-----------------------------------------------------------------------------
 * Name: _sm_sbtl_dds_mngr_get_dds
 *
 * Description: get subtitle dds object that match page_id
 *
 * Inputs: pt_this - pointer to the dds manager object
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
DvbDds* DvbDdsMgr::getDds(
    UINT16              ui2_page_id)
{
    INT32           i;
    DvbDds*  pt_dds;
    
    for (i=0; i<this->ui1_dds_lst_cnt; i++)
    {
        pt_dds = (DvbDds*)this->apt_dds_lst[i];
        
        if (ui2_page_id==pt_dds->ui2_page_id)
        {
            return (pt_dds);
        }
    }
    
    return (NULL);
}

/*-----------------------------------------------------------------------------
 * Name: _sm_sbtl_dds_mngr_insert_dds
 *
 * Description: insert subtitle dds object to apt_dds_lst
 *
 * Inputs: pt_this - pointer to the dds manager object
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
INT32 DvbDdsMgr::insertDds(
    DvbDds*      pt_dds)
{
    DvbDds** apt_dds_lst;
    
    /*buffer full, resize the buffer*/
    if (this->ui1_dds_lst_cnt==this->ui1_dds_lst_size)
    {
        apt_dds_lst = (DvbDds**)malloc(
                                       (this->ui1_dds_lst_size+_DVB_SBTL_DDS_LST_SIZE)*sizeof(DvbDds*));
    
        if (NULL==apt_dds_lst)
        {
            return (DVBR_INSUFFICIENT_MEMORY);
        }
        
        /* copy memory */
        memcpy(apt_dds_lst,
                 this->apt_dds_lst,
                 sizeof(DvbDds*) * this->ui1_dds_lst_cnt);
        
        free(this->apt_dds_lst);
        this->apt_dds_lst = (DvbDds**)apt_dds_lst;
        this->ui1_dds_lst_size += _DVB_SBTL_DDS_LST_SIZE;
    }
    
    this->apt_dds_lst[this->ui1_dds_lst_cnt] = (DvbDds*)pt_dds;
    this->ui1_dds_lst_cnt++;
    
    return (DVBR_OK);
}
#endif
#endif
    

