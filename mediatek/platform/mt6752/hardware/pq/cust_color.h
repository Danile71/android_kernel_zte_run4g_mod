#ifndef __CUST_COLOR_H__
#define __CUST_COLOR_H__

#include "ddp_drv.h"

#define PQ_DEVICE_NODE                      "/proc/mtk_mira"

#define PQ_PIC_MODE_STANDARD (0)
#define PQ_PIC_MODE_VIVID    (1)
#define PQ_PIC_MODE_USER_DEF (2)

#define PQ_PIC_MODE_DEFAULT                 "0"     // default is STANDARD
#define PQ_PIC_MODE_PROPERTY_STR            "persist.sys.pq.picmode"
#define PQ_CONTRAST_INDEX_RANGE_NUM         (10)
#define PQ_CONTRAST_INDEX_DEFAULT           "4"
#define PQ_CONTRAST_PROPERTY_STR            "persist.sys.pq.contrastidx"
#define PQ_GSAT_INDEX_RANGE_NUM             (10)
#define PQ_GSAT_INDEX_DEFAULT               "4"
#define PQ_GSAT_PROPERTY_STR                "persist.sys.pq.gsatidx"
#define PQ_PIC_BRIGHT_INDEX_RANGE_NUM       (10)
#define PQ_PIC_BRIGHT_INDEX_DEFAULT         "4"
#define PQ_PIC_BRIGHT_PROPERTY_STR          "persist.sys.pq.brightidx"
#define PQ_TDSHP_INDEX_RANGE_NUM            (10)
#define PQ_TDSHP_STANDARD_DEFAULT           "2"
#define PQ_TDSHP_VIVID_DEFAULT              "2"
#define PQ_TDSHP_USER_DEFAULT               "2"     // sync with VIVID
#define PQ_TDSHP_INDEX_DEFAULT              PQ_TDSHP_STANDARD_DEFAULT
#define PQ_TDSHP_PROPERTY_STR               "persist.sys.pq.shp.idx"
#define PQ_ADL_INDEX_RANGE_NUM              (1)
#define PQ_ADL_INDEX_DEFAULT                "0"
#define PQ_ADL_PROPERTY_STR                 "persist.sys.pq.adl.idx"    // default on, changes will be reset after reboot!

// for debug
#define PQ_DBG_SHP_EN_DEFAULT                   "2" // 0: disable, 1: enable, 2: default
#define PQ_DBG_SHP_EN_STR                       "debug.pq.shp.en"

#define PQ_DBG_ADL_EN_DEFAULT                   "2" // 0: disable, 1: enable, 2: default
#define PQ_DBG_ADL_EN_STR                       "debug.pq.adl.en"
#define PQ_DBG_ADL_DEBUG_DEFAULT                "0" // 0: disable, 255: log all on
#define PQ_DBG_ADL_DEBUG_STR                    "debug.pq.adl.dbg"

#define PQ_DBG_DSHP_EN_DEFAULT                  "2" // 0: disable, 1: enable, 2: default
#define PQ_DBG_DSHP_EN_STR                      "debug.pq.dshp.en"
#define PQ_DBG_DSHP_DEBUG_DEFAULT               "0" // 0: disable, 0xff: log all on
#define PQ_DBG_DSHP_DEBUG_STR                   "debug.pq.dshp.dbg"


extern const DISP_PQ_PARAM pqparam_standard;
extern const DISP_PQ_PARAM pqparam_vivid;
extern const DISP_PQ_PARAM pqparam_camera;
extern const DISPLAY_PQ_T pqindex;

#endif

