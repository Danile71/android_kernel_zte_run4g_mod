#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CBaseURLType
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CBaseURLType

#include "type_xs.CanyURIType.h"


namespace MPDSchema
{

class CBaseURLType : public ::MPDSchema::xs::CanyURIType
{
public:
	MPDSchema_EXPORT CBaseURLType(xercesc::DOMNode* const& init);
	MPDSchema_EXPORT CBaseURLType(CBaseURLType const& init);
	void operator=(CBaseURLType const& other) { m_node = other.m_node; }
	static altova::meta::ComplexType StaticInfo() { return altova::meta::ComplexType(types + _altova_ti_altova_CBaseURLType); }
	MPDSchema_EXPORT void operator=(const string_type& value);

	MemberAttribute<string_type,_altova_mi_altova_CBaseURLType_altova_serviceLocation, 0, 0> serviceLocation;	// serviceLocation Cstring

	MemberAttribute<string_type,_altova_mi_altova_CBaseURLType_altova_byteRange, 0, 0> byteRange;	// byteRange Cstring
	MPDSchema_EXPORT void SetXsiType();
};


}	// namespace MPDSchema

#endif // _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CBaseURLType
