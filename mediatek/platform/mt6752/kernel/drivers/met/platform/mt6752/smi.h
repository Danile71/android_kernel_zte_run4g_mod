#ifndef _SMI_H_
#define _SMI_H_

#include <linux/device.h>

struct met_smi {
	int		mode;
	int		master;
	unsigned int	port;
	unsigned int	rwtype;//0 for R+W, 1 for read, 2 for write
	unsigned int	desttype;//0 for EMI+internal mem, 1 for EMI, 3 for internal mem
	unsigned int	bustype;//0 for GMC, 1 for AXI
	//unsigned long requesttype;// 0:All, 1:ultra high, 2:pre-ultrahigh, 3:normal.
	struct kobject *kobj_bus_smi;
};

struct smi_cfg {
	int		master;
	unsigned int	port;
	unsigned int	rwtype;//0 for R+W, 1 for read, 2 for write
	unsigned int	desttype;//0 for EMI+internal mem, 1 for EMI, 3 for internal mem
	unsigned int	bustype;//0 for GMC, 1 for AXI
	//unsigned long requesttype;// 0:All, 1:ultra high, 2:pre-ultrahigh, 3:normal.
};

struct smi_mon_con {
	unsigned int	requesttype;// 0:All, 1:ultra high, 2:pre-ultrahigh, 3:normal.
};

/*
void smi_init(void);
void smi_uninit(void);

void smi_start(void);
void smi_stop(void);

int do_smi(void);
unsigned int smi_polling(unsigned int *smi_value);
*/

#endif // _SMI_H_
