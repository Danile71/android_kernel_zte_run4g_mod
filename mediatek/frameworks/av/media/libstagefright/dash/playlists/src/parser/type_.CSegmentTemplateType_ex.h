#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSegmentTemplateType_ex
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSegmentTemplateType_ex

#include "type_.CMultipleSegmentBaseType_ex.h"

namespace MPDSchema
{
class CSegmentTemplateType_ex :public CMultipleSegmentBaseType_ex
{
	public:
		CSegmentTemplateType_ex(CSegmentTemplateType* ptr);
		CSegmentTemplateType_ex();
		~CSegmentTemplateType_ex(){};

		std::string handle_media_url(std::string& media, std::string rep_id, int num, long long bandwidth, long time)throw (all_exceptions);
		std::string handle_index_url(std::string& index, std::string rep_id, long long bandwidth)throw (all_exceptions);
		std::string handle_initialisation_url(std::string& initialisation, std::string rep_id, long long bandwidth)throw (all_exceptions);
		std::string handle_bitstream_url(std::string& bitstream, std::string rep_id, long long bandwidth)throw (all_exceptions); 

		void segmenttemplate_init(CSegmentTemplateType* ptr);
		void segmenttemplate_url_init();
		//svoid segmenttemplate_process();

		bool initial_string_exists(){return (init_type != dash_enum::DASH_INIT_NONE);};
		std::string get_init_url_string();
		void set_segmenttemplate_ptr(CSegmentTemplateType* ptr);
		bool exists(){return (segmenttemplate_ptr != NULL);};
#if (MPD_PASER_DEBUG > 1)
	void segmenttemplate_verify();
#endif

public:
	std::string handle_url(std::string handle, std::string rep_id, int num, long long bandwidth,  long time)throw (all_exceptions);
	std::string handle_width(std::string& handle, std::string substr) throw (all_exceptions);
	CSegmentTemplateType* segmenttemplate_ptr;
	std::string media_ex;
	bool media_exists;
	std::string index_ex;
	bool index_exists;
	std::string initialisation_ex;
	bool initialisation_exists;
	std::string bistreamswitch_ex;
	bool bistreamswitch_exists;

    //for MHP
    //std::string::size_type number_start_indx;
    //std::string::size_type number_end_indx;
    int number_start_indx;
    int number_end_indx;
};

}

#endif

