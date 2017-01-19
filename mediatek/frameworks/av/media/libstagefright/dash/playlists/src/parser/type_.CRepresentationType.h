#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CRepresentationType
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CRepresentationType

#include "type_.CRepresentationBaseType.h"


namespace MPDSchema
{

class CRepresentationType : public ::MPDSchema::CRepresentationBaseType
{
public:
	MPDSchema_EXPORT CRepresentationType(xercesc::DOMNode* const& init);
	MPDSchema_EXPORT CRepresentationType(CRepresentationType const& init);
	void operator=(CRepresentationType const& other) { m_node = other.m_node; }
	static altova::meta::ComplexType StaticInfo() { return altova::meta::ComplexType(types + _altova_ti_altova_CRepresentationType); }

	MemberAttribute<string_type,_altova_mi_altova_CRepresentationType_altova_id, 0, 0> id;	// id Cstring

	MemberAttribute<unsigned,_altova_mi_altova_CRepresentationType_altova_bandwidth, 0, 0> bandwidth;	// bandwidth CunsignedInt

	MemberAttribute<unsigned,_altova_mi_altova_CRepresentationType_altova_qualityRanking, 0, 0> qualityRanking;	// qualityRanking CunsignedInt

	MemberAttribute<string_type,_altova_mi_altova_CRepresentationType_altova_dependencyId, 0, 0> dependencyId;	// dependencyId CStringVectorType

	MemberAttribute<string_type,_altova_mi_altova_CRepresentationType_altova_mediaStreamStructureId, 0, 0> mediaStreamStructureId;	// mediaStreamStructureId CStringVectorType
	MemberElement<CBaseURLType, _altova_mi_altova_CRepresentationType_altova_BaseURL> BaseURL;
	struct BaseURL { typedef Iterator<CBaseURLType> iterator; };
	MemberElement<CSubRepresentationType, _altova_mi_altova_CRepresentationType_altova_SubRepresentation> SubRepresentation;
	struct SubRepresentation { typedef Iterator<CSubRepresentationType> iterator; };
	MemberElement<CSegmentBaseType, _altova_mi_altova_CRepresentationType_altova_SegmentBase> SegmentBase;
	struct SegmentBase { typedef Iterator<CSegmentBaseType> iterator; };
	MemberElement<CSegmentListType, _altova_mi_altova_CRepresentationType_altova_SegmentList> SegmentList;
	struct SegmentList { typedef Iterator<CSegmentListType> iterator; };
	MemberElement<CSegmentTemplateType, _altova_mi_altova_CRepresentationType_altova_SegmentTemplate> SegmentTemplate;
	struct SegmentTemplate { typedef Iterator<CSegmentTemplateType> iterator; };
	MPDSchema_EXPORT void SetXsiType();
};
}	// namespace MPDSchema

#endif // _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CRepresentationType
