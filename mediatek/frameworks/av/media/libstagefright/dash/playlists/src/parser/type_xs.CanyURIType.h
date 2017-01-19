#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA_xs_ALTOVA_CanyURIType
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA_xs_ALTOVA_CanyURIType



namespace MPDSchema
{

namespace xs
{	

class CanyURIType : public TypeBase
{
public:
	MPDSchema_EXPORT CanyURIType(xercesc::DOMNode* const& init);
	MPDSchema_EXPORT CanyURIType(CanyURIType const& init);
	void operator=(CanyURIType const& other) { m_node = other.m_node; }
	static altova::meta::ComplexType StaticInfo() { return altova::meta::ComplexType(types + _altova_ti_xs_altova_CanyURIType); }
	MPDSchema_EXPORT void operator=(const string_type& value);
	MPDSchema_EXPORT operator string_type();
	MPDSchema_EXPORT void SetXsiType();
};



} // namespace xs

}	// namespace MPDSchema

#endif // _ALTOVA_INCLUDED_MPDSchema_ALTOVA_xs_ALTOVA_CanyURIType
