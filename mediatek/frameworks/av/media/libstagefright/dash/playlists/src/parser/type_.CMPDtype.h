#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CMPDtype
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CMPDtype


namespace MPDSchema
{

class CMPDtype : public TypeBase
{
public:
	MPDSchema_EXPORT CMPDtype(xercesc::DOMNode* const& init);
	MPDSchema_EXPORT CMPDtype(CMPDtype const& init);
	void operator=(CMPDtype const& other) { m_node = other.m_node; }
	static altova::meta::ComplexType StaticInfo() { return altova::meta::ComplexType(types + _altova_ti_altova_CMPDtype); }

	MemberAttribute<string_type,_altova_mi_altova_CMPDtype_altova_id, 0, 0> id;	// id Cstring

	MemberAttribute<string_type,_altova_mi_altova_CMPDtype_altova_profiles, 0, 0> profiles;	// profiles Cstring
	MemberAttribute<string_type,_altova_mi_altova_CMPDtype_altova_type, 0, 2> type;	// type CPresentationType

	MemberAttribute<altova::DateTime,_altova_mi_altova_CMPDtype_altova_availabilityStartTime, 0, 0> availabilityStartTime;	// availabilityStartTime CdateTime

	MemberAttribute<altova::DateTime,_altova_mi_altova_CMPDtype_altova_availabilityEndTime, 0, 0> availabilityEndTime;	// availabilityEndTime CdateTime

	MemberAttribute<altova::Duration,_altova_mi_altova_CMPDtype_altova_mediaPresentationDuration, 0, 0> mediaPresentationDuration;	// mediaPresentationDuration Cduration

	MemberAttribute<altova::Duration,_altova_mi_altova_CMPDtype_altova_minimumUpdatePeriod, 0, 0> minimumUpdatePeriod;	// minimumUpdatePeriod Cduration

	MemberAttribute<altova::Duration,_altova_mi_altova_CMPDtype_altova_minBufferTime, 0, 0> minBufferTime;	// minBufferTime Cduration

	MemberAttribute<altova::Duration,_altova_mi_altova_CMPDtype_altova_timeShiftBufferDepth, 0, 0> timeShiftBufferDepth;	// timeShiftBufferDepth Cduration

	MemberAttribute<altova::Duration,_altova_mi_altova_CMPDtype_altova_suggestedPresentationDelay, 0, 0> suggestedPresentationDelay;	// suggestedPresentationDelay Cduration

	MemberAttribute<altova::Duration,_altova_mi_altova_CMPDtype_altova_maxSegmentDuration, 0, 0> maxSegmentDuration;	// maxSegmentDuration Cduration

	MemberAttribute<altova::Duration,_altova_mi_altova_CMPDtype_altova_maxSubsegmentDuration, 0, 0> maxSubsegmentDuration;	// maxSubsegmentDuration Cduration
	MemberElement<CProgramInformationType, _altova_mi_altova_CMPDtype_altova_ProgramInformation> ProgramInformation;
	struct ProgramInformation { typedef Iterator<CProgramInformationType> iterator; };
	MemberElement<CBaseURLType, _altova_mi_altova_CMPDtype_altova_BaseURL> BaseURL;
	struct BaseURL { typedef Iterator<CBaseURLType> iterator; };
	MemberElement<xs::CanyURIType, _altova_mi_altova_CMPDtype_altova_Location> Location;
	struct Location { typedef Iterator<xs::CanyURIType> iterator; };
	MemberElement<CPeriodType, _altova_mi_altova_CMPDtype_altova_Period> Period;
	struct Period { typedef Iterator<CPeriodType> iterator; };
	MemberElement<CMetricsType, _altova_mi_altova_CMPDtype_altova_Metrics> Metrics;
	struct Metrics { typedef Iterator<CMetricsType> iterator; };
	MPDSchema_EXPORT void SetXsiType();
	
};

}	// namespace MPDSchema

#endif // _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CMPDtype
