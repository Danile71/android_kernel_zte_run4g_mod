#define LOG_TAG "display_session_test"

#include <sys/mman.h>
#include <dlfcn.h>
#include <cutils/xlog.h>
#include <cutils/atomic.h>
#include <hardware/hardware.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <sched.h>
#include <sys/resource.h>
#include <linux/fb.h>
#include <wchar.h>
#include <pthread.h>
#include <linux/mmprofile.h>
#include <linux/ion.h>
#include <linux/ion_drv.h>
#include <ion/ion.h>
#include <unistd.h>
#include "mtkfb.h"
#include <sync/sync.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/disp_session.h>

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

#include <gui/ISurfaceComposer.h>
#include <gui/SurfaceComposerClient.h>

#include <pthread.h>
#if defined(HAVE_AEE_FEATURE)
#include "../aee/binary/inc/aee.h"
#endif

#include <SkImageDecoder.h>
#include <SkBitmap.h>

using namespace android;



//#pragma GCC optimize ("O0")

typedef struct {
    int ion_fd;
    int ion_shardfd;
    struct ion_handle * handle;
    void* buffer ;
    unsigned int size;
    int fence_fd;
    int fence_idx;

}data_buffer_t;
//unsigned int bufsize=1024*1024*8+256;

static volatile char* inputBuffer;
static volatile char* outputBuffer;
//static struct fb_overlay_buffer overlaybuffer[4];
static disp_session_input_config session_input_config;
static disp_input_config fblayer[4];
//static int ion_fd[4];
static int display_fd;
#define LAYER_NUM 4
#define BUFFER_NUM 2
static data_buffer_t layerbuffer[LAYER_NUM][BUFFER_NUM];
#define  TIMEOUT_NEVER (-1)
static int layerindex[LAYER_NUM] ={0};
static int fbsize=0;

static int session_init_info(int width, int height)
{
	int i =0;
 	int layer = 0;
	void *inputBuffer = NULL;
	fbsize = width * height * 4;
	printf("width=%d, height=%d, fbsize=%d\n", width, height, fbsize);
	for(layer = 0;layer<LAYER_NUM;layer++)
	{
		for(i=0;i<BUFFER_NUM;i++)
	    	{ 
			int ion_fd = mt_ion_open("disp_session_test");
			unsigned int j =0;
			struct ion_handle* handle= NULL;
			int share_fd = -1;
			if (ion_fd < 0)
			{
				printf("Cannot open ion device\n");
				return 0;
			}

			if (ion_alloc_mm(ion_fd, fbsize, 4, 0, &handle))
			{
				printf("IOCTL[ION_IOC_ALLOC] failed!\n");
				return 0;
		        }

			if (ion_share(ion_fd, handle, &share_fd))
			{
			    	printf("IOCTL[ION_IOC_SHARE] failed!\n");
			    	return 0;
			}
			
			inputBuffer = ion_mmap(ion_fd, NULL, fbsize, PROT_READ|PROT_WRITE, MAP_SHARED, share_fd, 0);
			for (j=0; j< fbsize; j+=4)
			{
				#if 1
				//unsigned int value = (j%256) << (i*8);
				//if(i == 3) value = 0xffffff;
				unsigned int value = 0;
				#if 0
				unsigned int alpha = 0x22000000*layer;
				
				if(i==0) value = 0x00ffff;//blue
				else if(i==1) value = 0xff0000;//green
				else if(i==2) value = 0xffffff;//red
				else if(i==3) value = 0x000000;
				#endif
				
				if(layer == 0) value = i%2?0xff0000ff:0xffff0000;
				else if(layer == 1) value = i%2?0xffff0000:0xff00ff00;
				else if(layer == 2) value = i%2?0xff00ff00:0xff0000ff;
				else if(layer ==3) value = i%2?0xffffffff:0xff404040;

				*(volatile unsigned int*)(inputBuffer+j) = value;
				#endif
			}
			
			layerbuffer[layer][i].ion_fd = ion_fd;
			layerbuffer[layer][i].handle = handle;
			layerbuffer[layer][i].ion_shardfd = share_fd;
			layerbuffer[layer][i].buffer = inputBuffer;
			layerbuffer[layer][i].fence_fd = -1;
			layerbuffer[layer][i].fence_idx = 0;

			printf("layer=%d,index=%d ,ion_fd=%d\n", layer, i, ion_fd );
	    	}
	}

	return 1;
}

void releaseResource()
{
    	int i = 0;
	int layer = 0;
	for(layer=0;layer<LAYER_NUM;layer++)
	{
	    	for(i=0;i<BUFFER_NUM;i++)
	    	{ 
	        	ion_munmap(layerbuffer[layer][i].ion_fd, layerbuffer[layer][i].buffer , fbsize);
	        	ion_free(layerbuffer[layer][i].ion_fd, layerbuffer[layer][i].handle);
	        	ion_close(layerbuffer[layer][i].ion_fd);
			layerbuffer[layer][i].ion_fd = -1;
			printf("release resource, ion_fd=%d\n", layerbuffer[layer][i].ion_fd);
	    	}
	}
}

data_buffer_t * session_getfreebuffer(int layer_id)
{
	int index = layerindex[layer_id];
	if(layerbuffer[layer_id][index].fence_fd >= 0)
	{
	        int err = sync_wait(layerbuffer[layer_id][index].fence_fd, TIMEOUT_NEVER);
        	close(layerbuffer[layer_id][index].fence_fd);
        	layerbuffer[layer_id][index].fence_fd = -1;
    	}

	layerindex[layer_id]++;
	layerindex[layer_id] %= BUFFER_NUM;
    	//printf("get free buffer index=%d,ionfd=%d\n", index,layerbuffer[index].ion_fd);
    	return &(layerbuffer[layer_id][index]);
}

void session_prepare_input(int fd, unsigned int session_id, data_buffer_t * param, int layer_id, int layer_en)
{
	disp_buffer_info buffer;

	memset(&buffer, 0, sizeof(disp_buffer_info));
	buffer.session_id = session_id;
	buffer.layer_id   = layer_id;
	buffer.layer_en   = layer_en;
	buffer.ion_fd     = param->ion_shardfd;
	buffer.cache_sync = 1;
	ioctl(fd, DISP_IOCTL_PREPARE_INPUT_BUFFER, &buffer);
	param->fence_idx = buffer.index;
	param->fence_fd = buffer.fence_fd;
	//printf("prepare fence buffer idx=%d, fence_id=%d\n", param->fence_idx, param->fence_fd);
}

unsigned int session_prepare_output(int fd, unsigned int session_id, data_buffer_t * param, int layer_id, int layer_en)
{
	disp_buffer_info buffer;

	memset(&buffer, 0, sizeof(disp_buffer_info));
	buffer.session_id = session_id;
	buffer.layer_id   = layer_id;
	buffer.layer_en   = layer_en;
	buffer.ion_fd     = param->ion_shardfd;
	buffer.cache_sync = 1;
	ioctl(fd, DISP_IOCTL_PREPARE_OUTPUT_BUFFER, &buffer);
	param->fence_idx = buffer.index;
	param->fence_fd = buffer.fence_fd;

	return buffer.layer_id;
}


void session_setinput(int fd, disp_session_input_config *input_config, data_buffer_t * param, int width, int height, int layer_id, int layer_en)
{
	input_config->config_layer_num = 1;

	input_config->config[0].layer_id = layer_id;
	input_config->config[0].layer_enable = layer_en;
	input_config->config[0].src_base_addr = 0;
	input_config->config[0].src_phy_addr = 0;
	input_config->config[0].next_buff_idx = param->fence_idx;
	input_config->config[0].src_fmt = DISP_FORMAT_RGBA8888;
	input_config->config[0].src_pitch = width;
	input_config->config[0].src_offset_x = 0; 
	input_config->config[0].src_width = width;

	input_config->config[0].src_offset_y = 0;
	input_config->config[0].src_height = height;

	input_config->config[0].tgt_offset_x = 0;
	input_config->config[0].tgt_offset_y = 0;
	input_config->config[0].tgt_width = width;
	input_config->config[0].tgt_height = height;
	
	if(ioctl(fd, DISP_IOCTL_SET_INPUT_BUFFER, input_config) < 0)
    	{
       	 	printf("ioctl to set multi overlay layer enable fail\n");
    	}
}
static long int get_current_time_us(void)
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return (t.tv_sec & 0xFFF) * 1000000 + t.tv_usec;
}

#define TEST_RESULT(string, ret)	printf("Test Case: [%s] [%s]\n", string, (ret)?"PASS":"FAIL");


int test_case_0(int argc, char **argv)
{
	int ret = 0;
	int fd = 0;
	char dev_name[24];

	disp_session_config config;
	disp_session_info device_info;

	memset((void*)&config, 0, sizeof(disp_session_config));
	memset((void*)&device_info, 0, sizeof(disp_session_info));
	memset((void*)&session_input_config, 0, sizeof(disp_session_input_config));
	
	sprintf(dev_name, "/dev/%s", DISP_SESSION_DEVICE);
	printf("display session device is %s\n", dev_name);
	{
		fd = open(dev_name, O_RDWR);
		TEST_RESULT("OPEN DEVICE", (fd>0));
	}
	// blank SurfaceFlinger
	   sp<SurfaceComposerClient> client = new SurfaceComposerClient();
	   sp<IBinder> display = SurfaceComposerClient::getBuiltInDisplay(
			   ISurfaceComposer::eDisplayIdMain);
	   SurfaceComposerClient::blankDisplay(display);

	// create session
	{
		config.type = DISP_SESSION_PRIMARY;
		config.device_id = 0;
		ret = ioctl(fd, DISP_IOCTL_CREATE_SESSION, &config);
		TEST_RESULT("DISP_IOCTL_CREATE_SESSION", (ret==0));
		printf("create session for primary display, session_id = %d\n", config.session_id);
	}

	// get session info
	{
		device_info.session_id = config.session_id;
		ret = ioctl(fd, DISP_IOCTL_GET_SESSION_INFO, &device_info);
		TEST_RESULT("DISP_IOCTL_GET_SESSION_INFO", (ret==0));
		printf("get primary display information, session_id=0x%08x\n", config.session_id);	
		printf("-->maxLayerNum=%d\n", device_info.maxLayerNum);
		printf("-->isHwVsyncAvailable=%d\n", device_info.isHwVsyncAvailable);
		printf("-->displayType=%d\n", device_info.displayType);
		printf("-->displayWidth=%d\n", device_info.displayWidth);
		printf("-->displayHeight=%d\n", device_info.displayHeight);
		printf("-->displayFormat=%d\n", device_info.displayFormat);
		printf("-->displayMode=%d\n", device_info.displayMode);
		printf("-->vsyncFPS=%d\n", device_info.vsyncFPS);
		printf("-->physicalWidth=%d\n", device_info.physicalWidth);
		printf("-->physicalHeight=%d\n", device_info.physicalHeight);
		printf("-->isConnected=%d\n", device_info.isConnected);
	}

	// prepare input buffer
	{
		ret = session_init_info(device_info.displayWidth, device_info.displayHeight);
		TEST_RESULT("init info", (ret>0));
	}

	int loop = 1000;
	if(argc > 1)
	{
	    	sscanf(argv[1], "%d", &loop);
	}
	
	printf("loop count %d \n",loop);
	disp_session_vsync_config vsync_config;
	
	memset((void*)&vsync_config, 0, sizeof(disp_session_vsync_config));
	vsync_config.session_id = config.session_id;
	unsigned int vsync_cnt = 0;
	long int last_vsync_ts = 0;
	long int t = 0;
	data_buffer_t * buffer;
	while(loop)
    	{   			
		ret = ioctl(fd, DISP_IOCTL_WAIT_FOR_VSYNC, &vsync_config);
		if(ret)
		{
			TEST_RESULT("wait vsync", (ret==0));
		}
		
    		session_input_config.session_id = config.session_id;
        	buffer = session_getfreebuffer(0);
        	session_prepare_input(fd, config.session_id, buffer, 0, 1);
        	session_setinput(fd, &session_input_config, buffer, device_info.displayWidth, device_info.displayHeight, 0, 1);

		#if 1
        	buffer = session_getfreebuffer(1);
        	session_prepare_input(fd,config.session_id,  buffer, 1, 1);
		session_setinput(fd, &session_input_config, buffer, device_info.displayWidth, device_info.displayHeight, 1, 1);
		
        	buffer = session_getfreebuffer(2);
        	session_prepare_input(fd, config.session_id, buffer, 2, 1);
		session_setinput(fd, &session_input_config, buffer, device_info.displayWidth, device_info.displayHeight, 2, 1);

        	buffer = session_getfreebuffer(3);
        	session_prepare_input(fd, config.session_id, buffer, 3, 1);
		session_setinput(fd, &session_input_config, buffer, device_info.displayWidth, device_info.displayHeight, 3, 1);
		#endif
		
		ret = ioctl(fd, DISP_IOCTL_TRIGGER_SESSION, &config);
		if(ret)
		{
			TEST_RESULT("trigger session", (ret==0));
		}		
        	
        	loop--;
    	}
	
	buffer = session_getfreebuffer(3);
	session_prepare_input(fd, config.session_id, buffer, 3, 0);
	session_setinput(fd, &session_input_config, buffer, device_info.displayWidth, device_info.displayHeight, 3, 0);
	
	buffer = session_getfreebuffer(1);
	session_prepare_input(fd, config.session_id, buffer,1, 0);
	session_setinput(fd, &session_input_config, buffer, device_info.displayWidth, device_info.displayHeight, 1, 0);
	
	buffer = session_getfreebuffer(0);
	session_prepare_input(fd, config.session_id, buffer, 0, 0);
	session_setinput(fd, &session_input_config, buffer, device_info.displayWidth, device_info.displayHeight, 0, 0);
	ret = ioctl(fd, DISP_IOCTL_TRIGGER_SESSION, &config);

	// create session
	{
		config.type = DISP_SESSION_PRIMARY;
		config.device_id = 0;
		ret = ioctl(fd, DISP_IOCTL_DESTROY_SESSION, &config);
		TEST_RESULT("DISP_IOCTL_DESTROY_SESSION", (ret==0));
		printf("destroy session for primary display, session_id = %d\n", config.session_id);
	}
		
    	releaseResource();
		
		// unblank SurfaceFlinger
		SurfaceComposerClient::unblankDisplay(display);
		

    	return 0;
}

#define QUEUE_SIZE 128
unsigned int input_queue[QUEUE_SIZE];
unsigned int input_queue_cur = 0;
int input_queue_cnt = 0;

disp_session_input_config gsession_input_config;
pthread_mutex_t input_mutex;
disp_session_input_config *getinput(void)
{
	disp_session_input_config *p = (disp_session_input_config *)malloc(sizeof(disp_session_input_config));
	memset((void*)p, 0, sizeof(disp_session_input_config));
	//printf("get input:0x%08x\n", p);
	return p;
}

void queueinput(disp_session_input_config *p)
{
	if(p == NULL)
	{
		printf("%s, p is NULL\n", __func__);
		return;
	}

	if(input_queue_cnt>=QUEUE_SIZE)
	{
		printf("queue is full\n");
		return;
	}
	
	pthread_mutex_lock(&input_mutex);
	//printf("queue0:cur=%d,cnt=%d\n", input_queue_cur, input_queue_cnt);
	input_queue[input_queue_cur] = (unsigned int)p;
	input_queue_cur++;
	input_queue_cur %= QUEUE_SIZE;
	
	input_queue_cnt++;
	//printf("queue1:cur=%d,cnt=%d\n", input_queue_cur, input_queue_cnt);	
	pthread_mutex_unlock(&input_mutex);
}

disp_session_input_config *dequeueinput(void)
{
	disp_session_input_config *p = NULL;
	if(input_queue_cnt == 0)
	{
		return NULL;
	}
	
	pthread_mutex_lock(&input_mutex);
	{
		int i = 0;
		//printf("dequeue0:cur=%d,cnt=%d\n", input_queue_cur, input_queue_cnt);
		//for(i=0;i<input_queue_cnt;i++)
		//{
		//	printf("input %d = 0x%08x\n", i, input_queue[input_queue_cur-i]);
		//}
	}
	
	if(input_queue_cur >= input_queue_cnt)
		p = (disp_session_input_config*)input_queue[input_queue_cur - input_queue_cnt];
	else
		p = (disp_session_input_config*)input_queue[input_queue_cur + QUEUE_SIZE - input_queue_cnt];
	
	input_queue_cnt--;
	
	//printf("dequeue1:cur=%d,cnt=%d\n", input_queue_cur, input_queue_cnt);
	pthread_mutex_unlock(&input_mutex);

	return p;
}

void freeinput(disp_session_input_config *p)
{
	if(p)
	{
		free(p);
	}
}

void setinput(int fd, disp_session_input_config *input_config, data_buffer_t * param, int width, int height, int layer_id, int layer_en)
{
	input_config->config[input_config->config_layer_num].layer_id = layer_id;
	input_config->config[input_config->config_layer_num].layer_enable = layer_en;
	input_config->config[input_config->config_layer_num].src_base_addr = 0;
	input_config->config[input_config->config_layer_num].src_phy_addr = 0;
	input_config->config[input_config->config_layer_num].next_buff_idx = param->fence_idx;
	input_config->config[input_config->config_layer_num].src_fmt = DISP_FORMAT_RGBA8888;
	input_config->config[input_config->config_layer_num].src_pitch = width;
	input_config->config[input_config->config_layer_num].src_offset_x = 0; 
	input_config->config[input_config->config_layer_num].src_width = width;

	input_config->config[input_config->config_layer_num].src_offset_y = 0;
	input_config->config[input_config->config_layer_num].src_height = height;

	input_config->config[input_config->config_layer_num].tgt_offset_x = 0;
	input_config->config[input_config->config_layer_num].tgt_offset_y = 0;
	input_config->config[input_config->config_layer_num].tgt_width = width;
	input_config->config[input_config->config_layer_num].tgt_height = height;
	input_config->config[input_config->config_layer_num].alpha_enable = 1;
	input_config->config[input_config->config_layer_num].alpha = 0xff;
	
	input_config->config_layer_num ++;
}

void resetinput(void)
{	
	memset((void*)&gsession_input_config, 0, sizeof(disp_session_input_config));
}

data_buffer_t *alloc_buffer(int size)
{
	long int t = get_current_time_us();
	int ion_fd = mt_ion_open("disp_session_test");

	data_buffer_t *p = (data_buffer_t *)malloc(sizeof(data_buffer_t));
	if(p == NULL) 
	{
		return NULL;
	}

	memset((void*)p, 0, sizeof(data_buffer_t));

	unsigned int j =0;
	struct ion_handle* handle= NULL;
	int share_fd = -1;
	
	if (ion_fd < 0)
	{
		printf("Cannot open ion device\n");
		return 0;
	}

	if (ion_alloc_mm(ion_fd, size, 4, 0, &handle))
	{
		printf("IOCTL[ION_IOC_ALLOC] failed!\n");
		return 0;
        }

	if (ion_share(ion_fd, handle, &share_fd))
	{
	    	printf("IOCTL[ION_IOC_SHARE] failed!\n");
	    	return 0;
	}

	p->ion_fd = ion_fd;
	p->ion_shardfd = share_fd;
	p->handle = handle;
	p->size = size;
	p->buffer = ion_mmap(ion_fd, NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, share_fd, 0);
	memset(p->buffer, 0xff, size);
	//printf("alloc cost %dus\t", get_current_time_us()-t);
	return p;
}

void free_buffer(data_buffer_t *p)
{
	if(p)
	{
		ion_munmap(p->ion_fd, p->buffer , p->size);
		ion_free(p->ion_fd, p->handle);
		ion_share_close(p->ion_fd, p->ion_shardfd);
		ion_close(p->ion_fd);
		close(p->fence_fd);
		free(p);
	}
}

// single layer allocate/deallocate buffer test
int test_case_1(int argc, char **argv)
{
	int fd = -1;
	int ret = 0;
	int cnt = 1;
	data_buffer_t *p = NULL;
	data_buffer_t *p1 = NULL;

	disp_session_config config;
	memset((void*)&config, 0, sizeof(disp_session_config));

	disp_session_info device_info;
	memset((void*)&device_info, 0, sizeof(disp_session_info));

	disp_session_input_config session_input_config;
	memset((void*)&session_input_config, 0, sizeof(disp_session_input_config));

	disp_session_vsync_config vsync_config;
	memset((void*)&vsync_config, 0, sizeof(disp_session_vsync_config));

	fd = open("/dev/mtk_disp_mgr", O_RDWR);
	
	// blank SurfaceFlinger
	sp<SurfaceComposerClient> client = new SurfaceComposerClient();
	sp<IBinder> display = SurfaceComposerClient::getBuiltInDisplay(ISurfaceComposer::eDisplayIdMain);
	SurfaceComposerClient::blankDisplay(display);

	// create session
	config.type = DISP_SESSION_PRIMARY;
	config.device_id = 0;
	ret = ioctl(fd, DISP_IOCTL_CREATE_SESSION, &config);
	printf("create session for primary display, session_id = %d\n", config.session_id);

	device_info.session_id = config.session_id;

	ret = ioctl(fd, DISP_IOCTL_GET_SESSION_INFO, &device_info);

	#if 0
	session_input_config.session_id = config.session_id;
	p = alloc_buffer(device_info.displayWidth*device_info.displayHeight*4);
	session_setinput(fd, &session_input_config, p, device_info.displayWidth, device_info.displayHeight, 0,0);
	session_setinput(fd, &session_input_config, p, device_info.displayWidth, device_info.displayHeight, 1,0);
	session_setinput(fd, &session_input_config, p, device_info.displayWidth, device_info.displayHeight, 2,0);
	session_setinput(fd, &session_input_config, p, device_info.displayWidth, device_info.displayHeight, 3,0);
	free_buffer(p);
	#endif
	int loop = 1000;
	if(argc > 1)
	{
		sscanf(argv[1], "%d", &loop);
	}

	while(loop--)
	{
		ret = ioctl(fd, DISP_IOCTL_WAIT_FOR_VSYNC, &vsync_config);
		if(ret)
		{
			printf("wait vsync fail:%d\n", ret);
		}
		
		p = alloc_buffer(device_info.displayWidth*device_info.displayHeight*4);
		session_prepare_input(fd, config.session_id, p, 3, 1);
		printf("prepare %d|%d\t", p->fence_idx, p->fence_fd);
		vsync_config.session_id = config.session_id;
		
		session_input_config.session_id = config.session_id;
		if(p1)
			session_input_config.config_layer_num = 2;
		else
			session_input_config.config_layer_num = 1;
		
		session_input_config.config[0].layer_id = 3;
		session_input_config.config[0].layer_enable = 1;
		session_input_config.config[0].src_base_addr = 0;
		session_input_config.config[0].src_phy_addr = 0;
		session_input_config.config[0].next_buff_idx = p->fence_idx;
		session_input_config.config[0].src_fmt = DISP_FORMAT_RGBA8888;
		session_input_config.config[0].src_pitch = device_info.displayWidth;
		session_input_config.config[0].src_offset_x = 0; 
		session_input_config.config[0].src_width = device_info.displayWidth;
		session_input_config.config[0].src_offset_y = 0;
		session_input_config.config[0].src_height = device_info.displayHeight;
		session_input_config.config[0].tgt_offset_x = 0;
		session_input_config.config[0].tgt_offset_y = 0;
		session_input_config.config[0].tgt_width = device_info.displayWidth;
		session_input_config.config[0].tgt_height = device_info.displayHeight;

		if(p1)
		{
			session_input_config.config[1].layer_id = 2;
			session_input_config.config[1].layer_enable = 0;
			session_input_config.config[1].src_base_addr = 0;
			session_input_config.config[1].src_phy_addr = 0;
			session_input_config.config[1].next_buff_idx = p1->fence_idx;
			session_input_config.config[1].src_fmt = DISP_FORMAT_RGBA8888;
			session_input_config.config[1].src_pitch = device_info.displayWidth;
			session_input_config.config[1].src_offset_x = 0; 
			session_input_config.config[1].src_width = device_info.displayWidth;
			session_input_config.config[1].src_offset_y = 0;
			session_input_config.config[1].src_height = device_info.displayHeight;
			session_input_config.config[1].tgt_offset_x = 0;
			session_input_config.config[1].tgt_offset_y = 0;
			session_input_config.config[1].tgt_width = device_info.displayWidth;
			session_input_config.config[1].tgt_height = device_info.displayHeight;
		}
		
		if(ioctl(fd, DISP_IOCTL_SET_INPUT_BUFFER, &session_input_config) < 0)
		{
			printf("ioctl to set multi overlay layer enable fail\n");
		}
		
		ret = ioctl(fd, DISP_IOCTL_TRIGGER_SESSION, &config);
		if(ret)
		{
			printf("trigger fail:%d\n", ret);
		}
		
		if(p1)
		{
			ret = sync_wait(p1->fence_fd, TIMEOUT_NEVER);
			free_buffer(p1);
			printf("%d released\t", p1->fence_fd);
		}

//////////////////////////////////////////////////////////////////////		
		ret = ioctl(fd, DISP_IOCTL_WAIT_FOR_VSYNC, &vsync_config);
		if(ret)
		{
			printf("wait vsync fail:%d\n", ret);
		}

		p1 = alloc_buffer(device_info.displayWidth*device_info.displayHeight*4);
		session_prepare_input(fd, config.session_id, p1, 2, 1);
		printf("prepare %d|%d\t", p1->fence_idx, p1->fence_fd);

		session_input_config.config_layer_num = 2;
		
		session_input_config.config[0].layer_id = 3;
		session_input_config.config[0].layer_enable = 0;
		session_input_config.config[0].src_base_addr = 0;
		session_input_config.config[0].src_phy_addr = 0;
		session_input_config.config[0].next_buff_idx = p->fence_idx;
		session_input_config.config[0].src_fmt = DISP_FORMAT_RGBA8888;
		session_input_config.config[0].src_pitch = device_info.displayWidth;
		session_input_config.config[0].src_offset_x = 0; 
		session_input_config.config[0].src_width = device_info.displayWidth;
		
		session_input_config.config[0].src_offset_y = 0;
		session_input_config.config[0].src_height = device_info.displayHeight;
		
		session_input_config.config[0].tgt_offset_x = 0;
		session_input_config.config[0].tgt_offset_y = 0;
		session_input_config.config[0].tgt_width = device_info.displayWidth;
		session_input_config.config[0].tgt_height = device_info.displayHeight;
		
		session_input_config.config[1].layer_id = 2;
		session_input_config.config[1].layer_enable = 1;
		session_input_config.config[1].src_base_addr = 0;
		session_input_config.config[1].src_phy_addr = 0;
		session_input_config.config[1].next_buff_idx = p1->fence_idx;
		session_input_config.config[1].src_fmt = DISP_FORMAT_RGBA8888;
		session_input_config.config[1].src_pitch = device_info.displayWidth;
		session_input_config.config[1].src_offset_x = 0; 
		session_input_config.config[1].src_width = device_info.displayWidth;
		
		session_input_config.config[1].src_offset_y = 0;
		session_input_config.config[1].src_height = device_info.displayHeight;
		
		session_input_config.config[1].tgt_offset_x = 0;
		session_input_config.config[1].tgt_offset_y = 0;
		session_input_config.config[1].tgt_width = device_info.displayWidth;
		session_input_config.config[1].tgt_height = device_info.displayHeight;
		
		if(ioctl(fd, DISP_IOCTL_SET_INPUT_BUFFER, &session_input_config) < 0)
		{
			printf("ioctl to set multi overlay layer enable fail\n");
		}

		ret = ioctl(fd, DISP_IOCTL_TRIGGER_SESSION, &config);

		ret = sync_wait(p->fence_fd, TIMEOUT_NEVER);
		free_buffer(p);
		printf("%d released\n", p->fence_fd);
	}

	free_buffer(p1);
	
	SurfaceComposerClient::unblankDisplay(display);
	return 0;
}

void *trigger_loop_routine(void *name)
{
	unsigned int vsync_cnt = 0;
	int ret = -1;
	int fd = -1;
	disp_session_input_config *p = NULL;
	
	disp_session_config config;
	memset((void*)&config, 0, sizeof(disp_session_config));

	disp_session_vsync_config vsync_config;
	memset((void*)&vsync_config, 0, sizeof(disp_session_vsync_config));
	
	memset((void*)&gsession_input_config, 0, sizeof(disp_session_input_config));

	fd = open("/dev/mtk_disp_mgr", O_RDWR);
	
	config.type = DISP_SESSION_PRIMARY;
	config.device_id = 0;
	ret = ioctl(fd, DISP_IOCTL_CREATE_SESSION, &config);
	printf("%s,create session for primary display, session_id = %d\n", __func__, config.session_id);

	vsync_config.session_id = config.session_id;

	while(1)
	{
		vsync_cnt ++;
		if(ioctl(fd, DISP_IOCTL_WAIT_FOR_VSYNC, &vsync_config) < 0)
		{
			printf("wait vsync fail:%d\n", ret);
		}

		p = dequeueinput();
		if(p)
		{
			//printf("dequeued:%d,%d,%d\n", p->config[0].next_buff_idx, p->config_layer_num, p->config[0].layer_enable);
			p->session_id = config.session_id;
			if(ioctl(fd, DISP_IOCTL_SET_INPUT_BUFFER, p) < 0)
			{
				printf("ioctl to set multi overlay layer enable fail\n");
			}
			
			if(ioctl(fd, DISP_IOCTL_TRIGGER_SESSION, &config) <0)
			{
				printf("trigger fail:%d\n", ret);
			}

			freeinput(p);
		}
		else
		{
			printf("no input\t");
		}
		
		if(vsync_cnt %240 ==0)
		{
			//printf("vsync:%d\n", vsync_cnt);
		}
	}

	return NULL;
}

void *test_loop_routine0(void *name)
{
	unsigned int vsync_cnt = 0;
	int ret = -1;
	int fd = -1;
	data_buffer_t *p = NULL;
	data_buffer_t *p1 = NULL;
	disp_session_input_config *ps = NULL;
	
	disp_session_config config;
	disp_session_info device_info;
	memset((void*)&device_info, 0, sizeof(disp_session_info));
	
	memset((void*)&config, 0, sizeof(disp_session_config));

	fd = open("/dev/mtk_disp_mgr", O_RDWR);
	
	config.type = DISP_SESSION_PRIMARY;
	config.device_id = 0;
	ret = ioctl(fd, DISP_IOCTL_CREATE_SESSION, &config);
	printf("%s,create session for primary display, session_id = %d\n", __func__, config.session_id);
	
	device_info.session_id = config.session_id;

	ret = ioctl(fd, DISP_IOCTL_GET_SESSION_INFO, &device_info);

	device_info.displayWidth = 32;
	device_info.displayHeight = 32;
	
	while(1)
	{
		p = alloc_buffer(device_info.displayWidth*device_info.displayHeight*4);
		session_prepare_input(fd, config.session_id, p, 2, 1);
		printf("prepare %d|%d\t", p->fence_idx, p->fence_fd);

		ps = getinput();
		setinput(fd, ps, p, device_info.displayWidth, device_info.displayHeight, 2, 1);
		queueinput(ps);

		p1 = alloc_buffer(device_info.displayWidth*device_info.displayHeight*4);
		session_prepare_input(fd, config.session_id, p1, 2, 1);
		printf("prepare %d|%d\t", p1->fence_idx, p1->fence_fd);

		ps = getinput();
		setinput(fd, ps, p1, device_info.displayWidth, device_info.displayHeight, 2, 1);
		queueinput(ps);
		
		ps = getinput();
		setinput(fd,ps, p1, device_info.displayWidth, device_info.displayHeight, 2, 0);
		queueinput(ps);

		ret = sync_wait(p->fence_fd, TIMEOUT_NEVER);
		printf("%d released\t", p->fence_fd);
		free_buffer(p);

		
		ret = sync_wait(p1->fence_fd, TIMEOUT_NEVER);
		printf("%d released\n", p1->fence_fd);
		free_buffer(p1);

	}

	return NULL;
}


void *test_loop_routine1(void *name)
{
	unsigned int vsync_cnt = 0;
	int ret = -1;
	int fd = -1;
	data_buffer_t *p = NULL;
	data_buffer_t *p1 = NULL;
	disp_session_config config;
	disp_session_info device_info;
	memset((void*)&device_info, 0, sizeof(disp_session_info));
	
	memset((void*)&config, 0, sizeof(disp_session_config));

	fd = open("/dev/mtk_disp_mgr", O_RDWR);
	
	config.type = DISP_SESSION_PRIMARY;
	config.device_id = 0;
	ret = ioctl(fd, DISP_IOCTL_CREATE_SESSION, &config);
	printf("%s,create session for primary display, session_id = %d\n", __func__, config.session_id);
	
	device_info.session_id = config.session_id;

	ret = ioctl(fd, DISP_IOCTL_GET_SESSION_INFO, &device_info);
	
	p = alloc_buffer(device_info.displayWidth*device_info.displayHeight*4);
	
	p1 = alloc_buffer(device_info.displayWidth*device_info.displayHeight*4);

	resetinput();
	while(1)
	{
		session_prepare_input(fd, config.session_id, p, 3, 1);
		printf("prepare %d|%d\t", p->fence_idx, p->fence_fd);
		
		pthread_mutex_lock(&input_mutex);
		setinput(fd, &gsession_input_config, p, device_info.displayWidth, device_info.displayHeight, 3, 1);
		pthread_mutex_unlock(&input_mutex);
		
		if(p1->fence_fd)
		{
			ret = sync_wait(p1->fence_fd, TIMEOUT_NEVER);
			printf("%d released\t", p1->fence_fd);
			close(p1->fence_fd);
		}

		session_prepare_input(fd, config.session_id, p1, 3, 1);
		printf("prepare %d|%d\t", p1->fence_idx, p1->fence_fd);
		
		pthread_mutex_lock(&input_mutex);
		setinput(fd,&gsession_input_config, p1, device_info.displayWidth, device_info.displayHeight, 3, 1);
		pthread_mutex_unlock(&input_mutex);
		
		if(p->fence_fd)
		{
			ret = sync_wait(p->fence_fd, TIMEOUT_NEVER);
			printf("%d released\n", p->fence_fd);
			close(p->fence_fd);
		}
	}

	return NULL;
}

// single layer: set layer3 without aee layer enable; 
int ut_case_1(int argc, char **argv)
{
	int fd = -1;
	int ret = 0;
	int cnt = 1;
	data_buffer_t *p = NULL;

	disp_session_config config;
	memset((void*)&config, 0, sizeof(disp_session_config));

	disp_session_info device_info;
	memset((void*)&device_info, 0, sizeof(disp_session_info));

	disp_session_input_config session_input_config;
	memset((void*)&session_input_config, 0, sizeof(disp_session_input_config));

	disp_session_vsync_config vsync_config;
	memset((void*)&vsync_config, 0, sizeof(disp_session_vsync_config));

	printf("****************************ut_case_1***************************\n");
	
	// 1. open driver
	fd = open("/dev/mtk_disp_mgr", O_RDWR);
	printf("1. open driver, fd = %d\n", fd);

	// 2. create session
	config.type      = DISP_SESSION_PRIMARY;
	config.device_id = 0;
	
	ret = ioctl(fd, DISP_IOCTL_CREATE_SESSION, &config);
	printf("2. create session for primary display, session_id = %d, ret = %d\n", config.session_id, ret);

	// 3. get  session info
	device_info.session_id = config.session_id;

	ret = ioctl(fd, DISP_IOCTL_GET_SESSION_INFO, &device_info);
	printf("3. get session info, maxLayerNum = %d ,ret = %d\n", device_info.maxLayerNum, ret);


	// 4. prepare input buffer
	p = alloc_buffer(device_info.displayWidth*device_info.displayHeight*4);
	session_prepare_input(fd, config.session_id, p, 3, 1);
	printf("4. prepare input buffer %d|%d\t\n", p->fence_idx, p->fence_fd);


	// 5. wait vsync		
	vsync_config.session_id = config.session_id;
	ret = ioctl(fd, DISP_IOCTL_WAIT_FOR_VSYNC, &vsync_config);
	printf("5. wait vsync , ret = %d\n", ret);


	// 6. set input buffer(enable layer3)
	session_input_config.session_id = config.session_id;
	session_input_config.config_layer_num = 1;
	session_input_config.config[0].layer_id = 3;
	session_input_config.config[0].layer_enable = 1;
	session_input_config.config[0].src_base_addr = 0;
	session_input_config.config[0].src_phy_addr = 0;
	session_input_config.config[0].next_buff_idx = p->fence_idx;
	session_input_config.config[0].src_fmt = DISP_FORMAT_RGBA8888;
	session_input_config.config[0].src_pitch = device_info.displayWidth;
	session_input_config.config[0].src_offset_x = 0; 
	session_input_config.config[0].src_width = device_info.displayWidth;
	session_input_config.config[0].src_offset_y = 0;
	session_input_config.config[0].src_height = device_info.displayHeight;
	session_input_config.config[0].tgt_offset_x = 0;
	session_input_config.config[0].tgt_offset_y = 0;
	session_input_config.config[0].tgt_width = device_info.displayWidth;
	session_input_config.config[0].tgt_height = device_info.displayHeight;
		
	ret = ioctl(fd, DISP_IOCTL_SET_INPUT_BUFFER, &session_input_config);
	printf("6. set input buffer(enable layer3), ret = %d\n", ret);
	

	// 7. trigger session	
	ret = ioctl(fd, DISP_IOCTL_TRIGGER_SESSION, &config);
	printf("7. trigger session, ret = %d\n", ret);

	
	// 8. wait vsync			
	int loop = 60 * 5; // 5s
	vsync_config.session_id = config.session_id;

	while(loop--)
	{
	    ret = ioctl(fd, DISP_IOCTL_WAIT_FOR_VSYNC, &vsync_config);
	}
	printf("8. wait vsync(60*5) , ret = %d\n", ret);

	// 9. set input buffer(disable layer3)
	session_input_config.session_id = config.session_id;
	session_input_config.config_layer_num = 1;
	session_input_config.config[0].layer_id = 3;
	session_input_config.config[0].layer_enable = 0;
	session_input_config.config[0].src_base_addr = 0;
	session_input_config.config[0].src_phy_addr = 0;
	session_input_config.config[0].next_buff_idx = p->fence_idx;
	session_input_config.config[0].src_fmt = DISP_FORMAT_RGBA8888;
	session_input_config.config[0].src_pitch = device_info.displayWidth;
	session_input_config.config[0].src_offset_x = 0; 
	session_input_config.config[0].src_width = device_info.displayWidth;
	session_input_config.config[0].src_offset_y = 0;
	session_input_config.config[0].src_height = device_info.displayHeight;
	session_input_config.config[0].tgt_offset_x = 0;
	session_input_config.config[0].tgt_offset_y = 0;
	session_input_config.config[0].tgt_width = device_info.displayWidth;
	session_input_config.config[0].tgt_height = device_info.displayHeight;

	ret = ioctl(fd, DISP_IOCTL_SET_INPUT_BUFFER, &session_input_config);
	printf("9. set input buffer(disable layer3), ret = %d\n", ret);
	

	// 10. trigger session
	ret = ioctl(fd, DISP_IOCTL_TRIGGER_SESSION, &config);
	printf("10. trigger session, ret = %d\n", ret);


	// 11. free buffer and close fd
	ret = sync_wait(p->fence_fd, TIMEOUT_NEVER);
	printf("11. wait fence(%d) release, ret = %d\n", p->fence_fd, ret);
	free_buffer(p);


	// 12. wait vsync
	loop = 60 * 2; // 2s
	vsync_config.session_id = config.session_id;

	while(loop--)
	{
	    ret = ioctl(fd, DISP_IOCTL_WAIT_FOR_VSYNC, &vsync_config);
	}
	printf("12. wait vsync(60*2) , ret = %d\n", ret);

	// 13. close driver
	close(fd);

		
	printf("****************************************************************\n");
	return 0;
}


// single layer: set layer3 with aee layer enable; 
int ut_case_2(int argc, char **argv)
{
	int fd = -1;
	int ret = 0;
	int cnt = 1;
	data_buffer_t *p = NULL;

	disp_session_config config;
	memset((void*)&config, 0, sizeof(disp_session_config));

	disp_session_info device_info;
	memset((void*)&device_info, 0, sizeof(disp_session_info));

	disp_session_input_config session_input_config;
	memset((void*)&session_input_config, 0, sizeof(disp_session_input_config));

	disp_session_vsync_config vsync_config;
	memset((void*)&vsync_config, 0, sizeof(disp_session_vsync_config));
#if defined(HAVE_AEE_FEATURE)
	aee_system_exception("Display_UT", "UT_CASE_2", 0,"Enable AEE Layer");
#else
  printf("aee fail!\n");
#endif
	printf("****************************ut_case_2***************************\n");
	
	// 1. open driver
	fd = open("/dev/mtk_disp_mgr", O_RDWR);
	printf("1. open driver, fd = %d\n", fd);

	// 2. create session
	config.type      = DISP_SESSION_PRIMARY;
	config.device_id = 0;
	
	ret = ioctl(fd, DISP_IOCTL_CREATE_SESSION, &config);
	printf("2. create session for primary display, session_id = %d, ret = %d\n", config.session_id, ret);

	// 3. get  session info
	device_info.session_id = config.session_id;

	ret = ioctl(fd, DISP_IOCTL_GET_SESSION_INFO, &device_info);
	printf("3. get session info, maxLayerNum = %d ,ret = %d\n", device_info.maxLayerNum, ret);


	// 4. prepare input buffer
	p = alloc_buffer(device_info.displayWidth*device_info.displayHeight*4);
	session_prepare_input(fd, config.session_id, p, 3, 1);
	printf("4. prepare input buffer %d|%d\t\n", p->fence_idx, p->fence_fd);


	// 5. wait vsync		
	vsync_config.session_id = config.session_id;
	ret = ioctl(fd, DISP_IOCTL_WAIT_FOR_VSYNC, &vsync_config);
	printf("5. wait vsync , ret = %d\n", ret);


	// 6. set input buffer(enable layer3)
	session_input_config.session_id = config.session_id;
	session_input_config.config_layer_num = 1;
	session_input_config.config[0].layer_id = 3;
	session_input_config.config[0].layer_enable = 1;
	session_input_config.config[0].src_base_addr = 0;
	session_input_config.config[0].src_phy_addr = 0;
	session_input_config.config[0].next_buff_idx = p->fence_idx;
	session_input_config.config[0].src_fmt = DISP_FORMAT_RGBA8888;
	session_input_config.config[0].src_pitch = device_info.displayWidth;
	session_input_config.config[0].src_offset_x = 0; 
	session_input_config.config[0].src_width = device_info.displayWidth;
	session_input_config.config[0].src_offset_y = 0;
	session_input_config.config[0].src_height = device_info.displayHeight;
	session_input_config.config[0].tgt_offset_x = 0;
	session_input_config.config[0].tgt_offset_y = 0;
	session_input_config.config[0].tgt_width = device_info.displayWidth;
	session_input_config.config[0].tgt_height = device_info.displayHeight;
		
	ret = ioctl(fd, DISP_IOCTL_SET_INPUT_BUFFER, &session_input_config);
	printf("6. set input buffer(enable layer3), ret = %d\n", ret);
	

	// 7. trigger session	
	ret = ioctl(fd, DISP_IOCTL_TRIGGER_SESSION, &config);
	printf("7. trigger session, ret = %d\n", ret);

	
	// 8. wait vsync			
	int loop = 60 * 5; // 5s
	vsync_config.session_id = config.session_id;

	while(loop--)
	{
	    ret = ioctl(fd, DISP_IOCTL_WAIT_FOR_VSYNC, &vsync_config);
	}
	printf("8. wait vsync(60*5) , ret = %d\n", ret);

	// 9. set input buffer(disable layer3)
	session_input_config.session_id = config.session_id;
	session_input_config.config_layer_num = 1;
	session_input_config.config[0].layer_id = 3;
	session_input_config.config[0].layer_enable = 0;
	session_input_config.config[0].src_base_addr = 0;
	session_input_config.config[0].src_phy_addr = 0;
	session_input_config.config[0].next_buff_idx = p->fence_idx;
	session_input_config.config[0].src_fmt = DISP_FORMAT_RGBA8888;
	session_input_config.config[0].src_pitch = device_info.displayWidth;
	session_input_config.config[0].src_offset_x = 0; 
	session_input_config.config[0].src_width = device_info.displayWidth;
	session_input_config.config[0].src_offset_y = 0;
	session_input_config.config[0].src_height = device_info.displayHeight;
	session_input_config.config[0].tgt_offset_x = 0;
	session_input_config.config[0].tgt_offset_y = 0;
	session_input_config.config[0].tgt_width = device_info.displayWidth;
	session_input_config.config[0].tgt_height = device_info.displayHeight;

	ret = ioctl(fd, DISP_IOCTL_SET_INPUT_BUFFER, &session_input_config);
	printf("9. set input buffer(disable layer3), ret = %d\n", ret);
	

	// 10. trigger session
	ret = ioctl(fd, DISP_IOCTL_TRIGGER_SESSION, &config);
	printf("10. trigger session, ret = %d\n", ret);


	// 11. free buffer and close fd
	ret = sync_wait(p->fence_fd, TIMEOUT_NEVER);
	printf("11. wait fence(%d) release, ret = %d\n", p->fence_fd, ret);
	free_buffer(p);

	// 12. wait vsync
	loop = 60 * 2; // 2s
	vsync_config.session_id = config.session_id;

	while(loop--)
	{
	    ret = ioctl(fd, DISP_IOCTL_WAIT_FOR_VSYNC, &vsync_config);
	}
	printf("12. wait vsync(60*2) , ret = %d\n", ret);


	// 13. close driver
	close(fd);

		
	printf("****************************************************************\n");
	return 0;
}

// 4 layer: set 4 layers 
int ut_case_3(int argc, char **argv)
{
	int fd = -1;
	int ret = 0;
	int cnt = 1;
	data_buffer_t *p0 = NULL;
	data_buffer_t *p1 = NULL;	
	data_buffer_t *p2 = NULL;
	data_buffer_t *p3 = NULL;
	int size = 0;

	disp_session_config config;
	memset((void*)&config, 0, sizeof(disp_session_config));

	disp_session_info device_info;
	memset((void*)&device_info, 0, sizeof(disp_session_info));

	disp_session_input_config session_input_config;
	memset((void*)&session_input_config, 0, sizeof(disp_session_input_config));

	disp_session_vsync_config vsync_config;
	memset((void*)&vsync_config, 0, sizeof(disp_session_vsync_config));

	printf("****************************ut_case_3***************************\n");
	
	// 1. open driver
	fd = open("/dev/mtk_disp_mgr", O_RDWR);
	printf("1. open driver, fd = %d\n", fd);

	// 2. create session
	config.type      = DISP_SESSION_PRIMARY;
	config.device_id = 0;
	
	ret = ioctl(fd, DISP_IOCTL_CREATE_SESSION, &config);
	printf("2. create session for primary display, session_id = %d, ret = %d\n", config.session_id, ret);

	// 3. get  session info
	device_info.session_id = config.session_id;

	ret = ioctl(fd, DISP_IOCTL_GET_SESSION_INFO, &device_info);
	printf("3. get session info, maxLayerNum = %d ,ret = %d\n", device_info.maxLayerNum, ret);


	// 4. prepare input buffer
	size = device_info.displayWidth*device_info.displayHeight*4;
	p0 = alloc_buffer(size/4);
	p1 = alloc_buffer(size/4);
	p2 = alloc_buffer(size/4);
	p3 = alloc_buffer(size/4);
	
	memset(p0->buffer, 0xff, size/4);
	memset(p1->buffer, 0xA0, size/4);
	memset(p2->buffer, 0x80, size/4);
	memset(p3->buffer, 0x40, size/4);

	session_prepare_input(fd, config.session_id, p0, 0, 1);
	session_prepare_input(fd, config.session_id, p1, 1, 1);
	session_prepare_input(fd, config.session_id, p2, 2, 1);
	session_prepare_input(fd, config.session_id, p3, 3, 1);
	
	printf("4. prepare input buffer(0) %d|%d\t\n", p0->fence_idx, p0->fence_fd);
	printf("   prepare input buffer(1) %d|%d\t\n", p1->fence_idx, p1->fence_fd);
	printf("   prepare input buffer(2) %d|%d\t\n", p2->fence_idx, p2->fence_fd);
	printf("   prepare input buffer(3) %d|%d\t\n", p3->fence_idx, p3->fence_fd);


	// 5. wait vsync		
	vsync_config.session_id = config.session_id;
	ret = ioctl(fd, DISP_IOCTL_WAIT_FOR_VSYNC, &vsync_config);
	printf("5. wait vsync , ret = %d\n", ret);


	// 6. set input buffer(enable 4 layers)
	session_input_config.session_id = config.session_id;
	session_input_config.config_layer_num = 4;

	// layer3
	session_input_config.config[0].layer_id = 3;
	session_input_config.config[0].layer_enable = 1;
	session_input_config.config[0].src_base_addr = 0;
	session_input_config.config[0].src_phy_addr = 0;
	session_input_config.config[0].next_buff_idx = p3->fence_idx;
	session_input_config.config[0].src_fmt = DISP_FORMAT_RGBA8888;
	session_input_config.config[0].src_pitch = device_info.displayWidth;
	session_input_config.config[0].src_offset_x = 0; 
	session_input_config.config[0].src_width = device_info.displayWidth;
	session_input_config.config[0].src_offset_y = 0;
	session_input_config.config[0].src_height = device_info.displayHeight/4;
	session_input_config.config[0].tgt_offset_x = 0;
	session_input_config.config[0].tgt_offset_y = 0;
	session_input_config.config[0].tgt_width = device_info.displayWidth;
	session_input_config.config[0].tgt_height = device_info.displayHeight/4;

	// layer2
	session_input_config.config[1].layer_id = 2;
	session_input_config.config[1].layer_enable = 1;
	session_input_config.config[1].src_base_addr = 0;
	session_input_config.config[1].src_phy_addr = 0;
	session_input_config.config[1].next_buff_idx = p2->fence_idx;
	session_input_config.config[1].src_fmt = DISP_FORMAT_RGBA8888;
	session_input_config.config[1].src_pitch = device_info.displayWidth;
	session_input_config.config[1].src_offset_x = 0; 
	session_input_config.config[1].src_width = device_info.displayWidth;
	session_input_config.config[1].src_offset_y = 0;
	session_input_config.config[1].src_height = device_info.displayHeight/4;
	session_input_config.config[1].tgt_offset_x = 0;
	session_input_config.config[1].tgt_offset_y = device_info.displayHeight/4;
	session_input_config.config[1].tgt_width = device_info.displayWidth;
	session_input_config.config[1].tgt_height = device_info.displayHeight/4;

	// layer1
	session_input_config.config[2].layer_id = 1;
	session_input_config.config[2].layer_enable = 1;
	session_input_config.config[2].src_base_addr = 0;
	session_input_config.config[2].src_phy_addr = 0;
	session_input_config.config[2].next_buff_idx = p1->fence_idx;
	session_input_config.config[2].src_fmt = DISP_FORMAT_RGBA8888;
	session_input_config.config[2].src_pitch = device_info.displayWidth;
	session_input_config.config[2].src_offset_x = 0; 
	session_input_config.config[2].src_width = device_info.displayWidth;
	session_input_config.config[2].src_offset_y = 0;
	session_input_config.config[2].src_height = device_info.displayHeight/4;
	session_input_config.config[2].tgt_offset_x = 0;
	session_input_config.config[2].tgt_offset_y = device_info.displayHeight*2/4;
	session_input_config.config[2].tgt_width = device_info.displayWidth;
	session_input_config.config[2].tgt_height = device_info.displayHeight/4;

	// layer0
	session_input_config.config[3].layer_id = 0;
	session_input_config.config[3].layer_enable = 1;
	session_input_config.config[3].src_base_addr = 0;
	session_input_config.config[3].src_phy_addr = 0;
	session_input_config.config[3].next_buff_idx = p0->fence_idx;
	session_input_config.config[3].src_fmt = DISP_FORMAT_RGBA8888;
	session_input_config.config[3].src_pitch = device_info.displayWidth;
	session_input_config.config[3].src_offset_x = 0; 
	session_input_config.config[3].src_width = device_info.displayWidth;
	session_input_config.config[3].src_offset_y = 0;
	session_input_config.config[3].src_height = device_info.displayHeight/4;
	session_input_config.config[3].tgt_offset_x = 0;
	session_input_config.config[3].tgt_offset_y = device_info.displayHeight*3/4;
	session_input_config.config[3].tgt_width = device_info.displayWidth;
	session_input_config.config[3].tgt_height = device_info.displayHeight/4;
		
	ret = ioctl(fd, DISP_IOCTL_SET_INPUT_BUFFER, &session_input_config);
	printf("6. set input buffer(enable 4 layers), ret = %d\n", ret);
	

	// 7. trigger session	
	ret = ioctl(fd, DISP_IOCTL_TRIGGER_SESSION, &config);
	printf("7. trigger session, ret = %d\n", ret);

	
	// 8. wait vsync			
	int loop = 60 * 5; // 5s
	vsync_config.session_id = config.session_id;

	while(loop--)
	{
	    ret = ioctl(fd, DISP_IOCTL_WAIT_FOR_VSYNC, &vsync_config);
	}
	printf("8. wait vsync(60*5) , ret = %d\n", ret);


	// 9. set input buffer(disable 4 layers)
	session_input_config.session_id = config.session_id;
	session_input_config.config_layer_num = 4;

	// layer3
	session_input_config.config[0].layer_id = 3;
	session_input_config.config[0].layer_enable = 0;
	session_input_config.config[0].src_base_addr = 0;
	session_input_config.config[0].src_phy_addr = 0;
	session_input_config.config[0].next_buff_idx = p3->fence_idx;
	session_input_config.config[0].src_fmt = DISP_FORMAT_RGBA8888;
	session_input_config.config[0].src_pitch = device_info.displayWidth;
	session_input_config.config[0].src_offset_x = 0; 
	session_input_config.config[0].src_width = device_info.displayWidth;
	session_input_config.config[0].src_offset_y = 0;
	session_input_config.config[0].src_height = device_info.displayHeight/4;
	session_input_config.config[0].tgt_offset_x = 0;
	session_input_config.config[0].tgt_offset_y = 0;
	session_input_config.config[0].tgt_width = device_info.displayWidth;
	session_input_config.config[0].tgt_height = device_info.displayHeight/4;

	// layer2
	session_input_config.config[1].layer_id = 2;
	session_input_config.config[1].layer_enable = 0;
	session_input_config.config[1].src_base_addr = 0;
	session_input_config.config[1].src_phy_addr = 0;
	session_input_config.config[1].next_buff_idx = p2->fence_idx;
	session_input_config.config[1].src_fmt = DISP_FORMAT_RGBA8888;
	session_input_config.config[1].src_pitch = device_info.displayWidth;
	session_input_config.config[1].src_offset_x = 0; 
	session_input_config.config[1].src_width = device_info.displayWidth;
	session_input_config.config[1].src_offset_y = 0;
	session_input_config.config[1].src_height = device_info.displayHeight/4;
	session_input_config.config[1].tgt_offset_x = 0;
	session_input_config.config[1].tgt_offset_y = device_info.displayHeight/4;
	session_input_config.config[1].tgt_width = device_info.displayWidth;
	session_input_config.config[1].tgt_height = device_info.displayHeight/4;

	// layer1
	session_input_config.config[2].layer_id = 1;
	session_input_config.config[2].layer_enable = 0;
	session_input_config.config[2].src_base_addr = 0;
	session_input_config.config[2].src_phy_addr = 0;
	session_input_config.config[2].next_buff_idx = p1->fence_idx;
	session_input_config.config[2].src_fmt = DISP_FORMAT_RGBA8888;
	session_input_config.config[2].src_pitch = device_info.displayWidth;
	session_input_config.config[2].src_offset_x = 0; 
	session_input_config.config[2].src_width = device_info.displayWidth;
	session_input_config.config[2].src_offset_y = 0;
	session_input_config.config[2].src_height = device_info.displayHeight/4;
	session_input_config.config[2].tgt_offset_x = 0;
	session_input_config.config[2].tgt_offset_y = device_info.displayHeight*2/4;
	session_input_config.config[2].tgt_width = device_info.displayWidth;
	session_input_config.config[2].tgt_height = device_info.displayHeight/4;

	// layer0
	session_input_config.config[3].layer_id = 0;
	session_input_config.config[3].layer_enable = 0;
	session_input_config.config[3].src_base_addr = 0;
	session_input_config.config[3].src_phy_addr = 0;
	session_input_config.config[3].next_buff_idx = p0->fence_idx;
	session_input_config.config[3].src_fmt = DISP_FORMAT_RGBA8888;
	session_input_config.config[3].src_pitch = device_info.displayWidth;
	session_input_config.config[3].src_offset_x = 0; 
	session_input_config.config[3].src_width = device_info.displayWidth;
	session_input_config.config[3].src_offset_y = 0;
	session_input_config.config[3].src_height = device_info.displayHeight/4;
	session_input_config.config[3].tgt_offset_x = 0;
	session_input_config.config[3].tgt_offset_y = device_info.displayHeight*3/4;
	session_input_config.config[3].tgt_width = device_info.displayWidth;
	session_input_config.config[3].tgt_height = device_info.displayHeight/4;


	ret = ioctl(fd, DISP_IOCTL_SET_INPUT_BUFFER, &session_input_config);
	printf("9. set input buffer(disable layer3), ret = %d\n", ret);
	

	// 10. trigger session
	ret = ioctl(fd, DISP_IOCTL_TRIGGER_SESSION, &config);
	printf("10. trigger session, ret = %d\n", ret);


	// 11. free buffer and close fd
	ret = sync_wait(p3->fence_fd, TIMEOUT_NEVER);
	printf("11. wait fence(%d) release, ret = %d\n", p3->fence_fd, ret);
	ret = sync_wait(p2->fence_fd, TIMEOUT_NEVER);	
	printf("    wait fence(%d) release, ret = %d\n", p2->fence_fd, ret);
	ret = sync_wait(p1->fence_fd, TIMEOUT_NEVER);	
	printf("    wait fence(%d) release, ret = %d\n", p1->fence_fd, ret);
	ret = sync_wait(p0->fence_fd, TIMEOUT_NEVER);
	printf("    wait fence(%d) release, ret = %d\n", p0->fence_fd, ret);
	free_buffer(p3);
	free_buffer(p2);
	free_buffer(p1);
	free_buffer(p0);

	// 12. wait vsync
	loop = 60 * 2; // 2s
	vsync_config.session_id = config.session_id;

	while(loop--)
	{
	    ret = ioctl(fd, DISP_IOCTL_WAIT_FOR_VSYNC, &vsync_config);
	}
	printf("12. wait vsync(60*2) , ret = %d\n", ret);


	// 13. close driver
	close(fd);

		
	printf("****************************************************************\n");
	return 0;
}


// 4 layer: set 4 layers 
int ut_case_4(int argc, char **argv)
{
	int fd = -1;
	int ret = 0;
	int cnt = 1;
	data_buffer_t *p0 = NULL;
	data_buffer_t *p1 = NULL;	
	data_buffer_t *p2 = NULL;
	data_buffer_t *p3 = NULL;
	data_buffer_t *p4 = NULL;	
	data_buffer_t *p5 = NULL;
	int size = 0;

	disp_session_config config;
	memset((void*)&config, 0, sizeof(disp_session_config));

	disp_session_info device_info;
	memset((void*)&device_info, 0, sizeof(disp_session_info));

	disp_session_input_config session_input_config;
	memset((void*)&session_input_config, 0, sizeof(disp_session_input_config));

	disp_session_vsync_config vsync_config;
	memset((void*)&vsync_config, 0, sizeof(disp_session_vsync_config));

	printf("****************************ut_case_4***************************\n");
	
	// 1. open driver
	fd = open("/dev/mtk_disp_mgr", O_RDWR);
	printf("1. open driver, fd = %d\n", fd);

	// 2. create session
	config.type      = DISP_SESSION_PRIMARY;
	config.device_id = 0;
	
	ret = ioctl(fd, DISP_IOCTL_CREATE_SESSION, &config);
	printf("2. create session for primary display, session_id = %d, ret = %d\n", config.session_id, ret);

	// 3. get  session info
	device_info.session_id = config.session_id;

	ret = ioctl(fd, DISP_IOCTL_GET_SESSION_INFO, &device_info);
	printf("3. get session info, maxLayerNum = %d ,ret = %d\n", device_info.maxLayerNum, ret);


	// 4. prepare input buffer
	size = device_info.displayWidth*device_info.displayHeight*4;
	p0 = alloc_buffer(size/4);
	p1 = alloc_buffer(size/4);
	p2 = alloc_buffer(size/4);
	p3 = alloc_buffer(size/4);
	p4 = alloc_buffer(size/4);
	p5 = alloc_buffer(size/4);
	
	memset(p0->buffer, 0xff, size/4);
	memset(p1->buffer, 0xC0, size/4);
	memset(p2->buffer, 0xA0, size/4);
	memset(p3->buffer, 0x80, size/4);
	memset(p4->buffer, 0x60, size/4);
	memset(p5->buffer, 0x40, size/4);

	session_prepare_input(fd, config.session_id, p0, 0, 1);
	session_prepare_input(fd, config.session_id, p1, 1, 1);
	session_prepare_input(fd, config.session_id, p2, 2, 1);
	session_prepare_input(fd, config.session_id, p3, 3, 1);
	session_prepare_input(fd, config.session_id, p4, 2, 1);
	session_prepare_input(fd, config.session_id, p5, 3, 1);
	
	printf("4. prepare input buffer(0) %d|%d\t\n", p0->fence_idx, p0->fence_fd);
	printf("   prepare input buffer(1) %d|%d\t\n", p1->fence_idx, p1->fence_fd);
	printf("   prepare input buffer(2) %d|%d\t\n", p2->fence_idx, p2->fence_fd);
	printf("   prepare input buffer(3) %d|%d\t\n", p3->fence_idx, p3->fence_fd);
	printf("   prepare input buffer(4) %d|%d\t\n", p4->fence_idx, p4->fence_fd);
	printf("   prepare input buffer(5) %d|%d\t\n", p5->fence_idx, p5->fence_fd);


	// 5. wait vsync		
	vsync_config.session_id = config.session_id;
	ret = ioctl(fd, DISP_IOCTL_WAIT_FOR_VSYNC, &vsync_config);
	printf("5. wait vsync , ret = %d\n", ret);


	// 6. set input buffer(enable 4 layers)
	session_input_config.session_id = config.session_id;
	session_input_config.config_layer_num = 4;

	// layer3
	session_input_config.config[0].layer_id = 3;
	session_input_config.config[0].layer_enable = 1;
	session_input_config.config[0].src_base_addr = 0;
	session_input_config.config[0].src_phy_addr = 0;
	session_input_config.config[0].next_buff_idx = p3->fence_idx;
	session_input_config.config[0].src_fmt = DISP_FORMAT_RGBA8888;
	session_input_config.config[0].src_pitch = device_info.displayWidth;
	session_input_config.config[0].src_offset_x = 0; 
	session_input_config.config[0].src_width = device_info.displayWidth;
	session_input_config.config[0].src_offset_y = 0;
	session_input_config.config[0].src_height = device_info.displayHeight/4;
	session_input_config.config[0].tgt_offset_x = 0;
	session_input_config.config[0].tgt_offset_y = 0;
	session_input_config.config[0].tgt_width = device_info.displayWidth;
	session_input_config.config[0].tgt_height = device_info.displayHeight/4;

	// layer2
	session_input_config.config[1].layer_id = 2;
	session_input_config.config[1].layer_enable = 1;
	session_input_config.config[1].src_base_addr = 0;
	session_input_config.config[1].src_phy_addr = 0;
	session_input_config.config[1].next_buff_idx = p2->fence_idx;
	session_input_config.config[1].src_fmt = DISP_FORMAT_RGBA8888;
	session_input_config.config[1].src_pitch = device_info.displayWidth;
	session_input_config.config[1].src_offset_x = 0; 
	session_input_config.config[1].src_width = device_info.displayWidth;
	session_input_config.config[1].src_offset_y = 0;
	session_input_config.config[1].src_height = device_info.displayHeight/4;
	session_input_config.config[1].tgt_offset_x = 0;
	session_input_config.config[1].tgt_offset_y = device_info.displayHeight/4;
	session_input_config.config[1].tgt_width = device_info.displayWidth;
	session_input_config.config[1].tgt_height = device_info.displayHeight/4;

	// layer1
	session_input_config.config[2].layer_id = 1;
	session_input_config.config[2].layer_enable = 1;
	session_input_config.config[2].src_base_addr = 0;
	session_input_config.config[2].src_phy_addr = 0;
	session_input_config.config[2].next_buff_idx = p1->fence_idx;
	session_input_config.config[2].src_fmt = DISP_FORMAT_RGBA8888;
	session_input_config.config[2].src_pitch = device_info.displayWidth;
	session_input_config.config[2].src_offset_x = 0; 
	session_input_config.config[2].src_width = device_info.displayWidth;
	session_input_config.config[2].src_offset_y = 0;
	session_input_config.config[2].src_height = device_info.displayHeight/4;
	session_input_config.config[2].tgt_offset_x = 0;
	session_input_config.config[2].tgt_offset_y = device_info.displayHeight*2/4;
	session_input_config.config[2].tgt_width = device_info.displayWidth;
	session_input_config.config[2].tgt_height = device_info.displayHeight/4;

	// layer0
	session_input_config.config[3].layer_id = 0;
	session_input_config.config[3].layer_enable = 1;
	session_input_config.config[3].src_base_addr = 0;
	session_input_config.config[3].src_phy_addr = 0;
	session_input_config.config[3].next_buff_idx = p0->fence_idx;
	session_input_config.config[3].src_fmt = DISP_FORMAT_RGBA8888;
	session_input_config.config[3].src_pitch = device_info.displayWidth;
	session_input_config.config[3].src_offset_x = 0; 
	session_input_config.config[3].src_width = device_info.displayWidth;
	session_input_config.config[3].src_offset_y = 0;
	session_input_config.config[3].src_height = device_info.displayHeight/4;
	session_input_config.config[3].tgt_offset_x = 0;
	session_input_config.config[3].tgt_offset_y = device_info.displayHeight*3/4;
	session_input_config.config[3].tgt_width = device_info.displayWidth;
	session_input_config.config[3].tgt_height = device_info.displayHeight/4;
		
	ret = ioctl(fd, DISP_IOCTL_SET_INPUT_BUFFER, &session_input_config);
	printf("6. set input buffer(enable 4 layers), ret = %d\n", ret);
	

	// 7. trigger session	
	ret = ioctl(fd, DISP_IOCTL_TRIGGER_SESSION, &config);
	printf("7. trigger session, ret = %d\n", ret);

	
	// 8. wait vsync			
	int loop = 60 * 5; // 5s
	vsync_config.session_id = config.session_id;

	while(loop--)
	{
	    ret = ioctl(fd, DISP_IOCTL_WAIT_FOR_VSYNC, &vsync_config);
	}
	printf("8. wait vsync(60*5) , ret = %d\n", ret);


	// 9. set input buffer(disable layer3/change layer2)
	session_input_config.session_id = config.session_id;
	session_input_config.config_layer_num = 2;

	// layer3
	session_input_config.config[0].layer_id = 3;
	session_input_config.config[0].layer_enable = 0;
	session_input_config.config[0].src_base_addr = 0;
	session_input_config.config[0].src_phy_addr = 0;
	session_input_config.config[0].next_buff_idx = p3->fence_idx;
	session_input_config.config[0].src_fmt = DISP_FORMAT_RGBA8888;
	session_input_config.config[0].src_pitch = device_info.displayWidth;
	session_input_config.config[0].src_offset_x = 0; 
	session_input_config.config[0].src_width = device_info.displayWidth;
	session_input_config.config[0].src_offset_y = 0;
	session_input_config.config[0].src_height = device_info.displayHeight/4;
	session_input_config.config[0].tgt_offset_x = 0;
	session_input_config.config[0].tgt_offset_y = 0;
	session_input_config.config[0].tgt_width = device_info.displayWidth;
	session_input_config.config[0].tgt_height = device_info.displayHeight/4;

	// layer2
	session_input_config.config[1].layer_id = 2;
	session_input_config.config[1].layer_enable = 1;
	session_input_config.config[1].src_base_addr = 0;
	session_input_config.config[1].src_phy_addr = 0;
	session_input_config.config[1].next_buff_idx = p4->fence_idx;
	session_input_config.config[1].src_fmt = DISP_FORMAT_RGBA8888;
	session_input_config.config[1].src_pitch = device_info.displayWidth;
	session_input_config.config[1].src_offset_x = 0; 
	session_input_config.config[1].src_width = device_info.displayWidth;
	session_input_config.config[1].src_offset_y = 0;
	session_input_config.config[1].src_height = device_info.displayHeight/4;
	session_input_config.config[1].tgt_offset_x = 0;
	session_input_config.config[1].tgt_offset_y = device_info.displayHeight/4;
	session_input_config.config[1].tgt_width = device_info.displayWidth;
	session_input_config.config[1].tgt_height = device_info.displayHeight/4;

	ret = ioctl(fd, DISP_IOCTL_SET_INPUT_BUFFER, &session_input_config);
	printf("9. set input buffer(disable layer3/change layer2), ret = %d\n", ret);
	

	// 10. trigger session
	ret = ioctl(fd, DISP_IOCTL_TRIGGER_SESSION, &config);
	printf("10. trigger session, ret = %d\n", ret);


	// 11. free buffer and close fd
	ret = sync_wait(p3->fence_fd, TIMEOUT_NEVER);
	printf("11. wait fence(%d) release, ret = %d\n", p3->fence_fd, ret);
	ret = sync_wait(p2->fence_fd, TIMEOUT_NEVER);	
	printf("    wait fence(%d) release, ret = %d\n", p2->fence_fd, ret);
	free_buffer(p3);
	free_buffer(p2);

	// 12. wait vsync
	loop = 60 * 5; // 5s
	vsync_config.session_id = config.session_id;

	while(loop--)
	{
	    ret = ioctl(fd, DISP_IOCTL_WAIT_FOR_VSYNC, &vsync_config);
	}
	printf("12. wait vsync(60*5) , ret = %d\n", ret);


	// 13. set input buffer(enable layer3)
	session_input_config.session_id = config.session_id;
	session_input_config.config_layer_num = 1;

	// layer3
	session_input_config.config[0].layer_id = 3;
	session_input_config.config[0].layer_enable = 1;
	session_input_config.config[0].src_base_addr = 0;
	session_input_config.config[0].src_phy_addr = 0;
	session_input_config.config[0].next_buff_idx = p5->fence_idx;
	session_input_config.config[0].src_fmt = DISP_FORMAT_RGBA8888;
	session_input_config.config[0].src_pitch = device_info.displayWidth;
	session_input_config.config[0].src_offset_x = 0; 
	session_input_config.config[0].src_width = device_info.displayWidth;
	session_input_config.config[0].src_offset_y = 0;
	session_input_config.config[0].src_height = device_info.displayHeight/4;
	session_input_config.config[0].tgt_offset_x = 0;
	session_input_config.config[0].tgt_offset_y = 0;
	session_input_config.config[0].tgt_width = device_info.displayWidth;
	session_input_config.config[0].tgt_height = device_info.displayHeight/4;

	ret = ioctl(fd, DISP_IOCTL_SET_INPUT_BUFFER, &session_input_config);
	printf("13. set input buffer(enable layer3), ret = %d\n", ret);
	

	// 14. trigger session
	ret = ioctl(fd, DISP_IOCTL_TRIGGER_SESSION, &config);
	printf("14. trigger session, ret = %d\n", ret);


	// 15. wait vsync
	loop = 60 * 2; // 2s
	vsync_config.session_id = config.session_id;

	while(loop--)
	{
	    ret = ioctl(fd, DISP_IOCTL_WAIT_FOR_VSYNC, &vsync_config);
	}
	printf("15. wait vsync(60*2) , ret = %d\n", ret);



	// 136 close driver
	close(fd);

		
	printf("****************************************************************\n");
	return 0;
}

typedef struct {
	data_buffer_t* p;
	int layer_used; // layer_id
	data_buffer_t* need_free_p;
}buffer_t;

buffer_t buffer_control[8];


int get_free_buffer(buffer_t* buffer_control, unsigned int num)
{
	for(int i = 0 ; i < num; i++)
	{
		if(buffer_control[i].p == NULL)
		{
			return i;
		}
	}

	return -1;
}

int get_layer_buffer(buffer_t* buffer_control, unsigned int num, int layer_id)
{
	for(int i = 0 ; i < num; i++)
	{
		if(buffer_control[i].p != NULL && buffer_control[i].layer_used == layer_id && buffer_control[i].need_free_p == NULL)
		{
			return i;
		}
	}

	return -1;
}

void init_buffer_control(buffer_t* buffer_control, unsigned int num)
{
	for(int i = 0; i < num; i++)
	{
		buffer_control[i].p 		   = NULL;
		buffer_control[i].layer_used  = -1;
		buffer_control[i].need_free_p = NULL;
	}

}

void print_head(void)
{
	printf("   ---------------------------------------------------------------------------------------\n");
	printf("   buffer_control |%8d|%8d|%8d|%8d|%8d|%8d|%8d|%8d|\n",0,1,2,3,4,5,6,7);
	printf("   ---------------------------------------------------------------------------------------\n");
}

void print_tail(void)
{
	printf("   ---------------------------------------------------------------------------------------\n");

}

void print_layer_used(void)
{
	printf("   layer_id       |%8d|%8d|%8d|%8d|%8d|%8d|%8d|%8d|\n",
			buffer_control[0].layer_used,
			buffer_control[1].layer_used,
			buffer_control[2].layer_used,
			buffer_control[3].layer_used,
			buffer_control[4].layer_used,
			buffer_control[5].layer_used,
			buffer_control[6].layer_used,
			buffer_control[7].layer_used);
}

void print_fence_fd(void)
{
	printf("   fence_fd       |%8d|%8d|%8d|%8d|%8d|%8d|%8d|%8d|\n",
			buffer_control[0].p ? buffer_control[0].p->fence_fd : -1,
			buffer_control[1].p ? buffer_control[1].p->fence_fd : -1,
			buffer_control[2].p ? buffer_control[2].p->fence_fd : -1,
			buffer_control[3].p ? buffer_control[3].p->fence_fd : -1,
			buffer_control[4].p ? buffer_control[4].p->fence_fd : -1,
			buffer_control[5].p ? buffer_control[5].p->fence_fd : -1,
			buffer_control[6].p ? buffer_control[6].p->fence_fd : -1,
			buffer_control[7].p ? buffer_control[7].p->fence_fd : -1);
}

void print_fence_idx(void)
{
	printf("   fence_idx      |%8d|%8d|%8d|%8d|%8d|%8d|%8d|%8d|\n",
			buffer_control[0].p ? buffer_control[0].p->fence_idx : -1,
			buffer_control[1].p ? buffer_control[1].p->fence_idx : -1,
			buffer_control[2].p ? buffer_control[2].p->fence_idx : -1,
			buffer_control[3].p ? buffer_control[3].p->fence_idx : -1,
			buffer_control[4].p ? buffer_control[4].p->fence_idx : -1,
			buffer_control[5].p ? buffer_control[5].p->fence_idx : -1,
			buffer_control[6].p ? buffer_control[6].p->fence_idx : -1,
			buffer_control[7].p ? buffer_control[7].p->fence_idx : -1);

}

void print_need_free(void)
{
	printf("   need_free      |%8d|%8d|%8d|%8d|%8d|%8d|%8d|%8d|\n",
			buffer_control[0].need_free_p ? 1 : 0,
			buffer_control[1].need_free_p ? 1 : 0,
			buffer_control[2].need_free_p ? 1 : 0,
			buffer_control[3].need_free_p ? 1 : 0,
			buffer_control[4].need_free_p ? 1 : 0,
			buffer_control[5].need_free_p ? 1 : 0,
			buffer_control[6].need_free_p ? 1 : 0,
			buffer_control[7].need_free_p ? 1 : 0);

}

void decode_image_to_buffer(void *buffer)
{
	SkBitmap bp;
	SkImageDecoder::Format fmt;
	
	int ret = SkImageDecoder::DecodeFile("./system/res/images/lcd_test_00.png",
		                       &bp, 
		                       SkBitmap::kARGB_8888_Config,
		                       SkImageDecoder::kDecodePixels_Mode,
		                       &fmt);
	if(!ret)
	{
		printf("decode file fail\n");
	}
	else
	{
		printf("width=%d,heght=%d,rowbytesaspixels=%d,bytesperpixel=%d\n",
				bp.width(),bp.height(),bp.rowBytesAsPixels(),bp.bytesPerPixel());
		
		memcpy(buffer,(void*)bp.getPixels(),bp.width()*bp.height()*bp.bytesPerPixel());
	}
	
}
// 4 layer: set 4 layers 
int ut_case_5(int argc, char **argv)
{
	int fd = -1;
	int ret = 0;
	int cnt = 1;	
	int size = 0; // prepare buffer size
	int color[7] = {0x20,0x40,0x60,0x80,0xa0,0xc0,0xff};

	void *image_buffer = NULL;

	
	init_buffer_control(buffer_control,8);
	
	disp_session_config config;
	memset((void*)&config, 0, sizeof(disp_session_config));

	disp_session_info device_info;
	memset((void*)&device_info, 0, sizeof(disp_session_info));

	disp_session_input_config session_input_config;
	memset((void*)&session_input_config, 0, sizeof(disp_session_input_config));

	disp_session_vsync_config vsync_config;
	memset((void*)&vsync_config, 0, sizeof(disp_session_vsync_config));

	printf("****************************ut_case_5***************************\n");
	
	// 1. open driver
	fd = open("/dev/mtk_disp_mgr", O_RDWR);
	printf("1. open driver, fd = %d\n", fd);

	// 2. create session
	config.type      = DISP_SESSION_PRIMARY;
	config.device_id = 0;
	
	ret = ioctl(fd, DISP_IOCTL_CREATE_SESSION, &config);
	printf("2. create session for primary display, session_id = %d, ret = %d\n", config.session_id, ret);

	// 3. get  session info
	device_info.session_id = config.session_id;

	ret = ioctl(fd, DISP_IOCTL_GET_SESSION_INFO, &device_info);
	printf("3. get session info, maxLayerNum = %d ,ret = %d\n", device_info.maxLayerNum, ret);


	// 4. prepare input buffer
	size = device_info.displayWidth*device_info.displayHeight*4;

    image_buffer = malloc(size);
	if(image_buffer == NULL)
	{
		goto close_fd;
	}
	
	decode_image_to_buffer(image_buffer);

	
	printf("4. prepare buffer 4 layers\n");
	for(int i = 0; i < 4; i++)
	{
		printf("   ==%d/4: layer_id = %d, layer_enable = 1\n", i, i);
		print_head();
		// get free buffer point to alloc buffer
		int buffer_idx = get_free_buffer(buffer_control,8);
		if(buffer_idx == -1) 
		{
			// if i != 0 , need free buffer
			goto close_fd;
		}	
		
		buffer_control[buffer_idx].p = alloc_buffer(size/4);

		// set color
		//memset(buffer_control[buffer_idx].p->buffer, color[i], size/4);
		memcpy(buffer_control[buffer_idx].p->buffer,image_buffer+(3-i)*size/4,size/4);

		// buffer_control[buffer_idx] set layer used
		buffer_control[buffer_idx].layer_used = i;

		
		print_layer_used();

		// prepare input
		session_prepare_input(fd, config.session_id, buffer_control[buffer_idx].p, i, 1);

		print_fence_fd();
		print_fence_idx();
		print_need_free();
		print_tail();

	}

	// 5. wait vsync		
	vsync_config.session_id = config.session_id;
	ret = ioctl(fd, DISP_IOCTL_WAIT_FOR_VSYNC, &vsync_config);
	printf("5. wait vsync , ret = %d\n", ret);


	// 6. set input buffer(enable 4 layers)
	session_input_config.session_id = config.session_id;
	session_input_config.config_layer_num = 4;

	// layer3
	session_input_config.config[0].layer_id = 3;
	session_input_config.config[0].layer_enable = 1;
	session_input_config.config[0].src_base_addr = 0;
	session_input_config.config[0].src_phy_addr = 0;
	session_input_config.config[0].next_buff_idx = buffer_control[get_layer_buffer(buffer_control,8,3)].p->fence_idx;
	session_input_config.config[0].src_fmt = DISP_FORMAT_RGBA8888;
	session_input_config.config[0].src_pitch = device_info.displayWidth;
	session_input_config.config[0].src_offset_x = 0; 
	session_input_config.config[0].src_width = device_info.displayWidth;
	session_input_config.config[0].src_offset_y = 0;
	session_input_config.config[0].src_height = device_info.displayHeight/4;
	session_input_config.config[0].tgt_offset_x = 0;
	session_input_config.config[0].tgt_offset_y = 0;
	session_input_config.config[0].tgt_width = device_info.displayWidth;
	session_input_config.config[0].tgt_height = device_info.displayHeight/4;

	// layer2
	session_input_config.config[1].layer_id = 2;
	session_input_config.config[1].layer_enable = 1;
	session_input_config.config[1].src_base_addr = 0;
	session_input_config.config[1].src_phy_addr = 0;
	session_input_config.config[1].next_buff_idx = buffer_control[get_layer_buffer(buffer_control,8,2)].p->fence_idx;
	session_input_config.config[1].src_fmt = DISP_FORMAT_RGBA8888;
	session_input_config.config[1].src_pitch = device_info.displayWidth;
	session_input_config.config[1].src_offset_x = 0; 
	session_input_config.config[1].src_width = device_info.displayWidth;
	session_input_config.config[1].src_offset_y = 0;
	session_input_config.config[1].src_height = device_info.displayHeight/4;
	session_input_config.config[1].tgt_offset_x = 0;
	session_input_config.config[1].tgt_offset_y = device_info.displayHeight/4;
	session_input_config.config[1].tgt_width = device_info.displayWidth;
	session_input_config.config[1].tgt_height = device_info.displayHeight/4;

	// layer1
	session_input_config.config[2].layer_id = 1;
	session_input_config.config[2].layer_enable = 1;
	session_input_config.config[2].src_base_addr = 0;
	session_input_config.config[2].src_phy_addr = 0;
	session_input_config.config[2].next_buff_idx = buffer_control[get_layer_buffer(buffer_control,8,1)].p->fence_idx;
	session_input_config.config[2].src_fmt = DISP_FORMAT_RGBA8888;
	session_input_config.config[2].src_pitch = device_info.displayWidth;
	session_input_config.config[2].src_offset_x = 0; 
	session_input_config.config[2].src_width = device_info.displayWidth;
	session_input_config.config[2].src_offset_y = 0;
	session_input_config.config[2].src_height = device_info.displayHeight/4;
	session_input_config.config[2].tgt_offset_x = 0;
	session_input_config.config[2].tgt_offset_y = device_info.displayHeight*2/4;
	session_input_config.config[2].tgt_width = device_info.displayWidth;
	session_input_config.config[2].tgt_height = device_info.displayHeight/4;

	// layer0
	session_input_config.config[3].layer_id = 0;
	session_input_config.config[3].layer_enable = 1;
	session_input_config.config[3].src_base_addr = 0;
	session_input_config.config[3].src_phy_addr = 0;
	session_input_config.config[3].next_buff_idx = buffer_control[get_layer_buffer(buffer_control,8,0)].p->fence_idx;
	session_input_config.config[3].src_fmt = DISP_FORMAT_RGBA8888;
	session_input_config.config[3].src_pitch = device_info.displayWidth;
	session_input_config.config[3].src_offset_x = 0; 
	session_input_config.config[3].src_width = device_info.displayWidth;
	session_input_config.config[3].src_offset_y = 0;
	session_input_config.config[3].src_height = device_info.displayHeight/4;
	session_input_config.config[3].tgt_offset_x = 0;
	session_input_config.config[3].tgt_offset_y = device_info.displayHeight*3/4;
	session_input_config.config[3].tgt_width = device_info.displayWidth;
	session_input_config.config[3].tgt_height = device_info.displayHeight/4;
		
	ret = ioctl(fd, DISP_IOCTL_SET_INPUT_BUFFER, &session_input_config);
	printf("6. set input buffer(enable 4 layers), ret = %d\n", ret);
	

	// 7. trigger session	
	ret = ioctl(fd, DISP_IOCTL_TRIGGER_SESSION, &config);
	printf("7. trigger session, ret = %d\n", ret);


	while(1)
	{
	
		printf("==loop====================================================\n");
		
		// 8.2. set input buffer
		int config_layer_num = rand()%4 + 1;
		session_input_config.session_id = config.session_id;
		session_input_config.config_layer_num = config_layer_num;
		
		printf("8.1 prepare buffer %d layers\n",config_layer_num);
	
		for(int n = 0; n < config_layer_num; n++)
		{
			int layer_id = rand()%4;
			int layer_enable = rand()%2;

			printf("    ==%d/%d: layer_id = %d, layer_enable = %d\n", n, config_layer_num, layer_id,layer_enable);
            print_head();
			// enable or disable layer_id
			// case 1: layer_id was enable, enable again
			// case 2: layer_id was enable, disable it
			// case 3: layer_id was disable, enable it
			// case 4: layer_id was disable, disable again
			
			int layer_buffer_idx = get_layer_buffer(buffer_control,8,layer_id);
			if(layer_buffer_idx == -1) //layer_id was disable
			{
				if(layer_enable)
				{
					//case 3: prepare input buffer

					// get free buffer point to alloc buffer
					int buffer_idx = get_free_buffer(buffer_control,8);
				
					if(buffer_idx == -1) 
					{
						// need free buffer???
						goto close_fd;
					}
					buffer_control[buffer_idx].p = alloc_buffer(size/4);
					
					// set color
					int color_idx = rand()%4;
					//memset(buffer_control[buffer_idx].p->buffer, color[color_idx], size/4);				
					memcpy(buffer_control[buffer_idx].p->buffer,image_buffer+color_idx*size/4,size/4);
					
					// buffer_control[buffer_idx] set layer used
					buffer_control[buffer_idx].layer_used = layer_id;
					print_layer_used();
					
					// prepare input
					session_prepare_input(fd, config.session_id, buffer_control[buffer_idx].p, layer_id, 1);

					print_fence_fd();
					print_fence_idx();
					print_need_free();


					// layer_id
					session_input_config.config[n].layer_id = layer_id;
					session_input_config.config[n].layer_enable = layer_enable;
					session_input_config.config[n].src_base_addr = 0;
					session_input_config.config[n].src_phy_addr = 0;
					session_input_config.config[n].next_buff_idx = buffer_control[buffer_idx].p->fence_idx;
					session_input_config.config[n].src_fmt = DISP_FORMAT_RGBA8888;
					session_input_config.config[n].src_pitch = device_info.displayWidth;
					session_input_config.config[n].src_offset_x = 0; 
					session_input_config.config[n].src_width = device_info.displayWidth;
					session_input_config.config[n].src_offset_y = 0;
					session_input_config.config[n].src_height = device_info.displayHeight/4;
					session_input_config.config[n].tgt_offset_x = 0;
					session_input_config.config[n].tgt_offset_y = device_info.displayHeight*(3-layer_id)/4;
					session_input_config.config[n].tgt_width = device_info.displayWidth;
					session_input_config.config[n].tgt_height = device_info.displayHeight/4;

				}
				else
				{
					//case 4: do nothing
					print_layer_used();
					print_fence_fd();
					print_fence_idx();
					print_need_free();
					
					// layer_id
					session_input_config.config[n].layer_id = layer_id;
					session_input_config.config[n].layer_enable = layer_enable;
					session_input_config.config[n].src_base_addr = 0;
					session_input_config.config[n].src_phy_addr = 0;
					session_input_config.config[n].next_buff_idx = 0; //????
					session_input_config.config[n].src_fmt = DISP_FORMAT_RGBA8888;
					session_input_config.config[n].src_pitch = device_info.displayWidth;
					session_input_config.config[n].src_offset_x = 0; 
					session_input_config.config[n].src_width = device_info.displayWidth;
					session_input_config.config[n].src_offset_y = 0;
					session_input_config.config[n].src_height = device_info.displayHeight/4;
					session_input_config.config[n].tgt_offset_x = 0;
					session_input_config.config[n].tgt_offset_y = device_info.displayHeight*(3-layer_id)/4;
					session_input_config.config[n].tgt_width = device_info.displayWidth;
					session_input_config.config[n].tgt_height = device_info.displayHeight/4;
				}
			}
			else
			{
				// record p in need free p
				buffer_control[layer_buffer_idx].need_free_p = buffer_control[layer_buffer_idx].p;
				
				if(layer_enable)
				{
					//case 1:prepare input buffer / wait fence_fd release				

					// get free buffer point to alloc buffer
					int buffer_idx = get_free_buffer(buffer_control,8);				
				
					if(buffer_idx == -1) 
					{
						// need free buffer???
						goto close_fd;
					}
					buffer_control[buffer_idx].p = alloc_buffer(size/4);
					
					// set color
					int color_idx = rand()%4;
					//memset(buffer_control[buffer_idx].p->buffer, color[color_idx], size/4);
					memcpy(buffer_control[buffer_idx].p->buffer,image_buffer+color_idx*size/4,size/4);

					// buffer_control[buffer_idx] set layer used
					buffer_control[buffer_idx].layer_used = layer_id;

					print_layer_used();
					
					// prepare input
					session_prepare_input(fd, config.session_id, buffer_control[buffer_idx].p, layer_id, 1);

					print_fence_fd();
					print_fence_idx();
					print_need_free();
					
					// layer_id
					session_input_config.config[n].layer_id = layer_id;
					session_input_config.config[n].layer_enable = layer_enable;
					session_input_config.config[n].src_base_addr = 0;
					session_input_config.config[n].src_phy_addr = 0;
					session_input_config.config[n].next_buff_idx = buffer_control[buffer_idx].p->fence_idx;
					session_input_config.config[n].src_fmt = DISP_FORMAT_RGBA8888;
					session_input_config.config[n].src_pitch = device_info.displayWidth;
					session_input_config.config[n].src_offset_x = 0; 
					session_input_config.config[n].src_width = device_info.displayWidth;
					session_input_config.config[n].src_offset_y = 0;
					session_input_config.config[n].src_height = device_info.displayHeight/4;
					session_input_config.config[n].tgt_offset_x = 0;
					session_input_config.config[n].tgt_offset_y = device_info.displayHeight*(3-layer_id)/4;
					session_input_config.config[n].tgt_width = device_info.displayWidth;
					session_input_config.config[n].tgt_height = device_info.displayHeight/4;

					
				}
				else
				{
					//case 2: wait fence_fd release
					
					print_layer_used();
					print_fence_fd();
					print_fence_idx();
					print_need_free();

					// layer_id
					session_input_config.config[n].layer_id = layer_id;
					session_input_config.config[n].layer_enable = layer_enable;
					session_input_config.config[n].src_base_addr = 0;
					session_input_config.config[n].src_phy_addr = 0;
					session_input_config.config[n].next_buff_idx = buffer_control[layer_buffer_idx].p->fence_idx;
					session_input_config.config[n].src_fmt = DISP_FORMAT_RGBA8888;
					session_input_config.config[n].src_pitch = device_info.displayWidth;
					session_input_config.config[n].src_offset_x = 0; 
					session_input_config.config[n].src_width = device_info.displayWidth;
					session_input_config.config[n].src_offset_y = 0;
					session_input_config.config[n].src_height = device_info.displayHeight/4;
					session_input_config.config[n].tgt_offset_x = 0;
					session_input_config.config[n].tgt_offset_y = device_info.displayHeight*(3-layer_id)/4;
					session_input_config.config[n].tgt_width = device_info.displayWidth;
					session_input_config.config[n].tgt_height = device_info.displayHeight/4;
				}
			}
					
			print_tail();
		}

		// 8.2 wait vsync			
		int loop = 60 * 1; // 1s
		vsync_config.session_id = config.session_id;

		while(loop--)
		{
		    ret = ioctl(fd, DISP_IOCTL_WAIT_FOR_VSYNC, &vsync_config);
		}
		printf("8.2 wait vsync(60*1) , ret = %d\n", ret);

		
		ret = ioctl(fd, DISP_IOCTL_SET_INPUT_BUFFER, &session_input_config);	
		printf("8.3 set input buffer ret = %d\n", ret);

		// 8.3. trigger session	
		ret = ioctl(fd, DISP_IOCTL_TRIGGER_SESSION, &config);
		printf("8.4 trigger session, ret = %d\n", ret);


		// 8.4 free buffer
		for(int i = 0; i < 8; i++)
		{
			if(buffer_control[i].need_free_p != NULL)
			{
				printf("8.5 wait fence(%d) release, buffer_idx = %d", buffer_control[i].need_free_p->fence_fd, i);			
				ret = sync_wait(buffer_control[i].need_free_p->fence_fd, TIMEOUT_NEVER);
				printf(",ret = %d\n",ret);			
				free_buffer(buffer_control[i].need_free_p);
				buffer_control[i].p = NULL;
				buffer_control[i].need_free_p = NULL;
				buffer_control[i].layer_used = -1;
			}
		}
		print_head();
		print_layer_used();
		print_fence_fd();
		print_fence_idx();
		print_need_free();
		print_tail();
		
	}
	





	// 11. free buffer and close fd


close_fd:

	//  close driver
	close(fd);

		
	printf("****************************************************************\n");
	return 0;
}

void ut_case_6(void)
{

	int fd = -1;
	int ret = 0;
	int cnt = 1;
	data_buffer_t *p_input = NULL;
	data_buffer_t *p_output = NULL;
	unsigned int output_layer_id = 0;
	
	unsigned int tv_resolution_array[2][4] = {{1930,1910,1270,1190},{1210,1190,790,1910}};
	unsigned int tv_resolution_num = 4;
	unsigned int i = 0;

	disp_session_config config;
	memset((void*)&config, 0, sizeof(disp_session_config));

	disp_session_info device_info;
	memset((void*)&device_info, 0, sizeof(disp_session_info));

	disp_session_input_config session_input_config;
	memset((void*)&session_input_config, 0, sizeof(disp_session_input_config));
	
	disp_session_output_config session_output_config;
	memset((void*)&session_output_config, 0, sizeof(disp_session_output_config));

	disp_session_vsync_config vsync_config;
	memset((void*)&vsync_config, 0, sizeof(disp_session_vsync_config));

	printf("****************************ut_case_6***************************\n");

	// tv out resolution bonding test
	
	// 1. open driver
	fd = open("/dev/mtk_disp_mgr", O_RDWR);
	printf("1. open driver, fd = %d\n", fd);

	// 2. create session(memory)
	config.type      = DISP_SESSION_MEMORY;
	config.device_id = 0;
	
	ret = ioctl(fd, DISP_IOCTL_CREATE_SESSION, &config);
	printf("2. create session for sub display, session_id = %d, ret = %d\n", config.session_id, ret);

	// 3. get  session info
	device_info.session_id = config.session_id;

	ret = ioctl(fd, DISP_IOCTL_GET_SESSION_INFO, &device_info);
	printf("3. get session info, maxLayerNum = %d ,ret = %d\n", device_info.maxLayerNum, ret);

	for(i = 0; i < tv_resolution_num; i++)
	{
    	// 4. prepare input buffer
    	//p_input = alloc_buffer(device_info.displayWidth*device_info.displayHeight*4);	
    	p_input = alloc_buffer(tv_resolution_array[0][i]*tv_resolution_array[1][i]*4);
    	session_prepare_input(fd, config.session_id, p_input, 3, 1);
    	printf("4.%d prepare input buffer %d|%d\t\n",i, p_input->fence_idx, p_input->fence_fd);
    
    	// 5. prepare output buffer
    	//p_output = alloc_buffer(device_info.displayWidth*device_info.displayHeight*4);
    	p_output = alloc_buffer(tv_resolution_array[0][i]*tv_resolution_array[1][i]*4);
    	output_layer_id = session_prepare_output(fd, config.session_id, p_output, 3, 1);
    	printf("5.%d prepare output buffer layerid =%d, %d|%d\t\n", i,output_layer_id,p_output->fence_idx, p_output->fence_fd);
    
    
    	// 6. set input buffer(enable layer3)
    	session_input_config.session_id = config.session_id;
    	session_input_config.config_layer_num = 1;
    	session_input_config.config[0].layer_id = 3;
    	session_input_config.config[0].layer_enable = 1;
    	session_input_config.config[0].src_base_addr = 0;
    	session_input_config.config[0].src_phy_addr = 0;
    	session_input_config.config[0].next_buff_idx = p_input->fence_idx;
    	session_input_config.config[0].src_fmt = DISP_FORMAT_RGBA8888;
    	session_input_config.config[0].src_pitch = tv_resolution_array[0][i];
    	session_input_config.config[0].src_offset_x = 0; 
    	session_input_config.config[0].src_width = tv_resolution_array[0][i];
    	session_input_config.config[0].src_offset_y = 0;
    	session_input_config.config[0].src_height = tv_resolution_array[1][i];
    	session_input_config.config[0].tgt_offset_x = 0;
    	session_input_config.config[0].tgt_offset_y = 0;
    	session_input_config.config[0].tgt_width = tv_resolution_array[0][i];
    	session_input_config.config[0].tgt_height = tv_resolution_array[1][i];
    		
    	ret = ioctl(fd, DISP_IOCTL_SET_INPUT_BUFFER, &session_input_config);
    	printf("6.%d set input buffer, w = %d, h = %d, ret = %d\n",i,tv_resolution_array[0][i],tv_resolution_array[1][i],ret);
    
    	// 7. set output buffer
    	session_output_config.session_id = config.session_id;
    	session_output_config.config.fmt = DISP_FORMAT_RGBA8888;
    	session_output_config.config.x = 0;
    	session_output_config.config.y = 0;	
    	session_output_config.config.width = tv_resolution_array[0][i];
    	session_output_config.config.height = tv_resolution_array[1][i];
    	session_output_config.config.pitch = tv_resolution_array[0][i];
    	session_output_config.config.buff_idx = p_output->fence_idx;
    
    	ret = ioctl(fd, DISP_IOCTL_SET_OUTPUT_BUFFER, &session_output_config);
    	printf("7.%d set output buffer, ret = %d\n",i, ret);
    
    	
    	// 8. trigger session	
    	ret = ioctl(fd, DISP_IOCTL_TRIGGER_SESSION, &config);
    	printf("8.%d trigger session, ret = %d\n",i, ret);
    
    	
    
    	// 9. free buffer and close fd
    	ret = sync_wait(p_input->fence_fd, TIMEOUT_NEVER);
    	printf("9.%d wait fence(%d) release, ret = %d\n", i,p_input->fence_fd, ret);	
    	ret = sync_wait(p_output->fence_fd, TIMEOUT_NEVER);
    	printf("     wait fence(%d) release, ret = %d\n", p_output->fence_fd, ret);
    	free_buffer(p_input);
    	free_buffer(p_output);

	}
	
	// 10. destroy session	
	ret = ioctl(fd, DISP_IOCTL_DESTROY_SESSION, &config);
	printf("10. destroy session for sub display, session_id = %d, ret = %d\n", config.session_id, ret);
	

	// 13. close driver
	close(fd);


	// resolution and fps bonding test
	system("echo efuse_test>/d/mtkfb");

	printf("****************************************************************\n");
}
int ut_efuse_test()
{
	int ret = 0 ;
	system("echo _efuse_test>/d/mtkfb");
	printf("ut_efuse_test\n");
	return ret;
}
int main(int argc, char **argv)
{
#if 0
	// blank SurfaceFlinger
	sp<SurfaceComposerClient> client = new SurfaceComposerClient();
	sp<IBinder> display = SurfaceComposerClient::getBuiltInDisplay(
	ISurfaceComposer::eDisplayIdMain);
	SurfaceComposerClient::blankDisplay(display);
	
	pthread_t trigger_thread;
	pthread_attr_t trigger_thread_attr;
	pthread_attr_init(&trigger_thread_attr);
	
	pthread_t test_thread;
	pthread_attr_t test_thread_attr;
	pthread_attr_init(&test_thread_attr);
	memset((void*)&gsession_input_config, 0, sizeof(disp_session_input_config));
	pthread_mutexattr_t input_mutex_attr;
	pthread_mutexattr_init(&input_mutex_attr);
	pthread_mutex_init(&input_mutex, &input_mutex_attr);

	pthread_attr_setschedpolicy(&trigger_thread_attr, SCHED_FIFO);
	pthread_create(&trigger_thread, &trigger_thread_attr, trigger_loop_routine, (void*)("session_trigger"));

	pthread_create(&test_thread, &test_thread_attr, test_loop_routine0, (void*)("session_test"));
	
	int loop = 60;
	if(argc > 1)
	{
		sscanf(argv[1], "%d", &loop);
	}
	sleep(loop);
	
	SurfaceComposerClient::unblankDisplay(display);

	return 0;// test_case_1(argc, argv);
#else

	// blank SurfaceFlinger
	sp<SurfaceComposerClient> client = new SurfaceComposerClient();
	sp<IBinder> display = SurfaceComposerClient::getBuiltInDisplay(ISurfaceComposer::eDisplayIdMain);
	SurfaceComposerClient::blankDisplay(display);

  	int case_num = 0;
	if(argc > 1)
	{
		sscanf(argv[1],"%d", &case_num);
		switch (case_num)
		{
			case 1:
				ut_case_1(0,0);
				break;
			case 2:
				ut_case_2(0,0);
				break;
			case 3:
				ut_case_3(0,0);
				break;
			case 4:
				ut_case_4(0,0);
				break;
			case 5:
				ut_case_5(0,0);
				break;
			case 6:
				ut_case_6();
				break;
			case 7:	
				ut_efuse_test();
				break;	
			default:
				ut_case_1(0,0);
				ut_case_3(0,0);
				break;
				
		}

		
		//printf("case_num = %d, argc = %d, argv[0] = %s, argv[1] = %s\n", case_num,argc, argv[0], argv[1]);
	}
	else
	{
		ut_case_3(argc, argv);
	}

	SurfaceComposerClient::unblankDisplay(display);
	
	return 0;
#endif
}
