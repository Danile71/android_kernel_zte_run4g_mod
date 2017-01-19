#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CProgramInformationType
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CProgramInformationType



namespace MPDSchema
{

class CProgramInformationType : public TypeBase
{
public:
	MPDSchema_EXPORT CProgramInformationType(xercesc::DOMNode* const& init);
	MPDSchema_EXPORT CProgramInformationType(CProgramInformationType const& init);
	void operator=(CProgramInformationType const& other) { m_node = other.m_node; }
	static altova::meta::ComplexType StaticInfo() { return altova::meta::ComplexType(types + _altova_ti_altova_CProgramInformationType); }

	MemberAttribute<string_type,_altova_mi_altova_CProgramInformationType_altova_lang, 0, 0> lang;	// lang Clanguage

	MemberAttribute<string_type,_altova_mi_altova_CProgramInformationType_altova_moreInformationURL, 0, 0> moreInformationURL;	// moreInformationURL CanyURI
	MemberElement<xs::CstringType, _altova_mi_altova_CProgramInformationType_altova_Title> Title;
	struct Title { typedef Iterator<xs::CstringType> iterator; };
	MemberElement<xs::CstringType, _altova_mi_altova_CProgramInformationType_altova_Source> Source;
	struct Source { typedef Iterator<xs::CstringType> iterator; };
	MemberElement<xs::CstringType, _altova_mi_altova_CProgramInformationType_altova_Copyright> Copyright;
	struct Copyright { typedef Iterator<xs::CstringType> iterator; };
	MPDSchema_EXPORT void SetXsiType();
};


}	// namespace MPDSchema

#endif // _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CProgramInformationType
