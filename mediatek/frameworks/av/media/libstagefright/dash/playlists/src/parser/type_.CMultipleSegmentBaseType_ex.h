#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CMultipleSegmentBaseType_ex
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CMultipleSegmentBaseType_ex
#include "type_.CSegmentBaseType_ex.h"
#include "type_.CSegmentTimelineType_ex.h"

namespace MPDSchema
{
class CMultipleSegmentBaseType_ex:public CSegmentBaseType_ex
{
	public:
		CMultipleSegmentBaseType_ex();
		CMultipleSegmentBaseType_ex(CMultipleSegmentBaseType* ptr);
		~CMultipleSegmentBaseType_ex(){};

		void cmultisegmentbase_init(CMultipleSegmentBaseType* ptr, bool flag);
	//	void cmultisegmentbase_process();
		
#if (MPD_PASER_DEBUG > 1)
		void multisegmentbase_verify();
#endif

	public:
		bool is_set;
		CMultipleSegmentBaseType* cmultisegmentbase_ptr;
		CSegmentTimelineType_ex  segmenttimeline_ex;
		unsigned int duration_ex;
		unsigned int startNumber_ex;
		bool duration_exists;
		bool startNumber_exists;

		std::string BitstreamSwitching_ex;
		bool BitstreamSwitching_exists;
		bool BitstreamSwitching_byteRange_exists;
		std::string BitstreamSwitching_byteRange_ex;
};

}

#endif

