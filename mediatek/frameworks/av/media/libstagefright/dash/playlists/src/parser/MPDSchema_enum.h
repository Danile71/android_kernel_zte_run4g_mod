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
/* there are many type we needed to parser mpd file, all of these type are defined
    in this file 
 */

#ifndef _MPD_SCHEMA_ENUM_H_
#define _MPD_SCHEMA_ENUM_H_

#include <string>

 namespace dash_enum
 {
    
    
    // content type support
    enum dash_result_type
    {
        DASH_RET_OK,
        DASH_RET_EMPTY_MPD,
    };

	enum dash_init_type
	{
	    DASH_INIT_NONE,
		DASH_INIT_STRUCT,
		DASH_INIT_URL
	};
    
    enum dash_mpd_dynamic_type
    {
       MPD_TYPE_STATIC,
       MPD_TYPE_DYNAMIC_EMPTY,
       MPD_TYPE_DYNAMIC_NORMAL
    };
    
    enum dash_source_type
    {
        FILE_PATH,
        HTTP_URL,
        SOURCE_NONE
    };

    enum dash_container_type
    {
    	DASH_NONE,
        DASH_MP4,
        DASH_TS,
        DASH_OTHER
    };
    
    enum dash_content_type
    {
        none = 0x00,
        audio = 0x01,
        video = 0x02,
        subtitle = 0x04,
        c_ts = 0x10,
        thumbnail = 0x20
    };

    // mime type support
    enum dash_mime_type
    {
        m_none,
        // for ISO
        video_mp4,
        audio_mp4,
        application_mp4,
        //fo TS
        video_mp2t,
        ttml_xml
    };

    // segment type element 
    enum dash_segment_info_type
    {
        none_segment = 0x00,
        segment_base = 0x01,
        segment_list = 0x02,
        segment_template = 0x04
    };


	enum dash_segment_position
	{
	    pos_representation,
		pos_adaptation,
		pos_peroiod,
	};
	
    enum dash_baseurl_type
    {
        url_relative,
        url_absolute,
        url_none
    };


    enum dash_profile_type
    {
        profile_none,
        iso_main,
        iso_demand,
        iso_live,
        ts_main,
        ts_simple,
        hbbtv_live
    };

    enum dash_role_type
    {
        r_none_value,
        r_caption,
        r_subtitle,
        r_main,
        r_alternate,
        r_supplementary,
        r_commentary,
        r_dub,

        //extension
        r_audio_description,
        r_forced_subtitle,
        r_sdh,
        r_default_enable,
        r_r0_10
    };

    enum dash_audio_channel_type
    {
        //it can be extended
        a_none_a,
        a_stereo_a,
        a_ch_51,
        a_ch_61,
        a_ch_71,
        a_ch_71_ex
        
    };

    enum dash_segment_url_type
    {
        s_u_init,
        s_u_media,
        s_u_bit,
        s_u_index
    };

     const std::string ISO_MAIN = std::string("urn:mpeg:dash:profile:isoff-main:2011");
     const std::string ISO_DEMAND = std::string("urn:mpeg:dash:profile:isoff-on-demand:2011");
     const std::string ISO_LIVE = std::string("urn:mpeg:dash:profile:isoff-live:2011");
     const std::string TS_MAIN = std::string("urn:mpeg:dash:profile:mp2t-main:2011");
     const std::string TS_SIMPLE = std::string("urn:mpeg:dash:profile:mp2t-simple:2011");
     const std::string ISO_FULL = std::string("urn:mpeg:dash:profile:full:2011");

     const std::string HBBTV_LIVE = std::string("urn:hbbtv:dash:profile:isoff-live:2012");



     const std::string REP_ID = std::string("RepresentationID");
     const std::string NUMBER = std::string("Number");
     const std::string BANDWIDTH = std::string("Bandwidth");
     const std::string TIME = std::string("Time");
     const std::string DASH_NS = std::string("urn:mpeg:dash:schema:mpd:2011");


     const std::string ROLE_CAPTION = std::string("caption");
     const std::string ROLE_AUDIO_DESCRIPTION = std::string("audioDescription");
     const std::string ROLE_FORCED_SUBTITLE = std::string("forcedSubtitle");
     const std::string ROLE_SDH = std::string("sdh");
     const std::string ROLE_DEFAULT_ENABLE = std::string("defaultEnabled");

     const std::string ROLE_SUBTITLE = std::string("subtitle");
     const std::string ROLE_MAIN = std::string("main");
     const std::string ROLE_ALTERNATE = std::string("alternate");
     const std::string ROLE_SUPPLEMENTARY = std::string("supplementary");
     const std::string ROLE_DUB = std::string("dub");
     const std::string ROLE_R0_10 = std::string("r0 10");
     const std::string ROLE_COMMENTARY = std::string("commentary");
     const std::string ROLE_SCHEME = std::string("urn:mpeg:dash:role:2011");



     const std::string AUDIO_CHANNEL_SCHEME = std::string("urn:mpeg:dash:23003:3:audio_channel_configuration:2011");
     const std::string AUDIO_STEREO = std::string("2");
     const std::string AUDIO_CH51 = std::string("6");
     const std::string AUDIO_CH61 = std::string("11");
     const std::string AUDIO_CH71 = std::string("12");

     const std::string AUDIO_CH71_EX = std::string("7.1ch(2/0/0-3/0/2-0.1)");


     enum dash_frame_packing_type
     {
        f_none_frame,
        f_frame_5
	 
     };
     const std::string FRAME_PACKING_5 = std::string("5");
     const std::string FRAME_PACKING_SCHEME_ISO = std::string("urn:mpeg:dash:14496:10:frame_packing_arrangement_type:2011");
     const std::string FRAME_PACKING_SCHEME_TS = std::string("urn:mpeg:dash:13818:1:stereo_video_format_type:2011");


     enum dash_condition_type
     {
        c_false,
        c_true,
        c_int,
        c_none
     };
 
     // for role and accessbility value check
     const std::string KIND_ROLE_MAIN_VIDEO_AUDIO = std::string("main");
     const std::string KIND_ROLE_STEREO_VIDEO = std::string("10 r0");
     const std::string KIND_ROLE_DUB_AUDIO = std::string("dub");
     const std::string KIND_ACCESS_DESCRIPTION_AUDIO = std::string("audioDescription");
     const std::string KIND_ROLE_SUBTITLE_SUBTITLE = std::string("subtitle");
     const std::string KIND_ROLE_FORCED_SUBTITLE = std::string("forcedSubtitle");
     const std::string KIND_ROLE_DEFAULT_ENABLED_SUBTITLE = std::string("defaultEnabled");
     const std::string KIND_ROLE_COMMENTARY_SUBTITLE = std::string("commentary");
     const std::string KIND_ACCESS_CAPTION_SUBTITILE = std::string("caption");
     const std::string KIND_ACCESS_EASYREADER_SUBTITLE = std::string("easyReader");
     const std::string KIND_ACCESS_SDH_SUBTITLE = std::string("sdh");

     const std::string ROLE_DASH_ROLE_2011_SCHEMEURI = std::string("urn:mpeg:dash:role:2011");
     const std::string ROLE_DASH_STEREOID_2011_SCHEMEURI = std::string("urn:mpeg:dash:stereoid:2011");
     const std::string ACCESS_TVA_AUDIOPURPOSECS_SCHEMEURI = std::string("urn:tva:metadata:cs:AudioPurposeCS:2007");

     const std::string ROLE_DASH_ROLE_2011_VL_CAPTION = std::string("caption");
     const std::string ROLE_DASH_ROLE_2011_VL_SUBTITLE = std::string("caption");
     const std::string ROLE_DASH_ROLE_2011_VL_MAIN = std::string("main");
     const std::string ROLE_DASH_ROLE_2011_VL_ALTERNATE = std::string("alternate");
     const std::string ROLE_DASH_ROLE_2011_VL_SUPPLEMENTARY = std::string("supplementary");
     const std::string ROLE_DASH_ROLE_2011_VL_COMMENTARY = std::string("commentary");
     const std::string ROLE_DASH_ROLE_2011_VL_DUB = std::string("dub");

     const std::string ACCESS_TVA_AUDIOPURPOSECS_VL_1 = std::string("1");
     const std::string ACCESS_TVA_AUDIOPURPOSECS_VL_2 = std::string("2");
     const std::string ACCESS_TVA_AUDIOPURPOSECS_VL_3 = std::string("3");
     const std::string ACCESS_TVA_AUDIOPURPOSECS_VL_4 = std::string("4");
     const std::string ACCESS_TVA_AUDIOPURPOSECS_VL_5 = std::string("5");
     const std::string ACCESS_TVA_AUDIOPURPOSECS_VL_6 = std::string("6");
     const std::string ACCESS_TVA_AUDIOPURPOSECS_VL_7 = std::string("7");
}

using namespace dash_enum;

#endif

