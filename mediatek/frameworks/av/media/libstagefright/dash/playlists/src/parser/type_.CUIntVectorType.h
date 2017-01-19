#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CUIntVectorType
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CUIntVectorType



namespace MPDSchema
{

class CUIntVectorType : public TypeBase
{
public:
	MPDSchema_EXPORT CUIntVectorType(xercesc::DOMNode* const& init);
	MPDSchema_EXPORT CUIntVectorType(CUIntVectorType const& init);
	void operator=(CUIntVectorType const& other) { m_node = other.m_node; }
	static altova::meta::SimpleType StaticInfo() { return altova::meta::SimpleType(types + _altova_ti_altova_CUIntVectorType); }
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

#endif // _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CUIntVectorType
