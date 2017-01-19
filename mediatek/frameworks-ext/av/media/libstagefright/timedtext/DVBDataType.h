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
#ifndef __DVB_DATA_TYPE_H_
#define __DVB_DATA_TYPE_H_
  
 #define DVB_ENABLE_HD_SUBTITLE
 
typedef unsigned char  BYTE;
typedef unsigned char  UCHAR;
typedef unsigned short  UINT16;
typedef unsigned long  UINT32;
typedef unsigned long long  UINT64;
typedef char  CHAR;     // Debug, should be 'signed char'
typedef signed char  INT8;
typedef signed short  INT16;
typedef signed long  INT32;
typedef signed long long  INT64;
typedef unsigned char  UINT8;  

#ifndef VOID
#define VOID void
#endif  

#ifndef TRUE
    #define TRUE                (0 == 0)
#endif  // TRUE

#ifndef FALSE
    #define FALSE               (0 != 0)
#endif  // FALSE


/* Sutitle service internal used return value */
#define DVBR_CONTENT_UPDATE                ((INT32) 1)
#define DVBR_OK                            ((INT32) 0)
#define DVBR_INV_ARG                       ((INT32) -1)
#define DVBR_ALREADY_INIT                  ((INT32) -2)
#define DVBR_NOT_INIT                      ((INT32) -3)
#define DVBR_NOT_FOUND                     ((INT32) -4)
#define DVBR_INSUFFICIENT_MEMORY           ((INT32) -5)
#define DVBR_WRONG_SEGMENT_DATA            ((INT32) -6)
#define DVBR_HOOK_FUNC_NOT_FOUND           ((INT32) -7)
#define DVBR_INTERNAL_STATE_ERROR          ((INT32) -8)
#define DVBR_WGL_FAIL                      ((INT32) -9)
#define DVBR_PAGE_INST_OUT_OF_HANDLE       ((INT32) -10)
#define DVBR_DMX_FAIL                      ((INT32) -11)
#define DVBR_PTS_FAIL                      ((INT32) -12)
#define DVBR_SCDB_FAIL                     ((INT32) -13)
#define DVBR_PM_FAIL                       ((INT32) -14)
#define DVBR_MEM_USAGE_CHANGED             ((INT32) -15)

	  
#endif  /*__DVB_DATA_TYPE_H_*/

