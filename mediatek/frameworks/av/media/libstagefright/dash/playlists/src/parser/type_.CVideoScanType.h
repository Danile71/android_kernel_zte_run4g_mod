#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CVideoScanType
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CVideoScanType



namespace MPDSchema
{

class CVideoScanType : public TypeBase
{
public:
	MPDSchema_EXPORT CVideoScanType(xercesc::DOMNode* const& init);
	MPDSchema_EXPORT CVideoScanType(CVideoScanType const& init);
	void operator=(CVideoScanType const& other) { m_node = other.m_node; }
	static altova::meta::SimpleType StaticInfo() { return altova::meta::SimpleType(types + _altova_ti_altova_CVideoScanType); }

	enum EnumValues {
		Invalid = -1,
		k_progressive = 0, // progressive
		k_interlaced = 1, // interlaced
		k_unknown = 2, // unknown
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

#endif // _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CVideoScanType
