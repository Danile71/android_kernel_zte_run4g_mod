#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA_xs_ALTOVA_CanyType
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA_xs_ALTOVA_CanyType



namespace MPDSchema
{

namespace xs
{	

class CanyType : public TypeBase
{
public:
	MPDSchema_EXPORT CanyType(xercesc::DOMNode* const& init);
	MPDSchema_EXPORT CanyType(CanyType const& init);
	void operator=(CanyType const& other) { m_node = other.m_node; }
	static altova::meta::ComplexType StaticInfo() { return altova::meta::ComplexType(types + _altova_ti_xs_altova_CanyType); }
	MPDSchema_EXPORT void operator=(const string_type& value);
	MPDSchema_EXPORT operator string_type();
	MPDSchema_EXPORT void SetXsiType();
};



} // namespace xs

}	// namespace MPDSchema

#endif // _ALTOVA_INCLUDED_MPDSchema_ALTOVA_xs_ALTOVA_CanyType
