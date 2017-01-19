#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CRepresentationType_ex
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CRepresentationType_ex

#include "type_.CRepresentationBaseType_ex.h"
#include "type_.CSegmentBaseType_ex.h"
#include "type_.CSegmentListType_ex.h"
#include "type_.CSegmentTemplateType_ex.h"
#include "type_.CMultipleSegmentBaseType_ex.h"


namespace MPDSchema
{
class CRepresentationType_ex:public CRepresentationBaseType_ex
{
public:
	CRepresentationType_ex();
	CRepresentationType_ex(CRepresentationType* ptr);
	~CRepresentationType_ex(){};

	
	void representation_init(CRepresentationType* ptr)throw(all_exceptions);
	//void representation_process()throw(all_exceptions);


	void set_segment_base_ptr(CSegmentBaseType* ptr){segment_base.set_segmentbase_ptr(ptr);};
	void set_segment_list_ptr(CSegmentListType* ptr){segment_list.set_segmentlist_ptr(ptr);};
	void set_segment_template_ptr(CSegmentTemplateType* ptr){segment_template.set_segmenttemplate_ptr(ptr);};
	void set_segment_info(dash_enum::dash_segment_info_type t){segment_info = t;};
	dash_enum::dash_content_type get_content_type(){return content_type_ex;};
	dash_enum::dash_segment_info_type get_segment_info(){return segment_info;};
	void set_pos_representation(dash_enum::dash_segment_position pos){segment_pos |= pos;};
	void set_width_and_height(unsigned int width_, unsigned int height_, bool wid, bool hei);
	
#if (MPD_PASER_DEBUG > 1)
	void representation_verify(CRepresentationType* ptr);
#endif

	
public:
	void CRep_init();
	
public:
	dash_enum::dash_segment_info_type segment_info;
	dash_enum::dash_content_type content_type_ex;
	CSegmentListType_ex segment_list;
	CSegmentTemplateType_ex segment_template;
	CSegmentBaseType_ex segment_base;
	CRepresentationType* representation_ptr;
	int segment_pos;

    std::string id_ex;
	bool id_exists;
	unsigned int bandwidth_ex;
	bool bandwidth_exists;

	bool timeline_exists;

	unsigned int width;
	unsigned int height;
	unsigned int width_exists;
	unsigned int height_exists;
	
};

}
#endif

