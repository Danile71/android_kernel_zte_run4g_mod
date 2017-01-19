#define LOG_TAG "GuiExt-Test"

#include <string.h>
#include <cutils/xlog.h>
#include <hardware/gralloc.h>

#include <gui/BufferQueue.h>
#include <gui/GraphicBufferAlloc.h>

#include "GuiExtAux.h"

using namespace android;

void GPU_AUX_test()
{
    int w = 124;
    int h = 124;

    const int output_format = HAL_PIXEL_FORMAT_YV12;
    const int num_bufferQueue = 3;

    printf(" === AUX create BufferQueue \n");
    GuiExtAuxBufferQueueHandle bq = GuiExtAuxCreateBufferQueue(w, h, HAL_PIXEL_FORMAT_YV12, num_bufferQueue);

    sp<GraphicBuffer> src_gb = new android::GraphicBuffer(
            w, h, HAL_PIXEL_FORMAT_I420,
            GraphicBuffer::USAGE_HW_TEXTURE | GRALLOC_USAGE_SW_WRITE_OFTEN);

    printf(" AUX src_gb = %p \n", src_gb->getNativeBuffer());

    int dst_id = -1;
    int fence_fd = -1;

    /* run 10 iterations */
    for (int i = 0; i < 10; ++i)
    {
        if (dst_id != -1)
        {
            /* wait & close fence */
            if (fence_fd >= 0)
                close(fence_fd);

            GuiExtAuxReleaseBuffer(bq, dst_id, -1);
            printf(" AUX releaseBuffer id=%d \n", dst_id);
        }

        printf(" AUX kickConversion \n");
        GuiExtAuxKickConversion(bq, src_gb->getNativeBuffer());

        int new_bufer_id;
        int new_fence_fd;
        GuiExtAuxAcquireBuffer(bq, &new_bufer_id, &new_fence_fd);

        android_native_buffer_t * dst_buffer = GuiExtAuxRequestBuffer(bq, new_bufer_id);

        printf(" AUX acquireBuffer id=%d \n", new_bufer_id);

        dst_id = new_bufer_id;
        fence_fd = new_fence_fd;
    }

    /* clean up the last buffer */
    if (fence_fd >= 0)
        close(fence_fd);
    if (dst_id != -1)
        GuiExtAuxReleaseBuffer(bq, dst_id, -1);

    /* destory bufferqueue */
    GuiExtAuxDestroyBufferQueue(bq);
    printf(" === AUX destroy BufferQueue ================= \n");
}

int main(int argc, char *argv[])
{
	int i = 100;

    if (argc >=2) i = atoi(argv[1]);

	while (--i)
	{
		GPU_AUX_test();
	}

	printf("end\n");
	scanf("%d", &i);

	return 0;
}

