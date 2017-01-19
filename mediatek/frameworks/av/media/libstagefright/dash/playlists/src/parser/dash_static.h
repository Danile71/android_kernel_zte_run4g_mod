/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/


#ifndef _STATIC_CLASS_HXX_
#define _STATIC_CLASS_HXX_

#include "MPDSchema_enum.h"
#include "exception.h"


namespace dash_static
{

    const static int BUFFER_LEN = 50;
    

    class dash_handle
    {
        public:
			static void merge_segment_base(MPDSchema::CSegmentBaseType_ex* pre, MPDSchema::CSegmentBaseType_ex* next, bool rev);
			static void merge_segment_list(MPDSchema::CSegmentListType_ex* pre, MPDSchema::CSegmentListType_ex* next);
			static void merge_multi_segmentbase(MPDSchema::CMultipleSegmentBaseType_ex* pre, MPDSchema::CMultipleSegmentBaseType_ex* next, bool rev);
			static void merge_segment_template(MPDSchema::CSegmentTemplateType_ex* pre, MPDSchema::CSegmentTemplateType_ex* next, bool rev);
			static unsigned int get_startnumber_by_ptr(MPDSchema::CMultipleSegmentBaseType* ptr);
            static dash_enum::dash_segment_info_type get_segment_info_type(bool b, bool l, bool t)throw (all_exceptions);
            static dash_enum::dash_content_type get_content_type(std::string& c);
            static dash_enum::dash_mime_type get_mime_type(std::string& m);
            static dash_enum::dash_content_type get_content_type_by_mime(dash_enum::dash_mime_type m);
            static std::string get_string_by_int(int i);
            static bool segment_info_valid(int pre, int self, int next)throw (all_exceptions);  
            static int handle_byte_range(std::string& byte, unsigned int& last);
            static dash_enum::dash_container_type get_container_by_content_type(dash_enum::dash_content_type c); 
		#if (MPD_PASER_DEBUG > 1)
			static void print_duration(altova::Duration* ptr);
		    static void print_daytime(altova::DateTime* ptr);
		#endif
    };
}

using namespace dash_static;
#endif

