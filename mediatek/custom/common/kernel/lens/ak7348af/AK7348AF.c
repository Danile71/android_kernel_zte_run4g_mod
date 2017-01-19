/*
 * MD218A voice coil motor driver
 *
 *
 */

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include "AK7348AF.h"
#include "../camera/kd_camera_hw.h"

#define LENS_I2C_BUSNUM 0
static struct i2c_board_info __initdata kd_lens_dev={ I2C_BOARD_INFO("AK7348AF", 0x18)};


#define AK7348AF_DRVNAME "AK7348AF"
#define AK7348AF_VCM_WRITE_ID           0x18

#define AK7348AF_DEBUG
#ifdef AK7348AF_DEBUG
#define AK7348AFDB printk
#else
#define AK7348AFDB(x,...)
#endif

static spinlock_t g_AK7348AF_SpinLock;

static struct i2c_client * g_pstAK7348AF_I2Cclient = NULL;

static dev_t g_AK7348AF_devno;
static struct cdev * g_pAK7348AF_CharDrv = NULL;
static struct class *actuator_class = NULL;

static int  g_s4AK7348AF_Opened = 0;
static long g_i4MotorStatus = 0;
static long g_i4Dir = 0;
static unsigned long g_u4AK7348AF_INF = 0;
static unsigned long g_u4AK7348AF_MACRO = 1023;
static unsigned long g_u4TargetPosition = 0;
static unsigned long g_u4CurrPosition   = 0;

static int g_sr = 3;

static int s4AK7348AF_ReadReg(u16 a_u2Addr, unsigned short * RegData)
{
    int  i4RetValue = 0;
    char pBuff[1] = {(char)(a_u2Addr)};

    g_pstAK7348AF_I2Cclient->addr = (0x18 >> 1);
    g_pstAK7348AF_I2Cclient->ext_flag |= I2C_A_FILTER_MSG;

    i4RetValue = i2c_master_send(g_pstAK7348AF_I2Cclient, pBuff, 1);
    if (i4RetValue < 0 ) 
    {
        AK7348AFDB("[AK7348AF] read I2C send failed!!\n");
        return -1;
    }

    i4RetValue = i2c_master_recv(g_pstAK7348AF_I2Cclient, (u8*)RegData, 1);
    AK7348AFDB("[AK7348AF]I2C r (%x %x) \n",a_u2Addr,*RegData);
    if (i4RetValue != 1) 
    {
        AK7348AFDB("[AK7348AF] I2C read failed!! \n");
        return  -1;
    }
    return 0;
}

static int s4AK7348AF_WriteReg(u16 a_u2Addr, u16 a_u2Data)
{
    int  i4RetValue = 0;

    char puSendCmd[2] = {(char)(a_u2Addr) , (char)(a_u2Data)};
    g_pstAK7348AF_I2Cclient->addr = (0x18 >> 1);
    //g_pstAK7348AF_I2Cclient->ext_flag |= I2C_A_FILTER_MSG;
    i4RetValue = i2c_master_send(g_pstAK7348AF_I2Cclient, puSendCmd, 2);
    if (i4RetValue < 0) 
    {
        AK7348AFDB("[AK7348AF]1 I2C send failed!! 1\n");
        return -1;
    }

    return 0;
}

inline static int getAK7348AFInfo(__user stAK7348AF_MotorInfo * pstMotorInfo)
{
    stAK7348AF_MotorInfo stMotorInfo;
    stMotorInfo.u4MacroPosition   = g_u4AK7348AF_MACRO;
    stMotorInfo.u4InfPosition     = g_u4AK7348AF_INF;
    stMotorInfo.u4CurrentPosition = g_u4CurrPosition;
    stMotorInfo.bIsSupportSR      = TRUE;

	if (g_i4MotorStatus == 1)	{stMotorInfo.bIsMotorMoving = 1;}
	else						{stMotorInfo.bIsMotorMoving = 0;}

	if (g_s4AK7348AF_Opened >= 1)	{stMotorInfo.bIsMotorOpen = 1;}
	else						{stMotorInfo.bIsMotorOpen = 0;}

    if(copy_to_user(pstMotorInfo , &stMotorInfo , sizeof(stAK7348AF_MotorInfo)))
    {
        AK7348AFDB("[AK7348AF] copy to user failed when getting motor information \n");
    }

    return 0;
}

#ifdef LensdrvCM3
inline static int getAK7348AFMETA(__user stAK7348AF_MotorMETAInfo * pstMotorMETAInfo)
{
    stAK7348AF_MotorMETAInfo stMotorMETAInfo;
    stMotorMETAInfo.Aperture=2.8;      //fn
	stMotorMETAInfo.Facing=1;   
	stMotorMETAInfo.FilterDensity=1;   //X
	stMotorMETAInfo.FocalDistance=1.0;  //diopters
	stMotorMETAInfo.FocalLength=34.0;  //mm
	stMotorMETAInfo.FocusRange=1.0;    //diopters
	stMotorMETAInfo.InfoAvalibleApertures=2.8;
	stMotorMETAInfo.InfoAvalibleFilterDensity=1;
	stMotorMETAInfo.InfoAvalibleFocalLength=34.0;
	stMotorMETAInfo.InfoAvalibleHypeDistance=1.0;
	stMotorMETAInfo.InfoAvalibleMinFocusDistance=1.0;
	stMotorMETAInfo.InfoAvalibleOptStabilization=0;
	stMotorMETAInfo.OpticalAxisAng[0]=0.0;
	stMotorMETAInfo.OpticalAxisAng[1]=0.0;
	stMotorMETAInfo.Position[0]=0.0;
	stMotorMETAInfo.Position[1]=0.0;
	stMotorMETAInfo.Position[2]=0.0;
	stMotorMETAInfo.State=0;
	stMotorMETAInfo.u4OIS_Mode=0;
	
	if(copy_to_user(pstMotorMETAInfo , &stMotorMETAInfo , sizeof(stAK7348AF_MotorMETAInfo)))
	{
		AK7348AFDB("[AK7348AF] copy to user failed when getting motor information \n");
	}

    return 0;
}
#endif

inline static int moveAK7348AF(unsigned long a_u4Position)
{
    int ret = 0;
    if((a_u4Position > g_u4AK7348AF_MACRO) || (a_u4Position < g_u4AK7348AF_INF))
    {
        AK7348AFDB("[AK7348AF] out of range \n");
        return -EINVAL;
    }

    if (g_s4AK7348AF_Opened == 1)
    {
        unsigned short InitPos;//,InitPosL;
        //s4AK7348AF_ReadReg(0x87, &InitPosL);
        //ret = s4AK7348AF_ReadReg(0x86, &InitPos);
        //InitPos= (InitPos<<1) +(InitPosL>>7);
        InitPos=100;
        if(ret == 0)
        {
            AK7348AFDB("[AK7348AF] Init Pos %6d \n", InitPos);
            spin_lock(&g_AK7348AF_SpinLock);
            g_u4CurrPosition = (unsigned long)InitPos;
            spin_unlock(&g_AK7348AF_SpinLock);
        }
        else
        {	
            spin_lock(&g_AK7348AF_SpinLock);
            g_u4CurrPosition = 0;
            spin_unlock(&g_AK7348AF_SpinLock);
        }
        spin_lock(&g_AK7348AF_SpinLock);
        g_s4AK7348AF_Opened = 2;
        spin_unlock(&g_AK7348AF_SpinLock);
    }

    if (g_u4CurrPosition < a_u4Position)
    {
        spin_lock(&g_AK7348AF_SpinLock);	
        g_i4Dir = 1;
        spin_unlock(&g_AK7348AF_SpinLock);	
    }
    else if (g_u4CurrPosition > a_u4Position)
    {
        spin_lock(&g_AK7348AF_SpinLock);	
        g_i4Dir = -1;
        spin_unlock(&g_AK7348AF_SpinLock);			
    }
    else										{return 0;}

    spin_lock(&g_AK7348AF_SpinLock);    
    g_u4TargetPosition = a_u4Position;
    spin_unlock(&g_AK7348AF_SpinLock);	

    //AK7348AFDB("[AK7348AF] move [curr] %d [target] %d\n", g_u4CurrPosition, g_u4TargetPosition);

    spin_lock(&g_AK7348AF_SpinLock);
    g_sr = 3;
    g_i4MotorStatus = 0;
    spin_unlock(&g_AK7348AF_SpinLock);		
    s4AK7348AF_WriteReg(1, (unsigned short)(((g_u4TargetPosition>>1)&1)<<7));//adapter to 9bit
    if(s4AK7348AF_WriteReg(0, (unsigned short)((g_u4TargetPosition>>2)&0xFF)) == 0)//adapter to 9bit
    {
        spin_lock(&g_AK7348AF_SpinLock);		
        g_u4CurrPosition = (unsigned long)g_u4TargetPosition;
        spin_unlock(&g_AK7348AF_SpinLock);				
    }
    else
    {
        AK7348AFDB("[AK7348AF] set I2C failed when moving the motor \n");			
        spin_lock(&g_AK7348AF_SpinLock);
        g_i4MotorStatus = -1;
        spin_unlock(&g_AK7348AF_SpinLock);				
    }

    return 0;
}

inline static int setAK7348AFInf(unsigned long a_u4Position)
{
    spin_lock(&g_AK7348AF_SpinLock);
    g_u4AK7348AF_INF = a_u4Position;
    spin_unlock(&g_AK7348AF_SpinLock);	
    return 0;
}

inline static int setAK7348AFMacro(unsigned long a_u4Position)
{
    spin_lock(&g_AK7348AF_SpinLock);
    g_u4AK7348AF_MACRO = a_u4Position;
    spin_unlock(&g_AK7348AF_SpinLock);	
    return 0;	
}

////////////////////////////////////////////////////////////////
static long AK7348AF_Ioctl(
struct file * a_pstFile,
unsigned int a_u4Command,
unsigned long a_u4Param)
{
    long i4RetValue = 0;

    switch(a_u4Command)
    {
        case AK7348AFIOC_G_MOTORINFO :
            i4RetValue = getAK7348AFInfo((__user stAK7348AF_MotorInfo *)(a_u4Param));
        break;
		#ifdef LensdrvCM3
        case AK7348AFIOC_G_MOTORMETAINFO :
            i4RetValue = getAK7348AFMETA((__user stAK7348AF_MotorMETAInfo *)(a_u4Param));
        break;
		#endif
        case AK7348AFIOC_T_MOVETO :
            i4RetValue = moveAK7348AF(a_u4Param);
        break;
 
        case AK7348AFIOC_T_SETINFPOS :
            i4RetValue = setAK7348AFInf(a_u4Param);
        break;

        case AK7348AFIOC_T_SETMACROPOS :
            i4RetValue = setAK7348AFMacro(a_u4Param);
        break;
		
        default :
      	    AK7348AFDB("[AK7348AF] No CMD \n");
            i4RetValue = -EPERM;
        break;
    }

    return i4RetValue;
}

//Main jobs:
// 1.check for device-specified errors, device not ready.
// 2.Initialize the device if it is opened for the first time.
// 3.Update f_op pointer.
// 4.Fill data structures into private_data
//CAM_RESET
static int AK7348AF_Open(struct inode * a_pstInode, struct file * a_pstFile)
{
    AK7348AFDB("[AK7348AF] AK7348AF_Open - Start\n");
    if(g_s4AK7348AF_Opened)
    {
        AK7348AFDB("[AK7348AF] the device is opened \n");
        return -EBUSY;
    }
	
    spin_lock(&g_AK7348AF_SpinLock);
    g_s4AK7348AF_Opened = 1;
    spin_unlock(&g_AK7348AF_SpinLock);
    AK7348AFDB("[AK7348AF] AK7348AF_Open - End\n");
    //s4AK7348AF_WriteReg(1023);msleep(10);
    //s4AK7348AF_WriteReg(0);
    s4AK7348AF_WriteReg(2, 0); 
    s4AK7348AF_WriteReg(0, 100);

    return 0;
}

//Main jobs:
// 1.Deallocate anything that "open" allocated in private_data.
// 2.Shut down the device on last close.
// 3.Only called once on last time.
// Q1 : Try release multiple times.
static int AK7348AF_Release(struct inode * a_pstInode, struct file * a_pstFile)
{
    AK7348AFDB("[AK7348AF] AK7348AF_Release - Start\n");

    if (g_s4AK7348AF_Opened)
    {
        AK7348AFDB("[AK7348AF] feee \n");
        g_sr = 5;
	    s4AK7348AF_WriteReg(0, 100);
        msleep(10);
	    s4AK7348AF_WriteReg(0, 50);
        msleep(10);
            	            	    	    
        spin_lock(&g_AK7348AF_SpinLock);
        g_s4AK7348AF_Opened = 0;
        spin_unlock(&g_AK7348AF_SpinLock);

    }

    AK7348AFDB("[AK7348AF] AK7348AF_Release - End\n");

    return 0;
}

static const struct file_operations g_stAK7348AF_fops = 
{
    .owner = THIS_MODULE,
    .open = AK7348AF_Open,
    .release = AK7348AF_Release,
    .unlocked_ioctl = AK7348AF_Ioctl
};

inline static int Register_AK7348AF_CharDrv(void)
{
    struct device* vcm_device = NULL;

    AK7348AFDB("[AK7348AF] Register_AK7348AF_CharDrv - Start\n");

    //Allocate char driver no.
    if( alloc_chrdev_region(&g_AK7348AF_devno, 0, 1,AK7348AF_DRVNAME) )
    {
        AK7348AFDB("[AK7348AF] Allocate device no failed\n");

        return -EAGAIN;
    }

    //Allocate driver
    g_pAK7348AF_CharDrv = cdev_alloc();

    if(NULL == g_pAK7348AF_CharDrv)
    {
        unregister_chrdev_region(g_AK7348AF_devno, 1);

        AK7348AFDB("[AK7348AF] Allocate mem for kobject failed\n");

        return -ENOMEM;
    }

    //Attatch file operation.
    cdev_init(g_pAK7348AF_CharDrv, &g_stAK7348AF_fops);

    g_pAK7348AF_CharDrv->owner = THIS_MODULE;

    //Add to system
    if(cdev_add(g_pAK7348AF_CharDrv, g_AK7348AF_devno, 1))
    {
        AK7348AFDB("[AK7348AF] Attatch file operation failed\n");

        unregister_chrdev_region(g_AK7348AF_devno, 1);

        return -EAGAIN;
    }

    actuator_class = class_create(THIS_MODULE, "actuatordrv");
    if (IS_ERR(actuator_class)) {
        int ret = PTR_ERR(actuator_class);
        AK7348AFDB("Unable to create class, err = %d\n", ret);
        return ret;            
    }

    vcm_device = device_create(actuator_class, NULL, g_AK7348AF_devno, NULL, AK7348AF_DRVNAME);

    if(NULL == vcm_device)
    {
        return -EIO;
    }
    
    AK7348AFDB("[AK7348AF] Register_AK7348AF_CharDrv - End\n");    
    return 0;
}

inline static void Unregister_AK7348AF_CharDrv(void)
{
    AK7348AFDB("[AK7348AF] Unregister_AK7348AF_CharDrv - Start\n");

    //Release char driver
    cdev_del(g_pAK7348AF_CharDrv);

    unregister_chrdev_region(g_AK7348AF_devno, 1);
    
    device_destroy(actuator_class, g_AK7348AF_devno);

    class_destroy(actuator_class);

    AK7348AFDB("[AK7348AF] Unregister_AK7348AF_CharDrv - End\n");    
}

//////////////////////////////////////////////////////////////////////

static int AK7348AF_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int AK7348AF_i2c_remove(struct i2c_client *client);
static const struct i2c_device_id AK7348AF_i2c_id[] = {{AK7348AF_DRVNAME,0},{}};   
struct i2c_driver AK7348AF_i2c_driver = {                       
    .probe = AK7348AF_i2c_probe,                                   
    .remove = AK7348AF_i2c_remove,                           
    .driver.name = AK7348AF_DRVNAME,                 
    .id_table = AK7348AF_i2c_id,                             
};  

#if 0 
static int AK7348AF_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info) {         
    strcpy(info->type, AK7348AF_DRVNAME);                                                         
    return 0;                                                                                       
}      
#endif 
static int AK7348AF_i2c_remove(struct i2c_client *client) {
    return 0;
}

/* Kirby: add new-style driver {*/
static int AK7348AF_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int i4RetValue = 0;

    AK7348AFDB("[AK7348AF] AK7348AF_i2c_probe\n");

    /* Kirby: add new-style driver { */
    g_pstAK7348AF_I2Cclient = client;
    
    g_pstAK7348AF_I2Cclient->addr = (0x18 >> 1);
    
    //Register char driver
    i4RetValue = Register_AK7348AF_CharDrv();

    if(i4RetValue){

        AK7348AFDB("[AK7348AF] register char device failed!\n");

        return i4RetValue;
    }

    spin_lock_init(&g_AK7348AF_SpinLock);

    AK7348AFDB("[AK7348AF] Attached!! \n");

    return 0;
}

static int AK7348AF_probe(struct platform_device *pdev)
{
    return i2c_add_driver(&AK7348AF_i2c_driver);
}

static int AK7348AF_remove(struct platform_device *pdev)
{
    i2c_del_driver(&AK7348AF_i2c_driver);
    return 0;
}

static int AK7348AF_suspend(struct platform_device *pdev, pm_message_t mesg)
{
    return 0;
}

static int AK7348AF_resume(struct platform_device *pdev)
{
    return 0;
}

// platform structure
static struct platform_driver g_stAK7348AF_Driver = {
    .probe		= AK7348AF_probe,
    .remove	= AK7348AF_remove,
    .suspend	= AK7348AF_suspend,
    .resume	= AK7348AF_resume,
    .driver		= {
        .name	= "lens_actuator",
        .owner	= THIS_MODULE,
    }
};

static int __init AK7348AF_i2C_init(void)
{
    i2c_register_board_info(LENS_I2C_BUSNUM, &kd_lens_dev, 1);
	
    if(platform_driver_register(&g_stAK7348AF_Driver)){
        AK7348AFDB("failed to register AK7348AF driver\n");
        return -ENODEV;
    }

    return 0;
}

static void __exit AK7348AF_i2C_exit(void)
{
	platform_driver_unregister(&g_stAK7348AF_Driver);
}

module_init(AK7348AF_i2C_init);
module_exit(AK7348AF_i2C_exit);

MODULE_DESCRIPTION("AK7348AF lens module driver");
MODULE_AUTHOR("KY Chen <ky.chen@Mediatek.com>");
MODULE_LICENSE("GPL");


