#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSubsetType
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSubsetType



namespace MPDSchema
{

class CSubsetType : public TypeBase
{
public:
	MPDSchema_EXPORT CSubsetType(xercesc::DOMNode* const& init);
	MPDSchema_EXPORT CSubsetType(CSubsetType const& init);
	void operator=(CSubsetType const& other) { m_node = other.m_node; }
	static altova::meta::ComplexType StaticInfo() { return altova::meta::ComplexType(types + _altova_ti_altova_CSubsetType); }

	MemberAttribute<string_type,_altova_mi_altova_CSubsetType_altova_contains, 0, 0> contains;	// contains CUIntVectorType
	MPDSchema_EXPORT void SetXsiType();
};


}	// namespace MPDSchema

#endif // _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSubsetType
