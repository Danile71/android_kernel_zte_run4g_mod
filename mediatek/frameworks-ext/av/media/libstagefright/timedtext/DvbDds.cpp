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
#ifdef DVB_ENABLE_HD_SUBTITLE
#define LOG_TAG "DVBDDS"


 /*-----------------------------------------------------------------------------
					 macros, defines, typedefs, enums
  ----------------------------------------------------------------------------*/
 #define DVB_SBTL_DDS_GET_VERSION(pui1_data, ui1_version)\
    ui1_version = (((UINT8*)(pui1_data))[0]&0xF0)>>4

#define DVB_SBTL_DDS_GET_DISP_WIN_FLAG(pui1_data, ui1_disp_win_flag)\
	 ui1_disp_win_flag = (((UINT8*)(pui1_data))[0]&0x08)>>3

#define DVB_SBTL_DDS_GET_DISP_WIDTH(pui1_data, ui2_disp_width)\
	 GET_UINT16((pui1_data+1), ui2_disp_width)
 
#define DVB_SBTL_DDS_GET_DISP_HEIGHT(pui1_data, ui2_disp_height)\
	 GET_UINT16((pui1_data+3), ui2_disp_height)
 
#define DVB_SBTL_DDS_GET_DISP_WIN_HOR_POS_MIN(pui1_data, ui2_disp_win_hor_pos_min)\
	 GET_UINT16((pui1_data+5), ui2_disp_win_hor_pos_min)
 
#define DVB_SBTL_DDS_GET_DISP_WIN_HOR_POS_MAX(pui1_data, ui2_disp_win_hor_pos_max)\
	 GET_UINT16((pui1_data+7), ui2_disp_win_hor_pos_max)
 
#define DVB_SBTL_DDS_GET_DISP_WIN_VER_POS_MIN(pui1_data, ui2_disp_win_ver_pos_min)\
	 GET_UINT16((pui1_data+9), ui2_disp_win_ver_pos_min)
 
#define DVB_SBTL_DDS_GET_DISP_WIN_VER_POS_MAX(pui1_data, ui2_disp_win_ver_pos_max)\
	 GET_UINT16((pui1_data+11), ui2_disp_win_ver_pos_max)


#define DVB_SBTL_DDS_WIDTH_ASSUMED   720
#define DVB_SBTL_DDS_HEIGHT_ASSUMED  576
#define DVB_SBTL_DDS_WIDTH_MAX       1920
#define DVB_SBTL_DDS_HEIGHT_MAX      1080


 const UINT32 DvbDds::u4_dds_same_id_count = 8;
 /*-----------------------------------------------------------------------------
					 data declarations, extern, static, const
  ----------------------------------------------------------------------------*/
 
 /*-----------------------------------------------------------------------------
					 functions declarations
  ----------------------------------------------------------------------------*/

 
 /*-----------------------------------------------------------------------------
  * Name: DVB_sbtl_dds_create
  *
  * Description: create subtitle dds
  *
  * Inputs:
  *
  * Outputs:
  *
  * Returns:
  ----------------------------------------------------------------------------*/
 DvbDds::DvbDds(
	 UINT16 			 ui2_pid)
 {
	 INT32			 i4_ret;
	 
	 this->ui2_page_id = ui2_pid;
	
 }
  
 /*-----------------------------------------------------------------------------
  * Name: sm_sbtl_dds_delete
  *
  * Description: delete subtitle dds
  *
  * Inputs:
  *
  * Outputs:
  *
  * Returns:
  ----------------------------------------------------------------------------*/
 DvbDds::~DvbDds()
 {
	 
 }


 INT32 DvbDds::create(UINT8*              pui1_data, 
	                        UINT32          ui4_data_len, 
	                        UINT16          ui2_page_id,
	                        DvbDds**     ppt_this)
{
    INT32					 i4_ret;
    DvbDds*		 pt_this;

    *ppt_this = NULL;
    pt_this = new DvbDds(ui2_page_id);

    if (NULL == pt_this)
    {
        return (DVBR_INSUFFICIENT_MEMORY);
    }

    i4_ret = pt_this->parseSegment_impl(pui1_data,ui4_data_len,TRUE);

    if (DVBR_OK > i4_ret)
    {
        /* segment data error */
        //DVB_sbtl_clut_delete(pt_this, pt_service);
        delete pt_this;
        return (i4_ret);
    }
    else
    {
        *ppt_this = pt_this;
        return (DVBR_CONTENT_UPDATE);
    }
    return i4_ret;
}
 
 /*-----------------------------------------------------------------------------
  * Name: sm_sbtl_dds_judge_boundary
  *
  * Description: subtitle dds judge if the boundary is legal
  *
  * Inputs: pt_this - pointer to the dds object
  *
  * Outputs: -
  *
  * Returns: -
  ----------------------------------------------------------------------------*/
 INT32 DvbDds::judgeBoundary(
	 UINT16 			 ui2_width,
	 UINT16 			 ui2_height)
 {
	 if (NULL==this)
	 {
		 return (DVBR_WRONG_SEGMENT_DATA);
	 }
	 
	 /* per spec definition, the real widht/height is value+1 */
	 if (ui2_width>this->ui2_display_width)
	 {
		 return (DVBR_WRONG_SEGMENT_DATA);
	 }
	 
	 if (ui2_height>this->ui2_display_height)
	 {
		 return (DVBR_WRONG_SEGMENT_DATA);
	 }
	 
	 if (1==this->ui1_display_window_flag)
	 {
		 if ( (ui2_width<this->ui2_display_window_hor_pos_min)||
			  (ui2_width>this->ui2_display_window_hor_pos_max) )
		 {
			 return (DVBR_WRONG_SEGMENT_DATA);
		 }
		 
		 if ( (ui2_height<this->ui2_display_window_ver_pos_min)||
			  (ui2_height>this->ui2_display_window_ver_pos_max) )
		 {
			 return (DVBR_WRONG_SEGMENT_DATA);
		 }
	 }
	 
	 return (DVBR_OK);
 }
 
 
 /*-----------------------------------------------------------------------------
  * Name: sm_sbtl_dds_adjust_address_by_boundary
  *
  * Description: subtitle dds adjust the boundary
  *
  * Inputs: pt_this - pointer to the dds object
  *
  * Outputs: -
  *
  * Returns: -
  ----------------------------------------------------------------------------*/
 INT32 DvbDds::adjustAddressbyBoundary(
	 UINT16*			 pui2_width,
	 UINT16*			 pui2_height)
 {
	 if (NULL==this)
	 {
		 return (DVBR_WRONG_SEGMENT_DATA);
	 }
	 
	 /* per spec definition, the real widht/height is value+1 */
	 if (*pui2_width>this->ui2_display_width)
	 {
		 return (DVBR_WRONG_SEGMENT_DATA);
	 }
	 
	 if (*pui2_height>this->ui2_display_height)
	 {
		 return (DVBR_WRONG_SEGMENT_DATA);
	 }
	 
	 if (1==this->ui1_display_window_flag)
	 {
		 if (*pui2_width < this->ui2_display_window_hor_pos_min)
		 {
			 *pui2_width = this->ui2_display_window_hor_pos_min;
		 }
 
		 if (*pui2_width > this->ui2_display_window_hor_pos_max)
		 {
			 *pui2_width = this->ui2_display_window_hor_pos_max;
		 }
		 
		 if (*pui2_height < this->ui2_display_window_ver_pos_min)
		 {
			 *pui2_height = this->ui2_display_window_ver_pos_min;
		 }
 
		 if (*pui2_height > this->ui2_display_window_ver_pos_max)
		 {
			 *pui2_height = this->ui2_display_window_ver_pos_max;
		 }
	 }
	 
	 return (DVBR_OK);
 }
 
 
 /*-----------------------------------------------------------------------------
  * Name: sm_sbtl_dds_parse_segment
  *
  * Description: subtitle dds segment parser
  *
  * Inputs:
  *
  * Outputs:
  *
  * Returns:
  ----------------------------------------------------------------------------*/
 INT32 DvbDds::parseSegment(
	 UINT8* 			 pui1_data,
	 UINT32 			 ui4_data_len)
 {
	 return (parseSegment_impl(
								pui1_data, 
								ui4_data_len,
								FALSE));
 }
 
 /*-----------------------------------------------------------------------------
  * Name: sm_sbtl_dds_parse_segment_impl
  *
  * Description: subtitle dds parse implement
  *
  * Inputs: 
  *
  * Outputs:
  *
  * Returns:
  ----------------------------------------------------------------------------*/
 INT32 DvbDds::parseSegment_impl(
	 UINT8* 			 pui1_data, 
	 UINT32 			 ui4_data_len,
	 bool				 is_new_rgn)
 {
	 UINT8	 ui1_version_number;
	 UINT16  ui2_display_width;
	 UINT16  ui2_display_height;
	 UINT16  ui2_display_window_hor_pos_min;
	 UINT16  ui2_display_window_hor_pos_max;
	 UINT16  ui2_display_window_ver_pos_min;
	 UINT16  ui2_display_window_ver_pos_max;
	 
	 DVB_SBTL_DDS_GET_VERSION(pui1_data, ui1_version_number);
	 
	 if (FALSE==is_new_rgn)
	 {
		 if (ui1_version_number==this->ui1_version_number)
		 {
			 this->ui2_same_dds_cnt++;
			 
			 if (u4_dds_same_id_count > this->ui2_same_dds_cnt)
			 {
				 return (DVBR_CONTENT_UPDATE);
			 }
			 else
			 {
				 /* prevent overflow */
				 if (this->ui2_same_dds_cnt==0xffff)
				 {
					 this->ui2_same_dds_cnt = u4_dds_same_id_count+1;
				 }
				 
				 return (DVBR_OK);
			 }
		 }
	 }
	 
	 this->ui1_version_number = ui1_version_number;
	 DVB_SBTL_DDS_GET_DISP_WIN_FLAG(pui1_data, this->ui1_display_window_flag);
	 
	 DVB_SBTL_DDS_GET_DISP_WIDTH(pui1_data, ui2_display_width);
	 DVB_SBTL_DDS_GET_DISP_HEIGHT(pui1_data, ui2_display_height);
	 
	 /* Valid the boundary */
	 if ( ui2_display_width > DVB_SBTL_DDS_WIDTH_MAX )
	 {
		 return (DVBR_WRONG_SEGMENT_DATA);
	 }
 
	 if ( ui2_display_height > DVB_SBTL_DDS_HEIGHT_MAX )
	 {
		 return (DVBR_WRONG_SEGMENT_DATA);
	 }
 
	 /* Fill the data to DSS */
	 this->ui2_display_width = (UINT16)(ui2_display_width+1);
	 this->ui2_display_height = (UINT16)(ui2_display_height+1);
 
	 ALOGD( "DDS[width:%d][height:%d]\n", this->ui2_display_width, this->ui2_display_height );
 
	 if (this->ui1_display_window_flag==1)
	 {
		 DVB_SBTL_DDS_GET_DISP_WIN_HOR_POS_MIN(pui1_data, ui2_display_window_hor_pos_min);
		 DVB_SBTL_DDS_GET_DISP_WIN_HOR_POS_MAX(pui1_data, ui2_display_window_hor_pos_max);
		 DVB_SBTL_DDS_GET_DISP_WIN_VER_POS_MIN(pui1_data, ui2_display_window_ver_pos_min);
		 DVB_SBTL_DDS_GET_DISP_WIN_VER_POS_MAX(pui1_data, ui2_display_window_ver_pos_max);
			
		 if ( (ui2_display_window_hor_pos_min > DVB_SBTL_DDS_WIDTH_MAX)||
			  (ui2_display_window_hor_pos_min>ui2_display_window_hor_pos_max) )
		 {
			 return (DVBR_WRONG_SEGMENT_DATA);
		 }
		 
		 if ( (ui2_display_window_hor_pos_max > DVB_SBTL_DDS_WIDTH_MAX)||
			  ((ui2_display_window_hor_pos_max-ui2_display_window_hor_pos_min)>DVB_SBTL_DDS_WIDTH_MAX) )
		 {
			 return (DVBR_WRONG_SEGMENT_DATA);
		 }
		 
		 if ( (ui2_display_window_ver_pos_min>DVB_SBTL_DDS_HEIGHT_MAX)||
			  (ui2_display_window_ver_pos_min>ui2_display_window_ver_pos_max) )
		 {
			 return (DVBR_WRONG_SEGMENT_DATA);
		 }
		 
		 if ( (ui2_display_window_ver_pos_max>DVB_SBTL_DDS_HEIGHT_MAX)||
			  ((ui2_display_window_ver_pos_max-ui2_display_window_ver_pos_min)>DVB_SBTL_DDS_HEIGHT_MAX) )
		 {
			 return (DVBR_WRONG_SEGMENT_DATA);
		 }
		 
		 this->ui2_display_window_hor_pos_min = (UINT16)(ui2_display_window_hor_pos_min+1);
		 this->ui2_display_window_hor_pos_max = (UINT16)(ui2_display_window_hor_pos_max+1);
		 this->ui2_display_window_ver_pos_min = (UINT16)(ui2_display_window_ver_pos_min+1);
		 this->ui2_display_window_ver_pos_max = (UINT16)(ui2_display_window_ver_pos_max+1);
 
		 ALOGD( "   [h_min:%d][h_max:%d]\n", this->ui2_display_window_hor_pos_min, this->ui2_display_window_hor_pos_max );
		 ALOGD( "   [v_min:%d][v_max:%d]\n", this->ui2_display_window_ver_pos_min, this->ui2_display_window_ver_pos_max );
	 }
	 
	 return (DVBR_CONTENT_UPDATE);
 }
 
#endif /* DVB_ENABLE_HD_SUBTITLE */
#endif
