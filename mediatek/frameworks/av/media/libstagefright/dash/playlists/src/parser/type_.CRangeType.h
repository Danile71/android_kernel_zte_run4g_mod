#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CRangeType
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CRangeType



namespace MPDSchema
{

class CRangeType : public TypeBase
{
public:
	MPDSchema_EXPORT CRangeType(xercesc::DOMNode* const& init);
	MPDSchema_EXPORT CRangeType(CRangeType const& init);
	void operator=(CRangeType const& other) { m_node = other.m_node; }
	static altova::meta::ComplexType StaticInfo() { return altova::meta::ComplexType(types + _altova_ti_altova_CRangeType); }

	MemberAttribute<altova::Duration,_altova_mi_altova_CRangeType_altova_starttime, 0, 0> starttime;	// starttime Cduration

	MemberAttribute<altova::Duration,_altova_mi_altova_CRangeType_altova_duration, 0, 0> duration;	// duration Cduration
	MPDSchema_EXPORT void SetXsiType();
};


}	// namespace MPDSchema

#endif // _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CRangeType
