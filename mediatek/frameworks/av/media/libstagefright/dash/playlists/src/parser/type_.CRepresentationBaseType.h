#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CRepresentationBaseType
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CRepresentationBaseType



namespace MPDSchema
{

class CRepresentationBaseType : public TypeBase
{
public:
	MPDSchema_EXPORT CRepresentationBaseType(xercesc::DOMNode* const& init);
	MPDSchema_EXPORT CRepresentationBaseType(CRepresentationBaseType const& init);
	void operator=(CRepresentationBaseType const& other) { m_node = other.m_node; }
	static altova::meta::ComplexType StaticInfo() { return altova::meta::ComplexType(types + _altova_ti_altova_CRepresentationBaseType); }

	MemberAttribute<string_type,_altova_mi_altova_CRepresentationBaseType_altova_profiles, 0, 0> profiles;	// profiles Cstring

	MemberAttribute<unsigned,_altova_mi_altova_CRepresentationBaseType_altova_width, 0, 0> width;	// width CunsignedInt

	MemberAttribute<unsigned,_altova_mi_altova_CRepresentationBaseType_altova_height, 0, 0> height;	// height CunsignedInt

	MemberAttribute<string_type,_altova_mi_altova_CRepresentationBaseType_altova_sar, 0, 0> sar;	// sar Cstring

	MemberAttribute<string_type,_altova_mi_altova_CRepresentationBaseType_altova_frameRate, 0, 0> frameRate;	// frameRate Cstring

	MemberAttribute<string_type,_altova_mi_altova_CRepresentationBaseType_altova_audioSamplingRate, 0, 0> audioSamplingRate;	// audioSamplingRate Cstring

	MemberAttribute<string_type,_altova_mi_altova_CRepresentationBaseType_altova_mimeType, 0, 0> mimeType;	// mimeType Cstring

	MemberAttribute<string_type,_altova_mi_altova_CRepresentationBaseType_altova_segmentProfiles, 0, 0> segmentProfiles;	// segmentProfiles Cstring

	MemberAttribute<string_type,_altova_mi_altova_CRepresentationBaseType_altova_codecs, 0, 0> codecs;	// codecs Cstring

	MemberAttribute<double,_altova_mi_altova_CRepresentationBaseType_altova_maximumSAPPeriod, 0, 0> maximumSAPPeriod;	// maximumSAPPeriod Cdouble

	MemberAttribute<unsigned,_altova_mi_altova_CRepresentationBaseType_altova_startWithSAP, 0, 0> startWithSAP;	// startWithSAP CSAPType

	MemberAttribute<double,_altova_mi_altova_CRepresentationBaseType_altova_maxPlayoutRate, 0, 0> maxPlayoutRate;	// maxPlayoutRate Cdouble

	MemberAttribute<bool,_altova_mi_altova_CRepresentationBaseType_altova_codingDependency, 0, 0> codingDependency;	// codingDependency Cboolean
	MemberAttribute<string_type,_altova_mi_altova_CRepresentationBaseType_altova_scanType, 0, 3> scanType;	// scanType CVideoScanType
	MemberElement<CDescriptorType, _altova_mi_altova_CRepresentationBaseType_altova_FramePacking> FramePacking;
	struct FramePacking { typedef Iterator<CDescriptorType> iterator; };
	MemberElement<CDescriptorType, _altova_mi_altova_CRepresentationBaseType_altova_AudioChannelConfiguration> AudioChannelConfiguration;
	struct AudioChannelConfiguration { typedef Iterator<CDescriptorType> iterator; };
	MemberElement<CContentProtectionType, _altova_mi_altova_CRepresentationBaseType_altova_ContentProtection> ContentProtection;
	struct ContentProtection { typedef Iterator<CContentProtectionType> iterator; };
	MPDSchema_EXPORT void SetXsiType();
};



}	// namespace MPDSchema

#endif // _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CRepresentationBaseType
