#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSegmentTimelineType_ex
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSegmentTimelineType_ex


namespace MPDSchema
{
class CSegmentTimelineType_ex
{
   public:
	CSegmentTimelineType_ex(){segmenttimeline_ptr = NULL;};
	CSegmentTimelineType_ex(CSegmentTimelineType* ptr);
	~CSegmentTimelineType_ex(){};

	void segmenttimeline_init(CSegmentTimelineType* ptr);
	void set_s_t_value();
	void set_segmenttimeline_ptr(CSegmentTimelineType* ptr);

#if (MPD_PASER_DEBUG > 1)
	void segmenttimeline_verify();
#endif


public:
	CSegmentTimelineType* segmenttimeline_ptr;
	std::vector<CSType_ex> s_list;

};

}
#endif

