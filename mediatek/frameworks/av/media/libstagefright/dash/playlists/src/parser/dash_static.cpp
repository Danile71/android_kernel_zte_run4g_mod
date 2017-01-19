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


#include "mpdschema_prefix.h"
#include <string.h>
#include "MPDSchema.h"
#include <algorithm>


#include "XercesString.h"
#include "dash_static.h"

#include "MPDSchema_debug.h"
#include "MPDSchema_enum.h"
#include <stdio.h>
#include <stdlib.h>


void dash_handle::merge_segment_base(MPDSchema::CSegmentBaseType_ex* pre, MPDSchema::CSegmentBaseType_ex* next, bool rev)
{
    if(pre == NULL || next == NULL)
    {
        MPD_PARSER_DBG(("no pre or next to merge\n"));
		return; 
    }


	if(rev)
	{
		if(pre->indexRangeExact_exists && !next->indexRangeExact_exists)
		{
		    next->indexRangeExact_ex = pre->indexRangeExact_ex;
			next->indexRangeExact_exists = true;
		}

		if(pre->indexRange_exists && !next->indexRange_exists)
		{
		    next->indexRange_exists = true;
			next->indexRange_ex = pre->indexRange_ex;
		}

		if(pre->timescale_exists && !next->timescale_exists)
		{
		    next->timescale_ex = pre->timescale_ex;
			next->timescale_exists = true;
		}

		MPD_PARSER_DBG(("1  indexRangeExact_ex %d indexRange_ex = %s timescale_ex = %d\n",
			next->indexRangeExact_ex?1:0, next->indexRange_ex.c_str(), next->timescale_ex));

		if(pre->presentationTimeOffset_exists && !next->presentationTimeOffset_exists)
		{
		    next->presentationTimeOffset_ex = pre->presentationTimeOffset_ex;
			next->presentationTimeOffset_exists = true;
		}

		if(pre->Initialisation_exists && !next->Initialisation_exists)
		{
		    next->Initialisation_exists = true;

			if(pre->Initialisation_sourceURL_exists)
			{
			    next->Initialisation_sourceURL_exists = true;
				next->Initialisation_ex = pre->Initialisation_ex;
			}

			if(pre->Initialisation_byteRange_exists && !next->Initialisation_byteRange_exists)
			{
			    next->Initialisation_byteRange_ex = pre->Initialisation_byteRange_ex;
				next->Initialisation_byteRange_exists = true;
			}
		}

        MPD_PARSER_DBG(("1 presentationTimeOffset_ex = %d Initialisation_ex = %s Initialisation_byteRange_ex %s\n",
			next->presentationTimeOffset_ex, next->Initialisation_ex.c_str(), next->Initialisation_byteRange_ex.c_str()));
		if(pre->RepresentationIndex_exists && !next->RepresentationIndex_exists)
		{
		    next->RepresentationIndex_exists = true;

			if(pre->RepresentationIndex_sourceURL_exists)
			{
				next->RepresentationIndex_ex = pre->RepresentationIndex_ex;
				next->RepresentationIndex_sourceURL_exists = true;
			}

			if(pre->RepresentationIndex_byteRange_exists && !next->RepresentationIndex_byteRange_exists)
			{
			    next->RepresentationIndex_byteRange_ex = pre->RepresentationIndex_byteRange_ex;
				next->RepresentationIndex_byteRange_exists = true;
			}
		}

		MPD_PARSER_DBG(("1 RepresentationIndex_ex = %s, RepresentationIndex_byteRange_ex = %s\n", next->RepresentationIndex_ex.c_str(),
			next->RepresentationIndex_byteRange_ex.c_str()));
	}
	else
	{
	    if(next->indexRangeExact_exists)
	    {
	        pre->indexRangeExact_ex = next->indexRangeExact_ex;
			pre->indexRangeExact_exists = true;
	    }

		if(next->indexRange_exists)
		{
		    pre->indexRange_exists = true;
			pre->indexRange_ex = next->indexRange_ex;
		}

		if(next->timescale_exists)
		{
		    pre->timescale_ex = next->timescale_ex;
			pre->timescale_exists = true;
		}

		if(next->presentationTimeOffset_exists)
		{
		    pre->presentationTimeOffset_ex = next->presentationTimeOffset_ex;
			pre->presentationTimeOffset_exists = true;
		}

		if(next->Initialisation_exists)
		{
		    pre->Initialisation_exists = true;

			
			pre->Initialisation_ex = next->Initialisation_ex;
			pre->Initialisation_sourceURL_exists = next->Initialisation_sourceURL_exists;
		    pre->Initialisation_byteRange_ex = next->Initialisation_byteRange_ex;
			pre->Initialisation_byteRange_exists = next->Initialisation_byteRange_exists;
			
		}

		if(next->RepresentationIndex_exists)
		{
		    pre->RepresentationIndex_exists = true;
			pre->RepresentationIndex_ex = next->RepresentationIndex_ex;
			pre->RepresentationIndex_sourceURL_exists = next->RepresentationIndex_sourceURL_exists;
		    pre->RepresentationIndex_byteRange_ex = next->RepresentationIndex_byteRange_ex;
			pre->RepresentationIndex_byteRange_exists = next->RepresentationIndex_byteRange_exists;
		}

		MPD_PARSER_DBG(("0  indexRangeExact_ex %d indexRange_ex = %s timescale_ex = %d",
			pre->indexRangeExact_ex?1:0, pre->indexRange_ex.c_str(), pre->timescale_ex));


		MPD_PARSER_DBG(("0 presentationTimeOffset_ex = %d Initialisation_ex = %s Initialisation_byteRange_ex %s\n",
			pre->presentationTimeOffset_ex, pre->Initialisation_ex.c_str(), pre->Initialisation_byteRange_ex.c_str()));


		MPD_PARSER_DBG(("0 RepresentationIndex_ex = %s, RepresentationIndex_byteRange_ex = %s\n", pre->RepresentationIndex_ex.c_str(),
			pre->RepresentationIndex_byteRange_ex.c_str()));
		
	}
	
}


void dash_handle::merge_multi_segmentbase(MPDSchema::CMultipleSegmentBaseType_ex* pre, MPDSchema::CMultipleSegmentBaseType_ex* next, bool rev)
{
    if(pre == NULL || next == NULL)
    {
        MPD_PARSER_DBG(("no pre or next to merge\n"));
		return; 
    }

	if(rev)
	{
		if(pre->duration_exists && !next->duration_exists)
		{
		    next->duration_exists = true;
			next->duration_ex = pre->duration_ex;
		}

		if(pre->startNumber_exists && !next->startNumber_exists)
		{
		    next->startNumber_ex = pre->startNumber_ex;
			next->startNumber_exists = true;
		}

		if(pre->BitstreamSwitching_exists && !next->BitstreamSwitching_exists)
		{
		    next->BitstreamSwitching_exists = true;
			next->BitstreamSwitching_ex = pre->BitstreamSwitching_ex;

			if(pre->BitstreamSwitching_byteRange_exists && !next->BitstreamSwitching_byteRange_exists)
			{
			    next->BitstreamSwitching_byteRange_exists = true;
				next->BitstreamSwitching_byteRange_ex = pre->BitstreamSwitching_byteRange_ex;
			}
		}

		MPD_PARSER_DBG(("1 duration_ex = %d startNumber_ex = %d BitstreamSwitching_ex = %s BitstreamSwitching_byteRange_ex=%s\n",
			next->duration_ex, next->startNumber_ex, next->BitstreamSwitching_ex.c_str(), next->BitstreamSwitching_byteRange_ex.c_str()));


	}
	else
	{
	    if(next->duration_exists)
	    {
	        pre->duration_ex = next->duration_ex;
			pre->duration_exists = true;
	    }

		if(next->startNumber_exists)
		{
		    pre->startNumber_ex = next->startNumber_ex;
			pre->startNumber_exists = true;
		}


		if(next->BitstreamSwitching_byteRange_exists)
		{
		    pre->BitstreamSwitching_byteRange_ex = next->BitstreamSwitching_byteRange_ex;
			pre->BitstreamSwitching_byteRange_exists = next->BitstreamSwitching_byteRange_exists;
			pre->BitstreamSwitching_ex = next->BitstreamSwitching_ex;
			pre->BitstreamSwitching_exists = true;
		}

		MPD_PARSER_DBG(("0 duration_ex = %d startNumber_ex = %d BitstreamSwitching_ex = %s BitstreamSwitching_byteRange_ex=%s\n",
			pre->duration_ex, pre->startNumber_ex, pre->BitstreamSwitching_ex.c_str(), pre->BitstreamSwitching_byteRange_ex.c_str()));
	}
	
	merge_segment_base(pre, next, rev);
}

void dash_handle::merge_segment_template(MPDSchema::CSegmentTemplateType_ex* pre, MPDSchema::CSegmentTemplateType_ex* next, bool rev)
{
    if(pre == NULL || next == NULL)
    {
        MPD_PARSER_DBG(("no pre or next to merge\n"));
		return; 
    }

	merge_multi_segmentbase(pre, next, rev);

    if(rev)
    {
		if(pre->bistreamswitch_exists && !next->bistreamswitch_exists)
		{
		    next->bistreamswitch_ex = pre->bistreamswitch_ex;
			next->bistreamswitch_exists = true;
		}

		if(pre->media_exists && ! next->media_exists)
		{
		    next->media_ex = pre->media_ex;
			next->media_exists = true;
		}

		if(pre->index_exists && !next->index_exists)
		{
		    next->index_ex = pre->index_ex;
			next->index_exists = true;
		}

		if(pre->initialisation_exists && !next->initialisation_exists)
		{
		    next->initialisation_ex = pre->initialisation_ex;
			next->initialisation_exists = true;
		}

		MPD_PARSER_DBG(("1 bistreamswitch_ex = %s index_ex %s initialisation_ex %s media_ex %s\n",
			next->bistreamswitch_ex.c_str(),  next->index_ex.c_str(), next->initialisation_ex.c_str(),next->media_ex.c_str()));
    }
	else
	{
	    if(next->bistreamswitch_exists)
	    {
	        pre->bistreamswitch_ex = next->bistreamswitch_ex;
			pre->bistreamswitch_exists = true;
	    }

	    
	    if(next->media_exists)
	    {
	        pre->media_ex = next->media_ex;
			pre->media_exists = true;
	    }

		if(next->index_exists)
		{
		    pre->index_ex = next->index_ex;
			pre->index_exists = true;
		}

		if(next->initialisation_exists)
		{
		    pre->initialisation_ex = next->initialisation_ex;
			pre->initialisation_exists = true;
		}

		MPD_PARSER_DBG(("0 bistreamswitch_ex = %s index_ex %s initialisation_ex %s media_ex %s\n",
			pre->bistreamswitch_ex.c_str(),  pre->index_ex.c_str(), pre->initialisation_ex.c_str(),pre->media_ex.c_str()));
	}
}



void dash_handle::merge_segment_list(MPDSchema::CSegmentListType_ex* pre, MPDSchema::CSegmentListType_ex* next)
{
    if(pre == NULL || next == NULL)
    {
        MPD_PARSER_DBG(("no pre or next to merge\n"));
		return;
    }
	merge_multi_segmentbase(pre, next, true);
}


dash_enum::dash_container_type dash_handle::get_container_by_content_type(dash_enum::dash_content_type c)
{
    switch(c)
    {
        case dash_enum::c_ts:
            return dash_enum::DASH_TS;
        case dash_enum::none:
            return dash_enum::DASH_OTHER;
        default:
            return dash_enum::DASH_MP4;
    }
}

int dash_handle::handle_byte_range(std::string& byte, unsigned int& last)
{
    if(byte.length() == 0)
    {
        last = 0;
        return 0;
    }


    unsigned int pos = byte.find('-');
    if(pos == std::string::npos)
    {
        last = atoi(byte.c_str());
        return 0;
    }
    else if(pos == 0)
    {
        last = 0;
        return 0;
    }
    else
    {
        std::string f = byte.substr(0, pos);
        std::string l = byte.substr(pos + 1);

        last = atoi(l.c_str());
        return atoi(f.c_str());
    }
    
}


unsigned int dash_handle::get_startnumber_by_ptr(MPDSchema::CMultipleSegmentBaseType* ptr)
{
	if(ptr == NULL)
	{
	    MPD_PARSER_DBG(("error: no segment ptr\n"));
		return 1;
	}

	if(ptr->startNumber.exists())
	{
	    return ptr->startNumber;
	}

	return 1;
}


std::string dash_handle::get_string_by_int(int i)
{
    char buffer[dash_static::BUFFER_LEN];
    sprintf(buffer, "%d", i);
    std::string temp = std::string(buffer);
    return temp;    
}

dash_enum::dash_content_type dash_handle::get_content_type(std::string& c)
{
    if(c.compare("audio") == 0)
        return dash_enum::audio;
    else if(c.compare("video") == 0)
        return dash_enum::video;
    else if(c.compare("image") == 0)
        return dash_enum::subtitle;
    else if(c.compare("text") == 0)
        return dash_enum::subtitle;
    else 
        return dash_enum::none;
}

dash_enum::dash_mime_type dash_handle::get_mime_type(std::string& m)
{
    if(m.find("video/mp4") != std::string::npos)
        return dash_enum::video_mp4;
    else if(m.find("audio/mp4") != std::string::npos)
        return dash_enum::audio_mp4;
    else if(m.find("application/mp4") != std::string::npos)
        return dash_enum::application_mp4;
    else if(m.find("video/mp2t") != std::string::npos)
        return dash_enum::video_mp2t;
    else if(m.find("application/ttml+xml") != std::string::npos)
        return dash_enum::ttml_xml;
    else 
        return dash_enum::m_none;
}


dash_enum::dash_content_type dash_handle::get_content_type_by_mime(dash_enum::dash_mime_type m)
{
    if(m == dash_enum::video_mp4)
        return dash_enum::video;
    else if(m == dash_enum::audio_mp4)
        return dash_enum::audio;
    else if(m == dash_enum::video_mp2t)
        //need to handle content component
        return dash_enum::c_ts;
    else if(m == dash_enum::m_none)
        return dash_enum::none;
    else 
        return dash_enum::subtitle;
}



dash_enum::dash_segment_info_type dash_handle::get_segment_info_type(bool b, bool l, bool t)throw (all_exceptions)
{
    if(l && t)
    {
        MPD_PARSER_DBG(("template and list exist together\n"));
        throw all_exceptions("get_segment_info_type error segment info1\n");
    }
    else if(b && l)
    {
        MPD_PARSER_DBG(("base and list exist together\n"));
        throw all_exceptions("get_segment_info_type error segment info2\n");
    }
    else if(b && t)
    {
        MPD_PARSER_DBG(("base and template exist together\n"));
        throw all_exceptions("get_segment_info_type error segment info3\n");
    }
    else if(b)
        return dash_enum::segment_base;
    else if(l)
        return dash_enum::segment_list;
    else if(t)
        return dash_enum::segment_template;
    else 
        return dash_enum::none_segment;
    
    
}
 

bool dash_handle::segment_info_valid(int pre, int self, int next)throw (all_exceptions)
{
	MPD_PARSER_DBG(("all segment info = %d, %d %d \n", pre, self, next));
    if(pre + self == 6 || pre + next == 6 || self + next == 6)
    {
        MPD_PARSER_DBG(("list and template exist\n"));
        throw all_exceptions("segment_info_valid  exist segment template and segment list\n");
    }
    else
    {
        return true;
    }
}

#if (MPD_PASER_DEBUG > 1)
void dash_handle::print_duration(altova::Duration* ptr)
{
    if(ptr == NULL)
    {
        return;
    }

	MPD_PARSER_DBG(("PT%dH%dM%lfS\n",ptr->Hours(), ptr->Minutes(), ptr->Seconds()));
}
void dash_handle::print_daytime(altova::DateTime* ptr)
{
    if(ptr == NULL)
    {
        return ;
    }

	MPD_PARSER_DBG(("%d-%d-%dT%d:%d:%lf\n", ptr->Year(), ptr->Month(),
		ptr->Day(), ptr->Hour(), ptr->Minute(), ptr->Second()));
}
#endif


