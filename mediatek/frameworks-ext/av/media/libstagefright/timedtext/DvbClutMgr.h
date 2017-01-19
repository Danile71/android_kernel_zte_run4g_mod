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


#ifndef __DVB_CLUT_MGR_H_
#define __DVB_CLUT_MGR_H_
 
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
class DvbRegionMgr;
 
class DvbClutMgr
{
public:
	DvbClutMgr();

	~DvbClutMgr();

	VOID freeAll();

	INT32 parseSegment(DvbRegionMgr&     		pt_rgn_mngr, 
						    UINT8*                  pui1_data, 
						    UINT32                  ui4_data_len);

	DVB_NBIT_CLUT_T* Link_sub(
	    UINT8                   ui1_CLUT_id,
	    UINT8                   ui1_color_depth);


private:
    DvbClut**   			apt_CLUT_lst;

    UINT8                   ui1_CLUT_lst_size;

    UINT8                   ui1_CLUT_lst_cnt;

	DvbClut* getClut(UINT8  ui1_CLUT_id);
	INT32	insertClut(DvbClut* pt_CLUT);
	
};
 


#endif


