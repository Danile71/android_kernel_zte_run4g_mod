#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSubRepresentationType
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSubRepresentationType

#include "type_.CRepresentationBaseType.h"


namespace MPDSchema
{

class CSubRepresentationType : public ::MPDSchema::CRepresentationBaseType
{
public:
	MPDSchema_EXPORT CSubRepresentationType(xercesc::DOMNode* const& init);
	MPDSchema_EXPORT CSubRepresentationType(CSubRepresentationType const& init);
	void operator=(CSubRepresentationType const& other) { m_node = other.m_node; }
	static altova::meta::ComplexType StaticInfo() { return altova::meta::ComplexType(types + _altova_ti_altova_CSubRepresentationType); }

	MemberAttribute<unsigned,_altova_mi_altova_CSubRepresentationType_altova_level, 0, 0> level;	// level CunsignedInt

	MemberAttribute<string_type,_altova_mi_altova_CSubRepresentationType_altova_dependencyLevel, 0, 0> dependencyLevel;	// dependencyLevel CUIntVectorType

	MemberAttribute<unsigned,_altova_mi_altova_CSubRepresentationType_altova_bandwidth, 0, 0> bandwidth;	// bandwidth CunsignedInt

	MemberAttribute<string_type,_altova_mi_altova_CSubRepresentationType_altova_contentComponent, 0, 0> contentComponent;	// contentComponent CStringVectorType
	MPDSchema_EXPORT void SetXsiType();
};


}	// namespace MPDSchema

#endif // _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSubRepresentationType
