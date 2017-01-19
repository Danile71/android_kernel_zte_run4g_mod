#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CDescriptorType
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CDescriptorType



namespace MPDSchema
{

class CDescriptorType : public TypeBase
{
public:
	MPDSchema_EXPORT CDescriptorType(xercesc::DOMNode* const& init);
	MPDSchema_EXPORT CDescriptorType(CDescriptorType const& init);
	void operator=(CDescriptorType const& other) { m_node = other.m_node; }
	static altova::meta::ComplexType StaticInfo() { return altova::meta::ComplexType(types + _altova_ti_altova_CDescriptorType); }

	MemberAttribute<string_type,_altova_mi_altova_CDescriptorType_altova_schemeIdUri, 0, 0> schemeIdUri;	// schemeIdUri CanyURI

	MemberAttribute<string_type,_altova_mi_altova_CDescriptorType_altova_value2, 0, 0> value2;	// value Cstring
	MPDSchema_EXPORT void SetXsiType();
};


}	// namespace MPDSchema

#endif // _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CDescriptorType
