#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSegmentTemplateType
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSegmentTemplateType

#include "type_.CMultipleSegmentBaseType.h"


namespace MPDSchema
{

class CSegmentTemplateType : public ::MPDSchema::CMultipleSegmentBaseType
{
public:
	MPDSchema_EXPORT CSegmentTemplateType(xercesc::DOMNode* const& init);
	MPDSchema_EXPORT CSegmentTemplateType(CSegmentTemplateType const& init);
	void operator=(CSegmentTemplateType const& other) { m_node = other.m_node; }
	static altova::meta::ComplexType StaticInfo() { return altova::meta::ComplexType(types + _altova_ti_altova_CSegmentTemplateType); }

	MemberAttribute<string_type,_altova_mi_altova_CSegmentTemplateType_altova_media, 0, 0> media;	// media Cstring

	MemberAttribute<string_type,_altova_mi_altova_CSegmentTemplateType_altova_index, 0, 0> index;	// index Cstring

	MemberAttribute<string_type,_altova_mi_altova_CSegmentTemplateType_altova_initialisation2, 0, 0> initialisation2;	// initialisation Cstring

	MemberAttribute<string_type,_altova_mi_altova_CSegmentTemplateType_altova_initialization2, 0, 0> initialization2;	// initialization Cstring

	MemberAttribute<string_type,_altova_mi_altova_CSegmentTemplateType_altova_bitstreamSwitching2, 0, 0> bitstreamSwitching2;	// bitstreamSwitching Cstring
	MPDSchema_EXPORT void SetXsiType();

};


}	// namespace MPDSchema

#endif // _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSegmentTemplateType
