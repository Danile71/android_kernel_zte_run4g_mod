#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CMultipleSegmentBaseType
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CMultipleSegmentBaseType

#include "type_.CSegmentBaseType.h"


namespace MPDSchema
{

class CMultipleSegmentBaseType : public ::MPDSchema::CSegmentBaseType
{
public:
	MPDSchema_EXPORT CMultipleSegmentBaseType(xercesc::DOMNode* const& init);
	MPDSchema_EXPORT CMultipleSegmentBaseType(CMultipleSegmentBaseType const& init);
	void operator=(CMultipleSegmentBaseType const& other) { m_node = other.m_node; }
	static altova::meta::ComplexType StaticInfo() { return altova::meta::ComplexType(types + _altova_ti_altova_CMultipleSegmentBaseType); }

	MemberAttribute<unsigned,_altova_mi_altova_CMultipleSegmentBaseType_altova_duration, 0, 0> duration;	// duration CunsignedInt

	MemberAttribute<unsigned,_altova_mi_altova_CMultipleSegmentBaseType_altova_startNumber, 0, 0> startNumber;	// startNumber CunsignedInt
	MemberElement<CSegmentTimelineType, _altova_mi_altova_CMultipleSegmentBaseType_altova_SegmentTimeline> SegmentTimeline;
	struct SegmentTimeline { typedef Iterator<CSegmentTimelineType> iterator; };
	MemberElement<CURLType, _altova_mi_altova_CMultipleSegmentBaseType_altova_BitstreamSwitching> BitstreamSwitching;
	struct BitstreamSwitching { typedef Iterator<CURLType> iterator; };
	MPDSchema_EXPORT void SetXsiType();
};

}	// namespace MPDSchema

#endif // _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CMultipleSegmentBaseType
