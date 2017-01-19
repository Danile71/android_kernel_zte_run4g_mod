
#ifndef __GUIEXT_AUX_H__
#define __GUIEXT_AUX_H__

#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include <system/window.h>
#include <system/graphics.h>

__BEGIN_DECLS

struct GuiExtAuxBufferItem;
struct GuiExtAuxBufferQueue;
typedef struct GuiExtAuxBufferItem * GuiExtAuxBufferItemHandle;
typedef struct GuiExtAuxBufferQueue * GuiExtAuxBufferQueueHandle;

#define GpuAuxBufferItemHandle GuiExtAuxBufferItemHandle
#define GpuAuxBufferQueueHandle GuiExtAuxBufferQueueHandle

int GuiExtAuxIsSupportFormat(android_native_buffer_t * buf);

GuiExtAuxBufferQueueHandle GuiExtAuxCreateBufferQueue(int width, int height, int output_format, int num_max_buffer);
void GuiExtAuxDestroyBufferQueue(GuiExtAuxBufferQueueHandle bq);

void GuiExtAuxKickConversion(GuiExtAuxBufferQueueHandle bq, android_native_buffer_t * src_buffer);
int GuiExtAuxDoConversionIfNeed(GuiExtAuxBufferQueueHandle bq, android_native_buffer_t * dst_buffer, android_native_buffer_t * src_buffer);

void GuiExtAuxSetSize(GuiExtAuxBufferQueueHandle bq, int width, int height);
void GuiExtAuxSetName(GuiExtAuxBufferQueueHandle bq, const char * name);

int GuiExtAuxAcquireBuffer(GuiExtAuxBufferQueueHandle bq, int *bufSlot, int * fence_fd);
int GuiExtAuxReleaseBuffer(GuiExtAuxBufferQueueHandle bq, int bufSlot, int fence_fd);
int GuiExtAuxDequeueBuffer(GuiExtAuxBufferQueueHandle bq, int * bufSlot, int * fence_fd);
int GuiExtAuxQueueBuffer(GuiExtAuxBufferQueueHandle bq, int bufSlot, int fence_fd);

android_native_buffer_t * GuiExtAuxRequestBuffer(GuiExtAuxBufferQueueHandle bq, int bufSlot);

__END_DECLS

#endif

