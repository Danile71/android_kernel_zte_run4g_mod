#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CConditionalUintType
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CConditionalUintType



namespace MPDSchema
{

class CConditionalUintType : public TypeBase
{
public:
	MPDSchema_EXPORT CConditionalUintType(xercesc::DOMNode* const& init);
	MPDSchema_EXPORT CConditionalUintType(CConditionalUintType const& init);
	void operator=(CConditionalUintType const& other) { m_node = other.m_node; }
	static altova::meta::SimpleType StaticInfo() { return altova::meta::SimpleType(types + _altova_ti_altova_CConditionalUintType); }
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

#endif // _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CConditionalUintType
