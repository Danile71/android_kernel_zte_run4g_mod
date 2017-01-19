#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CFormatVersionType
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CFormatVersionType



namespace MPDSchema
{

class CFormatVersionType : public TypeBase
{
public:
	MPDSchema_EXPORT CFormatVersionType(xercesc::DOMNode* const& init);
	MPDSchema_EXPORT CFormatVersionType(CFormatVersionType const& init);
	void operator=(CFormatVersionType const& other) { m_node = other.m_node; }
	static altova::meta::ComplexType StaticInfo() { return altova::meta::ComplexType(types + _altova_ti_altova_CFormatVersionType); }

	MemberAttribute<string_type,_altova_mi_altova_CFormatVersionType_altova_major, 0, 0> major;	// major Cstring

	MemberAttribute<string_type,_altova_mi_altova_CFormatVersionType_altova_minor2, 0, 0> minor2;	// minor Cstring
	MPDSchema_EXPORT void SetXsiType();
};


}	// namespace MPDSchema

#endif // _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CFormatVersionType
