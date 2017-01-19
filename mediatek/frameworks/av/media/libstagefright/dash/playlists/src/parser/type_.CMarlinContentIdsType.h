#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CMarlinContentIdsType
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CMarlinContentIdsType



namespace MPDSchema
{

class CMarlinContentIdsType : public TypeBase
{
public:
	MPDSchema_EXPORT CMarlinContentIdsType(xercesc::DOMNode* const& init);
	MPDSchema_EXPORT CMarlinContentIdsType(CMarlinContentIdsType const& init);
	void operator=(CMarlinContentIdsType const& other) { m_node = other.m_node; }
	static altova::meta::ComplexType StaticInfo() { return altova::meta::ComplexType(types + _altova_ti_altova_CMarlinContentIdsType); }
	MemberElement<xs::CstringType, _altova_mi_altova_CMarlinContentIdsType_altova_MarlinContentId> MarlinContentId;
	struct MarlinContentId { typedef Iterator<xs::CstringType> iterator; };
	MPDSchema_EXPORT void SetXsiType();
};


}	// namespace MPDSchema

#endif // _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CMarlinContentIdsType
