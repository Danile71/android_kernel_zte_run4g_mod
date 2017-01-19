#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CPresentationType
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CPresentationType



namespace MPDSchema
{

class CPresentationType : public TypeBase
{
public:
	MPDSchema_EXPORT CPresentationType(xercesc::DOMNode* const& init);
	MPDSchema_EXPORT CPresentationType(CPresentationType const& init);
	void operator=(CPresentationType const& other) { m_node = other.m_node; }
	static altova::meta::SimpleType StaticInfo() { return altova::meta::SimpleType(types + _altova_ti_altova_CPresentationType); }

	enum EnumValues {
		Invalid = -1,
		k_static = 0, // static
		k_dynamic = 1, // dynamic
		EnumValueCount
	};
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


}	// namespace MPDSchema

#endif // _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CPresentationType
