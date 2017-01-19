#ifndef _MT_GPIO_BASE_H_
#define _MT_GPIO_BASE_H_

#include <mach/sync_write.h>
#include <mach/gpio_const.h>

#define GPIO_WR32(addr, data)   mt_reg_sync_writel(data, addr)
#define GPIO_RD32(addr)         __raw_readl(addr)
//#define GPIO_SET_BITS(BIT,REG)   ((*(volatile unsigned long*)(REG)) = (unsigned long)(BIT))
//#define GPIO_CLR_BITS(BIT,REG)   ((*(volatile unsigned long*)(REG)) &= ~((unsigned long)(BIT)))
#define GPIO_SW_SET_BITS(BIT,REG)   GPIO_WR32(REG,GPIO_RD32(REG) | ((unsigned long)(BIT)))
#define GPIO_SET_BITS(BIT,REG)   GPIO_WR32(REG, (unsigned long)(BIT))
#define GPIO_CLR_BITS(BIT,REG)   GPIO_WR32(REG,GPIO_RD32(REG) & ~((unsigned long)(BIT)))

/*----------------------------------------------------------------------------*/
typedef struct {         /*FIXME: check GPIO spec*/   
    unsigned int val;        
    unsigned int set;
    unsigned int rst;
    unsigned int _align1;
} VAL_REGS;
/*----------------------------------------------------------------------------*/
typedef struct {
    VAL_REGS    dir[6];             /*0x0000 ~ 0x005F: 96  bytes*/
    u8          rsv00[160];         /*0x0060 ~ 0x00FF: 160 bytes*/    
    VAL_REGS    dout[6];            /*0x0100 ~ 0x015F: 96  bytes*/
    u8          rsv01[160];         /*0x0160 ~ 0x01FF: 160 bytes*/
    VAL_REGS    din[6];             /*0x0200 ~ 0x025F: 96  bytes*/
    u8          rsv02[160];         /*0x0260 ~ 0x02FF: 160 bytes*/
    VAL_REGS    mode[24];           /*0x0300 ~ 0x047F: 384 bytes*/  
} GPIO_REGS;

#endif //_MT_GPIO_BASE_H_
