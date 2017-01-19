#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSAPType
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSAPType



namespace MPDSchema
{

class CSAPType : public TypeBase
{
public:
	MPDSchema_EXPORT CSAPType(xercesc::DOMNode* const& init);
	MPDSchema_EXPORT CSAPType(CSAPType const& init);
	void operator=(CSAPType const& other) { m_node = other.m_node; }
	static altova::meta::SimpleType StaticInfo() { return altova::meta::SimpleType(types + _altova_ti_altova_CSAPType); }
	void operator= (const unsigned& value) 
	{
		altova::XmlFormatter* Formatter = static_cast<altova::XmlFormatter*>(altova::IntegerFormatter);
		XercesTreeOperations::SetValue(GetNode(), Formatter->Format(value));
	}	
		
	operator unsigned()
	{
		return CastAs<unsigned >::Do(GetNode(), 0);
	}
};


}	// namespace MPDSchema

#endif // _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSAPType
