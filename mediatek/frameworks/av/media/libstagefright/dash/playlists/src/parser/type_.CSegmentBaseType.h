#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSegmentBaseType
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSegmentBaseType



namespace MPDSchema
{

class CSegmentBaseType : public TypeBase
{
public:
	MPDSchema_EXPORT CSegmentBaseType(xercesc::DOMNode* const& init);
	MPDSchema_EXPORT CSegmentBaseType(CSegmentBaseType const& init);
	void operator=(CSegmentBaseType const& other) { m_node = other.m_node; }
	static altova::meta::ComplexType StaticInfo() { return altova::meta::ComplexType(types + _altova_ti_altova_CSegmentBaseType); }

	MemberAttribute<unsigned,_altova_mi_altova_CSegmentBaseType_altova_timescale, 0, 0> timescale;	// timescale CunsignedInt

	MemberAttribute<unsigned,_altova_mi_altova_CSegmentBaseType_altova_presentationTimeOffset, 0, 0> presentationTimeOffset;	// presentationTimeOffset CunsignedInt

	MemberAttribute<string_type,_altova_mi_altova_CSegmentBaseType_altova_indexRange, 0, 0> indexRange;	// indexRange Cstring

	MemberAttribute<bool,_altova_mi_altova_CSegmentBaseType_altova_indexRangeExact, 0, 0> indexRangeExact;	// indexRangeExact Cboolean
	MemberElement<CURLType, _altova_mi_altova_CSegmentBaseType_altova_Initialisation> Initialisation;
	struct Initialisation { typedef Iterator<CURLType> iterator; };
	MemberElement<CURLType, _altova_mi_altova_CSegmentBaseType_altova_Initialization> Initialization;
	struct Initialization { typedef Iterator<CURLType> iterator; };
	MemberElement<CURLType, _altova_mi_altova_CSegmentBaseType_altova_RepresentationIndex> RepresentationIndex;
	struct RepresentationIndex { typedef Iterator<CURLType> iterator; };
	MPDSchema_EXPORT void SetXsiType();
	
};

}	// namespace MPDSchema

#endif // _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSegmentBaseType
