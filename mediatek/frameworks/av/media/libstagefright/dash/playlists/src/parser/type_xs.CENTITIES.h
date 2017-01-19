#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA_xs_ALTOVA_CENTITIES
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA_xs_ALTOVA_CENTITIES



namespace MPDSchema
{

namespace xs
{	

class CENTITIES : public TypeBase
{
public:
	MPDSchema_EXPORT CENTITIES(xercesc::DOMNode* const& init);
	MPDSchema_EXPORT CENTITIES(CENTITIES const& init);
	void operator=(CENTITIES const& other) { m_node = other.m_node; }
	static altova::meta::SimpleType StaticInfo() { return altova::meta::SimpleType(types + _altova_ti_xs_altova_CENTITIES); }
	void operator= (const string_type& value) 
	{
		altova::XmlFormatter* Formatter = static_cast<altova::XmlFormatter*>(altova::AnySimpleTypeFormatter);
		XercesTreeOperations::SetValue(GetNode(), Formatter->Format(value));
	}	
		
	operator string_type()
	{
		return CastAs<string_type >::Do(GetNode(), 0);
	}
};



} // namespace xs

}	// namespace MPDSchema

#endif // _ALTOVA_INCLUDED_MPDSchema_ALTOVA_xs_ALTOVA_CENTITIES
