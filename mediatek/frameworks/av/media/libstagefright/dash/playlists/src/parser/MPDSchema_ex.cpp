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
#include <ctype.h>


#include "XercesString.h"
#include "dash_static.h"

// turn off warning: "this" used in base initializer list


namespace MPDSchema
{

    CSType_ex::CSType_ex()
    {
       r_ex = 0;
       t_ex = 0;
       d_ex = 0;

       r_exists = false;
       t_exists = false;
       d_exists = false;

       S_ptr = NULL;
    }
    

    CSType_ex::CSType_ex(CSType* ptr)
    {
        S_ptr = ptr;

        if(S_ptr != NULL)
        {
            if(S_ptr->t.exists())
            {
                t_ex = (unsigned)S_ptr->t;
                t_exists = true;

                //MPD_PARSER_DBG(("t = %lu ", t_ex))
            }
            else
            {
                t_ex = 0;
                t_exists = false;
            }

            if(S_ptr->r.exists())
            {
                r_ex = (unsigned)S_ptr->r;
                r_exists = true;

                //MPD_PARSER_DBG(("r = %lu ", r_ex))
            }
            else
            {
                r_ex = 0;
                r_exists = false;
            }
            
            if(S_ptr->d.exists())
            {
                d_ex = (unsigned)S_ptr->d;
                d_exists = true;

                //MPD_PARSER_DBG(("d = %lu\n", d_ex))
            }
            else
            {
                d_ex = 0;
                d_exists = false;
            }
        }
    }
     
    int CSType_ex::get_start_time_for_r(int r_index)
    {
        if(start_time_for_r.size() > r_index)
        {
            return start_time_for_r.at(r_index);
        }
        else
        {
            MPD_PARSER_DBG(("Error: r_index = %d size = %d\n", r_index, start_time_for_r.size()));
            return 0;
        }
    }

    void CSType_ex::set_start_time_for_r(unsigned int value)
    {
        start_time_for_r.push_back(value);
    }


    bool CSType_ex::t_present()
    {

        return t_exists;
    }

    bool CSType_ex::d_present()
    {       
        return d_exists;
    }


    bool CSType_ex::r_present()
    {
        return r_exists;
    }
    
    unsigned int CSType_ex::get_d()
    {
        return d_ex;
    }


    unsigned int CSType_ex::get_t()
    {
        return t_ex;
    }

    void CSType_ex::set_t(unsigned int value)
    {
        t_ex = value;
    }

    
    void CSType_ex::set_d(unsigned int value)
    {
       
       d_ex = value;
    }

    
    unsigned int CSType_ex::get_r()
    {
        if(r_present())
        {
            return r_ex;
        }

        return 0;
    }

    CSegmentTimelineType_ex::CSegmentTimelineType_ex(CSegmentTimelineType* ptr)
    {
        segmenttimeline_ptr = ptr;
    }

    void CSegmentTimelineType_ex::set_segmenttimeline_ptr(CSegmentTimelineType* ptr)
    {
        segmenttimeline_ptr = ptr;
        segmenttimeline_init(ptr);
    }
    void CSegmentTimelineType_ex::segmenttimeline_init(CSegmentTimelineType* ptr)
    {
        segmenttimeline_ptr = ptr;
        if(segmenttimeline_ptr == NULL)
        {
            MPD_PARSER_DBG(("segmenttime line no pointer \n"));
            return ;
        }

        unsigned int index = 0;

        if(segmenttimeline_ptr->S.exists())
        {
            for(; index < segmenttimeline_ptr->S.count(); ++ index)
            {
                
                CSType_ex s(&(segmenttimeline_ptr->S[index]));          
                s_list.push_back(s);
            }

            set_s_t_value();
        }
    }


    
    void CSegmentTimelineType_ex::set_s_t_value()
    {
        if(segmenttimeline_ptr == NULL)
        {
             MPD_PARSER_DBG(("no timeline pointer\n"));
             return ;
        }

        if(segmenttimeline_ptr->S.exists())
        {
            unsigned int index = 0;
            int pre_t = -1;
            int pre_d = -1;
        
            for(; index < s_list.size(); ++ index)
            {
                if(!s_list[index].d_present())
                {
                    MPD_PARSER_DBG(("no d value begin it is impossible\n"));
                    throw all_exceptions("set_s_t_value error, d value begin\n");
                }
            				
                if(s_list[index].t_present())
                {
                    MPD_PARSER_DBG(("t_present\n"));
                }
                else
                {
                    MPD_PARSER_DBG(("no t_present\n"));
                    if (index == 0)//the first S
                    {
						s_list[index].set_t(0);
					}
                    else
                    {
                        s_list[index].set_t(pre_t + pre_d);
					} 				
                }
				
				MPD_PARSER_DBG(("pre_t[%u] = %d, pre_d = %d\n", index, pre_t, pre_d));
				pre_t = s_list[index].get_t();
                pre_d = s_list[index].get_d();
                MPD_PARSER_DBG(("current[%u] t = %d, d = %d\n", index, pre_t, pre_d));
				
                s_list[index].set_start_time_for_r(pre_t);

                if(s_list[index].get_r() != 0)
                {
                    
                    int repeat = s_list[index].get_r();
					MPD_PARSER_DBG(("current[%u] repeat = %d\n", index, repeat));
					
                    while(repeat)
                    {
                        s_list[index].set_start_time_for_r(pre_t + pre_d);
                        pre_t = pre_t + pre_d;
                        --repeat;
                    }                   
                }
            }
        }
    }

    
    CRepresentationType_ex::CRepresentationType_ex():MPDSchema::CRepresentationBaseType_ex()
    {
        CRep_init();
        representationbase_ptr = NULL;
        representation_ptr = NULL;
    }

    void CRepresentationType_ex::CRep_init()
    {
         segment_info = dash_enum::none_segment;
         content_type_ex = dash_enum::none;
         segment_pos = dash_enum::pos_representation;
         timeline_exists = false;
		 width = 0;
		 height = 0;
		 width_exists = false;
		 height_exists = false;
    }

    CRepresentationType_ex::CRepresentationType_ex(CRepresentationType* ptr)
    {
        MPD_PARSER_DBG(("\n"));
        CRep_init();
        representation_ptr = ptr;
        representationbase_ptr = ptr;
    }

#if 0
    void CRepresentationType_ex::representation_process()throw(all_exceptions)
    {

        if(representation_ptr == NULL)
        {
            MPD_PARSER_DBG(("init Error, no pointer set\n"));
            return ;
        }
        
        switch(segment_info)
        {
            case dash_enum::segment_base:
            {
                if(segment_base.exists())
                {
                    segment_base.segmentbase_url_init();
                    segment_base.segmentbase_process();
                }
                else
                {
                    MPD_PARSER_DBG(("None segment base ptr\n"));
                    throw all_exceptions("No segment base ptr for segment base info");
                }
            }
            break;

            
            case dash_enum::segment_list:
            {
                if(segment_list.exists())
                {
                    segment_list.segmentbase_url_init();
                    segment_list.CSegmentListType_process();
                }
                else
                {
                    MPD_PARSER_DBG(("None segment list ptr\n"));
                    throw all_exceptions("No segment base ptr for segment list info");
                }
            }
            break;

            case dash_enum::segment_template:
            {
                if(segment_template.exists())
                {
                    segment_template.segmenttemplate_url_init();
                    segment_template.segmenttemplate_process();
                }
                else
                {
                    MPD_PARSER_DBG(("None segment template ptr\n"));
                    throw all_exceptions("No segment base ptr for segment template info");
                }
            }

            break;

            default:
                break;
        }
    }

#endif

	void CRepresentationType_ex::set_width_and_height(unsigned int width_, unsigned int height_, bool wid, bool hei)
	{
	    if(!width_exists && wid)
	    {
	        width = width_;
			width_exists = true;
	    }

		if(!height_exists && hei)
		{
		   height = height_;
		   height_exists = true;
		}
	}

    void CRepresentationType_ex::representation_init(CRepresentationType* ptr)throw(all_exceptions)
    {

        representationbase_init(ptr);
        representation_ptr = ptr;
        if(representation_ptr == NULL)
        {
            MPD_PARSER_DBG(("init Error, no pointer set\n"));
            return ;
        }
        segment_info = dash_handle::get_segment_info_type(representation_ptr->SegmentBase.exists(),
            representation_ptr->SegmentList.exists(), representation_ptr->SegmentTemplate.exists());

//
		MPD_PARSER_DBG(("segment_info = %d\n", segment_info));
        if((segment_info == dash_enum::segment_base) && representation_ptr->SegmentBase.count() > 0)
        {
            segment_base.set_segmentbase_ptr(&representation_ptr->SegmentBase[0]);
          //  segment_base.segmentbase_init();
        }
        else if((segment_info == dash_enum::segment_list) && representation_ptr->SegmentList.count() > 0)
        {
            segment_list.set_segmentlist_ptr(&representation_ptr->SegmentList[0]);
           // segment_list.CSegmentListType_init();
        }
        else if((segment_info == dash_enum::segment_template) && representation_ptr->SegmentTemplate.count() > 0)
        {
            segment_template.set_segmenttemplate_ptr(&representation_ptr->SegmentTemplate[0]);
            if(representation_ptr->SegmentTemplate[0].SegmentTimeline.exists())
            {
                timeline_exists = true;
            }
			else if(representation_ptr->SegmentTemplate[0].duration.exists())
			{
			    timeline_exists = true;
			}
            //segment_template.segmenttemplate_init();
        }

        content_type_ex = dash_handle::get_content_type_by_mime(mime_type);

        if(representation_ptr->bandwidth.exists())
        {
            bandwidth_ex = (unsigned int)representation_ptr->bandwidth;
            bandwidth_exists = true;
        }

        if(representation_ptr->id.exists())
        {
            id_ex = (std::string)representation_ptr->id;
            id_exists = true;
        }

		if(representation_ptr->width.exists())
		{
		    width_exists = true;
			width = (unsigned int)representation_ptr->width;
		}

		if(representation_ptr->height.exists())
		{
		    height = (unsigned int)representation_ptr->height;
			height_exists = true;
		}

		MPD_PARSER_DBG(("content_type_ex = %d bandwidth_ex = %lu id_ex = %s\n", content_type_ex,
			bandwidth_ex, id_ex.c_str()));
    }


    CRepresentationBaseType_ex::CRepresentationBaseType_ex()
    {
        mime_type = dash_enum::m_none;
        representationbase_ptr = NULL;
    }

    CRepresentationBaseType_ex::CRepresentationBaseType_ex(CRepresentationBaseType* ptr)
    {
        mime_type = dash_enum::m_none;
        representationbase_ptr = ptr;
    }
    

    void CRepresentationBaseType_ex::representationbase_init(CRepresentationBaseType* ptr)
    {
        representationbase_ptr = ptr;
        if(representationbase_ptr == NULL)
        {
            MPD_PARSER_DBG(("init Error, no pointer set\n"));
            return ;
        }
        
        if(representationbase_ptr->mimeType.exists())
        {
            std::string mime_str = (std::string)representationbase_ptr->mimeType;
            mime_type = dash_handle::get_mime_type(mime_str);
        }

		MPD_PARSER_DBG(("mime_type = %d\n", mime_type));
    }



    CSegmentBaseType_ex::CSegmentBaseType_ex()
    {
        segmentbase_ptr = NULL;
        init_type = dash_enum::DASH_INIT_NONE;
        timescale_ex = 1;
        presentationTimeOffset_ex = 0;
        indexRangeExact_ex = false;
        indexRange_ex = std::string("");

        timescale_exists = false;
        presentationTimeOffset_exists = false;
        indexRangeExact_exists = false;
        indexRange_exists = false;

		Initialisation_byteRange_ex = std::string("");
		Initialisation_byteRange_exists = false;
		Initialisation_ex = std::string("");
		Initialisation_exists = false;

		RepresentationIndex_exists = false;
		RepresentationIndex_ex = std::string("");
		RepresentationIndex_byteRange_exists = false;
		RepresentationIndex_byteRange_ex = std::string("");
    }


    CSegmentBaseType_ex::CSegmentBaseType_ex(CSegmentBaseType* ptr)
    {
        segmentbase_ptr = ptr;
        init_type = dash_enum::DASH_INIT_NONE;
        timescale_ex = 1;
        presentationTimeOffset_ex = 0;
        indexRangeExact_ex = false;
        indexRange_ex = std::string("");

        timescale_exists = false;
        presentationTimeOffset_exists = false;
        indexRangeExact_exists = false;
        indexRange_exists = false;

		Initialisation_byteRange_ex = std::string("");
		Initialisation_byteRange_exists = false;
		Initialisation_ex = std::string("");
		Initialisation_exists = false;

		RepresentationIndex_exists = false;
		RepresentationIndex_ex = std::string("");
		RepresentationIndex_byteRange_exists = false;
		RepresentationIndex_byteRange_ex = std::string("");

		Initialisation_sourceURL_exists = false;
		RepresentationIndex_sourceURL_exists = false;
    }



    void CSegmentBaseType_ex::segmentbase_init(CSegmentBaseType* ptr)
    {
        segmentbase_ptr = ptr;
        if(segmentbase_ptr == NULL)
        {
            MPD_PARSER_DBG(("No segment base pointer\n"));
            return;
        }
        
        if(segmentbase_ptr->Initialisation.exists() || segmentbase_ptr->Initialization.exists())
        {
            init_type = dash_enum::DASH_INIT_STRUCT;
			Initialisation_exists = true;

			if(segmentbase_ptr->Initialisation.exists())
			{
			    if(segmentbase_ptr->Initialisation[0].sourceURL.exists())
			    {
			        Initialisation_sourceURL_exists = true;
			        Initialisation_ex = (std::string)segmentbase_ptr->Initialisation[0].sourceURL;
			    }

				if(segmentbase_ptr->Initialisation[0].range.exists())
				{
				    Initialisation_byteRange_ex = (std::string)segmentbase_ptr->Initialisation[0].range;
					Initialisation_byteRange_exists = true;
				}
			}
			else
			{
			    if(segmentbase_ptr->Initialization[0].sourceURL.exists())
			    {
			        Initialisation_ex = (std::string)segmentbase_ptr->Initialization[0].sourceURL;
			    }

				if(segmentbase_ptr->Initialization[0].range.exists())
				{
				    Initialisation_byteRange_ex = (std::string)segmentbase_ptr->Initialization[0].range;
					Initialisation_byteRange_exists = true;
				}
			}
        }


		MPD_PARSER_DBG(("Initialisation_ex = %s, Initialisation_byteRange_ex = %s\n", Initialisation_ex.c_str(),Initialisation_byteRange_ex.c_str()));

		if(segmentbase_ptr->RepresentationIndex.exists())
		{
		    RepresentationIndex_exists = true;

			if(segmentbase_ptr->RepresentationIndex[0].sourceURL.exists())
			{
			    RepresentationIndex_sourceURL_exists = true;
		    	RepresentationIndex_ex = (std::string)segmentbase_ptr->RepresentationIndex[0].sourceURL;
			}
			if(segmentbase_ptr->RepresentationIndex[0].range.exists())
			{
			    RepresentationIndex_byteRange_exists = true;
		        RepresentationIndex_byteRange_ex = (std::string)segmentbase_ptr->RepresentationIndex[0].range;
			}
		}

		MPD_PARSER_DBG(("RepresentationIndex_ex = %s, RepresentationIndex_byteRange_ex = %s\n", 
			RepresentationIndex_ex.c_str(),RepresentationIndex_byteRange_ex.c_str()));
        if(segmentbase_ptr->timescale.exists())
        {
            timescale_ex = segmentbase_ptr->timescale;
            timescale_exists = true;
        }

        if(segmentbase_ptr->presentationTimeOffset.exists())
        {
            presentationTimeOffset_ex = segmentbase_ptr->presentationTimeOffset;
            presentationTimeOffset_exists = true;
        }


        if(segmentbase_ptr->indexRange.exists())
        {
            
            indexRange_ex = segmentbase_ptr->indexRange;
            indexRange_exists = true;
        }

        if(segmentbase_ptr->indexRangeExact.exists())
        {
            indexRangeExact_ex = segmentbase_ptr->indexRangeExact;
            indexRangeExact_exists = true;
        }

		MPD_PARSER_DBG(("timescale = %lu, presentationTimeOffset_ex = %lu indexRange_ex = %s\n",
			timescale_ex, presentationTimeOffset_ex, indexRange_ex.c_str()));
    }
    

    void CSegmentBaseType_ex::segmentbase_url_init()
    {
       if(segmentbase_ptr == NULL)
        {
            MPD_PARSER_DBG(("No segment base pointer\n"));
            return;
        }
        if(segmentbase_ptr->Initialisation.exists() || segmentbase_ptr->Initialization.exists())
        {
            init_type = dash_enum::DASH_INIT_STRUCT;
        } 
    }


    std::string CSegmentBaseType_ex::get_segmentbase_url()
    {
        if(segmentbase_ptr == NULL)
        {
            MPD_PARSER_DBG(("No segment base pointer\n"));
            return std::string("");
        }
        
        switch(init_type)
        {
            case dash_enum::DASH_INIT_STRUCT:
            {
                if(segmentbase_ptr->Initialisation.exists())
                {
                    if(segmentbase_ptr->Initialisation[0].sourceURL.exists())
                    {
                        return segmentbase_ptr->Initialisation[0].sourceURL;
                    }
                }
                else if(segmentbase_ptr->Initialization.exists())
                {
                    if(segmentbase_ptr->Initialization[0].sourceURL.exists())
                    {
                        return segmentbase_ptr->Initialization[0].sourceURL;
                    }
                }
                
                return std::string("");
            }
            default:
                return std::string("");
        }
    }
    
    std::string CSegmentBaseType_ex::get_segmentbase_range()
    {
        if(segmentbase_ptr == NULL)
        {
            MPD_PARSER_DBG(("No segment base pointer\n"));
            return std::string("");
        }
        
        switch(init_type)
        {
            case dash_enum::DASH_INIT_STRUCT:
            {
                if(segmentbase_ptr->Initialisation.exists())
                {
                    if(segmentbase_ptr->Initialisation[0].range.exists())
                    {
                        return segmentbase_ptr->Initialisation[0].range;
                    }
                }
                else if(segmentbase_ptr->Initialization.exists())
                {
                    if(segmentbase_ptr->Initialization[0].range.exists())
                    {
                        return segmentbase_ptr->Initialization[0].range;
                    }
                }
                
                return std::string("");
            }
            default:
                return std::string("");
        }
    }

#if 0
    void CSegmentBaseType_ex::segmentbase_process()
    {
        if(segmentbase_ptr == NULL)
        {
            MPD_PARSER_DBG(("No segment base pointer\n"));
            return ;
        }
        
        if(!segmentbase_ptr->timescale.exists())
        {
            segmentbase_ptr->timescale = 1;
        }
    }
#endif


    void CSegmentBaseType_ex::merge_segmentbase_info(CSegmentBaseType* segmentbase_info, bool rev)
    {
        if(segmentbase_ptr == NULL)
        {
            MPD_PARSER_DBG(("No segment base pointer\n"));
            return ;
        }
        
        if(rev)
        {
            if(!segmentbase_ptr->timescale.exists() && segmentbase_info->timescale.exists())
            {
                segmentbase_ptr->timescale = (unsigned)segmentbase_info->timescale;
            }

            if(!segmentbase_ptr->presentationTimeOffset.exists() && segmentbase_info->presentationTimeOffset.exists())
            {
                segmentbase_ptr->presentationTimeOffset = (unsigned)segmentbase_info->presentationTimeOffset;
            }

            if(!segmentbase_ptr->indexRange.exists() && segmentbase_info->indexRange.exists())
            {
               segmentbase_ptr->indexRange = (std::string)segmentbase_info->indexRange;
            }

            if(!segmentbase_ptr->indexRangeExact.exists() && segmentbase_info->indexRangeExact.exists())
            {
                segmentbase_ptr->indexRangeExact = (bool)segmentbase_info->indexRangeExact;
            }
        }
        else
        {
            if(segmentbase_info->timescale.exists())
            {
                segmentbase_ptr->timescale = (unsigned)segmentbase_info->timescale;
            }

            if(segmentbase_info->presentationTimeOffset.exists())
            {
                segmentbase_ptr->presentationTimeOffset = (unsigned)segmentbase_info->presentationTimeOffset;
            }

            if(segmentbase_info->indexRange.exists())
            {
               segmentbase_ptr->indexRange = (std::string)segmentbase_info->indexRange;
            }

            if(segmentbase_info->indexRangeExact.exists())
            {
                segmentbase_ptr->indexRangeExact = (bool)segmentbase_info->indexRangeExact;
            }
        }
    }


    CMultipleSegmentBaseType_ex::CMultipleSegmentBaseType_ex()
    {
        is_set = false;
        cmultisegmentbase_ptr = NULL;
        duration_ex = 0;
        startNumber_ex = 1;

        duration_exists = false;
        startNumber_exists = false;

		BitstreamSwitching_byteRange_ex = std::string("");
		BitstreamSwitching_byteRange_exists = false;
		BitstreamSwitching_ex = std::string("");
		BitstreamSwitching_exists = false;
    }
    
    CMultipleSegmentBaseType_ex::CMultipleSegmentBaseType_ex(CMultipleSegmentBaseType* ptr):CSegmentBaseType_ex(ptr)
    {
        is_set = false;
        cmultisegmentbase_ptr = ptr;
        duration_ex = 0;
        startNumber_ex = 1;
        duration_exists = false;
        startNumber_exists = false;

        if(ptr != NULL && ptr->SegmentTimeline.exists())
        {
            segmenttimeline_ex.set_segmenttimeline_ptr(&ptr->SegmentTimeline[0]);
        }

		BitstreamSwitching_byteRange_ex = std::string("");
		BitstreamSwitching_byteRange_exists = false;
		BitstreamSwitching_ex = std::string("");
		BitstreamSwitching_exists = false;
    }

    
    void CMultipleSegmentBaseType_ex::cmultisegmentbase_init(CMultipleSegmentBaseType* ptr, bool flag)
    {
        cmultisegmentbase_ptr = ptr;
        
        if(cmultisegmentbase_ptr == NULL)
        {
            MPD_PARSER_DBG(("no multisemgnetbase ptr\n"));
            return;
        }


        if(cmultisegmentbase_ptr->SegmentTimeline.exists())
        {
            segmenttimeline_ex.set_segmenttimeline_ptr(&cmultisegmentbase_ptr->SegmentTimeline[0]);
			is_set = true;
        }
        

        if(cmultisegmentbase_ptr->duration.exists())
        {
            duration_ex = cmultisegmentbase_ptr->duration;
            duration_exists = true;
        }

		if(is_set && duration_exists)
		{
			MPD_PARSER_DBG(("duartion and time line exist same time\n"));
		    throw all_exceptions("duration and time line is together\n");
		}

		if(flag && is_set)
		{
		    MPD_PARSER_DBG(("segment list has timeline error\n"));
		    throw all_exceptions("segment list has timeline error\n");
		}

        if(cmultisegmentbase_ptr->startNumber.exists())
        {
            startNumber_ex = cmultisegmentbase_ptr->startNumber;
            startNumber_exists = true;
        }

		MPD_PARSER_DBG(("duration_ex = %d, startNumber = %d\n", duration_ex, startNumber_ex));
		if(cmultisegmentbase_ptr->BitstreamSwitching.exists())
		{
		    BitstreamSwitching_exists = true;
			if(cmultisegmentbase_ptr->BitstreamSwitching[0].sourceURL.exists())
			{
			    BitstreamSwitching_ex = (std::string)cmultisegmentbase_ptr->BitstreamSwitching[0].sourceURL;
			}

			if(cmultisegmentbase_ptr->BitstreamSwitching[0].range.exists())
			{
			    BitstreamSwitching_byteRange_exists = true;
				BitstreamSwitching_byteRange_ex = (std::string)cmultisegmentbase_ptr->BitstreamSwitching[0].range;
			}
		}

		 segmentbase_init(ptr);

		MPD_PARSER_DBG(("BitstreamSwitching_ex = %s BitstreamSwitching_byteRange_ex = %s\n", BitstreamSwitching_ex.c_str(),
			BitstreamSwitching_byteRange_ex.c_str()));
    }

#if 0    
    void CMultipleSegmentBaseType_ex::cmultisegmentbase_process()
    {
        segmentbase_process();
        if(cmultisegmentbase_ptr == NULL)
        {
            MPD_PARSER_DBG(("No multi segment base pointer\n"));
            return ;
        }
        
        if(!cmultisegmentbase_ptr->startNumber.exists())
        {
            cmultisegmentbase_ptr->startNumber = 1;
            MPD_PARSER_DBG(("...start number  = 1\n"));
        }

        // then time line
        if(cmultisegmentbase_ptr->duration.exists() && cmultisegmentbase_ptr->SegmentTimeline.exists())
        {
            MPD_PARSER_DBG(("in template, timeline and duration exist together\n"));
            throw all_exceptions("segmentTimeline_init error 1\n");
        }

        if(cmultisegmentbase_ptr->SegmentTimeline.exists() && cmultisegmentbase_ptr->SegmentTimeline.count() > 0 && !is_set)
        {
            segmenttimeline_ex.set_s_t_value();
            is_set = true;
        }
        
    }
#endif

    void CSegmentListType_ex::CSegmentListType_init(CSegmentListType* ptr)
    {
        cmultisegmentbase_init(ptr, true);
    }


    CSegmentListType_ex::CSegmentListType_ex(CSegmentListType* ptr):CMultipleSegmentBaseType_ex(ptr)
    {
        segmentlist_ptr = ptr;
    }


    void CSegmentListType_ex::set_segmentlist_ptr(CSegmentListType* ptr)
    {
        segmentlist_ptr = ptr;
        cmultisegmentbase_ptr = ptr;
        segmentbase_ptr = ptr;

        CSegmentListType_init(ptr);
    }

    
    CSegmentTemplateType_ex::CSegmentTemplateType_ex(CSegmentTemplateType* ptr):CMultipleSegmentBaseType_ex(ptr)
    {
        segmenttemplate_ptr = ptr;
        index_ex = std::string("");
        index_exists = false;
        media_ex = std::string("");
        media_exists = false;
        initialisation_ex = std::string("");
        initialisation_exists = false;
        bistreamswitch_ex = std::string("");
        bistreamswitch_exists = false;
		number_start_indx = -1;
		number_end_indx = -1;
    }

    CSegmentTemplateType_ex::CSegmentTemplateType_ex()
    {
        segmenttemplate_ptr = NULL;
        index_ex = std::string("");
        index_exists = false;
        media_ex = std::string("");
        media_exists = false;
        initialisation_ex = std::string("");
        initialisation_exists = false;
        bistreamswitch_ex = std::string("");
        bistreamswitch_exists = false;
		number_start_indx = -1;
		number_end_indx = -1;
    }

    void CSegmentTemplateType_ex::set_segmenttemplate_ptr(CSegmentTemplateType* ptr)
    {
        segmentbase_ptr = ptr;
        segmenttemplate_ptr = ptr;
        cmultisegmentbase_ptr = ptr;
        segmenttemplate_init(ptr);
    }
    std::string CSegmentTemplateType_ex::handle_media_url(std::string& media, std::string rep_id, int num, long long bandwidth, long time)throw (all_exceptions)
    {
        return handle_url(media, rep_id, num, bandwidth, time);
    }
    
    std::string CSegmentTemplateType_ex::handle_index_url(std::string& index, std::string rep_id, long long bandwidth)throw (all_exceptions)
    {
         return handle_url(index, rep_id, -1, bandwidth, -1);
    }
    
    std::string CSegmentTemplateType_ex::handle_initialisation_url(std::string& initialisation, std::string rep_id, long long bandwidth)throw (all_exceptions)
    {
        return handle_url(initialisation, rep_id, -1, bandwidth, -1);
    }
    
    std::string CSegmentTemplateType_ex::handle_bitstream_url(std::string& bitstream, std::string rep_id, long long bandwidth)throw (all_exceptions)
    {
        return handle_url(bitstream, rep_id, -1, bandwidth, -1);
    }

    std::string CSegmentTemplateType_ex::handle_url(std::string handle, std::string rep_id, int num, long long bandwidth, long time)throw (all_exceptions)
    {
         std::string temp = std::string("");
        std::string str_temp = std::string("");


        std::string::size_type before  = std::string::npos;

        while((before = handle.find('$')) != std::string::npos)
        {
                if(before > 0)
                {
                    temp.append(handle.substr(0, before));
                }

                handle = handle.substr(before + 1);

                if(handle.find('$') == 0)
                {
                    handle = handle.substr(1);
                    temp.append("$");
                }
                else if(handle.find(dash_enum::REP_ID) == 0 && rep_id.length() > 0)
                {
                    handle = handle.substr(dash_enum::REP_ID.length());
                    str_temp = handle_width(handle, rep_id);
                    temp.append(str_temp);
                }
                else if(handle.find(dash_enum::NUMBER) == 0 && num >= 0)
                {
                    char buffer[dash_static::BUFFER_LEN];
                    sprintf(buffer, "%d", num);
                    std::string number = std::string(buffer);
                    handle = handle.substr(dash_enum::NUMBER.length());
                    str_temp = handle_width(handle, number);
					number_start_indx = -1;
					number_end_indx = -1;
					number_start_indx = (int)(temp.size());
                    temp.append(str_temp);
					number_end_indx = (int)(temp.size() - 1);
					//MPD_PARSER_DBG(("url number start index == %d, end index == %d\n", number_start_indx, number_end_indx));
                }
                else if(handle.find(dash_enum::BANDWIDTH) == 0 && bandwidth >= 0)
                {
                    char buffer[dash_static::BUFFER_LEN];
                    sprintf(buffer, "%d", bandwidth);
                    std::string band = std::string(buffer);
                    handle = handle.substr(dash_enum::BANDWIDTH.length());
                    str_temp = handle_width(handle, band);
                    temp.append(str_temp);
                }
                else if(handle.find(dash_enum::TIME) == 0 && time >= 0)
                {
                    char buffer[dash_static::BUFFER_LEN];
                    sprintf(buffer, "%ld", time);
                    std::string t = std::string(buffer);
                    handle = handle.substr(dash_enum::TIME.length());
                    str_temp = handle_width(handle, t);
                    temp.append(str_temp);
                }
                else
                {
                    //throw exception here.
                    MPD_PARSER_DBG(("url invalid format type after $\n"));
                    throw all_exceptions("handle_url invalid format type after $\n");
                }
        }

        temp.append(handle);

        return temp;
    }
    
    std::string CSegmentTemplateType_ex::handle_width(std::string& handle, std::string substr) throw (all_exceptions)
    {
        std::string::size_type first;
        std::string::size_type last;
        
        if(handle.length() == 0 || substr.length() == 0)
        {
            return substr;
        }

        if((first = handle.find('%')) != 0)
        {
            //no width parameter
            if((first = handle.find('$')) != 0)
            {
                MPD_PARSER_DBG(("can't not find next $\n"));
                throw all_exceptions("handle_width can't find $ in next\n");
            }
            else
            {
                handle = handle.substr(1);
            }
            return substr;
        }
        else
        {
            if((last = handle.find('$')) == std::string::npos)
            {
                MPD_PARSER_DBG(("can't not find finally $\n"));
                throw all_exceptions("handle_width can't find finally $ in handle_width\n");
            }

            if(handle.at(last-1) != 'd')
            {
                MPD_PARSER_DBG(("can't not find d character\n"));
                throw all_exceptions("handle_width can't find d character in handle_width\n");
            }

            if(first + 1 >= last -2 )
            {
                MPD_PARSER_DBG(("can't not find width number\n"));
                throw all_exceptions("handle_width can't find width number in handle_width\n");
            }
            
            std::string width = handle.substr(first + 1, last - 2 - first);
            
            handle = handle.substr(last + 1);
            unsigned int i;

            for(i = 0; i < width.length(); ++ i)
            {
                if(isdigit(width.at(i)) == 0)
                {
                    MPD_PARSER_DBG(("can't not parser invalid width \n"));
                    throw all_exceptions("handle_width, invalid width defined in substr\n");
                }
            }

            if(atoi(width.c_str()) == 0 || (unsigned int)atoi(width.c_str()) < substr.length())
            {
                return substr;
            }
            else
            {
                int data = atoi(substr.c_str());
                int len = atoi(width.c_str());
                char buf[dash_static::BUFFER_LEN];
                sprintf(buf, "%d", len);
                std::string t = std::string(buf);
                
                char buffer[dash_static::BUFFER_LEN];
                std::string temp = std::string("%0");
                t.append("d");
                temp.append(t);
                sprintf(buffer,temp.c_str(),data);
                substr = std::string(buffer);
                return substr;
            }
        }
    }


    std::string CSegmentTemplateType_ex::get_init_url_string()
    {
        if(segmenttemplate_ptr == NULL)
        {
            MPD_PARSER_DBG(("error: no segment template pointer\n"));
            return std::string("");
        }
        
        switch(init_type)
        {
            case dash_enum::DASH_INIT_URL:
            {
                if(segmenttemplate_ptr->initialisation2.exists())
                {
                    return segmenttemplate_ptr->initialisation2;
                }
                else
                {
                    return segmenttemplate_ptr->initialization2;
                }
            }
            default:
                return get_segmentbase_url();
        }
    }


    void CSegmentTemplateType_ex::segmenttemplate_init(CSegmentTemplateType* ptr)
    {
        segmenttemplate_ptr = ptr;
        if(segmenttemplate_ptr == NULL)
        {
            MPD_PARSER_DBG(("error: no segment template pointer\n"));
            return ;
        }

        cmultisegmentbase_init(ptr, false);

        if(segmenttemplate_ptr->initialisation2.exists())
        {
            initialisation_ex = segmenttemplate_ptr->initialisation2;
            initialisation_exists = true;
        }

        if(segmenttemplate_ptr->initialization2.exists())
        {
            initialisation_ex = segmenttemplate_ptr->initialization2;
            initialisation_exists = true;
        }

		MPD_PARSER_DBG(("initialisation_ex = %s\n", initialisation_ex.c_str()));
        if(segmenttemplate_ptr->index.exists())
        {
            index_ex = segmenttemplate_ptr->index;
            index_exists = true;
        }

        if(segmenttemplate_ptr->media.exists())
        {
            media_ex = segmenttemplate_ptr->media;
            media_exists = true;
        }

        if(segmenttemplate_ptr->bitstreamSwitching2.exists())
        {
            bistreamswitch_ex = segmenttemplate_ptr->bitstreamSwitching2;
            bistreamswitch_exists = true;
        }

        MPD_PARSER_DBG(("index_ex = %s media_ex = %s bistreamswitch_ex = %s\n", index_ex.c_str(),
			media_ex.c_str(), bistreamswitch_ex.c_str()));
    }

    void CSegmentTemplateType_ex::segmenttemplate_url_init()
    {
        if(segmenttemplate_ptr == NULL)
        {
            MPD_PARSER_DBG(("error: no segment template pointer\n"));
            return ;
        }
        
        segmentbase_url_init();
        if(segmenttemplate_ptr->initialisation2.exists() || segmenttemplate_ptr->initialization2.exists())
        {
            init_type = dash_enum::DASH_INIT_URL;
        }
    }

#if 0
    void CSegmentTemplateType_ex::segmenttemplate_process()
    {
        cmultisegmentbase_process();
    }

#endif

    CAdaptationSetType_ex::CAdaptationSetType_ex()
    {
        segment_info = dash_enum::none_segment;
        content_type = dash_enum::none;
        adaptation_ptr = NULL;
        representationbase_ptr = NULL;
        timeline_exists = false;
		mimeType_exists = false;
		codec_exists = false;
		audioSampleRate_exists = false;
		frameRate_exists = false;
    }


    CAdaptationSetType_ex::CAdaptationSetType_ex(CAdaptationSetType* ptr):CRepresentationBaseType_ex(ptr)
    {
        MPD_PARSER_DBG(("\n"));
        segment_info = dash_enum::none_segment;
        content_type = dash_enum::none;
        adaptation_ptr = ptr;
        timeline_exists = false;
		mimeType_exists = false;
		codec_exists = false;
		audioSampleRate_exists = false;
		frameRate_exists = false;
    }


    
    void CAdaptationSetType_ex::AdaptationSet_init(CAdaptationSetType* ptr)throw(all_exceptions)
    {
        adaptation_ptr = ptr;
        
        if(adaptation_ptr == NULL)
        {
            MPD_PARSER_DBG(("Error empty adpatation set pointer\n"));
            return ;
        }

        representationbase_init(ptr);
        segment_info = dash_handle::get_segment_info_type(adaptation_ptr->SegmentBase.exists(),
        adaptation_ptr->SegmentList.exists(), adaptation_ptr->SegmentTemplate.exists());

		if(adaptation_ptr->mimeType.exists())
		{
		    mimeType_exists = true;
			mime_str = (std::string)adaptation_ptr->mimeType;
		}

		if(adaptation_ptr->codecs.exists())
		{
		    codec_exists = true;
			codec_str = (std::string)adaptation_ptr->codecs;
		}

	    if(adaptation_ptr->audioSamplingRate.exists())
		{
		    audioSampleRate_exists = true;
			audioSampleRate_str = (std::string)adaptation_ptr->audioSamplingRate;
		}

		if(adaptation_ptr->frameRate.exists())
		{
		    frameRate_exists = true;
			frameRate_str = (std::string)adaptation_ptr->frameRate;
		}

		unsigned int width = 0;
		unsigned int height = 0;
		bool wid = false;
		bool hei = false;

		if(adaptation_ptr->width.exists())
		{
		    wid = true;
			width = (unsigned int)adaptation_ptr->width;
		}

		if(adaptation_ptr->height.exists())
		{
		    hei = true;
			height = (unsigned int)adaptation_ptr->height;
		}

        if(adaptation_ptr->Representation.exists())
        {
            unsigned int index = 0;

            for(; index < adaptation_ptr->Representation.count(); ++ index)
            {
            	if(!mimeType_exists && adaptation_ptr->Representation[index].mimeType.exists())
            	{
            	    mimeType_exists = true;
					mime_str = (std::string)adaptation_ptr->Representation[index].mimeType;
            	}

				if(!codec_exists && adaptation_ptr->Representation[index].codecs.exists())
            	{
            	    codec_exists = true;
				    codec_str = (std::string)adaptation_ptr->Representation[index].codecs;
            	}

				if(!audioSampleRate_exists && adaptation_ptr->Representation[index].audioSamplingRate.exists())
            	{
            	    audioSampleRate_exists = true;
					audioSampleRate_str = (std::string)adaptation_ptr->Representation[index].audioSamplingRate;
            	}

				if(!frameRate_exists && adaptation_ptr->Representation[index].frameRate.exists())
            	{
            	    frameRate_exists = true;
					frameRate_str = (std::string)adaptation_ptr->Representation[index].frameRate;
            	}
                CRepresentationType_ex rep;
                rep.representation_init(&(adaptation_ptr->Representation[index]));
				rep.set_width_and_height(width, height,wid, hei);
                representation_list.push_back(rep);
            }
        }

		if((mimeType_exists != false) &&(mime_str.length() == 0))
		{
			mime_str = std::string(" ");
			MPD_PARSER_DBG(("mime_str exist, but null, change to contain a space character ... \n"));
		}
		if((codec_exists != false) &&(codec_str.length() == 0))
		{
			codec_str = std::string(" ");
			MPD_PARSER_DBG(("codec_str exist, but null, change to contain a space character ... \n"));
		}
		if((audioSampleRate_exists != false) &&(audioSampleRate_str.length() == 0))
		{
			audioSampleRate_str = std::string(" ");
			MPD_PARSER_DBG(("audioSampleRate_str exist, but null, change to contain a space character ... \n"));
		}
		if((frameRate_exists != false) &&(frameRate_str.length() == 0))
		{
			frameRate_str = std::string(" ");
			MPD_PARSER_DBG(("frameRate_str exist, but null, change to contain a space character ... \n"));
		}
	  		
        MPD_PARSER_DBG(("segment_info = %d mime_str = %s\n", segment_info, mime_str.c_str()));
		MPD_PARSER_DBG(("segment_info = %d codec_str = %s\n", segment_info, codec_str.c_str()));
        MPD_PARSER_DBG(("segment_info = %d audioSampleRate_str = %s\n", segment_info, audioSampleRate_str.c_str()));
        MPD_PARSER_DBG(("segment_info = %d frameRate_str = %s\n", segment_info, frameRate_str.c_str()));

        if((segment_info == dash_enum::segment_base) && adaptation_ptr->SegmentBase.count() > 0)
        {
            segment_base.set_segmentbase_ptr(&(adaptation_ptr->SegmentBase[0]));
            //segment_base.segmentbase_init();
        }
        
        else if((segment_info == dash_enum::segment_list) && adaptation_ptr->SegmentList.count() > 0)
        {
            segment_list.set_segmentlist_ptr(&(adaptation_ptr->SegmentList[0]));
            //segment_list.CSegmentListType_init();
        }
        else if((segment_info == dash_enum::segment_template) && adaptation_ptr->SegmentTemplate.count() > 0)
        {
            segment_template.set_segmenttemplate_ptr(&(adaptation_ptr->SegmentTemplate[0]));
            if(adaptation_ptr->SegmentTemplate[0].SegmentTimeline.exists())
            {
                timeline_exists = true;
            }
			else if(adaptation_ptr->SegmentTemplate[0].duration.exists())
			{
			    timeline_exists = true;
			}

        }

   
        // content type
        // get by content type
        if(adaptation_ptr->contentType.exists())
        {
            std::string type_str = (std::string)adaptation_ptr->contentType;
            content_type = dash_handle::get_content_type(type_str);
        }

        // get by mime type
        if(content_type == dash_enum::none)
        {
           content_type = dash_handle::get_content_type_by_mime(mime_type);
        }

        // get by representation mime type
        if(content_type == dash_enum::none && !representation_list.empty())
        {
            unsigned int index = 0;
            for(; index < representation_list.size(); ++ index)
            {
                content_type = representation_list[index].get_content_type();

                if(content_type != dash_enum::none)
                    break;
            }
        }

		MPD_PARSER_DBG(("content_type = %d mime_type = %d\n", content_type, mime_type));


		if(adaptation_ptr->ContentProtection.exists())
		{
			MPD_PARSER_DBG(("1111\n"));
		    if(adaptation_ptr->ContentProtection[0].MarlinContentIds.exists())
		    {
		    	MPD_PARSER_DBG(("2222\n"));
		        if(adaptation_ptr->ContentProtection[0].MarlinContentIds[0].MarlinContentId.exists())
		        {
		            std::string id = adaptation_ptr->ContentProtection[0].MarlinContentIds[0].MarlinContentId[0];
					MPD_PARSER_DBG(("id = %s\n", id.c_str()));
		        }
		    }
		    
		}
    } 

    // TODO:: may list and list, template and template....
    void CAdaptationSetType_ex::merge_segment_info(CPeriodType_ex* period)throw(all_exceptions)
    {
        if(!representation_list.empty())
        {
            unsigned int index = 0;

            for(; index < representation_list.size(); ++ index)
            {
               // segment info:
               //MPD_PARSER_DBG((" segment info %lu %lu %lu\n", period->get_segment_info(), segment_info, representation_list[index].get_segment_info()));
               if(dash_handle::segment_info_valid(period->get_segment_info(),
                segment_info, representation_list[index].get_segment_info()))
               {

				dash_enum::dash_segment_position pos = dash_enum::pos_representation;
				
               	if(segment_info == dash_enum::none_segment 
               		&& representation_list[index].get_segment_info() == dash_enum::none_segment)
               	{
               		if(period->get_segment_info() != dash_enum::none_segment)
               		{
               		    representation_list[index].segment_pos = dash_enum::pos_peroiod;
               		}
               		continue;
               	}
               	
               	if(period->get_segment_info() == dash_enum::none_segment
               		&& segment_info == dash_enum::none_segment)
               	{
               		representation_list[index].segment_pos = dash_enum::pos_representation;
               		continue;
               	}
               	
               	if(period->get_segment_info() == dash_enum::none_segment
               		&& representation_list[index].get_segment_info() == dash_enum::none_segment)
               	{
               	    if(segment_info != dash_enum::none_segment)
               	    {
               	        representation_list[index].segment_pos = dash_enum::pos_adaptation;
               	    }
					
               		continue;
               	}

			      

				  if(segment_info == dash_enum::none_segment
				  	&& period->segment_info != dash_enum::none_segment)
				  {
				      pos = dash_enum::pos_peroiod;
				  }
				  else if(segment_info != dash_enum::none_segment
				  	&& period->segment_info == dash_enum::none_segment)
				  {
				      pos = dash_enum::pos_adaptation;
				  }
				  else if(segment_info != dash_enum::none_segment
				  	&& period->segment_info != dash_enum::none_segment)
				  {
				      if(segment_info == dash_enum::segment_base)
				      {
				          switch(period->segment_info)
				          {
				              case dash_enum::segment_base:
							  	{
									pos = dash_enum::pos_adaptation;
									dash_handle::merge_segment_base(&period->segment_base, &segment_base, true);
				              	}
							  break;

							  case dash_enum::segment_list:
							  	{
									pos = dash_enum::pos_peroiod;
									dash_handle::merge_segment_base(&period->segment_list, &segment_base, false);
										
							  	}
							  break;

							  case dash_enum::segment_template:
							  	{
									pos = dash_enum::pos_peroiod;
									dash_handle::merge_segment_base(&period->segment_template, &segment_base, false);
							  	}
							  break;

							  default:
							  	break;
				          }
				      }
					  else if(segment_info == dash_enum::segment_list)
					  {
					      pos = dash_enum::pos_adaptation;
						  
					      switch(period->segment_info)
					      {
					          case dash_enum::segment_base:
							  	{
									dash_handle::merge_segment_base(&period->segment_base, &segment_list, true);
					          	}
							  break;

							  case dash_enum::segment_list:
							  	{
									dash_handle::merge_segment_list(&period->segment_list, &segment_list);
							  	}
							  break;

							  default:
							  	break;
							  
					      }
					  }

					  else if(segment_info == dash_enum::segment_template)
					  {
					      switch(period->segment_info)
					      {
					          case dash_enum::segment_base:
							  	{
									pos = dash_enum::pos_adaptation;
									dash_handle::merge_segment_base(&period->segment_base, &segment_template, true);
					          	}
							  break;

							  case dash_enum::segment_template:
							  	{
									// time line is in adaptation set
				                     if(period->timeline_exists && !timeline_exists)
				                     {
				                         pos = dash_enum::pos_peroiod;
				                         dash_handle::merge_segment_template(&period->segment_template,&segment_template, false);
				                     }
				                     else if(!period->timeline_exists)
				                     {
				                         pos = dash_enum::pos_adaptation;
				                         dash_handle::merge_segment_template(&period->segment_template,&segment_template, true);
				                     }
							  	}
					      }
					  }
				  }
				  else 
				  {
				      MPD_PARSER_DBG(("none segment info in period and adpatationset\n"));
				  }

				  MPD_PARSER_DBG(("pos = %d\n", pos));

				  switch(representation_list[index].segment_info)
				  {
				      case dash_enum::segment_base:
					  {
						  switch(pos)
						  {
						      case dash_enum::pos_peroiod:
							  {
							  	   switch(period->segment_info)
							  	   	{
							  	   	    case dash_enum::segment_base:
										{
											dash_handle::merge_segment_base(&period->segment_base, &representation_list[index].segment_base, true);
							  	   	    }
										break;

										case dash_enum::segment_list:
										{
											representation_list[index].segment_pos = dash_enum::pos_peroiod;
											dash_handle::merge_segment_base(&period->segment_list, &representation_list[index].segment_base, false);
										}
										break;

										case dash_enum::segment_template:
										{
											representation_list[index].segment_pos = dash_enum::pos_peroiod;
											dash_handle::merge_segment_base(&period->segment_template, &representation_list[index].segment_base, false);
										}
										break;

										default:
											assert(0);
							  	   	}
								  
						      }
							  break;

							  case dash_enum::pos_adaptation:
							  {
							  	   switch(segment_info)
							  	   	{
							  	   	    case dash_enum::segment_base:
										{
											dash_handle::merge_segment_base(&segment_base, &representation_list[index].segment_base, true);
							  	   	    }
										break;

										case dash_enum::segment_list:
										{
											representation_list[index].segment_pos = dash_enum::pos_adaptation;
											dash_handle::merge_segment_base(&segment_list, &representation_list[index].segment_base, false);
										}
										break;

										case dash_enum::segment_template:
										{
											representation_list[index].segment_pos = dash_enum::pos_adaptation;
											dash_handle::merge_segment_base(&segment_template, &representation_list[index].segment_base, false);
										}
										break;

										default:
											assert(0);
							  	   	}
							  }
							  break;

							  default:
							  	break;
						  }
				      }

					  break;


					  case dash_enum::segment_list:
					  {
					  	   switch(pos)
						   {
						      case dash_enum::pos_peroiod:
							  {
							  	   switch(period->segment_info)
							  	   	{
							  	   	    case dash_enum::segment_base:
										{
											dash_handle::merge_segment_base(&period->segment_base, &representation_list[index].segment_list, true);
							  	   	    }
										break;

										case dash_enum::segment_list:
										{
											dash_handle::merge_segment_base(&period->segment_list, &representation_list[index].segment_list, true);
										}
										break;
										
										default:
											assert(0);
							  	   	}
								  
						      }
							  break;

							  case dash_enum::pos_adaptation:
							  {
							  	   switch(segment_info)
							  	   	{
							  	   	    case dash_enum::segment_base:
										{
											dash_handle::merge_segment_base(&segment_base, &representation_list[index].segment_list, true);
							  	   	    }
										break;

										case dash_enum::segment_list:
										{
											dash_handle::merge_segment_list(&segment_list, &representation_list[index].segment_list);
										}
										break;

										default:
											assert(0);
							  	   	}
							  }
							  break;

							  default:
							  	break;
						 }
					  }

					  break;

					  case dash_enum::segment_template:
					  	{
					  	   switch(pos)
						   {
						      case dash_enum::pos_peroiod:
							  {
							  	   switch(period->segment_info)
							  	   	{
							  	   	    case dash_enum::segment_base:
										{
											dash_handle::merge_segment_base(&period->segment_base, &representation_list[index].segment_template, true);
							  	   	    }
										break;

										case dash_enum::segment_template:
										{
											  if(period->timeline_exists && !representation_list[index].timeline_exists)
						                      {
						                          representation_list[index].segment_pos = dash_enum::pos_peroiod;
						                          dash_handle::merge_segment_template(&period->segment_template, &representation_list[index].segment_template, false);
						                          
						                      }
						                      else
						                      {
						                          dash_handle::merge_segment_template(&period->segment_template, &representation_list[index].segment_template, true);
						                      }
										}
										break;
										
										default:
											assert(0);
							  	   	}
								  
						      }
							  break;

							  case dash_enum::pos_adaptation:
							  {
							  	   switch(segment_info)
							  	   	{
							  	   	    case dash_enum::segment_base:
										{
											dash_handle::merge_segment_base(&segment_base, &representation_list[index].segment_template, true);
							  	   	    }
										break;

										case dash_enum::segment_template:
										{
											  if(timeline_exists && !representation_list[index].timeline_exists)
						                      {
						                          representation_list[index].segment_pos = dash_enum::pos_adaptation;
						                          dash_handle::merge_segment_template(&segment_template, &representation_list[index].segment_template, false);
						                          
						                      }
						                      else
						                      {
						                          dash_handle::merge_segment_template(&segment_template, &representation_list[index].segment_template, true);
						                      }
										}
										break;

										default:
											assert(0);
							  	   	}
							  }
							  break;

							  default:
							  	break;
						 }
					  }

					  break;

					  default:
					  	representation_list[index].segment_pos = pos;
						break;
				  }
               }
			   
            }


			index = 0;

			for(;index < representation_list.size(); ++ index)
			{
			    MPD_PARSER_DBG(("segment_info = %d  pos = %d\n", representation_list[index].segment_info, representation_list[index].segment_pos));
			}
        }

    }


    CPeriodType_ex::CPeriodType_ex()
    {
        start_time = -1;
        duration_time = -1;
		start_time_exists = false;
		duration_time_exists = false;
		
        segment_info = dash_enum::none_segment;
        period_ptr = NULL;
        timeline_exists = false;
    }

    CPeriodType_ex::CPeriodType_ex(CPeriodType* ptr)
    {
        MPD_PARSER_DBG(("\n"));
        start_time = -1;
        duration_time = -1;
		start_time_exists = false;
		duration_time_exists = false;
        segment_info = dash_enum::none_segment;
        
        period_ptr =  ptr;
        timeline_exists = false;
    }


    bool CPeriodType_ex::start_present()
    {
       return start_time_exists;
    } 


    bool CPeriodType_ex::duration_present()
    {
        return duration_time_exists;
    } 

    void CPeriodType_ex::set_start_present()
    {
       start_time_exists = true;
    } 


    void CPeriodType_ex::set_duration_present()
    {
       duration_time_exists = true;
    }
    
    void CPeriodType_ex::period_init(CPeriodType* ptr)throw(all_exceptions)
    {
        period_ptr = ptr;
        if(period_ptr  == NULL)
        {
           MPD_PARSER_DBG(("period init No pointer\n"));
           return ;
        }

        if(period_ptr->duration.exists())
        {
            duration_time = altova::MPD_Times::get_duration_seconds(&((altova::Duration)period_ptr->duration));
            MPD_PARSER_DBG(("duration_time = %lf\n", duration_time));
			duration_time_exists = true;
        }

        if(period_ptr->start.exists())
        {
            start_time = altova::MPD_Times::get_duration_seconds(&((altova::Duration)period_ptr->start));
            MPD_PARSER_DBG(("start_time = %lf\n", start_time));
			start_time_exists = true;
        }


         segment_info = dash_handle::get_segment_info_type(period_ptr->SegmentBase.exists(),
            period_ptr->SegmentList.exists(), period_ptr->SegmentTemplate.exists());

        MPD_PARSER_DBG(("segment_info = %d\n", segment_info));
        if((segment_info == dash_enum::segment_base) && period_ptr->SegmentBase.count() > 0)
        {
            segment_base.set_segmentbase_ptr(&(period_ptr->SegmentBase[0]));
            //segment_base.segmentbase_init();
        }
        else if((segment_info == dash_enum::segment_list) && period_ptr->SegmentList.count() > 0)
        {
            segment_list.set_segmentlist_ptr(&(period_ptr->SegmentList[0]));
            //segment_list.CSegmentListType_init();
        }
        else if((segment_info == dash_enum::segment_template) && period_ptr->SegmentTemplate.count() > 0)
        {
            segment_template.set_segmenttemplate_ptr(&period_ptr->SegmentTemplate[0]);
            if(period_ptr->SegmentTemplate[0].SegmentTimeline.exists())
            {
                timeline_exists = true;
            }
			else if(period_ptr->SegmentTemplate[0].duration.exists())
			{
			    timeline_exists = true;
			}
            //segment_template.segmenttemplate_init();
        }


   
        unsigned int index = 0;

        if(period_ptr->AdaptationSet.count() > 0)
        {
            for(; index < period_ptr->AdaptationSet.count(); ++ index)
            {
                CAdaptationSetType_ex adp;
                adp.AdaptationSet_init(&(period_ptr->AdaptationSet[index]));
                adaptation_list.push_back(adp);
            }
        }
        else
        {
            MPD_PARSER_DBG(("empty MPD init\n"));
            throw all_exceptions("empty MPD no need to init\n");
        }
    }


    dash_enum::dash_content_type CPeriodType_ex::get_content_type()
    {
        unsigned int index = 0;
        dash_enum::dash_content_type type = dash_enum::none;
        
        if(adaptation_list.size() > 0)
        {
            for(; index < adaptation_list.size(); ++ index)
            {
                type = adaptation_list[0].get_content_type();

                if(type != dash_enum::none)
                {
                    MPD_PARSER_DBG(("content type found %d\n", type));
                    return type;
                }
            }
        }

        MPD_PARSER_DBG(("no content type found\n"));
        return type;
    }

    void CPeriodType_ex::merge_segment_info()throw(all_exceptions)
    {
        if(!adaptation_list.empty())
        {
            unsigned int index = 0;

            for(; index < adaptation_list.size(); ++ index)
            {
                MPD_PARSER_DBG(("adaptationset segment info %d\n", adaptation_list[index].get_segment_info()));
                adaptation_list[index].merge_segment_info(this);
            }
        }
    }

#if 0
    void CPeriodType_ex::period_process()throw(all_exceptions)
    {
        if(!adaptation_list.empty())
        {
            unsigned int index = 0;

            for(; index < adaptation_list.size(); ++ index)
            {
                adaptation_list[index].AdaptationSet_process();
            }
        }
    }
#endif

    double CPeriodType_ex::get_start_time()
    {
        return get_start_time_ex();
    }
    
    double CPeriodType_ex::get_duration_time()
    {
         return get_duration_time_ex();
    }


    CMPDtype_ex::CMPDtype_ex()
    {
        CMPD_init();
    }

    void CMPDtype_ex::CMPD_init()
    {
        dynamic_type = dash_enum::MPD_TYPE_STATIC;
        profile_type = dash_enum::profile_none;
        mpd_total_time = -1;
        mpd_container = dash_enum::DASH_NONE;
        mpd_ptr = NULL;
        location_ex = std::string("");
        location_exists = false;

        availableendtime_ex = 0;
        availableendtime_exists = false;

        availablestarttime_ex = 0;
        availablestarttime_exists = false;

        minbuffertime_ex = 0;
        minbuffertime_exists = false;

        miniupdatetime_ex = 0;
        miniupdatetime_exists = false;

        timeshiftbufferdepth_exists = false;
        timeshiftbufferdepth_ex = 0;
        suggestedpresentationdelay_exists = false;
        suggestedpresentationdelay_ex = 0;

        isSpecialDynamic = false;
    }

    CMPDtype_ex::CMPDtype_ex(CMPDtype* ptr)
    {
        MPD_PARSER_DBG(("\n"));
        CMPD_init();
        mpd_ptr = ptr;
    }



    void CMPDtype_ex::mpd_init(MPDSchema::CMPDtype* ptr)throw(all_exceptions)
    {
        mpd_ptr = ptr;
        if(mpd_ptr == NULL)
        {
            MPD_PARSER_DBG(("mpd init no pointer\n"));
            return;
        }

         mpd_total_time = -1;
   
        if(mpd_ptr->mediaPresentationDuration.exists())
        {   
            mpd_total_time = altova::MPD_Times::get_duration_seconds(&((altova::Duration)mpd_ptr->mediaPresentationDuration));
			if (0 == mpd_total_time)
			{
				mpd_total_time = -1;
			}
        }
        else
        {
        	if(mpd_ptr->type.exists() && ((std::string)mpd_ptr->type).compare("dynamic") == 0)
        	{
				if ((mpd_ptr->minimumUpdatePeriod.exists()))
				{
					mpd_total_time = get_minimumUpdatePeriod();
					if (0 == mpd_total_time)
					{
						mpd_total_time = -1;
					}
					MPD_PARSER_DBG(("dynamic mpd, change total time to minimumUpdatePeriod(%lf)\n", mpd_total_time));
				}
        	}
			else
			{
				MPD_PARSER_DBG(("static no mediaPresentationDuration, mpd file format error \n"));
                throw all_exceptions("static no mediaPresentationDuration, mpd file format error \n");
			}
		}
        
        miniupdatetime_ex = get_minimumUpdatePeriod();
        minbuffertime_ex = get_minbufferTime();
        availableendtime_ex = get_availabilityEndTime();
        availablestarttime_ex = get_availabilityStartTime();

        if(mpd_ptr->suggestedPresentationDelay.exists())
        {
            suggestedpresentationdelay_exists= true;
            suggestedpresentationdelay_ex = altova::MPD_Times::get_duration_seconds(&((altova::Duration)mpd_ptr->suggestedPresentationDelay));
        }

        if(mpd_ptr->timeShiftBufferDepth.exists())
        {
            timeshiftbufferdepth_exists = true;
            timeshiftbufferdepth_ex = altova::MPD_Times::get_duration_seconds(&((altova::Duration)mpd_ptr->timeShiftBufferDepth));
        }


        if(mpd_ptr->Location.exists())
        {
            location_ex = (std::string)mpd_ptr->Location[0];
            location_exists = true;
            
        }

		MPD_PARSER_DBG(("location = %s\n", location_ex.c_str()));
        dynamic_type = dash_enum::MPD_TYPE_STATIC;

        if(mpd_ptr->type.exists() && ((std::string)mpd_ptr->type).compare("dynamic") == 0)
        {
            if(!mpd_ptr->minimumUpdatePeriod.exists())
            {
            	if (mpd_ptr->mediaPresentationDuration.exists())
            	{       	
                	dynamic_type = dash_enum::MPD_TYPE_DYNAMIC_NORMAL;
					miniupdatetime_ex = mpd_total_time;
					miniupdatetime_exists = true;
					isSpecialDynamic = true;
					MPD_PARSER_DBG(("miniupdateperiod is not exist, but mediaPresentationDuration exist, change miniupdatetime to %lf \n", miniupdatetime_ex));
            	}
				else
				{
					MPD_PARSER_DBG(("mediaPresentationDuration & minimumUpdatePeriod not exist \n"));
                    throw all_exceptions("mediaPresentationDuration & minimumUpdatePeriod not exist \n");
				}
            }
            else if(mpd_ptr->minimumUpdatePeriod.exists() && mpd_ptr->Period.count() > 0
                && mpd_ptr->Period[0].AdaptationSet.count() > 0)
            {
                dynamic_type = dash_enum::MPD_TYPE_DYNAMIC_NORMAL;
            }
            else if(mpd_ptr->minimumUpdatePeriod.exists() && mpd_ptr->Period.count() == 0)
            {
               dynamic_type = dash_enum::MPD_TYPE_DYNAMIC_EMPTY;
            }
            else if(mpd_ptr->minimumUpdatePeriod.exists() && mpd_ptr->Period.count() > 0
                && mpd_ptr->Period[0].AdaptationSet.count() == 0)
            {
                dynamic_type = dash_enum::MPD_TYPE_DYNAMIC_EMPTY;
            }
        }

		if(dynamic_type == dash_enum::MPD_TYPE_STATIC
			&& mpd_ptr->Period.count() == 0)
		{
		    MPD_PARSER_DBG(("no period in mpd\n"));
            throw all_exceptions("empty static mpd\n");
		}

		if(dynamic_type == dash_enum::MPD_TYPE_STATIC 
			&& mpd_ptr->Period.count() > 0 && mpd_ptr->Period[0].AdaptationSet.count() == 0)
		{
		    MPD_PARSER_DBG(("empty mpd no adaptation set\n"));
			 throw all_exceptions("empty static mpd\n");
		}


       if(!mpd_ptr->profiles.exists())
       {
           MPD_PARSER_DBG(("no profile defined\n"));
           throw all_exceptions("set_mpd_profile_type mpd profile undefined");
       }

       

       std::string profile = (std::string)mpd_ptr->profiles;

       
       if(profile.find(dash_enum::ISO_MAIN) != std::string::npos)
        {
            profile_type = dash_enum::iso_main;
        }
        else if(profile.find(dash_enum::ISO_DEMAND) != std::string::npos)
        {
            profile_type = dash_enum::iso_demand;
        }
        else if(profile.find(dash_enum::TS_MAIN) != std::string::npos)
        {
            profile_type = dash_enum::ts_main;
        }
        else if(profile.find(dash_enum::TS_SIMPLE) != std::string::npos)
        {
            profile_type = dash_enum::ts_simple;
        }
        else if(profile.find(dash_enum::ISO_LIVE) != std::string::npos)
        {
            profile_type = dash_enum::iso_live;
        }
        else if(profile.find(dash_enum::ISO_FULL) != std::string::npos)
        {
                // futhur handle, ts or iso
            #if 0
                if(!is_ts_type(mpd))
                    profile_type = dash_enum::iso_main;
                else
                    profile_type = dash_enum::ts_main;
            #endif
                profile_type = dash_enum::iso_main;
        }
		else if(profile.find(dash_enum::HBBTV_LIVE) != std::string::npos)
        {
                profile_type = dash_enum::hbbtv_live;
        }
        // or throw exceptions ....
        else
        {
            MPD_PARSER_DBG(("profile unrecognized\n"));
            throw all_exceptions("profile unrecognized\n");
        }


        if(dynamic_type != dash_enum::MPD_TYPE_STATIC && profile_type == dash_enum::iso_demand)
        {
            // in this case , change dynamic to static:
            if(dynamic_type == dash_enum::MPD_TYPE_DYNAMIC_NORMAL)
            {
                dynamic_type = dash_enum::MPD_TYPE_STATIC;
            }
            else
            {
                MPD_PARSER_DBG(("profile and type is not ok\n"));
                throw all_exceptions("profile is on-demand, type is static error\n");
            }
        }

        MPD_PARSER_DBG(("profile_type = %d, dynamic = %d\n", profile_type, dynamic_type));
        if(dynamic_type != dash_enum::MPD_TYPE_DYNAMIC_EMPTY)
        {
            unsigned int index = 0;
            double pre_start = -1;
            double pre_dur = -1;
            unsigned int end = mpd_ptr->Period.count();
			int isrollback = 0;

			MPD_PARSER_DBG(("the period count == %u ... \n", end));
			
			while (index < end)
            {            
               MPD_PARSER_DBG(("the period [%d] ... \n", index));
			   			   
			   if (index != 0)
			   {
					pre_start = period_list[index - 1].get_start_time();
                    pre_dur = period_list[index - 1].get_duration_time();
			   }
			   else
			   {
					pre_start = -1;
                    pre_dur = -1;
			   }

			   MPD_PARSER_DBG(("the previous period start_time = %lf, duration = %lf\n", pre_start, pre_dur));

			   if (isrollback == 1)
			   {
					isrollback = 0;
			   }
			   else
			   {
					CPeriodType_ex per_ex;
               		per_ex.period_init(&mpd_ptr->Period[index]);
               		per_ex.merge_segment_info();
               		period_list.push_back(per_ex);
			   }               
			   		
               // calculate period start time and duration here:
           
                if(period_list[index].start_present() && period_list[index].duration_present())
                {
                    MPD_PARSER_DBG(("the period start & duration exist ... \n"));
					if (index != 0) //not first
					{
						if (pre_start + pre_dur != period_list[index].get_start_time())
                        {
                            MPD_PARSER_DBG(("period[%d] start time != previow start+duration\n", index));
                            throw all_exceptions("MPD format is error -- <1> --\n");
                        }
					}
                }
                else if (period_list[index].duration_present())
                {
                	MPD_PARSER_DBG(("the period only duration exist ... \n"));
										             
                    if (index == 0)// first period, only duration exists
                    {
                        period_list[index].set_start_time(0);
                    }
					else
					{   
						if ((pre_start == -1) || (pre_dur == -1))
						{
							MPD_PARSER_DBG(("period[%d] no pre duration or pre start to caculate this start time\n", index));
                            throw all_exceptions("MPD format is error -- <2> --\n");
						}
						period_list[index].set_start_time(pre_start + pre_dur);	
					}

					period_list[index].set_start_present();
					
					MPD_PARSER_DBG(("period[%d] set start time(%lf) [present_flag == %s]... \n", index,
									 period_list[index].get_start_time(),
						             ((period_list[index].start_present() == true)?"true":"false")));
                }
                else if (period_list[index].start_present())
                {
                	MPD_PARSER_DBG(("the period only start exist ... \n"));

					if (end == 1) //only one period
					{
						if (mpd_total_time != -1)
						{
							period_list[index].set_duration_time(mpd_total_time);
						}
						else
						{
							MPD_PARSER_DBG(("the last period[%d] no duration and mediaPresentationDuration Error\n", index));
                            throw all_exceptions("MPD format is error -- <3> --\n");
						}						
					}
					else if (index == end - 1) //the last period
					{
						if (mpd_total_time != -1)
						{
                            double tmp_dur_0 = 0;
							tmp_dur_0 = mpd_total_time - period_list[index].get_start_time();
							period_list[index].set_duration_time(tmp_dur_0);
						}
						else
						{
							MPD_PARSER_DBG(("the last period[%d] no duration and mediaPresentationDuration Error\n", index));
                            throw all_exceptions("MPD format is error -- <3>' --\n");
						}
					}
					else
					{
						unsigned int next = index + 1;
						bool next_stfg = false;
						bool next_dtfg = false;						
						double next_start = 0;
						double next_duration = 0;
						double sum_duration = 0;

						if (mpd_ptr->Period[next].start.exists())
					    {
					        next_start = altova::MPD_Times::get_duration_seconds(&((altova::Duration)mpd_ptr->Period[next].start));
					        MPD_PARSER_DBG(("next period[%d]start_time = %lf\n", next, next_start));
							next_stfg = true;
					    }
			            if (mpd_ptr->Period[next].duration.exists())
					    {
					        next_duration = altova::MPD_Times::get_duration_seconds(&((altova::Duration)mpd_ptr->Period[next].duration));
					        MPD_PARSER_DBG(("next period[%d]duration_time = %lf\n", next, next_duration));
							next_dtfg = true;
					    }
						
						if ((!next_stfg) && (!next_dtfg))
						{
							MPD_PARSER_DBG(("period[%d] the next period no duration and start Error\n", index));
                            throw all_exceptions("MPD format is error -- <4> --\n");
						}
						else if (next_stfg) // the next period start time exist
						{
							double tmp_dur_1 = 0;
							tmp_dur_1 = next_start- period_list[index].get_start_time();
							period_list[index].set_duration_time(tmp_dur_1);
						}
						else if (next_dtfg) //the next period only duration time exist
						{				
							if (next == end - 1) //the next period is the last
							{
								if (mpd_total_time != -1)
								{
									double tmp_dur_2 = 0;
									tmp_dur_2 = mpd_total_time - next_duration - period_list[index].get_start_time();
									period_list[index].set_duration_time(tmp_dur_2);
								}
								else
								{
									MPD_PARSER_DBG(("the last period[%d] no mediaPresentationDuration Error\n", index));
		                            throw all_exceptions("MPD format is error -- <5> --\n");
								}
							}
							else
							{								
								unsigned int lookup = 0;
								unsigned int count = 1;
								bool next2stfg = false;
								bool next2dtfg = false; 
								double next2st = 0;
								double next2dt = 0;

								sum_duration += next_duration;
								
								for (lookup = next + 1; lookup < end; lookup++)
								{
									MPD_PARSER_DBG(("sum_duration (%lf)\n", sum_duration));
									next2stfg = false;
						            next2dtfg = false; 
								    next2st = 0;
								    next2dt = 0;
									
									if(mpd_ptr->Period[lookup].start.exists())
								    {
								        next2st = altova::MPD_Times::get_duration_seconds(&((altova::Duration)mpd_ptr->Period[lookup].start));
								        MPD_PARSER_DBG(("next2period[%d]start_time = %lf\n", lookup, next2st));
										next2stfg = true;
								    }
						            if(mpd_ptr->Period[lookup].duration.exists())
								    {
								        next2dt = altova::MPD_Times::get_duration_seconds(&((altova::Duration)mpd_ptr->Period[lookup].duration));
								        MPD_PARSER_DBG(("next2period[%d]duration_time = %lf\n", lookup, next2dt));
										next2dtfg = true;
								    }
									if ((!next2stfg)&&(!next2dtfg))
									{
										MPD_PARSER_DBG(("period[%d] the next-next period no duration and start Error\n", index));
                            			throw all_exceptions("MPD format is error -- <6> --\n");
									}
									else if (next2stfg)
									{
										double tmp_dur_3 = 0;
									    tmp_dur_3 = next2st - sum_duration - period_list[index].get_start_time();
										period_list[index].set_duration_time(tmp_dur_3);
										break;
									}
									else if (next2dtfg)
									{
										count++;
										sum_duration += next2dt;
									}
									else
									{
										//do nothing...
									}
								}

								if (lookup >= end)// the left periods are only duration
								{
									if (mpd_total_time != -1)
									{
										double tmp_dur_4 = 0;
										tmp_dur_4 = mpd_total_time - sum_duration - period_list[index].get_start_time();
										period_list[index].set_duration_time(tmp_dur_4);
									}
									else
									{
										MPD_PARSER_DBG(("period[%d] the left period no duration Error[count == %d]\n", index, count));
                            			throw all_exceptions("MPD format is error -- <7> --\n");
									}
								}																			
							}
						}
					}    
			
					period_list[index].set_duration_present();
					MPD_PARSER_DBG(("period[%d] set duration time(%lf) [present_flag == %s]... \n", index,
									period_list[index].get_duration_time(),
									((period_list[index].duration_present() == true)?"true":"false")));
					
                }
                else 
                {
					MPD_PARSER_DBG(("the period start&duration not exist ... \n"));
					if (index == 0)
					{
						period_list[index].set_start_time(0);
					}
					else
					{
						if ((pre_start == -1) || (pre_dur == -1))
						{
							MPD_PARSER_DBG(("period[%d] no pre duration or pre start to caculate this start time\n", index));
                            throw all_exceptions("MPD format is error -- <8> --\n");
						}
						period_list[index].set_start_time(pre_start + pre_dur);	
					}

					period_list[index].set_start_present();
					
					MPD_PARSER_DBG(("period[%d] set start time(%lf) [present_flag == %s]... \n", index,
									 period_list[index].get_start_time(),
						             ((period_list[index].start_present() == true)?"true":"false")));

					isrollback = 1;
					
				}

				index++;
				
				if (isrollback == 0)
				{
					MPD_PARSER_DBG(("the current period[%u] start_time = %lf, duration = %lf\n", index - 1, 
									period_list[index - 1].get_start_time(), period_list[index - 1].get_duration_time()));      
				}
				else
				{
					index -= 1;
					MPD_PARSER_DBG(("period[%d] can not calculate start or duration, we should rollback to calculate again... \n", index));
				}				
            }

            if(end != period_list.size())
            {
                MPD_PARSER_DBG(("vector number error\n"));
                throw all_exceptions("period_prepare this start time does not equal to previow start+duration\n");
            }
            /*
			if(mpd_ptr->mediaPresentationDuration.exists())
			{		
				MPD_PARSER_DBG(("mediaPresentationDuration = %lf\n", mpd_total_time));
			}
            else
            {
            	if (1 == end) //only one period
            	{
					mpd_total_time = period_list[end - 1].get_duration_time() - period_list[end - 1].get_start_time(); 
				}
            	else
            	{
					mpd_total_time = pre_start + pre_dur + period_list[end - 1].get_duration_time();
				}
				MPD_PARSER_DBG(("total time(pre_start + pre_dur + last duration) = %lf\n", mpd_total_time));
			}
			*/
        }
    }



    void CMPDtype_ex::mpd_process()throw(all_exceptions)
    {
        if(dynamic_type != dash_enum::MPD_TYPE_DYNAMIC_EMPTY)
        {
            unsigned int index = 0;
            unsigned int end = period_list.size();

            mpd_container = dash_enum::DASH_OTHER;

            for(; index < end; ++ index)
            {
                mpd_container = dash_handle::get_container_by_content_type(period_list[index].get_content_type());

                if(mpd_container != dash_enum::DASH_OTHER)
                {
                    MPD_PARSER_DBG(("container found %d\n", mpd_container));
                    break;
                }
            }

			if(mpd_container == dash_enum::DASH_OTHER)
			{
			    switch(profile_type)
			    {
			        case dash_enum::iso_demand:
					case dash_enum::iso_live:
					case dash_enum::iso_main:
					case dash_enum::hbbtv_live:
						mpd_container = dash_enum::DASH_MP4;
						break;
					case dash_enum::ts_main:
					case dash_enum::ts_simple:
						mpd_container = dash_enum::DASH_TS;
						break;

					default:
						break;
			    }
			}

			MPD_PARSER_DBG(("container = %d\n", mpd_container));
						
            for(index = 0; index < end; index++)
            {
              	double startTm = period_list[index].get_start_time();
				double durationTm = period_list[index].get_duration_time();
				
              	if (mpd_total_time < (startTm + durationTm))
              	{
					MPD_PARSER_DBG(("CAUTION !!! -- mpd total time is not match with the sum of period ....\n"));
					//period_list[index].set_duration_time(mpd_total_time - startTm);
					MPD_PARSER_DBG(("mpd_total_time = %f\n", mpd_total_time));
					MPD_PARSER_DBG(("period[%d] duration = %f\n", index, period_list[index].get_duration_time()));
					break;
				}
                MPD_PARSER_DBG(("period[%d] duration = %f\n", index, period_list[index].get_duration_time()));
            }
/*
			if (index + 1 < end)
			{
				MPD_PARSER_DBG(("remove period[%d] - [%d] from period_list ... \n", index + 1, end - 1));
				period_list.erase(period_list.begin() + index + 1, period_list.end());
			}
*/
        }
    }


    double CMPDtype_ex::get_minbufferTime()
    {
        if(mpd_ptr == NULL)
        {
             MPD_PARSER_DBG(("no mpd pointer\n"));
             return 0;
        }
        
        if(mpd_ptr->minBufferTime.exists())
        {
            minbuffertime_exists = true;
            return altova::MPD_Times::get_duration_seconds(&((altova::Duration)mpd_ptr->minBufferTime));
        }
        else
        {
            return 0;
        }
    }

    double CMPDtype_ex::get_minimumUpdatePeriod()
    {
       if(mpd_ptr == NULL)
        {
             MPD_PARSER_DBG(("no mpd pointer\n"));
             return 0;
        }
       
       if(mpd_ptr->minimumUpdatePeriod.exists())
       {
           miniupdatetime_exists = true;
           return altova::MPD_Times::get_duration_seconds(&((altova::Duration)mpd_ptr->minimumUpdatePeriod));
       }
       else
       {
           return 0;
       }
    }

    long long CMPDtype_ex::get_availabilityStartTime()
    {
        if(mpd_ptr == NULL)
        {
             MPD_PARSER_DBG(("no mpd pointer\n"));
             return 0;
        }
        
        if(mpd_ptr->availabilityStartTime.exists())
        {
            availablestarttime_exists = true;
            return altova::MPD_Times::get_daytime_seconds(&((altova::DateTime)mpd_ptr->availabilityStartTime));
        }
        else
        {
            return 0;
        }
    }

    long long CMPDtype_ex::get_availabilityEndTime()
    {
        if(mpd_ptr == NULL)
        {
             MPD_PARSER_DBG(("no mpd pointer\n"));
             return 0;
        }
        
        if(mpd_ptr->availabilityEndTime.exists())
        {
            availableendtime_exists = true;
            return altova::MPD_Times::get_daytime_seconds(&((altova::DateTime)mpd_ptr->availabilityEndTime));
        }
        else
        {
            return 0;
        }
    }


    #if (MPD_PASER_DEBUG > 1)
    void CMPDtype_ex::mpd_verify(CMPDtype* mpd)
    {
        //mpd_ptr = mpd;
        if(mpd == NULL)
        {
            MPD_PARSER_DBG(("Error: no MPD pointer for verify\n"));
            return;
        }

        
        
        MPD_PARSER_DBG(("mpd total time = %lf ", mpd_total_time));
        if(mpd->mediaPresentationDuration.exists())
        {
            MPD_PARSER_DBG(("mediaPresentationDuration = %lf ", altova::MPD_Times::get_duration_seconds(&((altova::Duration)mpd->mediaPresentationDuration))));
        }
        MPD_PARSER_DBG(("profile_type = %d, dynamic_type = %d\n", profile_type, dynamic_type));
        if(mpd->profiles.exists())
        {
            MPD_PARSER_DBG(("profiles = %s", ((std::string)mpd->profiles).c_str()));
        }

        if(mpd->type.exists())
        {
            MPD_PARSER_DBG(("type = %s\n", ((std::string)mpd->type).c_str()));
        }

        
        MPD_PARSER_DBG(("container = %d  location = %s\n", mpd_container, location_ex.c_str()));

        MPD_PARSER_DBG(("availstart = %lld, end = %lld minupdate = %lf minbuffer = %lf\n", availablestarttime_ex,
            availableendtime_ex, miniupdatetime_ex, minbuffertime_ex))

        if(mpd->availabilityStartTime.exists())
        {
            MPD_PARSER_DBG(("availblestart :"));
            dash_handle::print_daytime(&(altova::DateTime)mpd->availabilityStartTime);
        }

        if(mpd->availabilityEndTime.exists())
        {
            MPD_PARSER_DBG(("availbleend :"));
            dash_handle::print_daytime(&(altova::DateTime)mpd->availabilityEndTime);
        }

        if(mpd->minBufferTime.exists())
        {
            MPD_PARSER_DBG(("minBuffertime :"));
            dash_handle::print_duration(&(altova::Duration)mpd->minBufferTime);
        }

        if(mpd->minimumUpdatePeriod.exists())
        {
            MPD_PARSER_DBG(("miniUpdate :"));
            dash_handle::print_duration(&(altova::Duration)mpd->minimumUpdatePeriod);
        }

        if(mpd->mediaPresentationDuration.exists())
        {
            MPD_PARSER_DBG(("mediaPresentation :"));
            dash_handle::print_duration(&(altova::Duration)mpd->mediaPresentationDuration);
        }
        
        unsigned int period_number = period_list.size();

        MPD_PARSER_DBG(("period_number = %d \n", period_number));

        unsigned int index = 0;

        for(; index < period_number; ++ index)
        {
            period_list[index].period_verify(&(mpd->Period[index]));
        }
        MPD_PARSER_DBG(("MPD END\n"));
    }

    void CPeriodType_ex::period_verify(CPeriodType* period)
    {
        
        if(period == NULL)
        {
            MPD_PARSER_DBG(("\tError: no Period pointer for verify\n"));
            return;
        }

        MPD_PARSER_DBG(("\t:Period BEGIN\n"));
        unsigned number = adaptation_list.size();

        MPD_PARSER_DBG(("\tstart time = %lf, duration = %lf\n", start_time, duration_time));
        MPD_PARSER_DBG(("\tsegment_info = %d, adpatationset number = %d\n", segment_info, number));

        
#if 0
            switch(segment_info)
            {
                case dash_enum::segment_list:
                    segment_list.segmentlist_verify();
                break;
    
                case dash_enum::segment_template:
                    segment_template.segmenttemplate_verify();
                break;
    
                case dash_enum::segment_base:
                    segment_base.segmentbase_verify();
            }
#endif
        if(period->start.exists())
        {
            MPD_PARSER_DBG(("start: "));
            dash_handle::print_duration(&(altova::Duration)period->start);
        }

        if(period->duration.exists())
        {
            MPD_PARSER_DBG(("duration: "));
            dash_handle::print_duration(&(altova::Duration)period->duration);
        }

        unsigned index = 0;

        for(; index < number; ++ index)
        {
            adaptation_list[index].adaptationset_verify(&(period->AdaptationSet[index]));
            adaptation_list[index].drm_verify(&(period->AdaptationSet[index]));
        }

        MPD_PARSER_DBG(("\tPeriod END\n"));
    }

    void CAdaptationSetType_ex::drm_verify(CAdaptationSetType* adaptation)
    {
        if(adaptation_ptr == NULL || adaptation == NULL)
        {
            MPD_PARSER_DBG(("no drm info for verify\n"));
            return;
        }


        
        //MPD_PARSER_DBG(("adaptation_ptr = 0x%x\n", adaptation))
        unsigned int number = adaptation->ContentProtection.count();
        unsigned int index = 0;

        MPD_PARSER_DBG(("number = %d\n", number))

        for(; index < number; ++ index)
        {
            if(adaptation->ContentProtection[index].FormatVersion.exists())
            {
                //MPD_PARSER_DBG(("\n..."));
                if(adaptation->ContentProtection[index].FormatVersion[0].major.exists())
                {
                    MPD_PARSER_DBG(("major = %s\n", ((std::string)adaptation->ContentProtection[index].FormatVersion[0].major).c_str()));

                }

                if(adaptation->ContentProtection[index].FormatVersion[0].minor2.exists())
                {
                    MPD_PARSER_DBG(("minor = %s\n", ((std::string)adaptation->ContentProtection[index].FormatVersion[0].minor2).c_str()));

                }

                
            }

            if(adaptation->ContentProtection[index].MarlinContentIds.exists())
            {
                unsigned int ind = 0;
                for(; ind < adaptation->ContentProtection[index].MarlinContentIds[0].MarlinContentId.count(); ++ ind)
                {
                    MPD_PARSER_DBG(("Id : %s\n", ((std::string)adaptation->ContentProtection[index].MarlinContentIds[0].MarlinContentId[ind]).c_str()));
                }
            }

            if(adaptation->ContentProtection[index].MarlinBroadband.exists())
            {
                if(adaptation->ContentProtection[index].MarlinBroadband[0].SilentRightsUrl.exists())
                {
                    MPD_PARSER_DBG(("SilentRightsUrl %s\n", ((std::string)adaptation->ContentProtection[index].MarlinBroadband[0].SilentRightsUrl[0]).c_str()));
                }

                if(adaptation->ContentProtection[index].MarlinBroadband[0].PreviewRightsUrl.exists())
                {
                    MPD_PARSER_DBG(("PreviewRightsUrl %s\n", ((std::string)adaptation->ContentProtection[index].MarlinBroadband[0].PreviewRightsUrl[0]).c_str()));
                }

                if(adaptation->ContentProtection[index].MarlinBroadband[0].RightsIssuerUrl.exists())
                {
                    MPD_PARSER_DBG(("RightsIssuerUrl %s\n", ((std::string)adaptation->ContentProtection[index].MarlinBroadband[0].RightsIssuerUrl[0]).c_str()));
                }
            }

            if(adaptation->ContentProtection[index].MS3.exists())
            {
                if(adaptation->ContentProtection[index].MS3[0].URIsAreTemplate.exists())
                {
                    MPD_PARSER_DBG(("URIsAreTemplate %s\n", ((std::string)adaptation->ContentProtection[index].MS3[0].URIsAreTemplate[0]).c_str()));
                }
            }
        }
        
    }
    void CAdaptationSetType_ex::adaptationset_verify(CAdaptationSetType* adaptation)
    {
       

        if(adaptation == NULL)
        {
            MPD_PARSER_DBG(("\t\tError: no AdaptationSet pointer for verify\n"));
            return;
        }
        MPD_PARSER_DBG(("************************AdaptationSet BEGIN**********************\n"));
        representationbase_verify(adaptation);
        unsigned number = representation_list.size();
        MPD_PARSER_DBG(("\t\tcontent_type = %d segment_info = %d\n", content_type, segment_info));
        MPD_PARSER_DBG(("\t\trepresentation number = %d\n", number));

#if 0
        switch(segment_info)
        {
            case dash_enum::segment_list:
                segment_list.segmentlist_verify();
            break;

            case dash_enum::segment_template:
                segment_template.segmenttemplate_verify();
            break;

            case dash_enum::segment_base:
                segment_base.segmentbase_verify();
        }
#endif




        if(adaptation->lang.exists())
        {
            MPD_PARSER_DBG(("language = %s", ((std::string)adaptation->lang).c_str()));
        }

        if(adaptation->subsegmentAlignment.exists())
        {
            MPD_PARSER_DBG(("subsegmentalign = %s", ((std::string)adaptation->subsegmentAlignment).c_str()));
        }
        
        unsigned index = 0;
        for(; index < number; ++ index)
        {
            representation_list[index].representation_verify(&adaptation->Representation[index]);
        }
        
        MPD_PARSER_DBG(("************************AdaptationSet END**********************\n"));
        
    }

    void CRepresentationType_ex::representation_verify(CRepresentationType* ptr)
    {
       

        if(ptr == NULL)
        {
            MPD_PARSER_DBG(("\t\t\tError: no Representation pointer for verify\n"));
            return;
        }
        MPD_PARSER_DBG(("&&&&&&&&&&&&&&&&&&&&Representation BEGIN&&&&&&&&&&&&&&&&&&&&&&&&&&&\n"));
        representationbase_verify(ptr);

        MPD_PARSER_DBG(("\t\t\tsegment_info = %d, content_type_ex = %d id=%s\n", segment_info, content_type_ex, id_ex.c_str()));
        MPD_PARSER_DBG(("\t\t\t segment_Pos = %d bandwidth = %lu\n", segment_pos, bandwidth_ex));

#if 0
        switch(segment_info)
        {
            case dash_enum::segment_list:
                segment_list.segmentlist_verify();
            break;

            case dash_enum::segment_template:
                segment_template.segmenttemplate_verify();
            break;

            case dash_enum::segment_base:
                segment_base.segmentbase_verify();
        }
#endif
        if(ptr->bandwidth.exists())
        {
            MPD_PARSER_DBG(("bandwidth= %lu", (unsigned)ptr->bandwidth));
        }

        if(ptr->id.exists())
        {
            MPD_PARSER_DBG(("id= %s", ((std::string)ptr->id).c_str()));
        }
        MPD_PARSER_DBG(("&&&&&&&&&&&&&&&&&&&&Representation END&&&&&&&&&&&&&&&&&&&&&&&&&&&\n"));
    }


    void CSegmentListType_ex::segmentlist_verify()
    {
        
        if(segmentlist_ptr == NULL)
        {
            MPD_PARSER_DBG(("\t\t\t no segmentList pointer for verify\n"));
            return;
        }

        MPD_PARSER_DBG(("$$$$$$$$$$$$$$SegmentList BEGIN$$$$$$$$$$$$$$$$$$$$$$\n"));
        multisegmentbase_verify();
        MPD_PARSER_DBG(("$$$$$$$$$$$$$$SegmentList END$$$$$$$$$$$$$$$$$$$$$$\n"));
    }

    void CMultipleSegmentBaseType_ex::multisegmentbase_verify()
    {
        
        
        if(cmultisegmentbase_ptr == NULL)
        {
            MPD_PARSER_DBG(("\t\t\t no segmentList pointer for verify\n"));
            return;
        }
        
        MPD_PARSER_DBG(("$$$$$$$$$$$$$$MulitSegmentBase BEGIN$$$$$$$$$$$$$$$$$$$$$$\n"));
        segmentbase_verify();
        segmenttimeline_ex.segmenttimeline_verify();
        MPD_PARSER_DBG(("$$$$$$$$$$$$$$MulitSegmentBase BEGIN$$$$$$$$$$$$$$$$$$$$$$\n"));
    }

    void CSegmentTemplateType_ex::segmenttemplate_verify()
    {
        
        if(segmenttemplate_ptr == NULL)
        {
            MPD_PARSER_DBG(("\t\t\t no segmentList pointer for verify\n"));
            return;
        }
        MPD_PARSER_DBG(("~~~~~~~~~~~~~~~~~~~~~~~SegmentTemplate BEGIN~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"));
        multisegmentbase_verify();
        MPD_PARSER_DBG(("~~~~~~~~~~~~~~~~~~~SegmentTemplate END~~~~~~~~~~~~~~~~~~~~~~~~~~\n"));
    }

    void CSegmentBaseType_ex::segmentbase_verify()
    {
        
        if(segmentbase_ptr == NULL)
        {
            MPD_PARSER_DBG(("\t\t\t no segmentList pointer for verify\n"));
            return;
        }
        MPD_PARSER_DBG(("~~~~~~~~~~~~~~~~~~~~~~~SegmentBase BEGIN~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"));
        MPD_PARSER_DBG(("init type = %d\n", init_type));
        MPD_PARSER_DBG(("~~~~~~~~~~~~~~~~~~~SegmentBase END~~~~~~~~~~~~~~~~~~~~~~~~~~\n"));
    }


    void CSegmentTimelineType_ex::segmenttimeline_verify()
    {
       
        if(segmenttimeline_ptr == NULL)
        {
            MPD_PARSER_DBG(("\t\t\t no segment timeline pointer for verify\n"));
            return;
        }
        MPD_PARSER_DBG(("#################Segmenttimeline BEGIN#######################\n"));
        unsigned int number = s_list.size();
        MPD_PARSER_DBG(("number = %d\n", number));

        unsigned int index = 0;

        for(;index < number; ++ index)
        {
            s_list[index].s_verify();
        }
        MPD_PARSER_DBG(("###################Segmenttimeline END######################\n"));
    }


    void CSType_ex::s_verify()
    {

        
        if(S_ptr == NULL)
        {
            MPD_PARSER_DBG(("\t\t\t no S pointer for verify\n"));
            return;
        }
        MPD_PARSER_DBG(("@@@@@@@@@@@@@@@@@@@@@@@ S BEGIN @@@@@@@@@@@@@@@@@@@@@@@\n"));
        unsigned int number = start_time_for_r.size();
        MPD_PARSER_DBG(("d = %d r = %d number = %d\n", get_d(), get_d(), number));
        unsigned int index = 0;

        for(; index < number; ++ index)
        {
            MPD_PARSER_DBG(("start time %d = %d\n", index, start_time_for_r[index]));
        }

        MPD_PARSER_DBG(("@@@@@@@@@@@@@@@@@@@@@@@ S END @@@@@@@@@@@@@@@@@@@@@@@@@@@\n"));
    }


    void CRepresentationBaseType_ex::representationbase_verify(CRepresentationBaseType* ptr)
    {
        

        if(ptr == NULL)
        {
            MPD_PARSER_DBG(("\t\t\t no Representationbase pointer for verify\n"));
            return;
        }
        MPD_PARSER_DBG(("^^^^^^^^^^^^^^^^ RepresentationBase BEGIN ^^^^^^^^^^^^^^^^^^^^\n"));
        MPD_PARSER_DBG(("mime_type = %d\n", mime_type));
        MPD_PARSER_DBG(("^^^^^^^^^^^^^^^^ RepresentationBase END ^^^^^^^^^^^^^^^^^^^^\n"));
    }
    #endif

    
}


