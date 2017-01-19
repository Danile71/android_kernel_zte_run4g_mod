#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CMS3Type
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CMS3Type



namespace MPDSchema
{

class CMS3Type : public TypeBase
{
public:
	MPDSchema_EXPORT CMS3Type(xercesc::DOMNode* const& init);
	MPDSchema_EXPORT CMS3Type(CMS3Type const& init);
	void operator=(CMS3Type const& other) { m_node = other.m_node; }
	static altova::meta::ComplexType StaticInfo() { return altova::meta::ComplexType(types + _altova_ti_altova_CMS3Type); }
	MemberElement<xs::CanyURIType, _altova_mi_altova_CMS3Type_altova_URIsAreTemplate> URIsAreTemplate;
	struct URIsAreTemplate { typedef Iterator<xs::CanyURIType> iterator; };
	MPDSchema_EXPORT void SetXsiType();
};


}	// namespace MPDSchema

#endif // _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CMS3Type
