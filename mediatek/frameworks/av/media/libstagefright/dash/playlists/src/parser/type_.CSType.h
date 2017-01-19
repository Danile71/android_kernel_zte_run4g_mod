#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSType
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSType



namespace MPDSchema
{

class CSType : public TypeBase
{
public:
	MPDSchema_EXPORT CSType(xercesc::DOMNode* const& init);
	MPDSchema_EXPORT CSType(CSType const& init);
	void operator=(CSType const& other) { m_node = other.m_node; }
	static altova::meta::ComplexType StaticInfo() { return altova::meta::ComplexType(types + _altova_ti_altova_CSType); }

	MemberAttribute<unsigned,_altova_mi_altova_CSType_altova_t, 0, 0> t;	// t CunsignedInt

	MemberAttribute<unsigned,_altova_mi_altova_CSType_altova_d, 0, 0> d;	// d CunsignedInt

	MemberAttribute<unsigned,_altova_mi_altova_CSType_altova_r, 0, 0> r;	// r CunsignedInt
};

}	// namespace MPDSchema

#endif // _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSType
