#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CContentProtectionType
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CContentProtectionType



namespace MPDSchema
{

class CContentProtectionType : public TypeBase
{
public:
	MPDSchema_EXPORT CContentProtectionType(xercesc::DOMNode* const& init);
	MPDSchema_EXPORT CContentProtectionType(CContentProtectionType const& init);
	void operator=(CContentProtectionType const& other) { m_node = other.m_node; }
	static altova::meta::ComplexType StaticInfo() { return altova::meta::ComplexType(types + _altova_ti_altova_CContentProtectionType); }

	MemberAttribute<string_type,_altova_mi_altova_CContentProtectionType_altova_schemeIdUri, 0, 0> schemeIdUri;	// schemeIdUri CanyURI

	MemberAttribute<string_type,_altova_mi_altova_CContentProtectionType_altova_value2, 0, 0> value2;	// value Cstring
	MemberElement<CFormatVersionType, _altova_mi_altova_CContentProtectionType_altova_FormatVersion> FormatVersion;
	struct FormatVersion { typedef Iterator<CFormatVersionType> iterator; };
	MemberElement<CMarlinContentIdsType, _altova_mi_altova_CContentProtectionType_altova_MarlinContentIds> MarlinContentIds;
	struct MarlinContentIds { typedef Iterator<CMarlinContentIdsType> iterator; };
	MemberElement<CMarlinBroadbandType, _altova_mi_altova_CContentProtectionType_altova_MarlinBroadband> MarlinBroadband;
	struct MarlinBroadband { typedef Iterator<CMarlinBroadbandType> iterator; };
	MemberElement<CMS3Type, _altova_mi_altova_CContentProtectionType_altova_MS3> MS3;
	struct MS3 { typedef Iterator<CMS3Type> iterator; };
	MPDSchema_EXPORT void SetXsiType();
};


}	// namespace MPDSchema

#endif // _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CContentProtectionType
