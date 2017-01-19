#ifndef _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSType_ex
#define _ALTOVA_INCLUDED_MPDSchema_ALTOVA__ALTOVA_CSType_ex

namespace MPDSchema
{

class CSType_ex
{
public:
  	 CSType_ex();
	 CSType_ex(CSType* ptr);
	 ~CSType_ex(){};
	 
	void set_S_ptr(CSType* ptr){S_ptr = ptr;};
	void set_start_time_for_r(unsigned int value);
	int get_start_time_for_r(int r_index);

	bool t_present();
	bool d_present();
	bool r_present();

	void set_t(unsigned int value);
	void set_d(unsigned int value);

	unsigned int get_t();
	unsigned int get_d();
	unsigned int get_r();
	
#if (MPD_PASER_DEBUG > 1)
	void s_verify();
#endif

public:
	std::vector<unsigned int> start_time_for_r;
	CSType* S_ptr;

	unsigned int t_ex;
	unsigned int d_ex;
	unsigned int r_ex;

	bool t_exists;
	bool d_exists;
	bool r_exists;

};
}
#endif
