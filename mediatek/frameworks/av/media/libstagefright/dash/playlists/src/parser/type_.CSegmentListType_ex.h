#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSegmentListType_ex
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSegmentListType_ex

#include "type_.CMultipleSegmentBaseType_ex.h"

namespace MPDSchema
{
class CSegmentListType_ex : public CMultipleSegmentBaseType_ex
{
public:
	CSegmentListType_ex(){segmentlist_ptr = NULL;};
	CSegmentListType_ex(CSegmentListType* ptr);
	~CSegmentListType_ex(){};

	void CSegmentListType_init(CSegmentListType* ptr);
	//void CSegmentListType_process();
	void set_segmentlist_ptr(CSegmentListType* ptr);
	bool exists(){return (segmentlist_ptr != NULL);}
	
#if (MPD_PASER_DEBUG > 1)
	void segmentlist_verify();
#endif

	
public:
	CSegmentListType* segmentlist_ptr;

};

}
#endif
