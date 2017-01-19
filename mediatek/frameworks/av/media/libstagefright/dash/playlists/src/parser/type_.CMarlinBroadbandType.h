#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CMarlinBroadbandType
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CMarlinBroadbandType



namespace MPDSchema
{

class CMarlinBroadbandType : public TypeBase
{
public:
	MPDSchema_EXPORT CMarlinBroadbandType(xercesc::DOMNode* const& init);
	MPDSchema_EXPORT CMarlinBroadbandType(CMarlinBroadbandType const& init);
	void operator=(CMarlinBroadbandType const& other) { m_node = other.m_node; }
	static altova::meta::ComplexType StaticInfo() { return altova::meta::ComplexType(types + _altova_ti_altova_CMarlinBroadbandType); }
	MemberElement<xs::CanyURIType, _altova_mi_altova_CMarlinBroadbandType_altova_SilentRightsUrl> SilentRightsUrl;
	struct SilentRightsUrl { typedef Iterator<xs::CanyURIType> iterator; };
	MemberElement<xs::CanyURIType, _altova_mi_altova_CMarlinBroadbandType_altova_PreviewRightsUrl> PreviewRightsUrl;
	struct PreviewRightsUrl { typedef Iterator<xs::CanyURIType> iterator; };
	MemberElement<xs::CanyURIType, _altova_mi_altova_CMarlinBroadbandType_altova_RightsIssuerUrl> RightsIssuerUrl;
	struct RightsIssuerUrl { typedef Iterator<xs::CanyURIType> iterator; };
	MPDSchema_EXPORT void SetXsiType();
};


}	// namespace MPDSchema

#endif // _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CMarlinBroadbandType
