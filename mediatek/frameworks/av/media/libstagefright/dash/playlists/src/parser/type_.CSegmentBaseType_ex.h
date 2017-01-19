#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSegmentBaseType_ex
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSegmentBaseType_ex


namespace MPDSchema
{
	class CSegmentBaseType_ex
	{
	public:
		CSegmentBaseType_ex();
		CSegmentBaseType_ex(CSegmentBaseType* ptr);
		~CSegmentBaseType_ex(){};
	
		dash_enum::dash_init_type get_init_type(){return init_type;};
		std::string get_segmentbase_url();
		std::string get_segmentbase_range();
	
		void segmentbase_init(CSegmentBaseType* ptr);
		void segmentbase_url_init();
	//	void segmentbase_process();
		void merge_segmentbase_info(CSegmentBaseType* segmentbase_info, bool rev);
		void set_segmentbase_ptr(CSegmentBaseType* ptr){segmentbase_ptr = ptr; segmentbase_init(ptr);};
		bool exists(){return (segmentbase_ptr != NULL);};
		
#if (MPD_PASER_DEBUG > 1)
		void segmentbase_verify();
#endif
	
	public:
		dash_enum::dash_init_type init_type;
		CSegmentBaseType* segmentbase_ptr;

		unsigned int timescale_ex;
		bool timescale_exists;
		unsigned int presentationTimeOffset_ex;
		bool presentationTimeOffset_exists;
		std::string indexRange_ex;
		bool indexRange_exists;
		bool indexRangeExact_ex;
		bool indexRangeExact_exists;

		bool Initialisation_exists;
		std::string Initialisation_ex;
		bool Initialisation_byteRange_exists;
		std::string Initialisation_byteRange_ex;
		bool RepresentationIndex_exists;
		std::string RepresentationIndex_ex;
		bool RepresentationIndex_byteRange_exists;
		std::string RepresentationIndex_byteRange_ex;

		bool Initialisation_sourceURL_exists;
		bool RepresentationIndex_sourceURL_exists;
	};

}

#endif
