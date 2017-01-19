#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/dmi.h>
#include <linux/acpi.h>
#include <linux/thermal.h>
#include <linux/platform_device.h>
#include <linux/aee.h>
#include <linux/xlog.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include <mach/system.h>
#include "mach/mtk_thermal_monitor.h"
#include "mach/mt_typedefs.h"
#include "mach/mt_thermal.h"


#include <mach/upmu_common_sw.h>
#include <mach/upmu_hw.h>
#include <mach/mt_pmic_wrap.h>
#include <mach/mt6311.h>
#include "mach/mt_cpufreq.h"

static unsigned int interval = 0; /* seconds, 0 : no auto polling */
static int trip_temp[10] = {125000,110000,100000,90000,80000,70000,65000,60000,55000,50000};

#if 1
static unsigned int cl_dev_sysrst_state = 0;
#endif
static struct thermal_zone_device *thz_dev;

#if 1
static struct thermal_cooling_device *cl_dev_sysrst= NULL;;
//static struct thermal_cooling_device *cl_dev_6311= NULL;;

#endif
static int mtkts6311_debug_log = 0;
static int kernelmode = 0;

static int g_THERMAL_TRIP[10] = {0,0,0,0,0,0,0,0,0,0};
static int num_trip=0;


static char g_bind0[20]="mtkts6311-sysrst";
static char g_bind1[20]="cpu01";
static char g_bind2[20]="";
static char g_bind3[20]="";
static char g_bind4[20]="";
static char g_bind5[20]="";
static char g_bind6[20]="";
static char g_bind7[20]="";
static char g_bind8[20]="";
static char g_bind9[20]="";


#define mtkts6311_TEMP_CRIT 150000 /* 150.000 degree Celsius */


#define mtkts6331_dprintk(fmt, args...)   \
do {									\
	if (mtkts6311_debug_log) {				\
		xlog_printk(ANDROID_LOG_INFO, "Thermal/PMIC_6311", fmt, ##args); \
	}								   \
} while(0)

#define PMIC6333_INT_TEMP_CUNT 0xF
//static kal_uint32 tempsetting_count=0;
typedef struct{
    INT32 regsetting;
    INT32 Temperature;
}pmic6333_TEMPERATURE;





#define mtkts6311_dprintk(fmt, args...)   \
do {									\
	if (mtkts6311_debug_log) {				\
		xlog_printk(ANDROID_LOG_INFO, "Power/6311_Thermal", fmt, ##args); \
	}								   \
} while(0)


extern int is_ext_buck_exist(void);


static int mtkts6311_get_temp(struct thermal_zone_device *thermal,
				   unsigned long *t)
{
	mtkts6311_dprintk("[mtkts6311_get_temp]\n");

	*t=50000;//default 50 degree


	if(mt6311_get_pmu_thr_status()==0x0)//T<110
	{
		*t=60000;
		mtkts6331_dprintk("mt6311_get_pmu_thr_status()==0x0\n");
		if(mt6311_get_rg_thr_l_int_status()==1)//receive interrupt
        {

			*t=80000;//60 degree
			printk("[Thermal/PMIC_6331]mt6311_get_thr_l_int_status()==1,6311 T=80 deg\n");
			mt6311_clr_thr_l_int_status();

			mt6311_set_rg_strup_thr_110_clr(1);//Set 0x28 bit 0=1 ;CLR 110 INT
			udelay(10);
			mt6311_set_rg_strup_thr_110_clr(0);

			mt6311_set_rg_strup_thr_110_irq_en(0);//Set 0x28 bit 2=0 ; disable 110 INT
            mt6311_set_rg_strup_thr_125_irq_en(1);//Set 0x28 bit 3=1; EN >125 INT

		}
	}
	else if(mt6311_get_pmu_thr_status()==0x1)//110<T<125
	{
		mtkts6331_dprintk("mt6311_get_pmu_thr_status()==0x1\n");

		*t=115000;//115 degree

		printk("[Thermal/PMIC_6331]6311 pmic temp : 110<T<125 deg\n");
	}
	else if(mt6311_get_pmu_thr_status()==0x3)//125<T<150
	{
		*t=130000;//130 degree
		//printk("[Thermal/PMIC_6331]mt6311_get_pmu_thr_status()==0x3\n");
		printk("[Thermal/PMIC_6331]6311 pmic temp 125<T<150 deg\n");

		if(mt6311_get_rg_thr_h_int_status()==1){//receive interrupt

            *t=135000;//135 degree
			printk("[Thermal/PMIC_6331]6311 pmic temp =140 deg\n");
            //printk("[Thermal/PMIC_6331]mt6311_get_rg_thr_h_int_status()==1,6311 T=135 deg\n");

            mtkts6331_dprintk("mt6311_get_thr_h_int_status()==1\n");
			mt6311_clr_thr_h_int_status();

			mt6311_set_rg_strup_thr_125_clr(1);//Set 0x28 bit 1=1 ;CLR 125 INT
			udelay(10);
	        mt6311_set_rg_strup_thr_125_clr(0);

			mt6311_set_rg_strup_thr_125_irq_en(0);//Set 0x28 bit 3=0 ; disable 125 INT
			mt6311_set_rg_strup_thr_110_irq_en(1);//Set 0x28 bit 2=1; EN <110 INT
		}

	}
	mtkts6331_dprintk("6311 pmic temp =%d\n",*t);
	return 0;
}

static int mtkts6311_bind(struct thermal_zone_device *thermal,
			struct thermal_cooling_device *cdev)
{
	int table_val=0;

	if(!strcmp(cdev->type, g_bind0))
	{
		table_val = 0;
	}
	else if(!strcmp(cdev->type, g_bind1))
	{
		table_val = 1;
	}
	else if(!strcmp(cdev->type, g_bind2))
	{
		table_val = 2;
	}
	else if(!strcmp(cdev->type, g_bind3))
	{
		table_val = 3;
	}
	else if(!strcmp(cdev->type, g_bind4))
	{
		table_val = 4;
	}
	else if(!strcmp(cdev->type, g_bind5))
	{
		table_val = 5;
	}
	else if(!strcmp(cdev->type, g_bind6))
	{
		table_val = 6;
	}
	else if(!strcmp(cdev->type, g_bind7))
	{
		table_val = 7;
	}
	else if(!strcmp(cdev->type, g_bind8))
	{
		table_val = 8;
	}
	else if(!strcmp(cdev->type, g_bind9))
	{
		table_val = 9;
	}
	else
	{
		return 0;
	}

	if (mtk_thermal_zone_bind_cooling_device(thermal, table_val, cdev)) {
		mtkts6311_dprintk("[mtkts6311_bind] error binding cooling dev\n");
		return -EINVAL;
	} else {
		mtkts6311_dprintk("[mtkts6311_bind] binding OK, %d\n", table_val);
	}

	return 0;
}

static int mtkts6311_unbind(struct thermal_zone_device *thermal,
			  struct thermal_cooling_device *cdev)
{
	int table_val=0;

	if(!strcmp(cdev->type, g_bind0))
	{
		table_val = 0;
	}
	else if(!strcmp(cdev->type, g_bind1))
	{
		table_val = 1;
	}
	else if(!strcmp(cdev->type, g_bind2))
	{
		table_val = 2;
	}
	else if(!strcmp(cdev->type, g_bind3))
	{
		table_val = 3;
	}
	else if(!strcmp(cdev->type, g_bind4))
	{
		table_val = 4;
	}
	else if(!strcmp(cdev->type, g_bind5))
	{
		table_val = 5;
	}
	else if(!strcmp(cdev->type, g_bind6))
	{
		table_val = 6;
	}
	else if(!strcmp(cdev->type, g_bind7))
	{
		table_val = 7;
	}
	else if(!strcmp(cdev->type, g_bind8))
	{
		table_val = 8;
	}
	else if(!strcmp(cdev->type, g_bind9))
	{
		table_val = 9;
	}
	else
		return 0;

	if (thermal_zone_unbind_cooling_device(thermal, table_val, cdev)) {
		mtkts6311_dprintk("[mtkts6311_unbind] error unbinding cooling dev\n");
		return -EINVAL;
	} else {
		mtkts6311_dprintk("[mtkts6311_unbind] unbinding OK\n");
	}

	return 0;
}

static int mtkts6311_get_mode(struct thermal_zone_device *thermal,
				enum thermal_device_mode *mode)
{
	*mode = (kernelmode) ? THERMAL_DEVICE_ENABLED
				 : THERMAL_DEVICE_DISABLED;
	return 0;
}

static int mtkts6311_set_mode(struct thermal_zone_device *thermal,
				enum thermal_device_mode mode)
{
	kernelmode = mode;
	return 0;
}

static int mtkts6311_get_trip_type(struct thermal_zone_device *thermal, int trip,
				 enum thermal_trip_type *type)
{
	*type = g_THERMAL_TRIP[trip];
	return 0;
}

static int mtkts6311_get_trip_temp(struct thermal_zone_device *thermal, int trip,
				 unsigned long *temp)
{
	*temp = trip_temp[trip];
	return 0;
}

static int mtkts6311_get_crit_temp(struct thermal_zone_device *thermal,
				 unsigned long *temperature)
{
	*temperature = mtkts6311_TEMP_CRIT;
	return 0;
}

/* bind callback functions to thermalzone */
static struct thermal_zone_device_ops mtkts6311_dev_ops = {
	.bind = mtkts6311_bind,
	.unbind = mtkts6311_unbind,
	.get_temp = mtkts6311_get_temp,
	.get_mode = mtkts6311_get_mode,
	.set_mode = mtkts6311_set_mode,
	.get_trip_type = mtkts6311_get_trip_type,
	.get_trip_temp = mtkts6311_get_trip_temp,
	.get_crit_temp = mtkts6311_get_crit_temp,
};

#if 1
/*
static int mtkts6311_get_max_state(struct thermal_cooling_device *cdev,
				unsigned long *state)
{
	mtkts6311_dprintk("[mtkts6311_get_max_state] \n");
	*state = 1;
	return 0;
}

static int mtkts6311_get_cur_state(struct thermal_cooling_device *cdev,
				unsigned long *state)
{
	mtkts6311_dprintk("[mtkts6311_get_cur_state] \n");
    *state = "cpu01";

	return 0;
}

static int mtkts6311_set_cur_state(struct thermal_cooling_device *cdev,
				unsigned long state)
{
	mtkts6311_dprintk("[mtkts6311_set_cur_state] \n");
 	mt_cpufreq_thermal_protect(700);

	return 0;

}

static struct thermal_cooling_device_ops mtktscpu_cooler_cpu_ops = {
	.get_max_state = mtkts6311_get_max_state,
	.get_cur_state = mtkts6311_get_cur_state,
	.set_cur_state = mtkts6311_set_cur_state,
};
*/


static int ts6311_sysrst_get_max_state(struct thermal_cooling_device *cdev,
				 unsigned long *state)
{
	mtkts6311_dprintk("ts6311_sysrst_get_max_state!!!\n");
	*state = 1;
	return 0;
}
static int ts6311_sysrst_get_cur_state(struct thermal_cooling_device *cdev,
				 unsigned long *state)
{
	mtkts6311_dprintk("ts6311_sysrst_get_cur_state = %d \n",cl_dev_sysrst_state);
	*state = cl_dev_sysrst_state;
	return 0;
}
static int ts6311_sysrst_set_cur_state(struct thermal_cooling_device *cdev,
				 unsigned long state)
{
	mtkts6311_dprintk("ts6311_sysrst_set_cur_state = %d \n",cl_dev_sysrst_state);
	cl_dev_sysrst_state = state;
	if(cl_dev_sysrst_state == 1)
	{
		printk("Power/6311_Thermal: reset, reset, reset!!!");
		printk("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
		printk("*****************************************");
		printk("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");

		BUG();
	}
	return 0;
}

static struct thermal_cooling_device_ops mtkts6311_cooling_sysrst_ops = {
	.get_max_state = ts6311_sysrst_get_max_state,
	.get_cur_state = ts6311_sysrst_get_cur_state,
	.set_cur_state = ts6311_sysrst_set_cur_state,
};


int mtkts6311_register_cooler(void)
{
	cl_dev_sysrst = mtk_thermal_cooling_device_register("mtkts6311-sysrst", NULL,
					   &mtkts6311_cooling_sysrst_ops);
/*
    cl_dev_6311 = mtk_thermal_cooling_device_register("cpu01", NULL,
					&mtktscpu_cooler_cpu_ops);
*/
   	return 0;
}

#endif

static int mtkts6311_read(struct seq_file *m, void *v)
{


		seq_printf(m , "[ mtkts6311_read] trip_0_temp=%d,trip_1_temp=%d,trip_2_temp=%d,trip_3_temp=%d,trip_4_temp=%d,\n\
trip_5_temp=%d,trip_6_temp=%d,trip_7_temp=%d,trip_8_temp=%d,trip_9_temp=%d,\n\
g_THERMAL_TRIP_0=%d,g_THERMAL_TRIP_1=%d,g_THERMAL_TRIP_2=%d,g_THERMAL_TRIP_3=%d,g_THERMAL_TRIP_4=%d,\n\
g_THERMAL_TRIP_5=%d,g_THERMAL_TRIP_6=%d,g_THERMAL_TRIP_7=%d,g_THERMAL_TRIP_8=%d,g_THERMAL_TRIP_9=%d,\n\
cooldev0=%s,cooldev1=%s,cooldev2=%s,cooldev3=%s,cooldev4=%s,\n\
cooldev5=%s,cooldev6=%s,cooldev7=%s,cooldev8=%s,cooldev9=%s,time_ms=%d\n",
								trip_temp[0],trip_temp[1],trip_temp[2],trip_temp[3],trip_temp[4],
								trip_temp[5],trip_temp[6],trip_temp[7],trip_temp[8],trip_temp[9],
								g_THERMAL_TRIP[0],g_THERMAL_TRIP[1],g_THERMAL_TRIP[2],g_THERMAL_TRIP[3],g_THERMAL_TRIP[4],
								g_THERMAL_TRIP[5],g_THERMAL_TRIP[6],g_THERMAL_TRIP[7],g_THERMAL_TRIP[8],g_THERMAL_TRIP[9],
								g_bind0,g_bind1,g_bind2,g_bind3,g_bind4,g_bind5,g_bind6,g_bind7,g_bind8,g_bind9,
								interval*1000);


	return 0;
}

int mtkts6311_register_thermal(void);
void mtkts6311_unregister_thermal(void);

static ssize_t mtkts6311_write(struct file *file, const char __user *buffer, size_t count, loff_t *data)
{
	int len=0,time_msec=0;
	int trip[10]={0};
	int t_type[10]={0};
	int i;
	char bind0[20],bind1[20],bind2[20],bind3[20],bind4[20];
	char bind5[20],bind6[20],bind7[20],bind8[20],bind9[20];
	char desc[512];


	len = (count < (sizeof(desc) - 1)) ? count : (sizeof(desc) - 1);
	if (copy_from_user(desc, buffer, len))
	{
		return 0;
	}
	desc[len] = '\0';

	if (sscanf(desc, "%d %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d",
							&num_trip, &trip[0],&t_type[0],bind0, &trip[1],&t_type[1],bind1,
												 &trip[2],&t_type[2],bind2, &trip[3],&t_type[3],bind3,
												 &trip[4],&t_type[4],bind4, &trip[5],&t_type[5],bind5,
											   &trip[6],&t_type[6],bind6, &trip[7],&t_type[7],bind7,
												 &trip[8],&t_type[8],bind8, &trip[9],&t_type[9],bind9,
												 &time_msec) == 32)
	{
		mtkts6311_dprintk("[mtkts6311_write] mtkts6311_unregister_thermal\n");
		mtkts6311_unregister_thermal();

		for(i=0; i<num_trip; i++)
			g_THERMAL_TRIP[i] = t_type[i];

		g_bind0[0]=g_bind1[0]=g_bind2[0]=g_bind3[0]=g_bind4[0]=g_bind5[0]=g_bind6[0]=g_bind7[0]=g_bind8[0]=g_bind9[0]='\0';

		for(i=0; i<20; i++)
		{
			g_bind0[i]=bind0[i];
			g_bind1[i]=bind1[i];
			g_bind2[i]=bind2[i];
			g_bind3[i]=bind3[i];
			g_bind4[i]=bind4[i];
			g_bind5[i]=bind5[i];
			g_bind6[i]=bind6[i];
			g_bind7[i]=bind7[i];
			g_bind8[i]=bind8[i];
			g_bind9[i]=bind9[i];
		}

		mtkts6311_dprintk("[mtkts6311_write] g_THERMAL_TRIP_0=%d,g_THERMAL_TRIP_1=%d,g_THERMAL_TRIP_2=%d,g_THERMAL_TRIP_3=%d,g_THERMAL_TRIP_4=%d,\
g_THERMAL_TRIP_5=%d,g_THERMAL_TRIP_6=%d,g_THERMAL_TRIP_7=%d,g_THERMAL_TRIP_8=%d,g_THERMAL_TRIP_9=%d,\n",
													g_THERMAL_TRIP[0],g_THERMAL_TRIP[1],g_THERMAL_TRIP[2],g_THERMAL_TRIP[3],g_THERMAL_TRIP[4],
													g_THERMAL_TRIP[5],g_THERMAL_TRIP[6],g_THERMAL_TRIP[7],g_THERMAL_TRIP[8],g_THERMAL_TRIP[9]);
		mtkts6311_dprintk("[mtkts6311_write] cooldev0=%s,cooldev1=%s,cooldev2=%s,cooldev3=%s,cooldev4=%s,\
cooldev5=%s,cooldev6=%s,cooldev7=%s,cooldev8=%s,cooldev9=%s\n",
													g_bind0,g_bind1,g_bind2,g_bind3,g_bind4,g_bind5,g_bind6,g_bind7,g_bind8,g_bind9);

		for(i=0; i<num_trip; i++)
		{
			trip_temp[i]=trip[i];
		}

		interval=time_msec / 1000;

		mtkts6311_dprintk("[mtkts6311_write] trip_0_temp=%d,trip_1_temp=%d,trip_2_temp=%d,trip_3_temp=%d,trip_4_temp=%d,\
trip_5_temp=%d,trip_6_temp=%d,trip_7_temp=%d,trip_8_temp=%d,trip_9_temp=%d,time_ms=%d\n",
						trip_temp[0],trip_temp[1],trip_temp[2],trip_temp[3],trip_temp[4],
						trip_temp[5],trip_temp[6],trip_temp[7],trip_temp[8],trip_temp[9],interval*1000);

		mtkts6311_dprintk("[mtkts6311_write] mtkts6311_register_thermal\n");
		mtkts6311_register_thermal();

		return count;
	}
	else
	{
		mtkts6311_dprintk("[mtkts6311_write] bad argument\n");
	}

	return -EINVAL;
}


int mtkts6311_register_thermal(void)
{
	mtkts6311_dprintk("[mtkts6311_register_thermal] \n");

	/* trips : trip 0~2 */
	thz_dev = mtk_thermal_zone_device_register("mtkts6311", num_trip, NULL,
					  &mtkts6311_dev_ops, 0, 0, 0, interval*1000);

	return 0;
}

#if 1
void mtkts6311_unregister_cooler(void)
{
	if (cl_dev_sysrst) {
		mtk_thermal_cooling_device_unregister(cl_dev_sysrst);
		cl_dev_sysrst = NULL;
	}
}
#endif

void mtkts6311_unregister_thermal(void)
{
	mtkts6311_dprintk("[mtkts6311_unregister_thermal] \n");

	if (thz_dev) {
		mtk_thermal_zone_device_unregister(thz_dev);
		thz_dev = NULL;
	}
}

#if 0
static int mtkts63116333_thermal_zone_handler(void )
{
	int temp=0;

    mtkts6311_dprintk( "[mtkts63116333_thermal_zone_handler] ,tempsetting_count=0x%x\n",tempsetting_count);

	mt6333_6311_int = get_thermal_mt6333_6311_int_status();

	if(mt6333_6311_int==1){ //receive thermal 6311 INT
		tempsetting_count--;
        if(tempsetting_count <=0 )//140 degree
		{
			tempsetting_count = 0;
            printk("6333 temp is over 140 degree\n");
		}

		set_thermal_mt6333_6311_int_status(0);
		//increase temperature
        mt6333_set_rg_strup_ther_rg_th((pmic6333_temp_map[tempsetting_count].regsetting));

        mtkts6311_dprintk("increase change INT threshold to tempsetting_count=%d\n",tempsetting_count);
	}
    else{
        tempsetting_count++;
        if(tempsetting_count >= PMIC6333_INT_TEMP_CUNT)//65 degree
		{
			tempsetting_count = PMIC6333_INT_TEMP_CUNT;//65 degree
            mtkts6311_dprintk("6333 temp is below 65 degree\n");
		}
        //decrease temperature
        mt6333_set_rg_strup_ther_rg_th((pmic6333_temp_map[tempsetting_count].regsetting));
        mtkts6311_dprintk("decrease change INT threshold to tempsetting_count=%d\n",tempsetting_count);
    }

    mtkts6311_dprintk("decrease pmic6333_temp_map[%d].regsetting=0x%x\n",tempsetting_count,pmic6333_temp_map[tempsetting_count].regsetting);
    mtkts6311_dprintk("decrease pmic6333_temp_map[%d].Temperature=%d\n",tempsetting_count,pmic6333_temp_map[tempsetting_count].Temperature);

    temp = pmic6333_temp_map[tempsetting_count].Temperature;

    if(temp >= 70) //printing high temperature
        printk("[Power/6311_Thermal] Buck 6333 T=%d\n",temp);


    mtkts6311_dprintk("mt6333_6311_int=%d\n",mt6333_6311_int);

    return (temp*1000);

}
#endif

static void mtktspmic6311_thermal_zone_init(void)
{

	mtkts6311_dprintk("[mtktspmic6311_thermal_zone_init] \n");

	mt6311_set_rg_thrdet_sel(0);          //0x6C  bit0=0 ; thermal 110/125 EN
    mt6311_set_rg_strup_thr_125_irq_en(1);//0x28  bit3=1 ; >125 INT EN
    mt6311_set_rg_strup_thr_110_irq_en(0);//0x28  bit3=1 ; <110 INT DIS
    mt6311_set_rg_int_en(1);              //0x18  bit1=1 ; Top INT EN

	mt6311_clr_thr_l_int_status();
	mt6311_clr_thr_h_int_status();
}

static int mtkts6311_open(struct inode *inode, struct file *file)
{
	return single_open(file, mtkts6311_read, NULL);
}

static const struct file_operations mtkts6311_fops = {
	.owner = THIS_MODULE,
	.open = mtkts6311_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.write = mtkts6311_write,
	.release = single_release,
};


static int __init mtkts6311_init(void)
{
	int err = 0;
	struct proc_dir_entry *entry = NULL;
	struct proc_dir_entry *mtkts6311_dir = NULL;

	mtkts6311_dprintk("[mtkts6311_init] \n");

	//return 1 means with 6311, else return 0
	if(is_ext_buck_exist() == 0)
	{
		return err;
	}

	mtktspmic6311_thermal_zone_init();


	err = mtkts6311_register_cooler();
	if(err)
		return err;

	err = mtkts6311_register_thermal();
	if (err)
		goto err_unreg;

	mtkts6311_dir = proc_mkdir("mtkts6311", NULL);
	if (!mtkts6311_dir)
	{
		mtkts6311_dprintk("[mtkts6311_init]: mkdir /proc/mtkts6311 failed\n");
	}
	else
	{
		entry = proc_create("mtkts6311", S_IRUGO | S_IWUSR | S_IWGRP,mtkts6311_dir, &mtkts6311_fops);
        //entry = proc_create("mtkts6311_log", S_IRUGO | S_IWUSR, mtkts6311_dir, &mtkts6311_log_fops);
	}


	return 0;

err_unreg:

		mtkts6311_unregister_cooler();

		return err;
}

static void __exit mtkts6311_exit(void)
{
	mtkts6311_dprintk("[mtkts6311_exit] \n");
	mtkts6311_unregister_thermal();

	mtkts6311_unregister_cooler();

}

late_initcall(mtkts6311_init);
module_exit(mtkts6311_exit);


