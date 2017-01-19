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
#include <linux/disp_session.h>

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

#include <gui/ISurfaceComposer.h>
#include <gui/SurfaceComposerClient.h>

#include <pthread.h>

//#pragma GCC optimize ("O0")
using namespace android;

typedef struct {
    int ion_fd;
    int ion_shardfd;
    struct ion_handle * handle;
    void* buffer ;
    int w;
    int h;
    int tgt_x;
    int tgt_y;
    int layer_en;
    int fence_fd;
    int fence_idx;

}layer_buf_t;

typedef struct {
    layer_buf_t **layer_buf;
    int layer_num;
    unsigned int id;
}frame_buf_t;

#define QUEUE_END_PTR   ((void*)1)

struct buf_queue {
    void **data;
    int head;
    int tail;
    int num;
    pthread_mutex_t lock;

};

struct buf_queue *prepare_buf_Q;
struct buf_queue *release_buf_Q;

static int test_input_layer_num = 7;
static int test_layer_width = 64;
static int test_layer_height = 128;
static int test_frame_cnt = 60;
static int force_primary_layer_num = 0;
static int force_mem_layer_num = 0;

struct buf_queue* buf_queue_create(int entry_num)
{
    struct buf_queue *q;
    q = (struct buf_queue*)calloc(1, sizeof(*q));

    q->data = (void**)calloc(1, entry_num * sizeof(void*));
    q->head = q->tail = 0;
    q->num = entry_num;
    
	pthread_mutexattr_t input_mutex_attr;
	pthread_mutexattr_init(&input_mutex_attr);
	pthread_mutex_init(&q->lock, &input_mutex_attr);

    return q;
}

int inqueue(struct buf_queue *q, void *ptr)
{
    int tmp;
	pthread_mutex_lock(&q->lock);

    tmp = q->head+1;
    tmp %= q->num;
    if(tmp == q->tail)
    {
        //printf("queue is full! head=%d, tail=%d\n", q->head, q->tail);
    	pthread_mutex_unlock(&q->lock);
        return -1;
    }

    q->data[q->head] = ptr;
    q->head = tmp;
    
	pthread_mutex_unlock(&q->lock);
    return 0;
}

void *dequeue(struct buf_queue *q)
{
    void* ptr;
	pthread_mutex_lock(&q->lock);

    if(q->tail == q->head)
    {
        //printf("queue is empty !! head=tail=%d\n", q->tail);
    	pthread_mutex_unlock(&q->lock);
        return NULL;
    }

    ptr = q->data[q->tail];
    q->tail++;
    q->tail %= q->num;

	pthread_mutex_unlock(&q->lock);

    return ptr;
}


layer_buf_t *alloc_layer_buffer(int layer_en, int w, int h, int tgt_x, int tgt_y)
{
    unsigned int size = w*h*4;
	int ion_fd = mt_ion_open("disp_session_test");

	layer_buf_t *p = (layer_buf_t *)malloc(sizeof(layer_buf_t));
	if(p == NULL) 
	{
		return NULL;
	}

    if(layer_en)
    {
    	memset((void*)p, 0, sizeof(layer_buf_t));

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
    	p->buffer = ion_mmap(ion_fd, NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, share_fd, 0);
    }
    p->w = w;
    p->h = h;
    p->tgt_x = tgt_x;
    p->tgt_y = tgt_y;
    p->layer_en = layer_en;
    p->fence_fd = -1;
    p->fence_idx = -1;
	return p;
}

void free_layer_buffer(layer_buf_t *p)
{
	if(p && p->layer_en)
	{
		ion_munmap(p->ion_fd, p->buffer , p->w * p->h * 4);
		ion_free(p->ion_fd, p->handle);
		ion_share_close(p->ion_fd, p->ion_shardfd);
		ion_close(p->ion_fd);
        if(p->fence_fd != -1)
    		close(p->fence_fd);
	}
	free(p);
}

void draw_layer_buf(layer_buf_t *layer_buf, int frame_id, int layer)
{
    int i, j;
    char r, g, b, a;

    r = rand() % 255;
    g = rand() % 255;
    b = rand() % 255;
    a = 255;
    
    char *data = (char*)layer_buf->buffer;
    for(i=0; i<layer_buf->h; i++)
    {
        for(j=0; j<layer_buf->w; j++)
        {
            int x = j*4 + i*layer_buf->w*4;
            data[x++] = r;
            data[x++] = g;
            data[x++] = b;
            data[x++] = a;
        }
    }
}
frame_buf_t* alloc_frame_buffer(int layer_num)
{
    static unsigned int gFrame_cnt=0;
    frame_buf_t * frame_buf;
    layer_buf_t * layer_buf;
    frame_buf = (frame_buf_t*)calloc(1, sizeof(*frame_buf));
    int i;
    int w=test_layer_width, h=test_layer_height;

    frame_buf->layer_buf = (layer_buf_t**)calloc(layer_num, sizeof(layer_buf_t*));
    for(i=0; i<layer_num; i++)
    {
        layer_buf = alloc_layer_buffer(1, w, h, w*i/2, h*i/2);
        draw_layer_buf(layer_buf, frame_buf->id, i);
        frame_buf->layer_buf[i] = layer_buf;
    }
    frame_buf->layer_num = layer_num;
    frame_buf->id = gFrame_cnt++;
    return frame_buf;
}

void free_frame_buffer(frame_buf_t *frame_buf)
{
    int i;
    for(i=0; i<frame_buf->layer_num; i++)
    {
        if(frame_buf->layer_buf[i])
        {
            free_layer_buffer(frame_buf->layer_buf[i]);
            frame_buf->layer_buf[i] = NULL;
        }
    }
    free(frame_buf->layer_buf);
    free(frame_buf);
}

#define  TIMEOUT_NEVER (-1)
void wait_frame_buf_done(frame_buf_t *frame_buf)
{
    int i;
    for(i=0; i<frame_buf->layer_num; i++)
    {
        if(frame_buf->layer_buf[i] 
            && frame_buf->layer_buf[i]->layer_en 
            && (frame_buf->layer_buf[i]->fence_fd != -1)
            )
    		sync_wait(frame_buf->layer_buf[i]->fence_fd, TIMEOUT_NEVER);
    }
}

void session_prepare_buf(int fd, unsigned int session_id, 
    layer_buf_t *layer_buf, int layer_id, int input)
{
	disp_buffer_info buffer;

	memset(&buffer, 0, sizeof(disp_buffer_info));
	buffer.session_id = session_id;
	buffer.layer_id   = layer_id;
	buffer.layer_en   = layer_buf->layer_en;
	buffer.ion_fd     = layer_buf->ion_shardfd;
	buffer.cache_sync = 1;
    if(input)
    	ioctl(fd, DISP_IOCTL_PREPARE_INPUT_BUFFER, &buffer);
    else
    	ioctl(fd, DISP_IOCTL_PREPARE_OUTPUT_BUFFER, &buffer);

	layer_buf->fence_idx = buffer.index;
	layer_buf->fence_fd = buffer.fence_fd;
	//printf("prepare session=%d idx=%d, layer=%d, %s\n", session_id,
    //    layer_buf->fence_idx, layer_id, input?"input":"output");
}

void session_setinput(int fd, unsigned int session_id,
                frame_buf_t *frame_buf, int layer_num)
{
    disp_session_input_config input_config;

    memset(&input_config, 0, sizeof(input_config));

	input_config.config_layer_num = layer_num;
    input_config.session_id = session_id;

    for(int i=0; i<layer_num; i++)
    {
    	input_config.config[i].layer_id = i;
    	input_config.config[i].layer_enable = frame_buf->layer_buf[i]->layer_en;
    	input_config.config[i].src_base_addr = 0;
    	input_config.config[i].src_phy_addr = 0;
    	input_config.config[i].next_buff_idx = frame_buf->layer_buf[i]->fence_idx;
    	input_config.config[i].src_fmt = DISP_FORMAT_RGBA8888;
    	input_config.config[i].src_pitch = frame_buf->layer_buf[i]->w;
    	input_config.config[i].src_offset_x = 0; 
    	input_config.config[i].src_width = frame_buf->layer_buf[i]->w;

    	input_config.config[i].src_offset_y = 0;
    	input_config.config[i].src_height = frame_buf->layer_buf[i]->h;

    	input_config.config[i].tgt_offset_x = frame_buf->layer_buf[i]->tgt_x;
    	input_config.config[i].tgt_offset_y = frame_buf->layer_buf[i]->tgt_y;
    	input_config.config[i].tgt_width = frame_buf->layer_buf[i]->w;
    	input_config.config[i].tgt_height = frame_buf->layer_buf[i]->h;
    	input_config.config[i].alpha_enable= 1;
        input_config.config[i].alpha= 0xff;
    }

	
	if(ioctl(fd, DISP_IOCTL_SET_INPUT_BUFFER, &input_config) < 0)
	{
   	 	printf("ioctl to set multi overlay layer enable fail\n");
	}
}

void session_setoutput(int fd, unsigned int session_id,
                layer_buf_t *out)
{
    disp_session_output_config out_cfg;
    memset(&out_cfg, 0, sizeof(out_cfg)); 
    
    out_cfg.session_id = session_id;
    out_cfg.config.va = (unsigned int)out->buffer;
    out_cfg.config.pa = 0;
    out_cfg.config.fmt= DISP_FORMAT_RGBA8888;
    out_cfg.config.x = 0;
    out_cfg.config.y = 0;
    out_cfg.config.width= out->w;
    out_cfg.config.height= out->h;
    out_cfg.config.pitch= out->w;
    out_cfg.config.buff_idx= out->fence_idx;

	if(ioctl(fd, DISP_IOCTL_SET_OUTPUT_BUFFER, &out_cfg) < 0)
	{
   	 	printf("ioctl to set multi overlay layer enable fail\n");
	}
}

void session_trigger(int fd, disp_session_config *config)
{
    int ret;
	if((ret = ioctl(fd, DISP_IOCTL_TRIGGER_SESSION, config)) <0)
	{
		printf("trigger fail:%d\n", ret);
	}
}

int cascade_layer_input(int fd, frame_buf_t *frame_buf, int prim_layer_num, int mem_layer_num,
            	disp_session_config *mem_config, int disp_w, int disp_h)
{
    while(frame_buf->layer_num > prim_layer_num)
    {
        int layer, input_num;
        //prepare mem input
        input_num = mem_layer_num;
        if(input_num > frame_buf->layer_num)
            input_num = frame_buf->layer_num;
        for(layer=0; layer<input_num; layer++)
        {
            session_prepare_buf(fd, mem_config->session_id, frame_buf->layer_buf[layer], layer, 1);
        }

        //prepare mem output
        layer_buf_t *out_buf;
        out_buf = alloc_layer_buffer(1, disp_w, disp_h, 0, 0);
        session_prepare_buf(fd, mem_config->session_id, out_buf, 0, 0);

        //set input
        session_setinput(fd,mem_config->session_id, frame_buf, input_num);
        //set output
        session_setoutput(fd, mem_config->session_id, out_buf);
        //trigger mem session
        session_trigger(fd, mem_config);
        //wait done
		sync_wait(out_buf->fence_fd, TIMEOUT_NEVER);
        close(out_buf->fence_fd);
        out_buf->fence_fd = -1;

        //update frame_buf
        for(layer=0; layer<input_num; layer++)
        {
            //free orignal input layer
            free_layer_buffer(frame_buf->layer_buf[layer]);
            frame_buf->layer_buf[layer] = NULL;
        }
        
        frame_buf->layer_buf[0] = out_buf;
        //move up remained layers
        for(layer=input_num; layer<frame_buf->layer_num; layer++)
        {
            frame_buf->layer_buf[layer-input_num+1] = frame_buf->layer_buf[layer];
            frame_buf->layer_buf[layer] = NULL;
        }
        frame_buf->layer_num -= input_num-1;

        printf("cascade %d layers using mem sesstion, %d remained\n", input_num, frame_buf->layer_num);
    }
    return 0;
}

void prepare_frame_buf(frame_buf_t* frame_buf)
{
    int layer;
	disp_session_input_config *ps = NULL;
    int i, j, ret;
	int fd = open("/dev/mtk_disp_mgr", O_RDWR);
	disp_session_vsync_config vsync_config;
	disp_session_config primary_config, mem_config;
	disp_session_info device_info;
    int prim_layer_num, mem_layer_num;

	primary_config.type = DISP_SESSION_PRIMARY;
	primary_config.device_id = 0;
	ret = ioctl(fd, DISP_IOCTL_CREATE_SESSION, &primary_config);
    
	mem_config.type = DISP_SESSION_MEMORY;
	mem_config.device_id = 0;
	ret = ioctl(fd, DISP_IOCTL_CREATE_SESSION, &mem_config);

	device_info.session_id = primary_config.session_id;
	ret = ioctl(fd, DISP_IOCTL_GET_SESSION_INFO, &device_info);
    prim_layer_num = force_primary_layer_num ? force_primary_layer_num : device_info.maxLayerNum;

	device_info.session_id = mem_config.session_id;
	ret = ioctl(fd, DISP_IOCTL_GET_SESSION_INFO, &device_info);
    mem_layer_num = force_mem_layer_num ? force_mem_layer_num : device_info.maxLayerNum;

    //reduce layer num for primary display using mem session
    cascade_layer_input(fd, frame_buf, prim_layer_num, mem_layer_num, &mem_config,
                        device_info.displayWidth, device_info.displayHeight);

    //prepare primary input
    for(layer=0; layer<frame_buf->layer_num; layer++)
    {
        session_prepare_buf(fd, primary_config.session_id, 
            frame_buf->layer_buf[layer], layer, 1);
    }
    close(fd);
}

/* producer thread
 * produce frames and prepare them to display.
 * if layer_num > primary_session_layer_num, 
 * memory session will be used to compose some layers.
 * It put created frames to prepare_buf_Q
 */
void buffer_producer(int loop)
{
	unsigned int vsync_cnt = 0;
	int ret = -1, i;
	int fd = -1;
    frame_buf_t* frame_buf;
    int buf_width, buf_height;
	
	fd = open("/dev/mtk_disp_mgr", O_RDWR);
	disp_session_vsync_config vsync_config;
	disp_session_config config;

	config.type = DISP_SESSION_PRIMARY;
	config.device_id = 0;
	ret = ioctl(fd, DISP_IOCTL_CREATE_SESSION, &config);
    vsync_config.session_id = config.session_id;

	for(i=0; i<loop; i++)
	{
		ioctl(fd, DISP_IOCTL_WAIT_FOR_VSYNC, &vsync_config);
        frame_buf = alloc_frame_buffer(test_input_layer_num);
        prepare_frame_buf(frame_buf);
        
        while(inqueue(prepare_buf_Q, frame_buf)!=0)
    		ioctl(fd, DISP_IOCTL_WAIT_FOR_VSYNC, &vsync_config);
        printf("<1> produced frame %u\n", frame_buf->id);
    }

    while(inqueue(prepare_buf_Q, QUEUE_END_PTR)!=0)
		ioctl(fd, DISP_IOCTL_WAIT_FOR_VSYNC, &vsync_config);

    close(fd);
	return;
}


/* displayer thread
 * dequeue a frame from prepare_buf_Q,
 * and then set to display driver, and trigger display.
 * After triggered, it put this frame into release_buf_Q for wait & release.
 */
void *buffer_displayer(void *unused)
{
	unsigned int vsync_cnt = 0;
	int ret = -1, i;
	int fd = -1;
    frame_buf_t* frame_buf;
	
	fd = open("/dev/mtk_disp_mgr", O_RDWR);
	disp_session_vsync_config vsync_config;
	disp_session_config primary_config;

	primary_config.type = DISP_SESSION_PRIMARY;
	primary_config.device_id = 0;
	ret = ioctl(fd, DISP_IOCTL_CREATE_SESSION, &primary_config);
    vsync_config.session_id = primary_config.session_id;

    while(1)
    {
		ioctl(fd, DISP_IOCTL_WAIT_FOR_VSYNC, &vsync_config);
        frame_buf = (frame_buf_t *)dequeue(prepare_buf_Q);
        if(!frame_buf)
            continue;
        else if(frame_buf == (frame_buf_t *)QUEUE_END_PTR)
            break;

        session_setinput(fd, primary_config.session_id, frame_buf, frame_buf->layer_num); 
        session_trigger(fd, &primary_config);

        while(inqueue(release_buf_Q, frame_buf)!=0)
    		ioctl(fd, DISP_IOCTL_WAIT_FOR_VSYNC, &vsync_config);
        
        printf("<1> display frame %u (%d layers)\n", frame_buf->id, frame_buf->layer_num);
	}
    while(inqueue(release_buf_Q, QUEUE_END_PTR)!=0)
		ioctl(fd, DISP_IOCTL_WAIT_FOR_VSYNC, &vsync_config);

    close(fd);
	return NULL;
}
/* buffer cleaner thread
 * It dequeue frame from release_buf_Q, wait it's fence (for display done)
 * then free it.
 */
void *buffer_cleanner(void *unused)
{
	int ret = -1, i;
	int fd = -1;
    frame_buf_t* frame_buf;
	
	fd = open("/dev/mtk_disp_mgr", O_RDWR);
	disp_session_vsync_config vsync_config;
	disp_session_config primary_config;

	primary_config.type = DISP_SESSION_PRIMARY;
	primary_config.device_id = 0;
	ret = ioctl(fd, DISP_IOCTL_CREATE_SESSION, &primary_config);
    vsync_config.session_id = primary_config.session_id;

    while(1)
    {
		ioctl(fd, DISP_IOCTL_WAIT_FOR_VSYNC, &vsync_config);
        frame_buf = (frame_buf_t *)dequeue(release_buf_Q);
        if(!frame_buf)
            continue;
        else if(frame_buf == (frame_buf_t *)QUEUE_END_PTR)
            break;
        wait_frame_buf_done(frame_buf);
        printf("<1> release frame %u\n", frame_buf->id);
        free_frame_buffer(frame_buf);
	}
    close(fd);
	return NULL;
}

static void parse_argument(int argc, char**argv)
{
    int ch, opt;

    #define print_usage() do{\
        printf("usage: %s\n"\
            " [-w layer_width]"\
            " [-h layer_height]"\
            " [-l layer_num]"\
            " [-f frame_num]"\
            " [-p primary_session_layer_num]"\
            " [-m memory_session_layer_num]"\
            , argv[0]);\
       }while(0)
    
    while((opt = getopt(argc, argv, "w:h:l:f:p:m:")) != -1)
    {
        switch(opt)
        {
            case 'w':
                test_layer_width = atoi(optarg);
                break;
            case 'h':
                test_layer_height = atoi(optarg);
                break;
            case 'l':
                test_input_layer_num = atoi(optarg);
                break;
            case 'f':
                test_frame_cnt = atoi(optarg);
                break;
            case 'p':
                force_primary_layer_num = atoi(optarg);
                break;
            case 'm':
                force_mem_layer_num = atoi(optarg);
                break;
            default:
                print_usage();
                exit(0);
        }
    }
}

int main(int argc, char **argv)
{
    parse_argument(argc, argv);
	// blank SurfaceFlinger
	sp<SurfaceComposerClient> client = new SurfaceComposerClient();
	sp<IBinder> display = SurfaceComposerClient::getBuiltInDisplay(
	ISurfaceComposer::eDisplayIdMain);
	SurfaceComposerClient::blankDisplay(display);

    prepare_buf_Q = buf_queue_create(64);
    release_buf_Q = buf_queue_create(64);

    pthread_t display_thread;
	pthread_create(&display_thread, NULL, buffer_displayer, NULL);

    pthread_t clean_thread;
	pthread_create(&clean_thread, NULL, buffer_cleanner, NULL);

    buffer_producer(test_frame_cnt);
	SurfaceComposerClient::unblankDisplay(display);

    void* status;
    pthread_join(display_thread, &status);
    pthread_join(clean_thread, &status);
    
	return 0;// test_case_1(argc, argv);
}
