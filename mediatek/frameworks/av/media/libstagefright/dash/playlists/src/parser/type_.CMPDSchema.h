#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CMPDSchema
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CMPDSchema



namespace MPDSchema
{

class CMPDSchema : public TypeBase
{
public:
	MPDSchema_EXPORT CMPDSchema(xercesc::DOMNode* const& init);
	MPDSchema_EXPORT CMPDSchema(CMPDSchema const& init);
	void operator=(CMPDSchema const& other) { m_node = other.m_node; }
	static altova::meta::ComplexType StaticInfo() { return altova::meta::ComplexType(types + _altova_ti_altova_CMPDSchema); }
	MemberElement<CMPDtype, _altova_mi_altova_CMPDSchema_altova_MPD> MPD;
	struct MPD { typedef Iterator<CMPDtype> iterator; };
	MPDSchema_EXPORT void SetXsiType();

	// document functions
	MPDSchema_EXPORT static CMPDSchema* LoadFromFile(const string_type& fileName);
	MPDSchema_EXPORT static CMPDSchema* LoadFromString(const string_type& xml);
	MPDSchema_EXPORT static CMPDSchema LoadFromBinary(const std::vector<unsigned char>& data);
	MPDSchema_EXPORT void SaveToFile( const string_type& fileName, bool prettyPrint );
	MPDSchema_EXPORT void SaveToFile( const string_type& fileName, bool prettyPrint, const string_type& encoding );
	MPDSchema_EXPORT void SaveToFile( const string_type& fileName, bool prettyPrint, const string_type& encoding, bool bBigEndian, bool bBOM );
	MPDSchema_EXPORT string_type SaveToString(bool prettyPrint);
	MPDSchema_EXPORT std::vector<unsigned char> SaveToBinary(bool prettyPrint);
	MPDSchema_EXPORT std::vector<unsigned char> SaveToBinary(bool prettyPrint, const string_type& encoding);
	MPDSchema_EXPORT std::vector<unsigned char> SaveToBinary(bool prettyPrint, const string_type& encoding, bool bBigEndian, bool bBOM);
 	MPDSchema_EXPORT static CMPDSchema CreateDocument();
	MPDSchema_EXPORT void DestroyDocument();
	MPDSchema_EXPORT void SetDTDLocation(const string_type& dtdLocation);
	MPDSchema_EXPORT void SetSchemaLocation(const string_type& schemaLocation);

protected:
	XercesTreeOperations::DocumentType GetDocumentNode() { return (XercesTreeOperations::DocumentType)m_node; }
};


}	// namespace MPDSchema

#endif // _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CMPDSchema
