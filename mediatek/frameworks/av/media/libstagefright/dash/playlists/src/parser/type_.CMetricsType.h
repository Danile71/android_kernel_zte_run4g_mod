#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CMetricsType
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CMetricsType



namespace MPDSchema
{

class CMetricsType : public TypeBase
{
public:
	MPDSchema_EXPORT CMetricsType(xercesc::DOMNode* const& init);
	MPDSchema_EXPORT CMetricsType(CMetricsType const& init);
	void operator=(CMetricsType const& other) { m_node = other.m_node; }
	static altova::meta::ComplexType StaticInfo() { return altova::meta::ComplexType(types + _altova_ti_altova_CMetricsType); }

	MemberAttribute<string_type,_altova_mi_altova_CMetricsType_altova_metrics, 0, 0> metrics;	// metrics Cstring
	MemberElement<CDescriptorType, _altova_mi_altova_CMetricsType_altova_Reporting> Reporting;
	struct Reporting { typedef Iterator<CDescriptorType> iterator; };
	MemberElement<CRangeType, _altova_mi_altova_CMetricsType_altova_Range> Range;
	struct Range { typedef Iterator<CRangeType> iterator; };
	MPDSchema_EXPORT void SetXsiType();
};


}	// namespace MPDSchema

#endif // _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CMetricsType
