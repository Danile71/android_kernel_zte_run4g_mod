#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CURLType
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CURLType



namespace MPDSchema
{

class CURLType : public TypeBase
{
public:
	MPDSchema_EXPORT CURLType(xercesc::DOMNode* const& init);
	MPDSchema_EXPORT CURLType(CURLType const& init);
	void operator=(CURLType const& other) { m_node = other.m_node; }
	static altova::meta::ComplexType StaticInfo() { return altova::meta::ComplexType(types + _altova_ti_altova_CURLType); }

	MemberAttribute<string_type,_altova_mi_altova_CURLType_altova_sourceURL, 0, 0> sourceURL;	// sourceURL CanyURI

	MemberAttribute<string_type,_altova_mi_altova_CURLType_altova_range, 0, 0> range;	// range Cstring
	MPDSchema_EXPORT void SetXsiType();
};


}	// namespace MPDSchema

#endif // _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CURLType
