#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CPeriodType_ex
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CPeriodType_ex

#include "type_.CSegmentBaseType_ex.h"
#include "type_.CSegmentListType_ex.h"
#include "type_.CSegmentTemplateType_ex.h"
#include "type_.CMultipleSegmentBaseType_ex.h"


namespace MPDSchema
{

class CPeriodType_ex
{
    public:
		CPeriodType_ex();
		CPeriodType_ex(CPeriodType* ptr);
		~CPeriodType_ex(){};

		void period_init(CPeriodType* ptr)throw(all_exceptions);
		//void period_process()throw(all_exceptions);
		void merge_segment_info()throw(all_exceptions);
		dash_enum::dash_segment_info_type get_segment_info(){return segment_info;};
		dash_enum::dash_content_type get_content_type();

		bool start_present();
		bool duration_present();

        void set_start_present();
        void set_duration_present();
        
		double get_start_time();
		double get_duration_time();

		void set_start_time(double value){start_time = value;};
		void set_duration_time(double value){duration_time = value;};
		
		double get_start_time_ex(){return start_time;};
		double get_duration_time_ex(){return duration_time;};

#if (MPD_PASER_DEBUG > 1)
		void period_verify(CPeriodType* period);
#endif
		
	public:
		double start_time;
		double duration_time;
		bool start_time_exists;
		bool duration_time_exists;
		dash_enum::dash_segment_info_type segment_info;
		CPeriodType* period_ptr;
		std::vector<CAdaptationSetType_ex> adaptation_list;

		CSegmentListType_ex segment_list;
		CSegmentTemplateType_ex segment_template;
		CSegmentBaseType_ex segment_base;
		bool timeline_exists;
};

}


#endif

