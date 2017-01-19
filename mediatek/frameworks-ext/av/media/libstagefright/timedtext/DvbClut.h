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

#ifndef __DVB_CLUT_H_
#define __DVB_CLUT_H_

 /*-----------------------------------------------------------------------------
					  include files
   ----------------------------------------------------------------------------*/
  
 #include "DVBDataType.h"
  
  /*-----------------------------------------------------------------------------
					  macros, defines, typedefs, enums
   ----------------------------------------------------------------------------*/
  
#define DVB_SBTL_RGN_DEPTH_2BIT      ((UINT8) 4)
#define DVB_SBTL_RGN_DEPTH_4BIT      ((UINT8) 2)
#define DVB_SBTL_RGN_DEPTH_8BIT      ((UINT8) 1)

/* CLUT Segment */
#define DVB_SBTL_CLUTDS_MIN_LEN                                  ((UINT32) (2))
#define DVB_SBTL_CLUTDS_FULL_RANGE_ENTRY_SIZE                    ((UINT32) (6))
#define DVB_SBTL_CLUTDS_HALF_RANGE_ENTRY_SIZE                    ((UINT32) (4))

#define DVB_SBTL_CLUTDS_GET_N_BITS_ENTRY_FLAG(pui1_data, ui1_n_bits_entry_flag) \
    ui1_n_bits_entry_flag = (((UINT8*) (pui1_data))[1] & 0xE0) >> 5

#define DVB_SBTL_CLUTDS_GET_FULL_RANGE_FLAG(pui1_data, ui1_full_range_flag) \
    ui1_full_range_flag = (((UINT8*)(pui1_data))[1] & 0x01)
    

#define DVB_SBTL_CLUTDS_GET_CLUT_ID(pui1_data, ui1_CLUT_id) \
    ui1_CLUT_id = (((UINT8*) (pui1_data))[0])

#define DVB_SBTL_CLUTDS_GET_VERSION(pui1_data, ui1_version) \
    ui1_version = (((UINT8*) (pui1_data))[1] & 0xF0) >> 4

#define DVB_SBTL_CLUTDS_GET_ENTRY_ID(pui1_data, ui1_entry_id) \
    ui1_entry_id = ((UINT8*) (pui1_data))[0]
    
#define DVB_SBTL_CLUTDS_GET_FULL_RANGE_Y(pui1_data, ui1_Y) \
    ui1_Y = ((UINT8*)(pui1_data))[2]

#define DVB_SBTL_CLUTDS_GET_FULL_RANGE_CR(pui1_data, ui1_CR) \
    ui1_CR = ((UINT8*)(pui1_data))[3]

#define DVB_SBTL_CLUTDS_GET_FULL_RANGE_CB(pui1_data, ui1_CB) \
    ui1_CB = ((UINT8*)(pui1_data))[4]

#define DVB_SBTL_CLUTDS_GET_FULL_RANGE_T(pui1_data, ui1_T) \
    ui1_T = ((UINT8*)(pui1_data))[5]

#define DVB_SBTL_CLUTDS_GET_HALF_RANGE_Y(pui1_data, ui1_Y) \
    ui1_Y = (((UINT8*)(pui1_data))[2] & 0xFC) >> 2

#define DVB_SBTL_CLUTDS_GET_HALF_RANGE_CR(pui1_data, ui1_CR) \
    ui1_CR = (((UINT8*)(pui1_data))[2] & 0x03) << 2; \
    ui1_CR += (((UINT8*)(pui1_data))[3] & 0xC0) >> 6

#define DVB_SBTL_CLUTDS_GET_HALF_RANGE_CB(pui1_data, ui1_CB) \
    ui1_CB = (((UINT8*)(pui1_data))[3] & 0x3C) >> 2

#define DVB_SBTL_CLUTDS_GET_HALF_RANGE_T(pui1_data, ui1_T) \
    ui1_T = ((UINT8*)(pui1_data))[3] & 0x03


#define DVB_SBTL_2BIT_CLUT_SIZE      ((UINT32) 4)
#define DVB_SBTL_4BIT_CLUT_SIZE      ((UINT32) 16)
#define DVB_SBTL_8BIT_CLUT_SIZE      ((UINT32) 256)
  
#define _PERCENT_100      ((UINT8) 255)
#define _PERCENT_083      ((UINT8) 212)
#define _PERCENT_075      ((UINT8) 192)
#define _PERCENT_066      ((UINT8) 170)
#define _PERCENT_050      ((UINT8) 128)
#define _PERCENT_033      ((UINT8) 85)
#define _PERCENT_025      ((UINT8) 64)
#define _PERCENT_016      ((UINT8) 43)
#define _PERCENT_000      ((UINT8) 0)
 
#define DVB_SBTL_CLUT_ADJ_YCBCR(ui1_Y, ui1_Cb, ui1_Cr) \
		 if (16 > ui1_Y)   { ui1_Y = 16;  } \
		 if (235 < ui1_Y)  { ui1_Y = 235; } \
		 if (16 > ui1_Cb)  { ui1_Cb = 16; } \
		 if (240 < ui1_Cb) { ui1_Cb = 240;} \
		 if (16 > ui1_Cr)  { ui1_Cr = 16; } \
		 if (240 < ui1_Cr) { ui1_Cr = 240;} \
 
  
  /*-----------------------------------------------------------------------------
					  data declarations, extern, static, const
   ----------------------------------------------------------------------------*/
  
  /*-----------------------------------------------------------------------------
					  class declarations
   ----------------------------------------------------------------------------*/
 struct GL_COLOR_T
 {
	 UINT8	 a;
 
	 union {
		 UINT8	 r;
		 UINT8	 y;
	 } u1;
 
	 union {
		 UINT8	 g;
		 UINT8	 u;
	 } u2;
 
	 union {
		 UINT8	 b;
		 UINT8	 v;
		 UINT8	 index;
	 } u3;
 };

struct DVB_NBIT_CLUT_T
{
    GL_COLOR_T*             pat_color_entry;

    UINT16                  ui2_link_cnt;
};


class DvbClut
{
public:	
	 DvbClut();
	 
	 ~DvbClut();
	 
	 UINT8	getId(){return ui1_id;};
	 
	 static INT32 create(UINT8*              pui1_data, 
	                        UINT32              ui4_data_len, 
	                        DvbClut**     ppt_this);
	 
	 INT32 parseSegment(
		 UINT8* 			 pui1_data, 
		 UINT32 			 ui4_data_len);
	 
	 DVB_NBIT_CLUT_T* linkSub(
		 UINT8				 ui1_color_depth);
	 
	 
	 static VOID release(DVB_NBIT_CLUT_T* pt_this, 
					UINT8				 ui1_color_depth);

	static const DVB_NBIT_CLUT_T* getDefaultClut(
		 UINT8 ui1_color_depth);
	 
private:
	 struct DVB_NBIT_CLUT_T*	 pt_2bit_CLUT;
 
	 struct DVB_NBIT_CLUT_T*	 pt_4bit_CLUT;
 
	 struct DVB_NBIT_CLUT_T*	 pt_8bit_CLUT;
	 
	 UINT8						 ui1_id;
 
	 UINT8						 ui1_version_number;


	  INT32 parseSegment_impl(
			 UINT8* 			 pui1_data, 
			 UINT32 			 ui4_data_len,
			 bool				 is_new_CLUT);
 
 	 INT32 dup(
			 UINT8					 ui1_color_depth,
			 DVB_NBIT_CLUT_T*	 pt_dup_nbit_clut,
			 DVB_NBIT_CLUT_T**	 ppt_this);
		 
 	 static VOID clutDelete(DVB_NBIT_CLUT_T*	pt_dvb_nbit_clut,
	 					UINT8					 ui1_color_depth);
 

	 INT32 getClutEntry(
		 UINT8* 			 pui1_data, 
		 UINT32 			 ui4_data_len,
		 UINT8				 ui1_n_bits_entry_flag,
		 UINT8				 ui1_full_range_flag,
		 UINT8				 ui1_entry_id,
		 UINT32*			 pui4_inc_size );
 };
#endif

