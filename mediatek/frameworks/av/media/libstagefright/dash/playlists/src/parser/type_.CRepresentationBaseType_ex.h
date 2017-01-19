#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CRepresentationBaseType_ex
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CRepresentationBaseType_ex

namespace MPDSchema
{
class CRepresentationBaseType_ex
{
public:
	CRepresentationBaseType_ex();
	CRepresentationBaseType_ex(CRepresentationBaseType* ptr);
	~CRepresentationBaseType_ex(){};

	
	dash_enum::dash_mime_type get_mime_type(){return mime_type;};
	void representationbase_init(CRepresentationBaseType* ptr);

#if (MPD_PASER_DEBUG > 1)
	void representationbase_verify(CRepresentationBaseType* ptr);
#endif

	
public:
	dash_enum::dash_mime_type mime_type;
	CRepresentationBaseType* representationbase_ptr;
};

}

#endif


