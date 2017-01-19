#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA_xs_ALTOVA_CstringType
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA_xs_ALTOVA_CstringType



namespace MPDSchema
{

namespace xs
{	

class CstringType : public TypeBase
{
public:
	MPDSchema_EXPORT CstringType(xercesc::DOMNode* const& init);
	MPDSchema_EXPORT CstringType(CstringType const& init);
	void operator=(CstringType const& other) { m_node = other.m_node; }
	static altova::meta::ComplexType StaticInfo() { return altova::meta::ComplexType(types + _altova_ti_xs_altova_CstringType); }
	MPDSchema_EXPORT void operator=(const string_type& value);
	MPDSchema_EXPORT operator string_type();
	MPDSchema_EXPORT void SetXsiType();
};



} // namespace xs

}	// namespace MPDSchema

#endif // _ALTOVA_INCLUDED_MPDSchema_ALTOVA_xs_ALTOVA_CstringType
