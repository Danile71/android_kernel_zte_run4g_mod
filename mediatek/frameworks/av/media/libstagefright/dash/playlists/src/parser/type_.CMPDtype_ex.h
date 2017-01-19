#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CMPDtype_ex
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CMPDtype_ex
namespace MPDSchema
{
class CMPDtype_ex
{
    public:
		CMPDtype_ex();
		CMPDtype_ex(CMPDtype* ptr);
		~CMPDtype_ex(){};
		void CMPD_init();

		void mpd_init(MPDSchema::CMPDtype* ptr)throw(all_exceptions);
		void mpd_process()throw(all_exceptions);
		double get_minbufferTime();
		double get_minimumUpdatePeriod();
		long long get_availabilityStartTime();
		long long get_availabilityEndTime();
		bool empty(){return dynamic_type == dash_enum::MPD_TYPE_DYNAMIC_EMPTY;};

#if (MPD_PASER_DEBUG > 1)
		void mpd_verify(CMPDtype* mpd);
#endif

	public:
		dash_enum::dash_mpd_dynamic_type dynamic_type;
		dash_enum::dash_profile_type profile_type;
		double mpd_total_time;
		dash_enum::dash_container_type mpd_container;
		CMPDtype* mpd_ptr;
		std::vector<CPeriodType_ex> period_list;

		double minbuffertime_ex;
		bool minbuffertime_exists;
		long long availablestarttime_ex;
		long long availableendtime_ex;
		bool availableendtime_exists;
		bool availablestarttime_exists;
		double miniupdatetime_ex;
		bool miniupdatetime_exists;

        bool timeshiftbufferdepth_exists;
        double timeshiftbufferdepth_ex;
        bool suggestedpresentationdelay_exists;
        double suggestedpresentationdelay_ex;


		std::string location_ex;
		bool location_exists;

        bool isSpecialDynamic;
};
}
#endif

