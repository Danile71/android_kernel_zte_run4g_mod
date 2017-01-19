#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSegmentListType
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSegmentListType

#include "type_.CMultipleSegmentBaseType.h"


namespace MPDSchema
{

class CSegmentListType : public ::MPDSchema::CMultipleSegmentBaseType
{
public:
	MPDSchema_EXPORT CSegmentListType(xercesc::DOMNode* const& init);
	MPDSchema_EXPORT CSegmentListType(CSegmentListType const& init);
	void operator=(CSegmentListType const& other) { m_node = other.m_node; }
	static altova::meta::ComplexType StaticInfo() { return altova::meta::ComplexType(types + _altova_ti_altova_CSegmentListType); }
	MemberElement<CSegmentURLType, _altova_mi_altova_CSegmentListType_altova_SegmentURL> SegmentURL;
	struct SegmentURL { typedef Iterator<CSegmentURLType> iterator; };
	MPDSchema_EXPORT void SetXsiType();

};

}	// namespace MPDSchema

#endif // _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSegmentListType
