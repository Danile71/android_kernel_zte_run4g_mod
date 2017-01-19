#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CContentComponentType
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CContentComponentType



namespace MPDSchema
{

class CContentComponentType : public TypeBase
{
public:
	MPDSchema_EXPORT CContentComponentType(xercesc::DOMNode* const& init);
	MPDSchema_EXPORT CContentComponentType(CContentComponentType const& init);
	void operator=(CContentComponentType const& other) { m_node = other.m_node; }
	static altova::meta::ComplexType StaticInfo() { return altova::meta::ComplexType(types + _altova_ti_altova_CContentComponentType); }

	MemberAttribute<unsigned,_altova_mi_altova_CContentComponentType_altova_id, 0, 0> id;	// id CunsignedInt

	MemberAttribute<string_type,_altova_mi_altova_CContentComponentType_altova_lang, 0, 0> lang;	// lang Clanguage

	MemberAttribute<string_type,_altova_mi_altova_CContentComponentType_altova_contentType, 0, 0> contentType;	// contentType Cstring

	MemberAttribute<string_type,_altova_mi_altova_CContentComponentType_altova_par, 0, 0> par;	// par Cstring
	MemberElement<CDescriptorType, _altova_mi_altova_CContentComponentType_altova_Accessibility> Accessibility;
	struct Accessibility { typedef Iterator<CDescriptorType> iterator; };
	MemberElement<CDescriptorType, _altova_mi_altova_CContentComponentType_altova_Role> Role;
	struct Role { typedef Iterator<CDescriptorType> iterator; };
	MemberElement<CDescriptorType, _altova_mi_altova_CContentComponentType_altova_Rating> Rating;
	struct Rating { typedef Iterator<CDescriptorType> iterator; };
	MemberElement<CDescriptorType, _altova_mi_altova_CContentComponentType_altova_Viewpoint> Viewpoint;
	struct Viewpoint { typedef Iterator<CDescriptorType> iterator; };
	MPDSchema_EXPORT void SetXsiType();
};


}	// namespace MPDSchema

#endif // _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CContentComponentType
