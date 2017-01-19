#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSegmentURLType
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSegmentURLType



namespace MPDSchema
{

class CSegmentURLType : public TypeBase
{
public:
	MPDSchema_EXPORT CSegmentURLType(xercesc::DOMNode* const& init);
	MPDSchema_EXPORT CSegmentURLType(CSegmentURLType const& init);
	void operator=(CSegmentURLType const& other) { m_node = other.m_node; }
	static altova::meta::ComplexType StaticInfo() { return altova::meta::ComplexType(types + _altova_ti_altova_CSegmentURLType); }

	MemberAttribute<string_type,_altova_mi_altova_CSegmentURLType_altova_media, 0, 0> media;	// media CanyURI

	MemberAttribute<string_type,_altova_mi_altova_CSegmentURLType_altova_mediaRange, 0, 0> mediaRange;	// mediaRange Cstring

	MemberAttribute<string_type,_altova_mi_altova_CSegmentURLType_altova_index, 0, 0> index;	// index CanyURI

	MemberAttribute<string_type,_altova_mi_altova_CSegmentURLType_altova_indexRange, 0, 0> indexRange;	// indexRange Cstring
	MPDSchema_EXPORT void SetXsiType();
};


}	// namespace MPDSchema

#endif // _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSegmentURLType
