#ifndef __M4U_LIB_PRIV_H__
#define __M4U_LIB_PRIV_H__



//====================================
// about portid
//====================================

enum
{
    M4U_PORT_DISP_OVL0           ,
    M4U_PORT_DISP_RDMA0          ,
    M4U_PORT_DISP_WDMA0          ,
    M4U_PORT_DISP_OVL1           ,
    M4U_PORT_DISP_RDMA1          ,
    M4U_PORT_DISP_WDMA1          ,
    M4U_PORT_UFOD_RDMA0          ,
    M4U_PORT_UFOD_RDMA1          ,
    M4U_PORT_UFOD_RDMA2          ,
    M4U_PORT_UFOD_RDMA3          ,
    M4U_PORT_UFOD_RDMA4          ,
    M4U_PORT_UFOD_RDMA5          ,
    M4U_PORT_UFOD_RDMA6          ,    
    M4U_PORT_UFOD_RDMA7          ,        
    M4U_PORT_MDP_RDMA            ,
    M4U_PORT_MDP_WDMA            ,
    M4U_PORT_MDP_WROT            ,

                               
    M4U_PORT_HW_VDEC_MC_EXT      ,
    M4U_PORT_HW_VDEC_PP_EXT      ,
    M4U_PORT_HW_VDEC_VLD_EXT     ,
    M4U_PORT_HW_VDEC_AVC_MV_EXT  ,
    M4U_PORT_HW_VDEC_PRED_RD_EXT ,
    M4U_PORT_HW_VDEC_PRED_WR_EXT ,
    M4U_PORT_HW_VDEC_PPWRAP_EXT  ,
                               
    M4U_PORT_IMGO                ,
    M4U_PORT_RRZO                ,
    M4U_PORT_AAO                 ,
    M4U_PORT_ESFKO               ,
    M4U_PORT_IMGO_S              ,
    M4U_PORT_LSCI                ,
    M4U_PORT_LSCI_D              ,
    M4U_PORT_BPCI                ,
    M4U_PORT_BPCI_D              ,
    M4U_PORT_UFDI                ,
    M4U_PORT_IMGI                ,
    M4U_PORT_IMG2O               ,
    M4U_PORT_IMG3O               ,
    M4U_PORT_VIPI                ,
    M4U_PORT_VIP2I               ,
    M4U_PORT_VIP3I               ,
    M4U_PORT_LCEI                ,
    M4U_PORT_RB                  ,
    M4U_PORT_RP                  ,
    M4U_PORT_WR                  ,
                                
    M4U_PORT_VENC_RCPU           ,
    M4U_PORT_VENC_REC            ,
    M4U_PORT_VENC_BSDMA          ,
    M4U_PORT_VENC_SV_COMV        ,
    M4U_PORT_VENC_RD_COMV        ,
    M4U_PORT_JPGENC_RDMA         ,
    M4U_PORT_JPGENC_BSDMA        ,
    M4U_PORT_JPGDEC_WDMA         ,
    M4U_PORT_JPGDEC_BSDMA        ,
    M4U_PORT_VENC_CUR_LUMA       ,
    M4U_PORT_VENC_CUR_CHROMA     ,
    M4U_PORT_VENC_REF_LUMA       ,
    M4U_PORT_VENC_REF_CHROMA     ,
    M4U_PORT_VENC_NBM_RDMA       ,
    M4U_PORT_VENC_NBM_WDMA       ,
                                
    M4U_PORT_MJC_MV_RD           ,
    M4U_PORT_MJC_MV_WR           ,
    M4U_PORT_MJC_DMA_RD          ,
    M4U_PORT_MJC_DMA_WR          ,

    M4U_PORT_DBG_I2C             ,
    M4U_PORT_SPM                 ,
    M4U_PORT_MD32                ,
    M4U_PORT_PTP_THERM           ,
    M4U_PORT_PWM                 ,
    M4U_PORT_MSDC1               ,
    M4U_PORT_MSDC2               ,
    M4U_PORT_SPI0                ,
    M4U_PORT_IC_USB              ,
    M4U_PORT_AUDIO               ,
    M4U_PORT_MSDC3               ,
    M4U_PORT_USB20               ,
    
    M4U_PORT_MSDC0               ,
    M4U_PORT_AP_DMA              ,
    M4U_PORT_GCPU                ,
    M4U_PORT_GCE                 ,
    M4U_PORT_DEBUGTOP0           ,
    M4U_PORT_DEBUGTOP1           ,
                                
    M4U_PORT_UNKNOWN             ,
                                 
};                               

#define M4U_PORT_NR M4U_PORT_UNKNOWN

//IOCTL commnad
#define MTK_M4U_MAGICNO 'g'
#define MTK_M4U_T_POWER_ON            _IOW(MTK_M4U_MAGICNO, 0, int)
#define MTK_M4U_T_POWER_OFF           _IOW(MTK_M4U_MAGICNO, 1, int)
#define MTK_M4U_T_DUMP_REG            _IOW(MTK_M4U_MAGICNO, 2, int)
#define MTK_M4U_T_DUMP_INFO           _IOW(MTK_M4U_MAGICNO, 3, int)
#define MTK_M4U_T_ALLOC_MVA           _IOWR(MTK_M4U_MAGICNO,4, int)
#define MTK_M4U_T_DEALLOC_MVA         _IOW(MTK_M4U_MAGICNO, 5, int)
#define MTK_M4U_T_INSERT_TLB_RANGE    _IOW(MTK_M4U_MAGICNO, 6, int)
#define MTK_M4U_T_INVALID_TLB_RANGE   _IOW(MTK_M4U_MAGICNO, 7, int)
#define MTK_M4U_T_INVALID_TLB_ALL     _IOW(MTK_M4U_MAGICNO, 8, int)
#define MTK_M4U_T_MANUAL_INSERT_ENTRY _IOW(MTK_M4U_MAGICNO, 9, int)
#define MTK_M4U_T_CACHE_SYNC          _IOW(MTK_M4U_MAGICNO, 10, int)
#define MTK_M4U_T_CONFIG_PORT         _IOW(MTK_M4U_MAGICNO, 11, int)
#define MTK_M4U_T_CONFIG_ASSERT       _IOW(MTK_M4U_MAGICNO, 12, int)
#define MTK_M4U_T_INSERT_WRAP_RANGE   _IOW(MTK_M4U_MAGICNO, 13, int)
#define MTK_M4U_T_MONITOR_START       _IOW(MTK_M4U_MAGICNO, 14, int)
#define MTK_M4U_T_MONITOR_STOP        _IOW(MTK_M4U_MAGICNO, 15, int)
#define MTK_M4U_T_RESET_MVA_RELEASE_TLB  _IOW(MTK_M4U_MAGICNO, 16, int)
#define MTK_M4U_T_CONFIG_PORT_ROTATOR _IOW(MTK_M4U_MAGICNO, 17, int)
#define MTK_M4U_T_QUERY_MVA           _IOW(MTK_M4U_MAGICNO, 18, int)
#define MTK_M4U_T_M4UDrv_CONSTRUCT    _IOW(MTK_M4U_MAGICNO, 19, int)
#define MTK_M4U_T_M4UDrv_DECONSTRUCT  _IOW(MTK_M4U_MAGICNO, 20, int)
#define MTK_M4U_T_DUMP_PAGETABLE      _IOW(MTK_M4U_MAGICNO, 21, int)
#define MTK_M4U_T_REGISTER_BUFFER     _IOW(MTK_M4U_MAGICNO, 22, int)
#define MTK_M4U_T_CACHE_FLUSH_ALL     _IOW(MTK_M4U_MAGICNO, 23, int)
#define MTK_M4U_T_CONFIG_PORT_ARRAY   _IOW(MTK_M4U_MAGICNO, 26, int)
#define MTK_M4U_T_CONFIG_MAU          _IOW(MTK_M4U_MAGICNO, 27, int)
#define MTK_M4U_T_CONFIG_TF           _IOW(MTK_M4U_MAGICNO, 28, int)



#endif
