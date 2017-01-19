#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/module.h>
#include <generated/autoconf.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/param.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <asm/io.h>
#include <asm/page.h>

#include <linux/workqueue.h>
#include <linux/semaphore.h>
#include <linux/slab.h>

#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/module.h>
#include <linux/wait.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include "debug.h"
#include "disp_drv_log.h"
#include "disp_lcm.h"
#include "disp_utils.h"

#include "ddp_hal.h"
#include "ddp_dump.h"
#include "ddp_path.h"
#include "ddp_drv.h"
#include "ddp_info.h"
#include "ddp_ovl.h"

#include <mach/m4u.h>
#include <mach/m4u_port.h>
#include "primary_display.h"
#include "cmdq_def.h"
#include "cmdq_record.h"
#include "cmdq_reg.h"
#include "cmdq_core.h"

#include "ddp_manager.h"
#include "disp_drv_platform.h"
#include "display_recorder.h"

#include "mtk_disp_mgr.h"

#include "disp_session.h"
#include "mtk_ovl.h"
#include "ddp_mmp.h"

extern unsigned int is_hwc_enabled;
extern int is_DAL_Enabled(void);
#define DISP_DISABLE_X_CHANNEL_ALPHA

#ifdef MTK_HDMI_SUPPORT
#include "extd_drv.h"
#endif

// TODO: revise this later @xuecheng
#include "mtkfb_fence.h"

typedef enum
{
	PREPARE_INPUT_FENCE,
	PREPARE_OUTPUT_FENCE,
	PREPARE_PRESENT_FENCE
}ePREPARE_FENCE_TYPE;


static dev_t mtk_disp_mgr_devno;
static struct cdev *mtk_disp_mgr_cdev;
static struct class *mtk_disp_mgr_class = NULL;
static disp_session_input_config g_session_input;

static SWITCH_MODE_INFO_STRUCT path_info;

int ovl1_remove = 0;
struct task_struct *disp_switch_mode_task = NULL;
wait_queue_head_t switch_mode_wq;
atomic_t switch_mode_event = ATOMIC_INIT(0);

static int mtk_disp_mgr_open(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t mtk_disp_mgr_read(struct file *file, char __user *data, size_t len, loff_t *ppos)
{
	return 0;
}

static int mtk_disp_mgr_release(struct inode *inode, struct file *file)
{
	return 0;
}

static int mtk_disp_mgr_flush(struct file * a_pstFile , fl_owner_t a_id)
{
	return 0;
}

static int mtk_disp_mgr_mmap(struct file *file, struct vm_area_struct * vma)
{
    static const unsigned long addr_min = 0x14007000;
    static const unsigned long addr_max = 0x14018000;
    static const unsigned long size =addr_max - addr_min;
    const unsigned long require_size = vma->vm_end - vma->vm_start;
    unsigned long pa_start = vma->vm_pgoff << PAGE_SHIFT;
    unsigned long pa_end = pa_start + require_size;
    DISPDBG("mmap size %d, vmpg0ff 0x%x, pastart 0x%x, paend 0x%x\n",
        require_size, vma->vm_pgoff, pa_start,pa_end);

    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

    if (require_size > size || 
        (pa_start < addr_min || pa_end > addr_max))
    {
        DISPERR("mmap size range over flow!!\n");
        return -EAGAIN;
    }
    if(remap_pfn_range(vma , 
                 vma->vm_start , 
                 vma->vm_pgoff , 
                 (vma->vm_end - vma->vm_start) , 
                 vma->vm_page_prot))
    {
        DISPERR("display mmap failed!!\n");
        return -EAGAIN;
    }
    
    return 0;
}
#define DDP_OUTPUT_LAYID DDP_OVL_LAYER_MUN

static unsigned int session_config[MAX_SESSION_COUNT];
static DEFINE_MUTEX(disp_session_lock);

int disp_get_session_number()
{
  int i=0;
  int session_cnt = 0;
  for(i=0;i<MAX_SESSION_COUNT;i++)
  {
      if(session_config[i]!=0)
        session_cnt++;
  }

  return session_cnt;
}

#ifdef OVL_CASCADE_SUPPORT
OVL_CONFIG_STRUCT ovl2mem_in_cached_config[DDP_OVL_LAYER_MUN] =
{
	{.layer = 0, .isDirty = 1},
	{.layer = 1, .isDirty = 1},
	{.layer = 2, .isDirty = 1},
	{.layer = 3, .isDirty = 1},
	{.layer = 4, .isDirty = 1},
	{.layer = 5, .isDirty = 1},
	{.layer = 6, .isDirty = 1},
	{.layer = 7, .isDirty = 1}
}; 
#else
OVL_CONFIG_STRUCT ovl2mem_in_cached_config[DDP_OVL_LAYER_MUN] =
{
	{.layer = 0, .isDirty = 1},
	{.layer = 1, .isDirty = 1},
	{.layer = 2, .isDirty = 1},
	{.layer = 3, .isDirty = 1}
}; 
#endif

disp_mem_output_config _ovl2mem_out_cached_config = 
{
	
};

int _session_inited(disp_session_config config)
{
	// TODO:
	#if 0
	int i, idx = -1;

	for (i = 0; i < MAX_SESSION_COUNT; i++)
	{
		if (session_config[i] == 0 && idx == -1)
		{
			idx = i;
		}

		if (session_config[i] == session)
		{
			ret = DCP_STATUS_ALREADY_EXIST;
			DISPMSG("session(0x%x) already exists\n", session);
			break;
		}
	}
	#endif
	return 0;
}
static int mem_out_active = 0;
wait_queue_head_t ovl1_wait_queue;
static int g_enable_clock = 0;
int disp_create_session (disp_session_config*config)
{
	int ret = 0;
	int is_session_inited = 0;
	unsigned int session = MAKE_DISP_SESSION(config->type, config->device_id);
	int i, idx = -1;
	//1.To check if this session exists already
	mutex_lock(&disp_session_lock);
	for (i = 0; i < MAX_SESSION_COUNT; i++)
	{
		if (session_config[i] == session)
		{
			is_session_inited = 1;
			idx = i;
			break;
		}
	}

	if(is_session_inited == 1)
	{
		config->session_id = session;
		goto done;
	}

	for (i = 0; i < MAX_SESSION_COUNT; i++)
	{
		if (session_config[i] == 0 && idx == -1)
		{
			idx = i;
			break;
		}
	}
	//1.To check if support this session (mode,type,dev)
	//TODO:

	//2. Create this session
	if (idx != -1)
	{
		config->session_id = session;
		session_config[idx] = session;
		DISPDBG("New session(0x%x)\n", session);
	}
	else
	{
		DISPERR("Invalid session creation request\n");
		ret = -1;
	}
done:
	mutex_unlock(&disp_session_lock);
	
	DISPPR_FENCE("new session:0x%x\n", config->session_id);
	is_hwc_enabled = 1;
	
	return ret;
}
bool release_session_buffer (DISP_SESSION_TYPE	type, unsigned int layerid, unsigned int layer_phy_addr);
int disp_destroy_session (disp_session_config* config)
{
	int ret = -1;
	unsigned int session = config->session_id;
	int i, idx;

    DISPMSG("disp_destroy_session, 0x%x", config->session_id);
    DISPPR_FENCE("destroy_session:0x%x\n", config->session_id);
    
	release_session_buffer(DISP_SESSION_TYPE(config->session_id), 0xFF, NULL);
	
	//1.To check if this session exists already, and remove it
	mutex_lock(&disp_session_lock);
	for (i = 0; i < MAX_SESSION_COUNT; i++)
	{
		if (session_config[i] == session)
		{
			session_config[i] = 0;
			ret = 0;
			break;
		}
	}

	mutex_unlock(&disp_session_lock);
	
    DISPPR_FENCE("destroy_session done\n");
	//2. Destroy this session
	if (ret == 0)
	{
		DISPDBG("Destroy session(0x%x)\n", session);
	}
	else
	{
		DISPMSG("session(0x%x) does not exists\n", session);
	}

	return ret;
}

bool release_session_buffer (DISP_SESSION_TYPE	type, unsigned int layerid, unsigned int layer_phy_addr)
{
    unsigned int session = 0;
    int i= 0;
    int buff_idx = 0;
    int release_layer_num = HW_OVERLAY_COUNT;

    if((type <  DISP_SESSION_EXTERNAL))
        return false;

    for (i = 0; i < MAX_SESSION_COUNT; i++)
	{
		if (DISP_SESSION_TYPE(session_config[i]) == type)
		{
			session = session_config[i] ;
			break;
		}
	}

    if(session == 0)
        ///||((layerid < HW_OVERLAY_COUNT) &&(layer_phy_addr == NULL)))
    {
        DISPERR("sess(0x%x) (type %d)(addr 0x%x) failed! \n", session, type, layer_phy_addr);
        return false;
    }

    mutex_lock(&disp_session_lock);

    if( layerid < release_layer_num)
    {
        buff_idx = mtkfb_query_release_idx(session, layerid, layer_phy_addr);

        if(buff_idx > 0)
            DISPMSG("release_session_buffer layerid %x addr 0x%x idx %d \n", layerid, layer_phy_addr, buff_idx);

        if(buff_idx > 0)
            mtkfb_release_fence(session, layerid, buff_idx);
    }
    else
    {
        if(type == DISP_SESSION_MEMORY)
            release_layer_num = HW_OVERLAY_COUNT + 1;
            
        for (i = 0; i < release_layer_num; i++)        
        {            
            mtkfb_release_layer_fence(session, i);
        }
        
        DISPMSG("release_session_buffer(type %d) release layer %d \n", type, release_layer_num);
    }
releae_exit:

    mutex_unlock(&disp_session_lock);
	return true;
}
int _ioctl_create_session(unsigned long arg)
{
	int ret = 0;
	void __user *argp = (void __user *)arg;
	disp_session_config config;
	if (copy_from_user(&config, argp, sizeof(config)))
	{
		DISPMSG("[FB]: copy_from_user failed! line:%d \n", __LINE__);
		return -EFAULT;
	}

	if((config.type == DISP_SESSION_MEMORY) && (get_ovl1_to_mem_on() == false))
	{
	    DISPMSG("[FB]: _ioctl_create_session! line:%d  %d\n", __LINE__, get_ovl1_to_mem_on());
	    return -EFAULT;
    }

	if (disp_create_session(&config) != 0)
	{
		ret = -EFAULT;
	}

	if (copy_to_user(argp, &config, sizeof(config)))
	{
		DISPMSG("[FB]: copy_to_user failed! line:%d \n", __LINE__);
		ret = -EFAULT;
	}

	return (ret);
}

int _ioctl_destroy_session(unsigned long arg)
{
	int ret = 0;
	void __user *argp = (void __user *)arg;
	disp_session_config config;
	if (copy_from_user(&config, argp, sizeof(config)))
	{
		DISPMSG("[FB]: copy_from_user failed! line:%d \n", __LINE__);
		return -EFAULT;
	}

	if (disp_destroy_session(&config) != 0)
	{
		ret = -EFAULT;
	}

	return (ret);
}

#if 0
int _mem_out_release_fence_callback(unsigned int userdata)
{
	int i = 0;
    unsigned int session_id = MAKE_DISP_SESSION(DISP_SESSION_PRIMARY,0);
	int fence_idx = userdata;

    MMP_MetaDataBitmap_t Bitmap;
    Bitmap.data1 = fence_idx;
    Bitmap.pData = mtkfb_query_buf_va(session_id, 4, fence_idx);
    Bitmap.start_pos =  0;
    Bitmap.width = _ovl2mem_out_cached_config.w;
    Bitmap.height= _ovl2mem_out_cached_config.h;
    Bitmap.format = MMProfileBitmapBGRA8888; 
    Bitmap.bpp = 32;
    Bitmap.pitch = _ovl2mem_out_cached_config.pitch;
    Bitmap.data_size = Bitmap.pitch * _ovl2mem_out_cached_config.h;
    Bitmap.down_sample_x = 4;
    Bitmap.down_sample_y = 4;
    MMProfileLogMetaBitmap(HDMI_MMP_Events.DDPKBitblt, MMProfileFlagPulse, &Bitmap)  ;
	if(fence_idx > 0)
		mtkfb_release_fence(session_id, DDP_OUTPUT_LAYID, fence_idx);

	///if(ticket == _ovl2mem_out_cached_config.ticket)
	///    primary_dislay_mirror_enable(false);

    DISPMSG("mem_out release fence idx:0x%x, cfgidx:0x%x\n", fence_idx,_ovl2mem_out_cached_config.buff_idx);
     
	return 0;
}
#endif                    

char *disp_session_mode_spy(unsigned int session_id)
{
	switch(DISP_SESSION_TYPE(session_id))
	{
		case DISP_SESSION_PRIMARY: return "P";
		case DISP_SESSION_EXTERNAL: return "E";
		case DISP_SESSION_MEMORY: return "M";
		default:	return "Unknown";
	}
}

static unsigned int get_current_ticket(void)
{	
	return dprec_get_vsync_count();
}

int _ioctl_trigger_session(unsigned long arg)
{
	int ret = 0;
	int i = 0;
	void __user *argp = (void __user *)arg;
	disp_session_config config;
	unsigned int session_id = 0;
	int present_fence_idx = -1;
	unsigned long ticket = 0;
	
	if (copy_from_user(&config, argp, sizeof(config)))
	{
		DISPMSG("[FB]: copy_from_user failed! line:%d \n", __LINE__);
		return -EFAULT;
	}

    session_id = config.session_id;
		
	ticket = primary_display_get_ticket();

	disp_session_sync_info *session_info = disp_get_session_sync_info_for_debug(session_id);

	if(session_info)
	{
		dprec_start(&session_info->event_trigger, 0, 0);
	}
	
	DISPPR_FENCE("T+/%s%d\n", disp_session_mode_spy(session_id), DISP_SESSION_DEV(session_id));

	if(DISP_SESSION_TYPE(session_id) == DISP_SESSION_PRIMARY)
	{
		//present_fence_idx = config.present_fence_idx;
		primary_display_trigger(0, NULL, 0);
    	
    	if(_ovl2mem_out_cached_config.mode >= DISP_SESSION_DIRECT_LINK_MIRROR_MODE)
    	{
    		DISPMSG(" mem out _ioctl_trigger_session,%d \n", config.mode);
    		primary_display_mem_out_trigger(0, NULL, _ovl2mem_out_cached_config.buff_idx);
    		DISPMSG(" mem out _ioctl_trigger_session idx 0x%x done \n", _ovl2mem_out_cached_config.buff_idx);            
    	}
	}
	else if(DISP_SESSION_TYPE(session_id) == DISP_SESSION_EXTERNAL)
	{
#ifdef MTK_HDMI_SUPPORT
		mutex_lock(&disp_session_lock);
	    
		ret = extd_hdmi_trigger(0, NULL, session_id);
    if (ovl1_remove == 2)
    {
        ovl1_remove ++;
    }
    else if (ovl1_remove == 3)
    {
        ovl_set_status(DDP_OVL1_STATUS_IDLE);
        ovl1_remove = 0;                
    }
		mutex_unlock(&disp_session_lock);
#endif
	}
	else if(DISP_SESSION_TYPE(session_id) == DISP_SESSION_MEMORY)
	{
        ovl2mem_trigger(1, NULL, 0);
	}
	else
	{
		DISPERR("session type is wrong:0x%08x\n", session_id);
		ret =  -1;
	}

	if(session_info)
	{
		dprec_done(&session_info->event_trigger, 0, 0);
	}

	return (ret);
}


int _ioctl_prepare_buffer(unsigned long arg, ePREPARE_FENCE_TYPE type)
{
	int ret = 0;
	void __user *argp = (void __user *)arg;
	disp_buffer_info info;
	struct mtkfb_fence_buf_info *buf, *buf2;

	if (copy_from_user(&info, (void __user *)arg, sizeof(info)))
	{
		printk("[FB Driver]: copy_from_user failed! line:%d \n", __LINE__);
		return (-EFAULT);
	} 

	if(type == PREPARE_INPUT_FENCE)
	{

	}
	else if(type == PREPARE_PRESENT_FENCE)
	{
		info.layer_id = disp_sync_get_present_timeline_id();
	}
	else if(type == PREPARE_OUTPUT_FENCE)
	{
        info.layer_id = disp_sync_get_output_timeline_id();
	}
	else
	{
		DISPPR_ERROR("type is wrong: %d\n", type);
	}
	
	if (info.layer_en && info.ion_fd >= 0)
	{
		buf = disp_sync_prepare_buf(&info);
		if (buf != NULL)
		{
			info.fence_fd = buf->fence;
			info.index = buf->idx;
		}
		else
		{
			DISPPR_ERROR("prepare fence failed, 0x%08x/l%d/e%d/ion%d/cache%d\n", info.session_id, info.layer_id, info.layer_en, info.ion_fd, info.cache_sync);
			DISPPR_FENCE("P+ FAIL /%s%d/l%d/e%d/ion%d/c%d/id%d/ffd%d\n",disp_session_mode_spy(info.session_id),   DISP_SESSION_DEV(info.session_id), info.layer_id, info.layer_en, info.ion_fd, info.cache_sync,info.index, info.fence_fd);				
			info.fence_fd = MTK_FB_INVALID_FENCE_FD; // invalid fd
			info.index = 0;
		}

		if(type == PREPARE_OUTPUT_FENCE)
		{
			if(!primary_display_is_decouple_mode())
			{
				info.interface_fence_fd = MTK_FB_INVALID_FENCE_FD; // invalid fd
				info.interface_index = 0;
			}
			else
			{
				info.layer_id = disp_sync_get_output_interface_timeline_id();
				buf2 = disp_sync_prepare_buf(&info);
				if (buf2 != NULL)
				{
					info.interface_fence_fd = buf2->fence;
					info.interface_index = buf2->idx;
				}
				else
				{
					DISPPR_ERROR("prepare fence failed, 0x%08x/l%d/e%d/ion%d/cache%d\n", info.session_id, info.layer_id, info.layer_en, info.ion_fd, info.cache_sync);
					DISPPR_FENCE("P+ FAIL /%s%d/l%d/e%d/ion%d/c%d/id%d/ffd%d\n",disp_session_mode_spy(info.session_id),   DISP_SESSION_DEV(info.session_id), info.layer_id, info.layer_en, info.ion_fd, info.cache_sync,info.index, info.fence_fd);				
					info.interface_fence_fd = MTK_FB_INVALID_FENCE_FD; // invalid fd
					info.interface_index = 0;
				}
			}
		}
	}
	else
	{
		DISPPR_ERROR("wrong prepare param, 0x%08x/l%d/e%d/ion%d/cache%d\n", info.session_id, info.layer_id, info.layer_en, info.ion_fd, info.cache_sync);
		DISPPR_FENCE("P+ FAIL /%s%d/l%d/e%d/ion%d/c%d/id%d/ffd%d\n",disp_session_mode_spy(info.session_id),   DISP_SESSION_DEV(info.session_id), info.layer_id, info.layer_en, info.ion_fd, info.cache_sync,info.index, info.fence_fd);							
		info.fence_fd = MTK_FB_INVALID_FENCE_FD;	  // invalid fd
		info.index = 0;
	}

	if (copy_to_user(argp, &info, sizeof(info)))
	{
		printk("[FB Driver]: copy_to_user failed! line:%d \n", __LINE__);
		ret =  -EFAULT;
	}
        
	return (ret);
}

const char* _disp_format_spy(DISP_FORMAT format)
{
	switch(format)
	{
		case     DISP_FORMAT_RGB565: 		return "RGB565";
		case     DISP_FORMAT_RGB888: 		return "RGB888";
		case     DISP_FORMAT_BGR888: 		return "BGR888";
		case     DISP_FORMAT_ARGB8888: 		return "ARGB8888";
		case     DISP_FORMAT_ABGR8888: 		return "ABGR8888";
		case     DISP_FORMAT_RGBA8888: 		return "RGBA8888";
		case     DISP_FORMAT_BGRA8888: 		return "BGRA8888";
		case     DISP_FORMAT_YUV422: 		return "YUV422";
		case     DISP_FORMAT_XRGB8888: 		return "XRGB8888";
		case     DISP_FORMAT_XBGR8888: 		return "XBGR8888";
		case	 DISP_FORMAT_RGBX8888: 		return "RGBX8888";
		case	 DISP_FORMAT_BGRX8888: 		return "BGRX8888";
		case     DISP_FORMAT_UYVY: 			return "UYVY";
		case     DISP_FORMAT_YUV420_P: 		return "YUV420_P";
		case	 DISP_FORMAT_YV12: 			return "YV12";
		default: return "unknown";
	}
}
static int _sync_convert_fb_layer_to_ovl_struct(unsigned int session_id, disp_input_config* src, OVL_CONFIG_STRUCT *dst, unsigned int dst_mva)
{
	unsigned int layerpitch = 0;
	unsigned int layerbpp = 0;

	dst->layer = src->layer_id;

	if (!src->layer_enable)
	{
		dst->layer_en = 0;
		dst->isDirty = true;
		return 0;
	}

	switch (src->src_fmt)
	{
		case DISP_FORMAT_YUV422:
			dst->fmt = eYUY2;
			layerpitch = 2;
			layerbpp = 16;
			break;

		case DISP_FORMAT_RGB565:
			dst->fmt = eRGB565;
			layerpitch = 2;
			layerbpp = 16;
			break;

		case DISP_FORMAT_RGB888:
			dst->fmt = eRGB888;
			layerpitch = 3;
			layerbpp = 24;
			break;

		case DISP_FORMAT_BGR888:
			dst->fmt = eBGR888;
			layerpitch = 3;
			layerbpp = 24;
			break;
			//xuecheng, ??????
		case DISP_FORMAT_ARGB8888:
			//dst->fmt = eRGBA8888;
			dst->fmt = eARGB8888;
			layerpitch = 4;
			layerbpp = 32;
			break;

		case DISP_FORMAT_ABGR8888:
			dst->fmt = eABGR8888;
			layerpitch = 4;
			layerbpp = 32;
			break;
		case DISP_FORMAT_RGBA8888:
			dst->fmt = eRGBA8888;
			layerpitch = 4;
			layerbpp = 32;
			break;

		case DISP_FORMAT_BGRA8888:
			//dst->fmt = eABGR8888;
			dst->fmt = eBGRA8888;
			layerpitch = 4;
			layerbpp = 32;
			break;
		case DISP_FORMAT_XRGB8888:
			dst->fmt = eARGB8888;
			layerpitch = 4;
			layerbpp = 32;
			break;

		case DISP_FORMAT_XBGR8888:
			dst->fmt = eABGR8888;
			layerpitch = 4;
			layerbpp = 32;
			break;
		case DISP_FORMAT_RGBX8888:
			dst->fmt = eRGBA8888;
			layerpitch = 4;
			layerbpp = 32;
			break;
		case DISP_FORMAT_BGRX8888:
			dst->fmt = eBGRA8888;
			layerpitch = 4;
			layerbpp = 32;
			break;

		case DISP_FORMAT_UYVY:
			dst->fmt = eUYVY;
			layerpitch = 2;
			layerbpp = 16;
			break;

		default:
			DISPERR("Invalid color format: 0x%x\n", src->src_fmt);
			return -1;
	}

    	dst->vaddr = (unsigned int)src->src_base_addr;
    	dst->security = src->security;
#if defined (MTK_FB_ION_SUPPORT)
	if (src->src_phy_addr != NULL)
	{
		dst->addr = (unsigned int)src->src_phy_addr;
	}
	else
	{
		dst->addr = dst_mva;
	}
#else
    	dst->addr = (unsigned int)src->src_phy_addr;
#endif
    	dst->isTdshp = src->isTdshp;
    	dst->buff_idx = src->next_buff_idx;
	dst->identity = src->identity;
	dst->connected_type = src->connected_type;

    	//set Alpha blending
	dst->aen = src->alpha_enable;  
	dst->alpha = src->alpha;

	dst->sur_aen = src->sur_aen;
	dst->src_alpha = src->src_alpha;
	dst->dst_alpha = src->dst_alpha;

#ifdef DISP_DISABLE_X_CHANNEL_ALPHA
	if (DISP_FORMAT_ARGB8888 == src->src_fmt ||
	    DISP_FORMAT_ABGR8888 == src->src_fmt || 
	    DISP_FORMAT_RGBA8888 == src->src_fmt || 
	    DISP_FORMAT_BGRA8888 == src->src_fmt ||
        src->buffer_source==DISP_BUFFER_ALPHA)
	{
	    	// just keep what user set
	    	// dst->aen = TRUE;
			// dst->sur_aen = TRUE;
	}
	else
	{
	    	dst->aen = FALSE;
			dst->sur_aen = FALSE;
	}
#endif

	//set src width, src height
	dst->src_x = src->src_offset_x;
	dst->src_y = src->src_offset_y;
	dst->src_w = src->src_width;
	dst->src_h = src->src_height;
	dst->dst_x = src->tgt_offset_x;
	dst->dst_y = src->tgt_offset_y;
	dst->dst_w = src->tgt_width;
	dst->dst_h = src->tgt_height;
	if(src->buffer_source != DISP_BUFFER_ALPHA)
	{
	    if (dst->dst_w > dst->src_w)   
		    dst->dst_w = dst->src_w;
	    if (dst->dst_h > dst->src_h)    
		    dst->dst_h = dst->src_h;
    }
    
	dst->src_pitch = src->src_pitch*layerpitch;
    	//set color key
	dst->key = src->src_color_key;
	dst->keyEn = src->src_use_color_key;

    //data transferring is triggerred in MTKFB_TRIG_OVERLAY_OUT
	dst->layer_en= src->layer_enable;
	dst->isDirty = true;
	if(src->buffer_source==DISP_BUFFER_ALPHA)
	{
	    dst->source = OVL_LAYER_SOURCE_RESERVED;  // dim layer, constant alpha
	}
	else if(src->buffer_source==DISP_BUFFER_ION || src->buffer_source==DISP_BUFFER_MVA)
	{
	    dst->source = OVL_LAYER_SOURCE_MEM;  // from memory
	}
	else
	{
        DISPERR("unknow source = %d", src->buffer_source);
		dst->source = OVL_LAYER_SOURCE_MEM;
	}

	
	return 0;
}

static int _sync_convert_fb_layer_to_disp_input(unsigned int session_id, disp_input_config* src, primary_disp_input_config *dst, unsigned int dst_mva)
{
	unsigned int layerpitch = 0;
	unsigned int layerbpp = 0;

	dst->layer = src->layer_id;

	if (!src->layer_enable)
	{
		dst->layer_en = 0;
		dst->isDirty = true;
		return 0;
	}

	switch (src->src_fmt)
	{
		case DISP_FORMAT_YUV422:
			dst->fmt = eYUY2;
			layerpitch = 2;
			layerbpp = 16;
			break;

		case DISP_FORMAT_RGB565:
			dst->fmt = eRGB565;
			layerpitch = 2;
			layerbpp = 16;
			break;

		case DISP_FORMAT_RGB888:
			dst->fmt = eRGB888;
			layerpitch = 3;
			layerbpp = 24;
			break;

		case DISP_FORMAT_BGR888:
			dst->fmt = eBGR888;
			layerpitch = 3;
			layerbpp = 24;
			break;
			//xuecheng, ??????
		case DISP_FORMAT_ARGB8888:
			dst->fmt = eARGB8888;
			layerpitch = 4;
			layerbpp = 32;
			break;

		case DISP_FORMAT_ABGR8888:
			dst->fmt = eABGR8888;
			layerpitch = 4;
			layerbpp = 32;
			break;
		case DISP_FORMAT_RGBA8888:
			dst->fmt = eRGBA8888;
			layerpitch = 4;
			layerbpp = 32;
			break;

		case DISP_FORMAT_BGRA8888:
			//dst->fmt = eABGR8888;
			dst->fmt = eBGRA8888;
			layerpitch = 4;
			layerbpp = 32;
			break;
		case DISP_FORMAT_XRGB8888:
			dst->fmt = eARGB8888;
			layerpitch = 4;
			layerbpp = 32;
			break;

		case DISP_FORMAT_XBGR8888:
			dst->fmt = eABGR8888;
			layerpitch = 4;
			layerbpp = 32;
			break;

		case DISP_FORMAT_RGBX8888:
			dst->fmt = eRGBA8888;
			layerpitch = 4;
			layerbpp = 32;
			break;

		case DISP_FORMAT_BGRX8888:
			dst->fmt = eBGRA8888;
			layerpitch = 4;
			layerbpp = 32;
			break;

		case DISP_FORMAT_UYVY:
			dst->fmt = eUYVY;
			layerpitch = 2;
			layerbpp = 16;
			break;

        case DISP_FORMAT_YV12:
		    dst->fmt = eYV12;
		    layerpitch = 1;
			layerbpp = 8;
		    break;
		default:
			DISPERR("Invalid color format: 0x%x\n", src->src_fmt);
			return -1;
	}

	dst->vaddr = (unsigned int)src->src_base_addr;
	dst->security = src->security;
// set_overlay will not use fence+ion handle
#if defined (MTK_FB_ION_SUPPORT)
	if (src->src_phy_addr != NULL)
	{
		dst->addr = (unsigned int)src->src_phy_addr;
	}
	else
	{
		dst->addr = dst_mva;
	}
#else
		dst->addr = (unsigned int)src->src_phy_addr;
#endif

	dst->isTdshp = src->isTdshp;
	dst->buff_idx = src->next_buff_idx;
	dst->identity = src->identity;
	dst->connected_type = src->connected_type;

	//set Alpha blending
	dst->aen = src->alpha_enable;
	dst->alpha = src->alpha;
    
    dst->sur_aen = src->sur_aen;
    dst->src_alpha = src->src_alpha;
    dst->dst_alpha = src->dst_alpha;

#ifdef DISP_DISABLE_X_CHANNEL_ALPHA
    if (DISP_FORMAT_ARGB8888 == src->src_fmt ||
        DISP_FORMAT_ABGR8888 == src->src_fmt || 
        DISP_FORMAT_RGBA8888 == src->src_fmt || 
        DISP_FORMAT_BGRA8888 == src->src_fmt ||
        src->buffer_source == DISP_BUFFER_ALPHA)
    {
        // just keep what user set
        // dst->aen = TRUE;
        // dst->sur_aen = TRUE;
    }
    else
    {
        dst->aen = FALSE;
        dst->sur_aen = FALSE;
    }
#endif

	//set src width, src height
	dst->src_x = src->src_offset_x;
	dst->src_y = src->src_offset_y;
	dst->src_w = src->src_width;
	dst->src_h = src->src_height;
	dst->dst_x = src->tgt_offset_x;
	dst->dst_y = src->tgt_offset_y;
	dst->dst_w = src->tgt_width;
	dst->dst_h = src->tgt_height;	
	if(src->buffer_source != DISP_BUFFER_ALPHA)
	{
	    if (dst->dst_w > dst->src_w)   dst->dst_w = dst->src_w;
 	    if (dst->dst_h > dst->src_h)	dst->dst_h = dst->src_h;
    }
    
	dst->src_pitch = src->src_pitch*layerpitch;

	//set color key
	dst->key = src->src_color_key;
	dst->keyEn = src->src_use_color_key;

	//data transferring is triggerred in MTKFB_TRIG_OVERLAY_OUT
	dst->layer_en= src->layer_enable;
	dst->isDirty = true;
	dst->buff_source= src->buffer_source;

	return 0;
}

static int set_memory_buffer(disp_session_input_config * input)
{
	int i = 0;
	int layer_id = 0;
	unsigned int dst_size = 0;
	unsigned int dst_mva = 0; 
	unsigned int session_id = input->session_id;
	disp_session_sync_info *session_info = disp_get_session_sync_info_for_debug(session_id);
	
	ovl2mem_in_config input_params[HW_OVERLAY_COUNT];
	
	memset((void*)&input_params, 0, sizeof(input_params));
	
	for (i = 0; i < input->config_layer_num; i++)
	{
		dst_mva = 0;
		layer_id = input->config[i].layer_id;
		if(input->config[i].layer_enable )
		{
			if(input->config[i].buffer_source == DISP_BUFFER_ALPHA)
			{
				DISPPR_FENCE("ML %d is dim layer,fence %d\n",
						input->config[i].layer_id, input->config[i].next_buff_idx);
						//input->config[i].src_offset_x = 0;
						//input->config[i].src_offset_y = 0;
						//input->config[i].tgt_offset_x = 0;
						//input->config[i].tgt_offset_y = 0;
						input->config[i].sur_aen = 0;
						input->config[i].src_fmt = DISP_FORMAT_RGB888;
						input->config[i].src_pitch = input->config[i].src_width;
						input->config[i].src_phy_addr = get_dim_layer_mva_addr();
						input->config[i].next_buff_idx = 0;
			}		
				
			if(input->config[i].src_phy_addr)
			{
				dst_mva = input->config[i].src_phy_addr;
			}
			else
			{
				disp_sync_query_buf_info(session_id, layer_id, (unsigned int)input->config[i].next_buff_idx, &dst_mva, &dst_size);
			}

			if (dst_mva == 0) 
			{
				input->config[i].layer_enable = 0;
			}

			DISPPR_FENCE("S+/ML%d/e%d/id%d/%dx%d(%d,%d)(%d,%d)/%s/%d/0x%08x/mva0x%08x/t%d\n",
					input->config[i].layer_id,
					input->config[i].layer_enable,
					input->config[i].next_buff_idx,
					input->config[i].src_width,
					input->config[i].src_height,
					input->config[i].src_offset_x,
					input->config[i].src_offset_y,
					input->config[i].tgt_offset_x,
					input->config[i].tgt_offset_y,
					_disp_format_spy(input->config[i].src_fmt),
					input->config[i].src_pitch,
					input->config[i].src_phy_addr,
					dst_mva, get_ovl2mem_ticket());
		}
		else
		{
			DISPPR_FENCE("S+/ML%d/e%d/id%d\n", input->config[i].layer_id, input->config[i].layer_enable, input->config[i].next_buff_idx);
		}

		_sync_convert_fb_layer_to_ovl_struct(input->session_id, &(input->config[i]),&ovl2mem_in_cached_config[layer_id], dst_mva);
		///disp_sync_put_cached_layer_info(session_id, layer_id, &input->config[i], get_ovl2mem_ticket());
		mtkfb_update_buf_ticket(session_id, layer_id, input->config[i].next_buff_idx,get_ovl2mem_ticket());
		_sync_convert_fb_layer_to_disp_input(input->session_id, &(input->config[i]),&input_params[layer_id], dst_mva);
		input_params[layer_id].dirty = 1;

        if(input->config[i].layer_enable)
    	{
    		mtkfb_update_buf_info(input->session_id, input->config[i].layer_id,
    	                                input->config[i].next_buff_idx, 0, input->config[i].frm_sequence);
        }
        
		if(session_info)
		{
			dprec_submit(&session_info->event_setinput, input->config[i].next_buff_idx,
			        (input->config_layer_num<<28)|(input->config[i].layer_id<<24)|(input->config[i].src_fmt<<12)|input->config[i].layer_enable);
		}
	}

	ovl2mem_input_config(&input_params) ;

	return 0;
}

static int set_external_buffer(disp_session_input_config * input)
{
#ifdef MTK_HDMI_SUPPORT
	int i = 0;
	int ret = 0;
	int layer_id = 0;
	unsigned int dst_size = 0;
	unsigned int dst_mva = 0; 
	unsigned int session_id = 0;
	unsigned int mva_offset = 0;
	
	session_id = input->session_id;
	disp_session_sync_info *session_info = disp_get_session_sync_info_for_debug(session_id);
	if(extd_hdmi_path_get_mode() == EXTD_RDMA_DPI_MODE)
	{
		input->config_layer_num = 1;
		if (ovl1_remove == 1)
		{
			DISPMSG("Donglei - config M4U Port DISP_MODULE_RDMA1\n");
			M4U_PORT_STRUCT sPort;
			sPort.ePortID = M4U_PORT_DISP_RDMA1;
			sPort.Virtuality = 1;					   
			sPort.Security = 0;
			sPort.Distance = 1;
			sPort.Direction = 0;
			ret = m4u_config_port(&sPort);
			if(ret != 0)
			{
				DISPMSG("config M4U Port DISP_MODULE_RDMA1 FAIL\n");
			}
			ovl1_remove++;
		}
	}

	ext_disp_input_config extd_input[HW_OVERLAY_COUNT];
	memset((void*)&extd_input, 0, sizeof(extd_input));

	for (i = 0; i < input->config_layer_num; ++i)
	{
		dst_mva = 0;
		layer_id = input->config[i].layer_id;
		if(input->config[i].layer_enable)
		{
			if(input->config[i].buffer_source == DISP_BUFFER_ALPHA)
			{
				DISPPR_FENCE("EL %d is dim layer,fence %d\n",
					input->config[i].layer_id, input->config[i].next_buff_idx);
				input->config[i].src_offset_x = 0;
				input->config[i].src_offset_y = 0;
				//input->config[i].tgt_offset_x = 0;
				//input->config[i].tgt_offset_y = 0;
				input->config[i].sur_aen = 0;
				input->config[i].src_fmt = DISP_FORMAT_RGB888;
				input->config[i].src_pitch = input->config[i].src_width;
				input->config[i].src_phy_addr = get_dim_layer_mva_addr();
				input->config[i].next_buff_idx = 0;
			}
			if(input->config[i].src_phy_addr)
			{
				dst_mva = input->config[i].src_phy_addr;
			}
			else
			{
				disp_sync_query_buf_info(session_id, layer_id, (unsigned int)input->config[i].next_buff_idx, &dst_mva, &dst_size);
				//if(dst_size < input->config[i].src_pitch * input->config[i].src_height)
				//	DISPERR("");
			}

			if(dst_mva == 0)
			{
				input->config[i].layer_enable = 0;
			}

			DISPPR_FENCE("S+/EL%d/e%d/id%d/%dx%d(%d,%d)(%d,%d)/%s/%d/0x%08x/mva0x%08x\n",
				input->config[i].layer_id,
				input->config[i].layer_enable,
				input->config[i].next_buff_idx,
				input->config[i].src_width,
				input->config[i].src_height,
				input->config[i].src_offset_x,
				input->config[i].src_offset_y,
				input->config[i].tgt_offset_x,
				input->config[i].tgt_offset_y,
				_disp_format_spy(input->config[i].src_fmt),
				input->config[i].src_pitch,
				input->config[i].src_phy_addr,
				dst_mva);
		}
		else
		{
			DISPPR_FENCE("S+/EL%d/e%d/id%d\n", input->config[i].layer_id, input->config[i].layer_enable, input->config[i].next_buff_idx);
		}

		///_sync_convert_fb_layer_to_ovl_struct(input->session_id, &(input->config[i]),&external_cached_layer_config[layer_id], dst_mva);
		disp_sync_put_cached_layer_info(session_id, layer_id, &input->config[i], dst_mva);
		_sync_convert_fb_layer_to_disp_input(input->session_id, &(input->config[i]),&extd_input[layer_id], dst_mva);

		extd_input[layer_id].dirty = 1;

		if(input->config[i].layer_enable)
		{
			// OVL addr is not the start address of buffer, which is calculated by pitch and ROI.
			unsigned int hw_fmt, Bpp, bpp, x, y, pitch;
			x = input->config[i].src_offset_x;
			y = input->config[i].src_offset_y;
			pitch = input->config[i].src_pitch;
			disp_fmt_to_hw_fmt(input->config[i].src_fmt, &hw_fmt, &Bpp, &bpp);
			mva_offset = (x + y * pitch) * Bpp;
			mtkfb_update_buf_info(input->session_id, input->config[i].layer_id, input->config[i].next_buff_idx, mva_offset, input->config[i].frm_sequence);
		}

		if(session_info)
		{
			dprec_submit(&session_info->event_setinput, input->config[i].next_buff_idx,
					(input->config_layer_num<<28)|(input->config[i].layer_id<<24)|(input->config[i].src_fmt<<12)|input->config[i].layer_enable);
		}
	}

	ret = extd_hdmi_config_input_multiple(&extd_input, input->config[0].next_buff_idx);

    if (ret == -2)
    {
        for (i = 0; i < input->config_layer_num; i++)		  
        {			 
            mtkfb_release_layer_fence(input->session_id, i);
        }
    }
#endif

	return 0;
}

static int _remove_assert_layer(disp_session_input_config *input)
{
	int i, assert_layer, idx=-1;
	assert_layer = primary_display_get_option("ASSERT_LAYER");

	for(i=0; i<input->config_layer_num; i++) {
		if(input->config[i].layer_id == assert_layer) {
			idx = i;
			break;
		}
	}
	if(idx == -1)
		return -1;
	
	DISPPR_FENCE("skip aee layer L%d\n", assert_layer);

	//remove this layer config
	for(i=idx+1; i<input->config_layer_num; i++) {
		input->config[i-1] = input->config[i];
	}
	input->config_layer_num--;
	return 0;
}

static inline void remove_assert_layer(disp_session_input_config *input)
{
	if(!is_DAL_Enabled())
		return;
	while(!_remove_assert_layer(input));
}

static int set_primary_buffer(disp_session_input_config *input)
{
	int i = 0;
	int layer_id = 0;
	unsigned int dst_size = 0;
	unsigned int dst_mva = 0; 
	unsigned int session_id = 0;
	unsigned int mva_offset = 0;
	
	session_id = input->session_id;
	disp_session_sync_info *session_info = disp_get_session_sync_info_for_debug(session_id);

	remove_assert_layer(input);
	if(input->config_layer_num==0 || input->config_layer_num>OVL_LAYER_NUM)
	{
	    DISPERR("set_primary_buffer, config_layer_num invalid = %d! \n", input->config_layer_num);
	    return 0;
    }
    
	for (i = 0; i < input->config_layer_num; i++)
	{
		dst_mva = 0;
		layer_id = input->config[i].layer_id;
		if(layer_id>OVL_LAYER_NUM)
		{
           DISPERR("set_primary_buffer, invalid layer_id = %d! \n", layer_id);
	       continue;
		}
		
		if(input->config[i].layer_enable)
		{
			unsigned int hw_fmt, Bpp, bpp, x, y, pitch;

			if(input->config[i].buffer_source == DISP_BUFFER_ALPHA)
			{
				DISPPR_FENCE("PL %d is dim layer,fence %d\n",
				input->config[i].layer_id, input->config[i].next_buff_idx);
				
				input->config[i].sur_aen = 0;
				input->config[i].src_fmt = DISP_FORMAT_RGB888;
				input->config[i].src_pitch = input->config[i].src_width;
				input->config[i].src_phy_addr = get_dim_layer_mva_addr();
				input->config[i].next_buff_idx = 0;
			}
			
			if(input->config[i].src_phy_addr)
				dst_mva = input->config[i].src_phy_addr;
			else
				disp_sync_query_buf_info(session_id, layer_id, 
							(unsigned int)input->config[i].next_buff_idx, &dst_mva, &dst_size);

			input->config[i].src_phy_addr = dst_mva;

			if(dst_mva == 0)
				input->config[i].layer_enable = 0;

			// OVL addr is not the start address of buffer, which is calculated by pitch and ROI.
			x = input->config[i].src_offset_x;
			y = input->config[i].src_offset_y;
			pitch = input->config[i].src_pitch;
			disp_fmt_to_hw_fmt(input->config[i].src_fmt, &hw_fmt, &Bpp, &bpp);
			mva_offset = (x + y * pitch) * Bpp;
			mtkfb_update_buf_info(input->session_id, input->config[i].layer_id, 
					input->config[i].next_buff_idx, mva_offset, input->config[i].frm_sequence);
			
			DISPPR_FENCE("S+/PL%d/e%d/id%d/%dx%d(%d,%d)(%d,%d)/%s/%d/0x%08x/mva0x%08x\n",
				input->config[i].layer_id,
				input->config[i].layer_enable,
				input->config[i].next_buff_idx,
				input->config[i].src_width,
				input->config[i].src_height,
				input->config[i].src_offset_x,
				input->config[i].src_offset_y,
				input->config[i].tgt_offset_x,
				input->config[i].tgt_offset_y,
				_disp_format_spy(input->config[i].src_fmt),
				input->config[i].src_pitch,
				input->config[i].src_phy_addr,
				dst_mva);
		}
		else
		{
			DISPPR_FENCE("S+/PL%d/e%d/id%d\n", input->config[i].layer_id, input->config[i].layer_enable, input->config[i].next_buff_idx);
		}

		disp_sync_put_cached_layer_info(session_id, layer_id, &input->config[i], dst_mva);

		if(session_info)
			dprec_submit(&session_info->event_setinput, input->config[i].next_buff_idx, dst_mva);
	}

	primary_display_config_input_multiple(input);

	return 0;

}

int _ioctl_set_input_buffer(unsigned long arg)
{
	int ret = 0;
	void __user *argp = (void __user *)arg;
	unsigned int session_id = 0;

	mutex_lock(&disp_session_lock);
	if (copy_from_user(&g_session_input, argp, sizeof(g_session_input)))
	{
		DISPMSG("[FB]: copy_from_user failed! line:%d \n", __LINE__);
		mutex_unlock(&disp_session_lock);		
		return -EFAULT;
	}

	session_id = g_session_input.session_id;

	disp_session_sync_info *session_info = disp_get_session_sync_info_for_debug(session_id);
	if(session_info)
	{
		dprec_start(&session_info->event_setinput, 0, g_session_input.config_layer_num);
	}

	DISPPR_FENCE("S+/%s%d/count%d\n", disp_session_mode_spy(session_id), DISP_SESSION_DEV(session_id), g_session_input.config_layer_num);
		
	if(DISP_SESSION_TYPE(session_id) == DISP_SESSION_PRIMARY)
	{
		ret = set_primary_buffer(&g_session_input);
	}
	else if(DISP_SESSION_TYPE(session_id) == DISP_SESSION_EXTERNAL)
	{
		ret = set_external_buffer(&g_session_input);
	}
	else if(DISP_SESSION_TYPE(session_id) == DISP_SESSION_MEMORY)
	{
		ret = set_memory_buffer(&g_session_input);
	}
	else
	{
		DISPERR("session type is wrong:0x%08x\n", session_id);
		mutex_unlock(&disp_session_lock);
		return -1;
	}

	if(session_info)
	{
		dprec_done(&session_info->event_setinput, 0, g_session_input.config_layer_num);
	}
	memset(&g_session_input, 0,sizeof(g_session_input));
	mutex_unlock(&disp_session_lock);	
	return ret;
}

static int _sync_convert_fb_layer_to_disp_output(unsigned int session_id, disp_output_config* src, disp_mem_output_config *dst, unsigned int dst_mva)
{
	unsigned int layerpitch = 0;
	unsigned int layerbpp = 0;

    dst->mode = _ovl2mem_out_cached_config.mode;
	switch (src->fmt)
	{
		case DISP_FORMAT_YUV422:
			dst->fmt = eYUY2;
			layerpitch = 2;
			layerbpp = 16;
			break;

		case DISP_FORMAT_RGB565:
			dst->fmt = eRGB565;
			layerpitch = 2;
			layerbpp = 16;
			break;

		case DISP_FORMAT_RGB888:
			dst->fmt = eRGB888;
			layerpitch = 3;
			layerbpp = 24;
			break;
			
		case DISP_FORMAT_BGR888:
			dst->fmt = eBGR888;
			layerpitch = 3;
			layerbpp = 24;
			break;
			//xuecheng, ??????
		case DISP_FORMAT_ARGB8888:
			dst->fmt = eARGB8888;
			layerpitch = 4;
			layerbpp = 32;
			break;

		case DISP_FORMAT_ABGR8888:
			//dst->fmt = eABGR8888;			
			dst->fmt = eRGBA8888;
			layerpitch = 4;
			layerbpp = 32;
			break;
		case DISP_FORMAT_RGBA8888:
			dst->fmt = eRGBA8888;
			layerpitch = 4;
			layerbpp = 32;
			break;

		case DISP_FORMAT_BGRA8888:
			//dst->fmt = eABGR8888;			
			dst->fmt = eBGRA8888;
			layerpitch = 4;
			layerbpp = 32;
			break;
		case DISP_FORMAT_XRGB8888:
			dst->fmt = eARGB8888;
			layerpitch = 4;
			layerbpp = 32;
			break;

		case DISP_FORMAT_XBGR8888:
			dst->fmt = eRGBA8888;
			layerpitch = 4;
			layerbpp = 32;
			break;

		case DISP_FORMAT_UYVY:
			dst->fmt = eUYVY;
			layerpitch = 2;
			layerbpp = 16;
			break;

        case DISP_FORMAT_YV12:
		    dst->fmt = eYV12;
		    layerpitch = 1;
			layerbpp = 8;
		    break;
		    
		default:
			DISPERR("Invalid color format: 0x%x\n", src->fmt);
			return -1;
	}

	dst->vaddr = (unsigned int)src->va;
	dst->security = src->security;
	dst->w = src->width;
	dst->h = src->height;
	
// set_overlay will not use fence+ion handle
#if defined (MTK_FB_ION_SUPPORT)
	if (src->pa != NULL) 
	{
		dst->addr = (unsigned int)src->pa;
	}
	else 
	{
		dst->addr = dst_mva;
	}
#else
		dst->addr = (unsigned int)src->pa;
#endif

	dst->buff_idx = src->buff_idx;
	dst->interface_idx = src->interface_idx;

#if 0 //#ifdef DISP_DISABLE_X_CHANNEL_ALPHA
	//set Alpha blending
	dst->alpha = 0xFF;
	if (DISP_FORMAT_ARGB8888 == src->fmt ||
	    DISP_FORMAT_ABGR8888 == src->fmt || 
	    DISP_FORMAT_RGBA8888 == src->fmt || 
	    DISP_FORMAT_BGRA8888 == src->fmt ||
	    src->buffer_source==DISP_BUFFER_ALPHA)
	{
			dst->aen = TRUE;
	}
	else
	{
			dst->aen = FALSE;
	}
#endif
	dst->x = src->x;
	dst->y = src->y;
	dst->pitch = src->pitch*layerpitch;
		
#if 0
	DISPMSG("_convert_fb_layer_to_disp_input():id=%u, en=%u, next_idx=%u, vaddr=0x%x, paddr=0x%x, src fmt=%s, dst fmt=%u, pitch=%u, xoff=%u, yoff=%u, w=%u, h=%u, aen=%u\n",
						dst->layer,
						dst->layer_en,
						dst->buff_idx,
						(unsigned int)(dst->vaddr),
						(unsigned int)(dst->addr),
						_disp_format_spy(src->src_fmt),
						_disp_format_spy(dst->fmt),
						dst->src_pitch,
						dst->src_x,
						dst->src_y,
						dst->src_w,
						dst->src_h,
						dst->aen);
#endif
}


int _ioctl_set_output_buffer(unsigned long arg)
{
	int ret = 0;
	void __user *argp = (void __user *)arg;
	disp_session_output_config session_output;
	unsigned int session_id = 0;
	unsigned int dst_mva = 0;
	unsigned int dst_va = 0;
	unsigned int dst_size = 0;
	
	if (copy_from_user(&session_output, argp, sizeof(session_output))) 
	{
		DISPMSG("[FB]: copy_from_user failed! line:%d \n", __LINE__);
		return -EFAULT;
	}
	
    session_id = session_output.session_id;
	
	disp_session_sync_info *session_info = disp_get_session_sync_info_for_debug(session_id);
	if(session_info)
	{
		dprec_start(&session_info->event_setoutput, session_output.config.buff_idx, 0);
	}
	else
	{
		DISPERR("can't get session_info for session_id:0x%08x\n", session_id);
	}
	
    DISPMSG(" _ioctl_set_output_buffer session = 0x%x, idx %x, seq %x\n",session_id, session_output.config.buff_idx , session_output.config.frm_sequence);
    if(DISP_SESSION_TYPE(session_id) == DISP_SESSION_PRIMARY)
    {
        disp_mem_output_config primary_output;
        memset((void*)&primary_output, 0, sizeof(primary_output));
        if(session_output.config.pa)
		{
			dst_mva = session_output.config.pa;
		}
		else
        {
            dst_mva = mtkfb_query_buf_mva(session_id, disp_sync_get_output_timeline_id(), (unsigned int)session_output.config.buff_idx);
        }

        _sync_convert_fb_layer_to_disp_output(session_output.session_id, &(session_output.config), &primary_output, dst_mva);
        primary_output.dirty = 1;
		if(primary_display_is_decouple_mode())
		{
			// xuecheng, UGLY WORKAROUND!!! should use a common struct for input/output cache info
			disp_input_config src;
			memset((void*)&src, 0, sizeof(disp_input_config));
			src.layer_id = disp_sync_get_output_interface_timeline_id();
			src.layer_enable = 1;
			src.next_buff_idx = session_output.config.interface_idx;
			
			disp_sync_put_cached_layer_info(session_id, disp_sync_get_output_interface_timeline_id(), &src, dst_mva);

			memset((void*)&src, 0, sizeof(disp_input_config));
			src.layer_id = disp_sync_get_output_timeline_id();
			src.layer_enable = 1;
			src.next_buff_idx = session_output.config.buff_idx;
			
			disp_sync_put_cached_layer_info(session_id, disp_sync_get_output_timeline_id(), &src, dst_mva);
		}
		
        	DISPPR_FENCE("S+O/%s%d/L%d(id%d)/L%d(id%d)/%dx%d(%d,%d)/%s/%d/0x%08x/mva0x%08x\n", 
					disp_session_mode_spy(session_id), DISP_SESSION_DEV(session_id), 
					disp_sync_get_output_timeline_id(),
					session_output.config.buff_idx,
					disp_sync_get_output_interface_timeline_id(),
					session_output.config.interface_idx,
					session_output.config.width, 
					session_output.config.height, 
					session_output.config.x, 
					session_output.config.y,
					_disp_format_spy(session_output.config.fmt),
					session_output.config.pitch,
					session_output.config.pitchUV,
					dst_mva);
					
        memcpy(&_ovl2mem_out_cached_config, &primary_output, sizeof(primary_output));
        primary_display_config_output(&primary_output) ; 

		mtkfb_update_buf_info(session_output.session_id, disp_sync_get_output_interface_timeline_id(),
	                                session_output.config.buff_idx, 0, session_output.config.frm_sequence);
        if(session_info)
		{
			dprec_submit(&session_info->event_setoutput, session_output.config.buff_idx, dst_mva);
		}
        DISPMSG("_ioctl_set_output_buffer done idx 0x%x, mva %x, fmt %x, w %x, h %x (%x %x), p %x\n",
                                    session_output.config.buff_idx,
	                                dst_mva, session_output.config.fmt, session_output.config.width,
	                                session_output.config.height, session_output.config.x,
	                                session_output.config.y,session_output.config.pitch);
        
    }   
    else if(DISP_SESSION_TYPE(session_id) == DISP_SESSION_MEMORY)
    {
        ovl2mem_out_config primary_output;
        memset((void*)&primary_output, 0, sizeof(primary_output));
        if(session_output.config.pa)
		{
			dst_mva = session_output.config.pa;
		}
		else
        {
            dst_mva = mtkfb_query_buf_mva(session_id, disp_sync_get_output_timeline_id()  , (unsigned int)session_output.config.buff_idx);
        }

        mtkfb_update_buf_ticket(session_id, disp_sync_get_output_timeline_id(), session_output.config.buff_idx, get_ovl2mem_ticket());
        _sync_convert_fb_layer_to_disp_output(session_output.session_id, &(session_output.config), &primary_output, dst_mva);
        primary_output.dirty = 1;

        DISPPR_FENCE("S+/%s%d/L%d/id%d/%dx%d(%d,%d)/%s/%d/%d/0x%08x/mva0x%08x/t%d\n", 
					disp_session_mode_spy(session_id), DISP_SESSION_DEV(session_id), 
					4,
					session_output.config.buff_idx,
					session_output.config.width, 
					session_output.config.height, 
					session_output.config.x, 
					session_output.config.y,
					_disp_format_spy(session_output.config.fmt),
					session_output.config.pitch,
					session_output.config.pitchUV,
					session_output.config.pa,
					dst_mva, get_ovl2mem_ticket());

		if(dst_mva)
        {
            ovl2mem_output_config(&primary_output) ; 
            memcpy(&_ovl2mem_out_cached_config, &primary_output, sizeof(primary_output));
        }
        else
            DISPERR("error buffer idx 0x%x\n",session_output.config.buff_idx);

		mtkfb_update_buf_info(session_output.session_id, disp_sync_get_output_timeline_id(),
	                                session_output.config.buff_idx, 0, session_output.config.frm_sequence);
	                                
        if(session_info)
		{
			dprec_submit(&session_info->event_setoutput, session_output.config.buff_idx, dst_mva);
		}		
		DISPMSG("_ioctl_set_output_buffer done idx 0x%x, mva %x, fmt %x, w %x, h %x, p %x\n",
                                    session_output.config.buff_idx,
	                                dst_mva, session_output.config.fmt, session_output.config.width,
	                                session_output.config.height, session_output.config.pitch);
    }
	
	if(session_info)
	{
		dprec_done(&session_info->event_setoutput, session_output.config.buff_idx, 0);
	}

	return ret;
}


int _ioctl_get_info(unsigned long arg)
{
	int ret = 0;
	void __user *argp = (void __user *)arg;
	disp_session_info info;
	int dev = 0;
	unsigned int session_id = 0;

	if (copy_from_user(&info, argp, sizeof(info)))
	{
		DISPMSG("[FB]: copy_from_user failed! line:%d \n", __LINE__);
		return -EFAULT;
	}

	session_id = info.session_id;

	if(DISP_SESSION_TYPE(session_id) == DISP_SESSION_PRIMARY)
	{
		primary_display_get_info(&info);
	}
	else if(DISP_SESSION_TYPE(session_id) == DISP_SESSION_EXTERNAL)
	{
#ifdef MTK_HDMI_SUPPORT
		// this is for session test
        _get_ext_disp_info(&info);
#endif
	}
	else if(DISP_SESSION_TYPE(session_id) == DISP_SESSION_MEMORY)
	{
        ovl2mem_get_info(&info);
	}
	else
	{
		DISPERR("session type is wrong:0x%08x\n", session_id);
		return -1;
	}


	if (copy_to_user(argp, &info, sizeof(info)))
	{
		DISPMSG("[FB]: copy_to_user failed! line:%d \n", __LINE__);
		ret = -EFAULT;
	}

	return (ret);
}

int _ioctl_get_is_driver_suspend(unsigned long arg)
{
	int ret = 0;
	void __user *argp = (void __user *)arg;
	unsigned int is_suspend = 0;

    is_suspend = primary_display_is_sleepd();
    DISPMSG("ioctl_get_is_driver_suspend, is_suspend=%d\n", is_suspend);    
	if (copy_to_user(argp, &is_suspend, sizeof(int)))
	{
		DISPMSG("[FB]: copy_to_user failed! line:%d \n", __LINE__);
		ret = -EFAULT;
	}

	return (ret);
}

int _ioctl_wait_vsync(unsigned long arg)
{
	int ret = 0;
	void __user *argp = (void __user *)arg;
	disp_session_vsync_config vsync_config;
	int dev = 0;

	if (copy_from_user(&vsync_config, argp, sizeof(vsync_config)))
	{
		DISPMSG("[FB]: copy_from_user failed! line:%d \n", __LINE__);
		return -EFAULT;
	}
	
	disp_session_sync_info *session_info = disp_get_session_sync_info_for_debug(vsync_config.session_id);
	if(session_info)
	{
		dprec_start(&session_info->event_waitvsync, 0, 0);
	}
	
	ret = primary_display_wait_for_vsync(&vsync_config);

	if(session_info)
	{
		dprec_done(&session_info->event_waitvsync, 0, 0);
	}

	return (ret);
}

int _ioctl_set_vsync(unsigned long arg)
{
	int ret = 0;
	void __user *argp = (void __user *)arg;
	unsigned int fps = 0;
	int dev = 0;

	if (copy_from_user(&fps, argp, sizeof(unsigned int)))
	{
		DISPMSG("[FB]: copy_from_user failed! line:%d \n", __LINE__);
		return -EFAULT;
	}

	ret = primary_display_force_set_vsync_fps(fps);

	return (ret);
}

int _ioctl_set_session_mode(unsigned long arg)
{
    int ret = 0;
    int i = 0;
    int session_id = 0;
    void __user *argp = (void __user *)arg;
    disp_session_config config_info;
    int dev = 0;
    
    if (copy_from_user(&config_info, argp, sizeof(disp_session_config))) 
    {
        DISPMSG("[FB]: copy_from_user failed! line:%d \n", __LINE__);
        return -EFAULT;
    }

    primary_display_switch_mode(config_info.mode, config_info.session_id);
    if(config_info.mode == DISP_SESSION_DIRECT_LINK_MIRROR_MODE || config_info.mode == DISP_SESSION_DECOUPLE_MIRROR_MODE)
    {
        _ovl2mem_out_cached_config.mode = config_info.mode;
    }
    else
    {
        //if(_ovl2mem_out_cached_config.mode > 0)
        //{
        //    mtkfb_release_layer_fence(config_info.session_id, DDP_OUTPUT_LAYID);
        //}
		_ovl2mem_out_cached_config.mode = 0;
    }

    if (path_info.switching == 1)
    {
        return ret;
    }

    for (i = 0; i < MAX_SESSION_COUNT; i++)
    {
        if (session_config[i] != 0 && DISP_SESSION_TYPE(session_config[i]) != DISP_SESSION_PRIMARY)
        {
            session_id = session_config[i];
            break;
        }
    }
    
    if ((config_info.mode != path_info.old_mode) || 
		(session_id > 0 && path_info.old_session == DISP_SESSION_PRIMARY) ||
		(session_id == 0 && path_info.old_session != DISP_SESSION_PRIMARY))	
    {
        DISPMSG("_ioctl_set_session_mode mode, switch mode in \n"); 
        if (path_info.switching == 0)
        {
            int disp_type = 0;
#ifdef MTK_HDMI_SUPPORT
            disp_type = extd_get_device_type();
#endif
#ifdef OVL_CASCADE_SUPPORT 
            if (session_id > 0 && (config_info.mode < DISP_SESSION_DIRECT_LINK_MIRROR_MODE || disp_type == DISP_IF_HDMI_SMARTBOOK))
            {

                if (ovl_get_status() == DDP_OVL1_STATUS_PRIMARY && primary_display_is_sleepd()==0) 
                {
                    ovl_set_status(DDP_OVL1_STATUS_SUB_REQUESTING);
                }
            }
#ifdef MTK_HDMI_SUPPORT
            if (session_id > 0 && path_info.old_session == DISP_SESSION_EXTERNAL 
            	&& config_info.mode >= DISP_SESSION_DIRECT_LINK_MIRROR_MODE)
            {
                hdmi_set_layer_num(1);              
            }
#endif
#endif
            
            DISPMSG("_ioctl_set_session_mode mode, wake up \n"); 
            path_info.switching = 1;
            path_info.cur_mode = config_info.mode;
            path_info.ext_sid  = session_id;
            atomic_set(&switch_mode_event, 1);
            wake_up_interruptible(&switch_mode_wq);
        }
    }
	
    return (ret);
}

static int wait_ovl1_available()
{
    int ret = 1;
    int loop_cnt = 0;
#ifdef OVL_CASCADE_SUPPORT 
    if (ovl_get_status() == DDP_OVL1_STATUS_PRIMARY) 
    {
        if (primary_display_is_sleepd() == 0)
        {
            ovl_set_status(DDP_OVL1_STATUS_SUB_REQUESTING);
        }
        else
        {
            // directly split OVL1 from primary
            unsigned int dp_handle = 0;
            unsigned int cmdq_handle = 0;
            dpmgr_path_get_handle(&dp_handle, &cmdq_handle);
            if(dp_handle != 0)
            {
                DISPMSG("split OVL1 in suspen mode\n");
                dpmgr_path_power_on((disp_path_handle)(dp_handle), 0);
                g_enable_clock = 1;
                dpmgr_path_disable_cascade((disp_path_handle)(dp_handle), 0);
                ovl_set_status(DDP_OVL1_STATUS_SUB);
            }	            
        }
    }
	
    while ((ovl_get_status() != DDP_OVL1_STATUS_SUB) && (ovl_get_status() != DDP_OVL1_STATUS_IDLE) && (loop_cnt++ < 10))
    {
        DISPMSG("cascade switch: 2.ovl1 status=%d\n", ovl_get_status());
	
        // wait untile primary release OVL1
        ret = wait_event_interruptible_timeout(ovl1_wait_queue, (ovl_get_status() == DDP_OVL1_STATUS_SUB || ovl_get_status() == DDP_OVL1_STATUS_IDLE), HZ/10);
        if(ret < 0)
        {
            DISPERR("wait OVL1 interrupted by other ret=%d\n", ret);						 
        }
        else if(ret == 0)
        {
            DISPERR("wait OVL1 timeout! ret=0! \n");				
        }
        else
        {
            DISPMSG("wait OVL1 done! \n");
        }
    }
		
    if (ovl_get_status() == DDP_OVL1_STATUS_IDLE) 
    {
        ovl_set_status(DDP_OVL1_STATUS_SUB);
        ret = 1;
    }
#endif

    return ret;
}

static int create_external_display_path(int session, int mode)
{
    int ret = 0;
    int loop_cnt = 0;
    int extd_type = DISP_IF_MHL;
    
    if (DISP_SESSION_TYPE(session) == DISP_SESSION_MEMORY)
    {
        if (mode < DISP_SESSION_DIRECT_LINK_MIRROR_MODE)
        {
            if (wait_ovl1_available() > 0)
            {
                ovl2mem_init(session);
                ovl2mem_setlayernum(4);
            }
            else
            {
                DISPERR("mhl path: OVL1 can not be split out! \n");
            }
        }
    }
    else if (DISP_SESSION_TYPE(session) == DISP_SESSION_EXTERNAL)
    {
#ifdef MTK_HDMI_SUPPORT
        disp_path_handle dp_handle = 0;
        cmdqRecHandle pHandle = 0;
        extd_get_path_handle(&dp_handle, &pHandle);
        extd_type = extd_get_device_type();
        DISPMSG("external device type is:%d \n", extd_type);

        if (path_info.old_session == DISP_SESSION_EXTERNAL)
        {
            extd_hdmi_path_resume();
        }

        if (mode < DISP_SESSION_DIRECT_LINK_MIRROR_MODE || extd_type == DISP_IF_HDMI_SMARTBOOK)
        {
            if (wait_ovl1_available() > 0)
            {
                if (path_info.old_session == DISP_SESSION_EXTERNAL && extd_type != DISP_IF_HDMI_SMARTBOOK)
                {
                    if (extd_hdmi_path_get_mode() != EXTD_DIRECT_LINK_MODE)
                    {
                        dpmgr_insert_ovl1_sub(dp_handle, pHandle);
                        extd_hdmi_path_set_mode(EXTD_DIRECT_LINK_MODE);
                    }
                }
                else
                {
                    extd_hdmi_path_init(EXTD_DIRECT_LINK_MODE, session);
                }
					
                hdmi_set_layer_num(4);
            }
            else
            {
                DISPMSG("mhl path: OVL1 can not be split out! \n");
                extd_hdmi_path_init(EXTD_RDMA_DPI_MODE, session);
                hdmi_set_layer_num(1);
            }
        }
        else
        {
            if (path_info.old_session == DISP_SESSION_EXTERNAL)
            {
                dpmgr_remove_ovl1_sub(dp_handle, pHandle);
                extd_hdmi_path_set_mode(EXTD_RDMA_DPI_MODE);
                hdmi_set_layer_num(1);
                ovl1_remove = 1;
            }
            else
            {
                extd_hdmi_path_init(EXTD_RDMA_DPI_MODE, session);
                hdmi_set_layer_num(1);
            }
       }
#endif 
    }

    return ret; 
}

static void destroy_external_display_path(int session, int mode)
{
    if ((path_info.old_session == DISP_SESSION_PRIMARY) ||
		(path_info.old_session == DISP_SESSION_MEMORY && path_info.old_mode >= DISP_SESSION_DIRECT_LINK_MIRROR_MODE))
    {
        return;
    }

    if (path_info.old_session == DISP_SESSION_EXTERNAL)
    {
#ifdef MTK_HDMI_SUPPORT
#ifdef OVL_CASCADE_SUPPORT 
        disp_path_handle dp_handle;
        cmdqRecHandle pHandle;
        int ori_mode = extd_get_path_handle(&dp_handle, &pHandle);
        if (ovl_get_status() == DDP_OVL1_STATUS_SUB && ori_mode == EXTD_RDMA_DPI_MODE) 
        {
            extd_hdmi_path_set_mode(EXTD_RDMA_DPI_MODE);
            ovl_set_status(DDP_OVL1_STATUS_IDLE);
        }
#endif		
        extd_hdmi_path_deinit();
        //hdmi_set_layer_num(1);
#ifdef OVL_CASCADE_SUPPORT 
        if (extd_hdmi_path_get_mode() == EXTD_DIRECT_LINK_MODE)
        {
            ovl_set_status(DDP_OVL1_STATUS_IDLE);
        }     
#endif

        release_session_buffer(DISP_SESSION_EXTERNAL, 0xFF, NULL);
#endif 
    }
    else if(path_info.old_session == DISP_SESSION_MEMORY)
    {

        ovl2mem_deinit();
        ovl2mem_setlayernum(0);
#ifdef OVL_CASCADE_SUPPORT 
        ovl_set_status(DDP_OVL1_STATUS_IDLE);
        
        if(g_enable_clock == 1)
        {
		        unsigned int dp_handle = 0;
		        unsigned int cmdq_handle = 0;
		        dpmgr_path_get_handle(&dp_handle, &cmdq_handle);
		        if(dp_handle!=0)
		        {
			          dpmgr_path_power_off((disp_path_handle)(dp_handle), 0);
			          g_enable_clock = 0;
		        }
       }
#endif

        release_session_buffer(DISP_SESSION_MEMORY, 0xFF, NULL);
    }	 

    return;
}

static int disp_switch_mode_kthread(void *data)
{
    struct sched_param param = { .sched_priority = RTPM_PRIO_SCRN_UPDATE };
    sched_setscheduler(current, SCHED_RR, &param);
    
    int ret = 0;
    DISPMSG("disp_switch_mode_kthread in! \n");
    
    for (;;)
    {
        wait_event_interruptible(switch_mode_wq, atomic_read(&switch_mode_event));
        atomic_set(&switch_mode_event, 0);

        DISPPR_FENCE("disp_switch_mode_kthread, need switch mode, mode:%d, session:0x%x\n", path_info.cur_mode, path_info.ext_sid);

        if (path_info.ext_sid > 0)
        {
            ret = create_external_display_path(path_info.ext_sid, path_info.cur_mode);
            if (ret == 0)
            {
                path_info.old_session = DISP_SESSION_TYPE(path_info.ext_sid);
                path_info.old_mode    = path_info.cur_mode;
            }
        }
        else
        {
            destroy_external_display_path(path_info.ext_sid, DISP_SESSION_DIRECT_LINK_MODE);
            path_info.old_session = DISP_SESSION_PRIMARY;
            path_info.old_mode    = DISP_SESSION_DIRECT_LINK_MODE;
        }

        path_info.switching = 0;
        path_info.ext_sid   = 0;
        if (kthread_should_stop())
        {
            break;
        }
    }

    return 0;
}

const char* _session_ioctl_spy(unsigned int cmd)
{
	switch(cmd)
	{
		case DISP_IOCTL_CREATE_SESSION:
		{
			return "DISP_IOCTL_CREATE_SESSION";
		}
		case DISP_IOCTL_DESTROY_SESSION:
		{
			return "DISP_IOCTL_DESTROY_SESSION";
		}
		case DISP_IOCTL_TRIGGER_SESSION:
		{
			return "DISP_IOCTL_TRIGGER_SESSION";
		}
		case DISP_IOCTL_SET_INPUT_BUFFER:
		{
			return "DISP_IOCTL_SET_INPUT_BUFFER";
		}
		case DISP_IOCTL_PREPARE_INPUT_BUFFER:
		{
			return "DISP_IOCTL_PREPARE_INPUT_BUFFER";
		}
		case DISP_IOCTL_WAIT_FOR_VSYNC:
        	{
            		return "DISP_IOCL_WAIT_FOR_VSYNC";
		}
		case DISP_IOCTL_GET_SESSION_INFO:
			return "DISP_IOCTL_GET_SESSION_INFO";
		case DISP_IOCTL_AAL_EVENTCTL:
			return "DISP_IOCTL_AAL_EVENTCTL";
	        case DISP_IOCTL_AAL_GET_HIST:
				return "DISP_IOCTL_AAL_GET_HIST";
	        case DISP_IOCTL_AAL_INIT_REG:
				return "DISP_IOCTL_AAL_INIT_REG";
	        case DISP_IOCTL_AAL_SET_PARAM:
				return "DISP_IOCTL_AAL_SET_PARAM";
	        case DISP_IOCTL_SET_GAMMALUT:
				return "DISP_IOCTL_SET_GAMMALUT";
	        case DISP_IOCTL_SET_CCORR:
				return "DISP_IOCTL_SET_CCORR";
	        case DISP_IOCTL_SET_PQPARAM:
				return "DISP_IOCTL_SET_PQPARAM";
	        case DISP_IOCTL_GET_PQPARAM:
				return "DISP_IOCTL_GET_PQPARAM";
	        case DISP_IOCTL_SET_C1_PQPARAM:
				return "DISP_IOCTL_SET_C1_PQPARAM";
	        case DISP_IOCTL_GET_C1_PQPARAM:
				return "DISP_IOCTL_GET_C1_PQPARAM";
	        case DISP_IOCTL_SET_PQINDEX:
				return "DISP_IOCTL_SET_PQINDEX";
	        case DISP_IOCTL_SET_TDSHPINDEX:
				return "DISP_IOCTL_SET_TDSHPINDEX";
	        case DISP_IOCTL_GET_TDSHPINDEX:
				return "DISP_IOCTL_GET_TDSHPINDEX";
	        case DISP_IOCTL_SET_PQ_CAM_PARAM:
				return "DISP_IOCTL_SET_PQ_CAM_PARAM";
	        case DISP_IOCTL_GET_PQ_CAM_PARAM:
				return "DISP_IOCTL_GET_PQ_CAM_PARAM";
	        case DISP_IOCTL_SET_PQ_GAL_PARAM:
				return "DISP_IOCTL_SET_PQ_GAL_PARAM";
	        case DISP_IOCTL_GET_PQ_GAL_PARAM:
				return "DISP_IOCTL_GET_PQ_GAL_PARAM";
	        case DISP_IOCTL_OD_CTL:
				return "DISP_IOCTL_OD_CTL";
		default:
		{
			return "unknown";
		}
	}
}

long mtk_disp_mgr_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    	int ret = -1;

	//DISPMSG("mtk_disp_mgr_ioctl, cmd=%s, arg=0x%08x\n", _session_ioctl_spy(cmd), arg);

	switch(cmd)
	{
		case DISP_IOCTL_CREATE_SESSION:
		{
			return _ioctl_create_session(arg);
		}
		case DISP_IOCTL_DESTROY_SESSION:
		{
			return _ioctl_destroy_session(arg);
		}
		case DISP_IOCTL_TRIGGER_SESSION:
		{
			return _ioctl_trigger_session(arg);
		}
		case DISP_IOCTL_GET_PRESENT_FENCE:
		{
			return _ioctl_prepare_buffer(arg, PREPARE_PRESENT_FENCE);
		}
		case DISP_IOCTL_PREPARE_INPUT_BUFFER:
		{
			return _ioctl_prepare_buffer(arg, PREPARE_INPUT_FENCE);
		}    
		case DISP_IOCTL_SET_INPUT_BUFFER:
		{
			return _ioctl_set_input_buffer(arg);
		}
		case DISP_IOCTL_WAIT_FOR_VSYNC:
    		{
    			return _ioctl_wait_vsync(arg);
		}
		case DISP_IOCTL_GET_SESSION_INFO:
        	{
			return _ioctl_get_info(arg);
       		}
        case DISP_IOCTL_GET_IS_DRIVER_SUSPEND:
        	{
			return _ioctl_get_is_driver_suspend(arg);
       		}	
		case DISP_IOCTL_SET_VSYNC_FPS:
		{
			return _ioctl_set_vsync(arg);
		}
		case DISP_IOCTL_SET_SESSION_MODE:
		{
		    return _ioctl_set_session_mode(arg);
		}
		case DISP_IOCTL_PREPARE_OUTPUT_BUFFER:
		{
		    return _ioctl_prepare_buffer(arg, PREPARE_OUTPUT_FENCE);
		}
        	case DISP_IOCTL_SET_OUTPUT_BUFFER:
		{
		    return  _ioctl_set_output_buffer(arg);
		}
		case DISP_IOCTL_GET_LCMINDEX:
		{
			return primary_display_get_lcm_index();
		}
	        case DISP_IOCTL_AAL_EVENTCTL:
	        case DISP_IOCTL_AAL_GET_HIST:
	        case DISP_IOCTL_AAL_INIT_REG:
	        case DISP_IOCTL_AAL_SET_PARAM:
	        case DISP_IOCTL_SET_GAMMALUT:
	        case DISP_IOCTL_SET_CCORR:
	        case DISP_IOCTL_SET_PQPARAM:
	        case DISP_IOCTL_GET_PQPARAM:
	        case DISP_IOCTL_SET_C1_PQPARAM:
	        case DISP_IOCTL_GET_C1_PQPARAM:
	        case DISP_IOCTL_SET_PQINDEX:
	        case DISP_IOCTL_SET_TDSHPINDEX:
	        case DISP_IOCTL_GET_TDSHPINDEX:
	        case DISP_IOCTL_SET_PQ_CAM_PARAM:
	        case DISP_IOCTL_GET_PQ_CAM_PARAM:
	        case DISP_IOCTL_SET_PQ_GAL_PARAM:
	        case DISP_IOCTL_GET_PQ_GAL_PARAM:
	        case DISP_IOCTL_PQ_SET_BYPASS_COLOR:
	        case DISP_IOCTL_PQ_SET_WINDOW:
	        case DISP_IOCTL_OD_CTL:
	        case DISP_IOCTL_WRITE_REG:
	        case DISP_IOCTL_READ_REG:
	        case DISP_IOCTL_MUTEX_CONTROL:
	        case DISP_IOCTL_PQ_GET_TDSHP_FLAG:
	        case DISP_IOCTL_PQ_SET_TDSHP_FLAG:
	        case DISP_IOCTL_PQ_GET_DC_PARAM:
	        case DISP_IOCTL_PQ_SET_DC_PARAM:
            case DISP_IOCTL_WRITE_SW_REG:
            case DISP_IOCTL_READ_SW_REG:
        	{
            		ret = primary_display_user_cmd(cmd,arg);
            		break;
        	}
		default:
		{
			DISPERR("[session]ioctl not supported, 0x%08x\n", cmd);
		}
	}

    	return ret;
}

#ifdef CONFIG_COMPAT
static long mtk_disp_mgr_compat_ioctl(struct file *file, unsigned int cmd,  unsigned long arg)
{
	long ret = -ENOIOCTLCMD;

	switch(cmd) {	
	
    // add cases here for 32bit/64bit conversion
    // ...
    
	default:
		return mtk_disp_mgr_ioctl(file,  cmd,  arg);
	}
	
	return ret;
}
#endif


static struct file_operations mtk_disp_mgr_fops =
{
	.owner          		= THIS_MODULE,
    .mmap                   = mtk_disp_mgr_mmap,
	.unlocked_ioctl 	    = mtk_disp_mgr_ioctl,
#ifdef CONFIG_COMPAT
	.unlocked_ioctl         = mtk_disp_mgr_compat_ioctl,
#endif		
	.open           		= mtk_disp_mgr_open,
	.release        		= mtk_disp_mgr_release,
	.flush          		= mtk_disp_mgr_flush,
	.read           		= mtk_disp_mgr_read,
};

static int mtk_disp_mgr_probe(struct platform_device *pdev)
{
	struct class_device;
	struct class_device *class_dev = NULL;

    printk("mtk_disp_mgr_probe called! \n");
	
	if (alloc_chrdev_region(&mtk_disp_mgr_devno, 0, 1, DISP_SESSION_DEVICE))
	{
	    	return -EFAULT;
	}

	mtk_disp_mgr_cdev = cdev_alloc();
	mtk_disp_mgr_cdev->owner = THIS_MODULE;
	mtk_disp_mgr_cdev->ops = &mtk_disp_mgr_fops;

	cdev_add(mtk_disp_mgr_cdev, mtk_disp_mgr_devno, 1);

	mtk_disp_mgr_class = class_create(THIS_MODULE, DISP_SESSION_DEVICE);
	class_dev = (struct class_device *)device_create(mtk_disp_mgr_class, NULL, mtk_disp_mgr_devno, NULL, DISP_SESSION_DEVICE);
	disp_sync_init();

	path_info.old_mode = DISP_SESSION_DIRECT_LINK_MODE;
	path_info.old_session = DISP_SESSION_PRIMARY;
	path_info.switching   = 0;
	path_info.ext_sid     = 0;
	init_waitqueue_head(&ovl1_wait_queue);
	init_waitqueue_head(&switch_mode_wq);
	disp_switch_mode_task = kthread_create(disp_switch_mode_kthread, NULL, "disp_switch_mode_kthread");
	wake_up_process(disp_switch_mode_task);
	
	return 0;
}

static int mtk_disp_mgr_remove(struct platform_device *pdev)
{
    	return 0;
}

static void mtk_disp_mgr_shutdown(struct platform_device *pdev)
{
	return;
}

static int mtk_disp_mgr_suspend(struct platform_device *pdev, pm_message_t mesg)
{
    	return 0;
}

static int mtk_disp_mgr_resume(struct platform_device *pdev)
{
    	return 0;
}


static struct platform_driver mtk_disp_mgr_driver =
{
	.probe		= mtk_disp_mgr_probe,
	.remove		= mtk_disp_mgr_remove,
	.shutdown	= mtk_disp_mgr_shutdown,
	.suspend		= mtk_disp_mgr_suspend,
	.resume		= mtk_disp_mgr_resume,
	.driver     		=
   	{
        	.name = DISP_SESSION_DEVICE,
    	},
};

static void mtk_disp_mgr_device_release(struct device *dev)
{

}

static u64 mtk_disp_mgr_dmamask = ~(u32)0;

static struct platform_device mtk_disp_mgr_device = {
	.name	= DISP_SESSION_DEVICE,
	.id      	= 0,
	.dev     	=
	{
        	.release = mtk_disp_mgr_device_release,
        	.dma_mask = &mtk_disp_mgr_dmamask,
        	.coherent_dma_mask = 0xffffffff,
    	},
    	.num_resources = 0,
};

static int __init mtk_disp_mgr_init(void)
{
    	if(platform_device_register(&mtk_disp_mgr_device))
    	{
        	return -ENODEV;
    	}

    	if(platform_driver_register(&mtk_disp_mgr_driver))
    	{
        	platform_device_unregister(&mtk_disp_mgr_device);
        	return -ENODEV;
    	}


    	return 0;
}

static void __exit mtk_disp_mgr_exit(void)
{
	cdev_del(mtk_disp_mgr_cdev);
	unregister_chrdev_region(mtk_disp_mgr_devno, 1);

	platform_driver_unregister(&mtk_disp_mgr_driver);
	platform_device_unregister(&mtk_disp_mgr_device);

	device_destroy(mtk_disp_mgr_class, mtk_disp_mgr_devno);
	class_destroy(mtk_disp_mgr_class);
}

module_init(mtk_disp_mgr_init);
module_exit(mtk_disp_mgr_exit);

MODULE_DESCRIPTION("MediaTek Display Manager");
