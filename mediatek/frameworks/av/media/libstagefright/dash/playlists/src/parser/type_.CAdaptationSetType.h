#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CAdaptationSetType
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CAdaptationSetType

#include "type_.CRepresentationBaseType.h"


namespace MPDSchema
{

class CAdaptationSetType : public ::MPDSchema::CRepresentationBaseType
{
public:
	MPDSchema_EXPORT CAdaptationSetType(xercesc::DOMNode* const& init);
	MPDSchema_EXPORT CAdaptationSetType(CAdaptationSetType const& init);
	void operator=(CAdaptationSetType const& other) { m_node = other.m_node; }
	static altova::meta::ComplexType StaticInfo() { return altova::meta::ComplexType(types + _altova_ti_altova_CAdaptationSetType); }

	MemberAttribute<unsigned,_altova_mi_altova_CAdaptationSetType_altova_id, 0, 0> id;	// id CunsignedInt

	MemberAttribute<unsigned,_altova_mi_altova_CAdaptationSetType_altova_group, 0, 0> group;	// group CunsignedInt

	MemberAttribute<string_type,_altova_mi_altova_CAdaptationSetType_altova_lang, 0, 0> lang;	// lang Clanguage

	MemberAttribute<string_type,_altova_mi_altova_CAdaptationSetType_altova_contentType, 0, 0> contentType;	// contentType Cstring

	MemberAttribute<string_type,_altova_mi_altova_CAdaptationSetType_altova_par, 0, 0> par;	// par Cstring

	MemberAttribute<unsigned,_altova_mi_altova_CAdaptationSetType_altova_minBandwidth, 0, 0> minBandwidth;	// minBandwidth CunsignedInt

	MemberAttribute<unsigned,_altova_mi_altova_CAdaptationSetType_altova_maxBandwidth, 0, 0> maxBandwidth;	// maxBandwidth CunsignedInt

	MemberAttribute<unsigned,_altova_mi_altova_CAdaptationSetType_altova_minWidth, 0, 0> minWidth;	// minWidth CunsignedInt

	MemberAttribute<unsigned,_altova_mi_altova_CAdaptationSetType_altova_maxWidth, 0, 0> maxWidth;	// maxWidth CunsignedInt

	MemberAttribute<unsigned,_altova_mi_altova_CAdaptationSetType_altova_minHeight, 0, 0> minHeight;	// minHeight CunsignedInt

	MemberAttribute<unsigned,_altova_mi_altova_CAdaptationSetType_altova_maxHeight, 0, 0> maxHeight;	// maxHeight CunsignedInt

	MemberAttribute<string_type,_altova_mi_altova_CAdaptationSetType_altova_minFrameRate, 0, 0> minFrameRate;	// minFrameRate Cstring

	MemberAttribute<string_type,_altova_mi_altova_CAdaptationSetType_altova_maxFrameRate, 0, 0> maxFrameRate;	// maxFrameRate Cstring

	MemberAttribute<string_type,_altova_mi_altova_CAdaptationSetType_altova_segmentAlignment, 0, 0> segmentAlignment;	// segmentAlignment CConditionalUintType

	MemberAttribute<string_type,_altova_mi_altova_CAdaptationSetType_altova_subsegmentAlignment, 0, 0> subsegmentAlignment;	// subsegmentAlignment CConditionalUintType

	MemberAttribute<unsigned,_altova_mi_altova_CAdaptationSetType_altova_subsegmentStartsWithSAP, 0, 0> subsegmentStartsWithSAP;	// subsegmentStartsWithSAP CSAPType

	MemberAttribute<bool,_altova_mi_altova_CAdaptationSetType_altova_bitstreamSwitching, 0, 0> bitstreamSwitching;	// bitstreamSwitching Cboolean
	MemberElement<CDescriptorType, _altova_mi_altova_CAdaptationSetType_altova_Accessibility> Accessibility;
	struct Accessibility { typedef Iterator<CDescriptorType> iterator; };
	MemberElement<CDescriptorType, _altova_mi_altova_CAdaptationSetType_altova_Role> Role;
	struct Role { typedef Iterator<CDescriptorType> iterator; };
	MemberElement<CDescriptorType, _altova_mi_altova_CAdaptationSetType_altova_Rating> Rating;
	struct Rating { typedef Iterator<CDescriptorType> iterator; };
	MemberElement<CDescriptorType, _altova_mi_altova_CAdaptationSetType_altova_Viewpoint> Viewpoint;
	struct Viewpoint { typedef Iterator<CDescriptorType> iterator; };
	MemberElement<CContentComponentType, _altova_mi_altova_CAdaptationSetType_altova_ContentComponent> ContentComponent;
	struct ContentComponent { typedef Iterator<CContentComponentType> iterator; };
	MemberElement<CBaseURLType, _altova_mi_altova_CAdaptationSetType_altova_BaseURL> BaseURL;
	struct BaseURL { typedef Iterator<CBaseURLType> iterator; };
	MemberElement<CSegmentBaseType, _altova_mi_altova_CAdaptationSetType_altova_SegmentBase> SegmentBase;
	struct SegmentBase { typedef Iterator<CSegmentBaseType> iterator; };
	MemberElement<CSegmentListType, _altova_mi_altova_CAdaptationSetType_altova_SegmentList> SegmentList;
	struct SegmentList { typedef Iterator<CSegmentListType> iterator; };
	MemberElement<CSegmentTemplateType, _altova_mi_altova_CAdaptationSetType_altova_SegmentTemplate> SegmentTemplate;
	struct SegmentTemplate { typedef Iterator<CSegmentTemplateType> iterator; };
	MemberElement<CRepresentationType, _altova_mi_altova_CAdaptationSetType_altova_Representation> Representation;
	struct Representation { typedef Iterator<CRepresentationType> iterator; };
	MPDSchema_EXPORT void SetXsiType();
	
};
}	// namespace MPDSchema

#endif // _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CAdaptationSetType
