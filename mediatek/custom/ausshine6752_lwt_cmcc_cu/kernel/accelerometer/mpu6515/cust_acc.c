#include <linux/types.h>
#include <cust_acc.h>
#include <mach/mt_pm_ldo.h>

/*---------------------------------------------------------------------------*/
int cust_acc_power(struct acc_hw *hw, unsigned int on, char* devname)
{
#ifndef FPGA_EARLY_PORTING
    if (hw->power_id == MT65XX_POWER_NONE)
        return 0;
    if (on)
        return hwPowerOn(hw->power_id, hw->power_vol, devname);
    else
        return hwPowerDown(hw->power_id, devname); 
#else
    return 0;
#endif
}
/*---------------------------------------------------------------------------*/
static struct acc_hw cust_acc_hw = {
    .i2c_num = 1,
    .direction = 1,
    .power_id = MT65XX_POWER_NONE,  /*!< LDO is not used */
    .power_vol= VOL_DEFAULT,        /*!< LDO is not used */
    .firlen = 16,                   /*!< don't enable low pass fileter */
    .power = cust_acc_power,        
    .is_batch_supported = true,
};
/*---------------------------------------------------------------------------*/
struct acc_hw* get_cust_acc_hw(void) 
{
    return &cust_acc_hw;
}
