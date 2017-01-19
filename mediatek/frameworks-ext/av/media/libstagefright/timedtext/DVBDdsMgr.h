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


#ifndef __DVB_DDS_MGR_H_
#define __DVB_DDS_MGR_H_
 
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

class DvbDdsMgr
{
public:
		DvbDdsMgr();
		~DvbDdsMgr();

		VOID freeAll();
 
	 INT32 Judgeboundary(
		 UINT16 				 ui2_width,
		 UINT16 				 ui2_height);
	 
	 INT32 adjust_address(
		 UINT16*				 pui2_width,
		 UINT16*				 pui2_height);
	 
	 INT32 parseSegment(
		 UINT8* 			 pui1_data, 
		 UINT32 			 ui4_data_len,
		 UINT16 			 ui2_page_id);
	 
	 INT32 updateLogiSize(
		 UINT16 			 ui2_page_id);
	 
	 INT32 resetLogiSize(
		 UINT16 			 ui2_page_id);
	 

	 DvbDds* getDds(UINT16 ui2_page_id);
	 //INT32 sdSize(
	//	 UINT16 			 ui2_page_id);
	 
private:		
    UINT8                   ui1_dds_lst_size;
    UINT8                   ui1_dds_lst_cnt;


    DvbDds**    apt_dds_lst;
	
	INT32 insertDds(DvbDds*      pt_dds);

	 
};


 
#endif  /*__DVB_DDS_MGR_H_*/

