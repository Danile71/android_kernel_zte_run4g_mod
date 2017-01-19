#ifndef _DRV_COMM_H
#define _DRV_COMM_H
#include <linux/mu3d/hal/mu3d_hal_hw.h>
#include <linux/io.h>
#include <mach/sync_write.h>

#undef EXTERN

#ifdef _DRV_COMM_H
#define EXTERN
#else
#define EXTERN extern
#endif


/* CONSTANTS */

#ifndef FALSE
  #define FALSE   0
#endif

#ifndef TRUE
  #define TRUE    1
#endif

/* TYPES */

typedef unsigned int    DEV_UINT32;
typedef int    			DEV_INT32;
typedef unsigned short  DEV_UINT16;
typedef short    		DEV_INT16;
typedef unsigned char   DEV_UINT8;
typedef char   			DEV_INT8;

typedef enum {
    RET_SUCCESS = 0,
    RET_FAIL,
} USB_RESULT;

#ifdef NEVER
#define os_writeb(addr,data) {\
	(*((volatile DEV_UINT8 *)(addr)) = (DEV_UINT8)data);\
	if(0) printk("****** os_writeb [0x%08x] = 0x%08x (%s#%d)\n", (unsigned int)addr, data, __func__, __LINE__);\
	}

#define os_writew(addr,data) {\
	(*((volatile DEV_UINT16 *)(addr)) = (DEV_UINT16)data);\
	if(0) printk("****** os_writew [0x%08x] = 0x%08x (%s#%d)\n", (unsigned int)addr, data, __func__, __LINE__);\
	}

#define os_writel(addr,data) {\
	(*((volatile DEV_UINT32 *)(addr)) = (DEV_UINT32)data);\
	if(0) printk("****** os_writel [0x%08x] = 0x%08x (%s#%d)\n", (unsigned int)addr, data, __func__, __LINE__);\
	}

#define os_readl(addr)  *((volatile DEV_UINT32 *)(addr))
#define os_writelmsk(addr, data, msk) \
	{ os_writel(addr, ((os_readl(addr) & ~(msk)) | ((data) & (msk)))); \
	}
#define os_setmsk(addr, msk) \
	{ os_writel(addr, os_readl(addr) | msk); \
	}
#define os_clrmsk(addr, msk) \
   { os_writel(addr, os_readl(addr) &~ msk); \
   }
/*msk the data first, then umsk with the umsk.*/
#define os_writelmskumsk(addr, data, msk, umsk) \
{\
   os_writel(addr, ((os_readl(addr) & ~(msk)) | ((data) & (msk))) & (umsk));\
}

#define USB_END_OFFSET(_bEnd, _bOffset)	((0x10*(_bEnd-1)) + _bOffset)
#define USB_ReadCsr32(_bOffset, _bEnd) \
	os_readl(USB_END_OFFSET(_bEnd, _bOffset))
#define USB_WriteCsr32(  _bOffset, _bEnd, _bData) \
    os_writel( USB_END_OFFSET(_bEnd, _bOffset), _bData)
#else
static inline void os_writeb( unsigned int addr, unsigned char data)
{
	writeb(data, (void __iomem *)addr);
	if(0) printk("%s writeb [%p] = 0x%08x\n", __func__, (void *)addr, data);
}

static inline void os_writew( unsigned int addr, unsigned short data)
{
	writew(data, (void __iomem *)addr);
	if(0) printk("%s writew [%p] = 0x%08x\n", __func__, (void *)addr, data);
}

static inline void os_writel( unsigned int addr, unsigned int data)
{
	writel(data, (void __iomem *)addr);
	if(0) printk("%s writel [%p] = 0x%08x\n", __func__, (void *)addr, data);
}

#define os_readl(addr)  readl((void __iomem *)addr)

static inline void os_writelmsk( unsigned int addr, unsigned int data, unsigned int msk)
{
	unsigned int tmp = readl((void __iomem *)addr);
	mb();
	writel(((tmp & ~(msk)) | ((data) & (msk))), (void __iomem *)addr);
}

static inline void os_setmsk( unsigned int addr, unsigned int msk)
{
	unsigned int tmp = readl((void __iomem *)addr);
	if(0) printk("%s setmsk [%p] = 0x%08x\n", __func__, (void *)addr, tmp);
	mb();
	writel((tmp | msk), (void __iomem *)addr);
	if(0) printk("%s setmsk [%p] = 0x%08x\n", __func__, (void *)addr, readl((void __iomem *)addr));
}

static inline void os_clrmsk( unsigned int addr, unsigned int msk)
{
	unsigned int tmp = readl((void __iomem *)addr);
	if(0) printk("%s clrmsk [%p] = 0x%08x\n", __func__, (void *)addr, tmp);
	mb();
	writel((tmp & ~(msk)), (void __iomem *)addr);
	if(0) printk("%s clrmsk [%p] = 0x%08x\n", __func__, (void *)addr, readl((void __iomem *)addr));
}

/*msk the data first, then umsk with the umsk.*/
static inline void os_writelmskumsk( unsigned int addr, unsigned int data,
		unsigned int msk, unsigned int umsk)
{
	unsigned int tmp = readl((void __iomem *)addr);
	mb();
	writel(((tmp & ~(msk)) | ((data) & (msk))) & (umsk), (void __iomem *)addr);
}

static inline int wait_for_value(unsigned int addr, unsigned int msk,
		unsigned int value, unsigned int ms_intvl, unsigned int count)
{
	u32 i;
	for (i = 0; i < count; i++) {
		if ((os_readl(addr) & msk) == value)
			return RET_SUCCESS;
		mb();
		mdelay(ms_intvl);
	}
	return RET_FAIL;
}

static inline int wait_for_value_us(unsigned int addr, unsigned int msk,
		unsigned int value, unsigned int us_intvl, unsigned int count)
{
	u32 i;
	for (i = 0; i < count; i++) {
		if ((os_readl(addr) & msk) == value)
			return RET_SUCCESS;
		mb();
		udelay(us_intvl);
	}
	return RET_FAIL;
}

#define USB_END_OFFSET(_bEnd, _bOffset)	((0x10*(_bEnd-1)) + _bOffset)

#define USB_ReadCsr32(_bOffset, _bEnd) \
			readl((void __iomem *)(USB_END_OFFSET(_bEnd, _bOffset)))

#define USB_WriteCsr32(  _bOffset, _bEnd, _bData) \
			writel( _bData, (void __iomem *)(USB_END_OFFSET(_bEnd, _bOffset)));mb()

#endif

#define div_and_rnd_up(x, y) (((x) + (y) - 1) / (y))

#endif   /*_DRV_COMM_H*/
