#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSegmentTimelineType
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSegmentTimelineType



namespace MPDSchema
{

class CSegmentTimelineType : public TypeBase
{
public:
	MPDSchema_EXPORT CSegmentTimelineType(xercesc::DOMNode* const& init);
	MPDSchema_EXPORT CSegmentTimelineType(CSegmentTimelineType const& init);
	void operator=(CSegmentTimelineType const& other) { m_node = other.m_node; }
	static altova::meta::ComplexType StaticInfo() { return altova::meta::ComplexType(types + _altova_ti_altova_CSegmentTimelineType); }
	MemberElement<CSType, _altova_mi_altova_CSegmentTimelineType_altova_S> S;
	struct S { typedef Iterator<CSType> iterator; };
	MPDSchema_EXPORT void SetXsiType();
};

}	// namespace MPDSchema

#endif // _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSegmentTimelineType
