#include <linux/interrupt.h>
#include <asm/barrier.h>
#include <mach/mt_reg_base.h>
#include <mach/md32_helper.h>
#include <mach/md32_ipi.h>


struct ipi_desc ipi_desc[MD32_NR_IPI];
struct share_obj *md32_send_obj, *md32_rcv_obj;
struct mutex md32_ipi_mutex;

extern unsigned char *md32_send_buff;
extern unsigned char *md32_recv_buff;

/*
  @param id:       If NR_IPI, only return the IPI register status,
  otherwise return the specific id's IPI status
*/
#if 0
ipi_status md32_ipi_status(ipi_id id)
{
    if(HOST_TO_MD32_REG & MD32_IPC_INT){
        if(id == NR_IPI || id == md32_share_obj->id)
            return BUSY;
        else
            return DONE;
    } else {
        return DONE;
    }
}
#endif

static void ipi_md2host(void)
{
    HOST_TO_MD32_REG = 0x1;
}

void md32_ipi_handler(void)
{
    if(ipi_desc[md32_rcv_obj->id].handler)
    {
        memcpy_from_md32(md32_recv_buff, (void *)md32_rcv_obj->share_buf, md32_rcv_obj->len);
        ipi_desc[md32_rcv_obj->id].handler(md32_rcv_obj->id, md32_recv_buff, md32_rcv_obj->len);
    }
    MD32_TO_SPM_REG = 0x0;
}

void md32_ipi_init(void)
{
    mutex_init(&md32_ipi_mutex);
    md32_rcv_obj = MD32_DTCM;
    md32_send_obj = md32_rcv_obj + 1;
    printk("md32_rcv_obj = %x \n", md32_rcv_obj);
    memset(md32_send_obj, 0, SHARE_BUF_SIZE);
}

/*
  @param id:       IPI ID
  @param handler:  IPI handler
  @param name:     IPI name
*/
ipi_status md32_ipi_registration( ipi_id id, ipi_handler_t handler, const char *name)
{
    if(id < MD32_NR_IPI){
        ipi_desc[id].name = name;

        if(handler == NULL)
            return ERROR;

        ipi_desc[id].handler = handler;
        return DONE;
    } else{
        return ERROR;
    }
}

/*
  @param id:       IPI ID
  @param buf:      the pointer of data
  @param len:      data length
  @param wait:     If true, wait (atomically) until data have been gotten by Host
*/
ipi_status md32_ipi_send( ipi_id id, void* buf, unsigned int  len, unsigned int wait)
{
    if(id < MD32_NR_IPI) {
        if(len > sizeof(md32_send_obj->share_buf) || buf == NULL)
            return ERROR;

        if(HOST_TO_MD32_REG)
            return BUSY;

        mutex_lock(&md32_ipi_mutex);


        if(HOST_TO_MD32_REG)
        {
            mutex_unlock(&md32_ipi_mutex);
            return BUSY;
        }    

        memcpy(md32_send_buff, buf, len);

        memcpy_to_md32((void *)md32_send_obj->share_buf, md32_send_buff, len);
        md32_send_obj->len = len;
        md32_send_obj->id = id;
        dsb();
        ipi_md2host();

        if(wait)
            while(HOST_TO_MD32_REG);

        mutex_unlock(&md32_ipi_mutex);
    }else
        return ERROR;

    return DONE;
}

