/******************************************************************************
*[File]             ahb_pdma.c
*[Version]          v1.0
*[Revision Date]    2013-03-13
*[Author]
*[Description]
*    The program provides AHB PDMA driver
*[Copyright]
*    Copyright (C) 2013 MediaTek Incorporation. All Rights Reserved.
******************************************************************************/



/*
** $Log: ahb_pdma.c $
 *
 * 03 13 2013 vend_samp.lin
 * Add AHB PDMA support
 * 1) Initial version
**
*/

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

#define MODULE_AHB_DMA

#include <linux/version.h>      /* constant of kernel version */

#include <linux/kernel.h>       /* bitops.h */

#include <linux/timer.h>        /* struct timer_list */
#include <linux/jiffies.h>      /* jiffies */
#include <linux/delay.h>        /* udelay and mdelay macro */

#if CONFIG_ANDROID
#include <linux/wakelock.h>
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 12)
#include <linux/irq.h>          /* IRQT_FALLING */
#endif

#include <linux/netdevice.h>    /* struct net_device, struct net_device_stats */
#include <linux/etherdevice.h>  /* for eth_type_trans() function */
#include <linux/wireless.h>     /* struct iw_statistics */
#include <linux/if_arp.h>
#include <linux/inetdevice.h>   /* struct in_device */

#include <linux/ip.h>           /* struct iphdr */

#include <linux/string.h>       /* for memcpy()/memset() function */
#include <linux/stddef.h>       /* for offsetof() macro */

#include <linux/proc_fs.h>      /* The proc filesystem constants/structures */

#include <linux/rtnetlink.h>    /* for rtnl_lock() and rtnl_unlock() */
#include <linux/kthread.h>      /* kthread_should_stop(), kthread_run() */
#include <asm/uaccess.h>        /* for copy_from_user() */
#include <linux/fs.h>           /* for firmware download */
#include <linux/vmalloc.h>

#include <linux/kfifo.h>        /* for kfifo interface */
#include <linux/cdev.h>         /* for cdev interface */

#include <linux/firmware.h>     /* for firmware download */

#include <linux/random.h>


#include <asm/io.h>             /* readw and writew */

#include <linux/module.h>

#include "../../../../../../platform/mt6752/kernel/core/include/mach/mt_clkmgr.h"

#include "hif.h"
#include "hif_pdma.h"

#include <mach/emi_mpu.h>

//#if (CONF_MTK_AHB_DMA == 1)

//#define PDMA_DEBUG_SUP

#ifdef PDMA_DEBUG_SUP
#define PDMA_DBG(msg)   printk msg
#else
#define PDMA_DBG(msg)
#endif /* PDMA_DEBUG_SUP */

static UINT32 gDmaReg[AP_DMA_HIF_0_LENGTH/4+1];


/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/


/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/


/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/


/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/
static VOID
HifPdmaConfig (
    IN void                     *HifInfoSrc,
    IN void                     *Conf
    );

static VOID
HifPdmaStart(
    IN void                     *HifInfoSrc
    );

static VOID
HifPdmaStop(
    IN void                     *HifInfoSrc
    );

static MTK_WCN_BOOL
HifPdmaPollStart(
    IN void                     *HifInfoSrc
    );

static MTK_WCN_BOOL
HifPdmaPollIntr(
    IN void                     *HifInfoSrc
    );

static VOID
HifPdmaAckIntr(
    IN void                     *HifInfoSrc
    );


static VOID
HifPdmaClockCtrl(
    IN UINT32                   FlgIsEnabled
    );

static VOID
HifPdmaRegDump(
    IN void                     *HifInfoSrc
    );

static VOID
HifPdmaReset(
    IN void                     *HifInfoSrc
    );


/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/


/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/
GL_HIF_DMA_OPS_T HifPdmaOps = {
    .DmaConfig = HifPdmaConfig,
    .DmaStart = HifPdmaStart,
    .DmaStop = HifPdmaStop,
    .DmaPollStart = HifPdmaPollStart,
    .DmaPollIntr = HifPdmaPollIntr,
    .DmaAckIntr = HifPdmaAckIntr,
    .DmaClockCtrl = HifPdmaClockCtrl,
    .DmaRegDump = HifPdmaRegDump,
    .DmaReset = HifPdmaReset
};


/*******************************************************************************
*                        P U B L I C   F U N C T I O N S
********************************************************************************
*/

/*----------------------------------------------------------------------------*/
/*!
* \brief Config PDMA TX/RX.
*
* \param[in] prGlueInfo         Pointer to the GLUE_INFO_T structure.
* \param[in] Conf               Pointer to the settings.
*
* \retval NONE
*/
/*----------------------------------------------------------------------------*/
VOID
HifPdmaInit (
    GL_HIF_INFO_T               *HifInfo
    )
{
    extern phys_addr_t gConEmiPhyBase;

    /* IO remap PDMA register memory */
    HifInfo->DmaRegBaseAddr = ioremap(AP_DMA_HIF_BASE, AP_DMA_HIF_0_LENGTH);

    /* assign PDMA operators */
    HifInfo->DmaOps = &HifPdmaOps;

    /* enable PDMA mode */
    HifInfo->fgDmaEnable = TRUE;

#if 1 // MPU Setting
    // WIFI using TOP 512KB
    printk("[wlan] MPU region 12, 0x%08x - 0x%08x\n", gConEmiPhyBase, gConEmiPhyBase + 512*1024);
    emi_mpu_set_region_protection(gConEmiPhyBase,
        gConEmiPhyBase + 512*1024 - 1,
        12,
        SET_ACCESS_PERMISSON(NO_PROTECTION,FORBIDDEN,FORBIDDEN,FORBIDDEN,FORBIDDEN,NO_PROTECTION,FORBIDDEN,FORBIDDEN));
#endif

    PDMA_DBG(("PDMA> HifPdmaInit ok!\n"));
}


/*******************************************************************************
*                       P R I V A T E   F U N C T I O N S
********************************************************************************
*/

/*----------------------------------------------------------------------------*/
/*!
* \brief Config PDMA TX/RX.
*
* \param[in] HifInfo            Pointer to the GL_HIF_INFO_T structure.
* \param[in] Param              Pointer to the settings.
*
* \retval NONE
*/
/*----------------------------------------------------------------------------*/
static VOID
HifPdmaConfig (
    IN void                     *HifInfoSrc,
    IN void                     *Param
    )
{
    GL_HIF_INFO_T *HifInfo = (GL_HIF_INFO_T *)HifInfoSrc;
    MTK_WCN_HIF_DMA_CONF *Conf = (MTK_WCN_HIF_DMA_CONF *)Param;
    UINT32 RegVal;
    

    /* Assign fixed value */
    Conf->Burst = HIF_PDMA_BURST_4_4; /* vs. HIF_BURST_4DW */
    Conf->Fix_en = FALSE;

    /* AP_P_DMA_G_DMA_2_CON */
    PDMA_DBG(("PDMA> Conf->Dir = %d\n", Conf->Dir));

    /* AP_DMA_HIF_0_CON */
    RegVal = HIF_DMAR_READL(HifInfo, AP_DMA_HIF_0_CON);
    RegVal &= ~(ADH_CR_BURST_LEN | ADH_CR_FIX_EN | ADH_CR_DIR);
    RegVal |= (((Conf->Burst<<ADH_CR_BURST_LEN_OFFSET)&ADH_CR_BURST_LEN) | \
            (Conf->Fix_en<<ADH_CR_FIX_EN_OFFSET) | \
            (Conf->Dir));
    HIF_DMAR_WRITEL(HifInfo, AP_DMA_HIF_0_CON, RegVal);
    PDMA_DBG(("PDMA> AP_DMA_HIF_0_CON = 0x%08x\n", RegVal));

    /* AP_DMA_HIF_0_SRC_ADDR */
    HIF_DMAR_WRITEL(HifInfo, AP_DMA_HIF_0_SRC_ADDR, Conf->Src);
    PDMA_DBG(("PDMA> AP_DMA_HIF_0_SRC_ADDR = 0x%08x\n",  Conf->Src));

    /* AP_DMA_HIF_0_DST_ADDR */
    HIF_DMAR_WRITEL(HifInfo, AP_DMA_HIF_0_DST_ADDR, Conf->Dst);
    PDMA_DBG(("PDMA> AP_DMA_HIF_0_DST_ADDR = 0x%08x\n",  Conf->Dst));

    /* AP_DMA_HIF_0_LEN */
    HIF_DMAR_WRITEL(HifInfo, AP_DMA_HIF_0_LEN, (Conf->Count & ADH_CR_LEN));
    PDMA_DBG(("PDMA> AP_DMA_HIF_0_LEN = %ld\n",  (Conf->Count & ADH_CR_LEN)));

}/* End of HifPdmaConfig */


/*----------------------------------------------------------------------------*/
/*!
* \brief Start PDMA TX/RX.
*
* \param[in] HifInfo            Pointer to the GL_HIF_INFO_T structure.
*
* \retval NONE
*/
/*----------------------------------------------------------------------------*/
static VOID
HifPdmaStart(
    IN void                     *HifInfoSrc
    )
{
    GL_HIF_INFO_T *HifInfo = (GL_HIF_INFO_T *)HifInfoSrc;
    UINT32 RegVal, RegId, RegIdx;


    /* Enable interrupt */
    RegVal = HIF_DMAR_READL(HifInfo, AP_DMA_HIF_0_INT_EN);
    HIF_DMAR_WRITEL(HifInfo, AP_DMA_HIF_0_INT_EN, (RegVal | ADH_CR_INTEN_FLAG_0));

	/* keep old register settings */
	RegIdx = 0;
    for(RegId=0; RegId<AP_DMA_HIF_0_LENGTH; RegId+=4)
    {
        gDmaReg[RegIdx++] = HIF_DMAR_READL(HifInfo, RegId);
	}

    /* Start DMA */
    RegVal = HIF_DMAR_READL(HifInfo, AP_DMA_HIF_0_EN);
    HIF_DMAR_WRITEL(HifInfo, AP_DMA_HIF_0_EN, (RegVal | ADH_CR_EN));

    PDMA_DBG(("PDMA> HifPdmaStart...\n"));

} /* End of HifPdmaStart */


/*----------------------------------------------------------------------------*/
/*!
* \brief Stop PDMA TX/RX.
*
* \param[in] HifInfo            Pointer to the GL_HIF_INFO_T structure.
*
* \retval NONE
*/
/*----------------------------------------------------------------------------*/
static VOID
HifPdmaStop(
    IN void                     *HifInfoSrc
    )
{
    GL_HIF_INFO_T *HifInfo = (GL_HIF_INFO_T *)HifInfoSrc;
    UINT32 RegVal;
//    UINT32 pollcnt;


    /* Disable interrupt */
    RegVal = HIF_DMAR_READL(HifInfo, AP_DMA_HIF_0_INT_EN);
    HIF_DMAR_WRITEL(HifInfo, AP_DMA_HIF_0_INT_EN, (RegVal & ~(ADH_CR_INTEN_FLAG_0)));


#if 0 /* DE says we donot need to do it */
    /* Stop DMA */
    RegVal = HIF_DMAR_READL(HifInfo, AP_DMA_HIF_0_STOP);
    HIF_DMAR_WRITEL(HifInfo, AP_DMA_HIF_0_STOP, (RegVal | ADH_CR_STOP));


    /* Polling START bit turn to 0 */
    pollcnt = 0;
    do {
        RegVal = HIF_DMAR_READL(HifInfo, AP_DMA_HIF_0_EN);
        if (pollcnt++ > 100000) {
            /* TODO: warm reset PDMA */
        }
    } while(RegVal&ADH_CR_EN);
#endif

} /* End of HifPdmaStop */


/*----------------------------------------------------------------------------*/
/*!
* \brief Enable PDMA TX/RX.
*
* \param[in] HifInfo            Pointer to the GL_HIF_INFO_T structure.
*
* \retval NONE
*/
/*----------------------------------------------------------------------------*/
static MTK_WCN_BOOL
HifPdmaPollStart(
    IN void                     *HifInfoSrc
    )
{
    GL_HIF_INFO_T *HifInfo = (GL_HIF_INFO_T *)HifInfoSrc;
	UINT32 RegVal;


    RegVal = HIF_DMAR_READL(HifInfo, AP_DMA_HIF_0_EN);
	return (((RegVal & ADH_CR_EN) != 0) ? TRUE : FALSE);

} /* End of HifPdmaPollStart */


/*----------------------------------------------------------------------------*/
/*!
* \brief Poll PDMA TX/RX done.
*
* \param[in] HifInfo            Pointer to the GL_HIF_INFO_T structure.
*
* \retval NONE
*/
/*----------------------------------------------------------------------------*/
static MTK_WCN_BOOL
HifPdmaPollIntr(
    IN void                     *HifInfoSrc
    )
{
    GL_HIF_INFO_T *HifInfo = (GL_HIF_INFO_T *)HifInfoSrc;
	UINT32 RegVal;


	RegVal = HIF_DMAR_READL(HifInfo, AP_DMA_HIF_0_INT_FLAG);
	return (((RegVal & ADH_CR_FLAG_0) != 0) ? TRUE : FALSE);

} /* End of HifPdmaPollIntr */


/*----------------------------------------------------------------------------*/
/*!
* \brief Acknowledge PDMA TX/RX done.
*
* \param[in] HifInfo            Pointer to the GL_HIF_INFO_T structure.
*
* \retval NONE
*/
/*----------------------------------------------------------------------------*/
static VOID
HifPdmaAckIntr(
    IN void                     *HifInfoSrc
    )
{
    GL_HIF_INFO_T *HifInfo = (GL_HIF_INFO_T *)HifInfoSrc;
	UINT32 RegVal;


	/* Write 0 to clear interrupt */
	RegVal = HIF_DMAR_READL(HifInfo, AP_DMA_HIF_0_INT_FLAG);
	HIF_DMAR_WRITEL(HifInfo, AP_DMA_HIF_0_INT_FLAG, (RegVal & ~ADH_CR_FLAG_0));

} /* End of HifPdmaAckIntr */


/*----------------------------------------------------------------------------*/
/*!
* \brief Acknowledge PDMA TX/RX done.
*
* \param[in] FlgIsEnabled       TRUE: enable; FALSE: disable
*
* \retval NONE
*/
/*----------------------------------------------------------------------------*/
static VOID
HifPdmaClockCtrl(
    IN UINT32                   FlgIsEnabled
    )
{
    if (FlgIsEnabled == TRUE)
        enable_clock(MT_CG_INFRA_APDMA, "WLAN");
    else
        disable_clock(MT_CG_INFRA_APDMA, "WLAN");
}


/*----------------------------------------------------------------------------*/
/*!
* \brief Dump PDMA related registers.
*
* \param[in] HifInfo            Pointer to the GL_HIF_INFO_T structure.
*
* \retval NONE
*/
/*----------------------------------------------------------------------------*/
static VOID
HifPdmaRegDump(
    IN void                     *HifInfoSrc
    )
{
    GL_HIF_INFO_T *HifInfo = (GL_HIF_INFO_T *)HifInfoSrc;
    UINT32 RegId, RegVal, RegIdx;
    UINT32 RegNum = 0;
	INT8 *pucIoAddr; 


    printk("PDMA> Register content before start 0x%x=\n\t", AP_DMA_HIF_BASE);
    for(RegId=0, RegIdx=0; RegId<AP_DMA_HIF_0_LENGTH; RegId+=4)
    {
        RegVal = gDmaReg[RegIdx++];
        printk("0x%08x ", RegVal);

        if (RegNum++ >= 3)
        {
            printk("\n");
            printk("PDMA> Register content 0x%x=\n\t", AP_DMA_HIF_BASE+RegId+4);
            RegNum = 0;
        }
    }

    printk("\nPDMA> Register content after start 0x%x=\n\t", AP_DMA_HIF_BASE);
    for(RegId=0; RegId<AP_DMA_HIF_0_LENGTH; RegId+=4)
    {
        RegVal = HIF_DMAR_READL(HifInfo, RegId);
        printk("0x%08x ", RegVal);

        if (RegNum++ >= 3)
        {
            printk("\n");
            printk("PDMA> Register content 0x%x=\n\t", AP_DMA_HIF_BASE+RegId+4);
            RegNum = 0;
        }
    }

	/*
		root@k2v1:/ # cat /proc/clkmgr/clk_test
		cat /proc/clkmgr/clk_test
		********** clk register dump **********
		[00][CG_INFRA0]=[0x8367b801]
		[01][CG_INFRA1]=[0x004f0872]
		[02][CG_DISP0]=[0xfffffffc][0xfff063fc]
		[03][CG_DISP1]=[0xfffffff0]
		[04][CG_IMAGE]=[0x00000000]
		[05][CG_MFG ]=[0x00000000]
		[06][CG_AUDIO]=[0x00000000]
		[07][CG_VDEC0]=[0x00000000][0x00000000]
		[08][CG_VDEC1]=[0x00000000][0x00000000]
		[09][CG_MJC ]=[0x00000000]
		[10][CG_VENC ]=[0x00000000]

		********** clk_test help **********
		enable clk: echo enable id [mod_name] > /proc/clkmgr/clk_test
		disable clk: echo disable id [mod_name] > /proc/clkmgr/clk_test
		read state: echo id > /proc/clkmgr/clk_test

		root@k2v1:/ # cat /proc/clkmgr/clk_stat
		cat /proc/clkmgr/clk_stat

		********** clk stat dump **********

		*****[01][CG_INFRA1]*****
		[00]state=1, cnt=0
		[01]state=0, cnt=0 (clkmgr,1,1)
		[02]state=0, cnt=0 (SD,470,470)
		[03]state=1, cnt=0
		[04]state=0, cnt=0 (SD,1,1)
		[05]state=0, cnt=0 (clkmgr,1,1)
		[06]state=0, cnt=0 (clkmgr,1,1)
		[07]state=1, cnt=0
		[08]state=1, cnt=0
		[10]state=1, cnt=1 (AUXADC,72293,72292)
		[18]state=0, cnt=0 (VFIFO,2,2) (i2c,503,503) (btif_driver,2529,2529) (WLAN,10977,10977)
		[20]state=1, cnt=1 (DEVAPC,1,0)
		[23]state=1, cnt=0
		[24]state=1, cnt=0
		[25]state=1, cnt=0
		[26]state=1, cnt=0
		[31]state=1, cnt=0

		where [18]state=0, cnt=0 (VFIFO,2,2) (i2c,503,503) (btif_driver,2529,2529) (WLAN,10977,10977)
		DMA clock is off.
	*/

	pucIoAddr = ioremap(0x10001000, 0x100);
	if (pucIoAddr != 0)
	{
		printk("\nPDMA> clock status = 0x%x\n\n", *(volatile unsigned int *)(pucIoAddr + 0x94));
		iounmap(pucIoAddr);
	}
	pucIoAddr = ioremap(0x10201180, 0x10);
	if (pucIoAddr)
	{
		printk("0x10201180~0x10201090: 0x%x, 0x%x, 0x%x, 0x%x", 
			*(volatile unsigned int*)pucIoAddr, *(volatile unsigned int*)(pucIoAddr+4),
			*(volatile unsigned int*)(pucIoAddr+8), *(volatile unsigned int*)(pucIoAddr+12));
		iounmap(pucIoAddr);
	}
	pucIoAddr = ioremap(0x1000320c, 0x8);
	if (pucIoAddr)
	{
		printk("0x1000320C~0x10003214: 0x%x, 0x%x", 
			*(volatile unsigned int*)pucIoAddr, *(volatile unsigned int*)(pucIoAddr+4));
		iounmap(pucIoAddr);
	}
	pucIoAddr = ioremap(0x11000008, 0x4);
	if (pucIoAddr)
	{/* DMA global register status, to observe the channel status of all channels */
		UINT16 chnlStatus = 0;
		UINT8 i = 1, j = 0;
		UINT8 *pucChnlStatus = NULL;
		chnlStatus = (*(volatile UINT32*)pucIoAddr) & 0xffff;
		printk("0x11000008: 0x%x", *(volatile unsigned int*)pucIoAddr);
		iounmap(pucIoAddr);
		for (; i<16; i++) {
			if ((chnlStatus & 1<<i) == 0)
				continue;
			pucChnlStatus = (UINT8*)ioremap(AP_DMA_HIF_BASE+i*0x80, 0x70);
			printk("AP_DMA_BASE+%x status:\n", i*0x80);
			printk("channel enabled: %u\n", *(volatile UINT32*)(pucChnlStatus+0x8));
			printk("channel direction: %u\n", *(volatile UINT32*)(pucChnlStatus+0x18));
			printk("channel debug status: %u\n", *(volatile UINT32*)(pucChnlStatus+0x50));
			iounmap(pucChnlStatus);
		}
	}
	
}


/*----------------------------------------------------------------------------*/
/*!
* \brief Reset DMA.
*
* \param[in] HifInfo            Pointer to the GL_HIF_INFO_T structure.
*
* \retval NONE
*/
/*----------------------------------------------------------------------------*/
static VOID
HifPdmaReset(
    IN void                     *HifInfoSrc
    )
{
    GL_HIF_INFO_T *HifInfo = (GL_HIF_INFO_T *)HifInfoSrc;
    UINT32 LoopCnt;


    /* do warm reset: DMA will wait for current traction finished */
    printk("\nDMA> do warm reset...\n");

    /* normally, we need to sure that bit0 of AP_P_DMA_G_DMA_2_EN is 1 here */

    HIF_DMAR_WRITEL(HifInfo, AP_DMA_HIF_0_RST, 0x01);

    for(LoopCnt=0; LoopCnt<10000; LoopCnt++)
    {
        if (!HifPdmaPollStart(HifInfo))
            break; /* reset ok */
    }

    if (HifPdmaPollStart(HifInfo))
    {
        /* do hard reset because warm reset fails */
        printk("\nDMA> do hard reset...\n");
        HIF_DMAR_WRITEL(HifInfo, AP_DMA_HIF_0_RST, 0x02);
        msleep(1);
        HIF_DMAR_WRITEL(HifInfo, AP_DMA_HIF_0_RST, 0x00);
    }
}

//#endif /* CONF_MTK_AHB_DMA */

/* End of ahb_pdma.c */
