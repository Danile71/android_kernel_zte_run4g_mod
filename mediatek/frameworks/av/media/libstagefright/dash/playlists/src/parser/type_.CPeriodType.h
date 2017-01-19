#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CPeriodType
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CPeriodType


namespace MPDSchema
{

class CPeriodType : public TypeBase
{
public:
	MPDSchema_EXPORT CPeriodType(xercesc::DOMNode* const& init);
	MPDSchema_EXPORT CPeriodType(CPeriodType const& init);
	void operator=(CPeriodType const& other) { m_node = other.m_node; }
	static altova::meta::ComplexType StaticInfo() { return altova::meta::ComplexType(types + _altova_ti_altova_CPeriodType); }

	MemberAttribute<string_type,_altova_mi_altova_CPeriodType_altova_id, 0, 0> id;	// id Cstring

	MemberAttribute<altova::Duration,_altova_mi_altova_CPeriodType_altova_start, 0, 0> start;	// start Cduration

	MemberAttribute<altova::Duration,_altova_mi_altova_CPeriodType_altova_duration, 0, 0> duration;	// duration Cduration

	MemberAttribute<bool,_altova_mi_altova_CPeriodType_altova_bitstreamSwitching, 0, 0> bitstreamSwitching;	// bitstreamSwitching Cboolean
	MemberElement<CBaseURLType, _altova_mi_altova_CPeriodType_altova_BaseURL> BaseURL;
	struct BaseURL { typedef Iterator<CBaseURLType> iterator; };
	MemberElement<CSegmentBaseType, _altova_mi_altova_CPeriodType_altova_SegmentBase> SegmentBase;
	struct SegmentBase { typedef Iterator<CSegmentBaseType> iterator; };
	MemberElement<CSegmentListType, _altova_mi_altova_CPeriodType_altova_SegmentList> SegmentList;
	struct SegmentList { typedef Iterator<CSegmentListType> iterator; };
	MemberElement<CSegmentTemplateType, _altova_mi_altova_CPeriodType_altova_SegmentTemplate> SegmentTemplate;
	struct SegmentTemplate { typedef Iterator<CSegmentTemplateType> iterator; };
	MemberElement<CAdaptationSetType, _altova_mi_altova_CPeriodType_altova_AdaptationSet> AdaptationSet;
	struct AdaptationSet { typedef Iterator<CAdaptationSetType> iterator; };
	MemberElement<CSubsetType, _altova_mi_altova_CPeriodType_altova_Subset> Subset;
	struct Subset { typedef Iterator<CSubsetType> iterator; };
	MPDSchema_EXPORT void SetXsiType();

};


}	// namespace MPDSchema

#endif // _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CPeriodType
