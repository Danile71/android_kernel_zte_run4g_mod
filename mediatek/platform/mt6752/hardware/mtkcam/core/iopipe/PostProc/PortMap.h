/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_CORE_IOPIPE_POSTPROC_PORTMAP_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_CORE_IOPIPE_POSTPROC_PORTMAP_H_
//

#include <mtkcam/iopipe/Port.h>

using namespace NSImageio;
using namespace NSIspio;

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {
//
  static const PortID       PORT_IMGI   (EPortType_Memory, NSImageio::NSIspio::EPortIndex_IMGI/*index*/, 0/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
  static const PortID       PORT_VIPI   (EPortType_Memory, NSImageio::NSIspio::EPortIndex_VIPI/*index*/, 0/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
  static const PortID       PORT_VIP2I  (EPortType_Memory, NSImageio::NSIspio::EPortIndex_VIP2I/*index*/, 0/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
  static const PortID       PORT_VIP3I  (EPortType_Memory, NSImageio::NSIspio::EPortIndex_VIP3I/*index*/, 0/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
  static const PortID       PORT_IMGO   (EPortType_Memory, NSImageio::NSIspio::EPortIndex_IMGO/*index*/, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
  static const PortID       PORT_MFBO   (EPortType_Memory, NSImageio::NSIspio::EPortIndex_MFBO/*index*/, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
  static const PortID       PORT_IMG2O  (EPortType_Memory, NSImageio::NSIspio::EPortIndex_IMG2O/*index*/, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
  static const PortID       PORT_IMG3O  (EPortType_Memory, NSImageio::NSIspio::EPortIndex_IMG3O/*index*/, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
  static const PortID       PORT_IMG3BO (EPortType_Memory, NSImageio::NSIspio::EPortIndex_IMG3BO/*index*/, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
  static const PortID       PORT_IMG3CO (EPortType_Memory, NSImageio::NSIspio::EPortIndex_IMG3CO/*index*/, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
  static const PortID       PORT_WROTO  (EPortType_Memory, NSImageio::NSIspio::EPortIndex_WROTO/*index*/, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
  static const PortID       PORT_WDMAO  (EPortType_Memory, NSImageio::NSIspio::EPortIndex_WDMAO/*index*/, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
  static const PortID       PORT_JPEGO  (EPortType_Memory, NSImageio::NSIspio::EPortIndex_JPEGO/*index*/, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
  
//
/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSIoPipe
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_MTKCAM_CORE_IOPIPE_POSTPROC_PORTMAP_H_

