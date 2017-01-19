
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <uree/system.h>
#include <uree/mem.h>
#include <uree/dbg.h>
#include <tz_cross/ta_test.h>
#include <tz_cross/ta_mem.h>
#include <tz_cross/ta_dbg.h>
#include <pthread.h>
#include <unistd.h>
#include <linux/smp.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/ioctl.h>


static UREE_SESSION_HANDLE dbg_session;


void error_exit (char *s)
{
    printf ("error_exit = %s\n", s);

    while (1)
        ;
}

void error_exit_1 (char *s)
{
    printf ("error_exit = %s\n", s);

    while (1)
        ;
}


int check_add_mem_test (int *buf_p, int test_result, uint32_t test_offset, uint32_t test_size, int count)
{
    int result = 0;

    for (; count != 0; count --)
    {
        result += *buf_p ++;
    }

    if (result != test_result)
    {
        return result;
    }

    return 0;
}


#define minor_log


int do_add_mem_test (pid_t pid, uint32_t id, UREE_SESSION_HANDLE mem_session, uint32_t shm_size, uint32_t test_offset, uint32_t test_size)
{
    uint32_t *shm_p;
    UREE_SHAREDMEM_PARAM shm_param;
    TZ_RESULT ret;
    UREE_SHAREDMEM_HANDLE shm_handle = 0;
    int i;
    UREE_SESSION_HANDLE session;
    MTEEC_PARAM param[4];

#ifndef minor_log
    printf ("%d >>===> [%d] test add mem <====\n", pid, id);
#endif

    ret = UREE_CreateSession(TZ_TA_TEST_UUID, &session);
    if (ret != TZ_RESULT_SUCCESS)
    {
        printf("    [%d] CreateSession Error: 0x%x, %d\n", id, (uint32_t) session, ret);
        error_exit_1 ("do_add_mem_test");
    }

    shm_p = (uint32_t *)malloc(shm_size);

    shm_param.buffer = shm_p;
    shm_param.size = shm_size;
    ret = UREE_RegisterSharedmem(mem_session, &shm_handle, &shm_param);
    if (ret != TZ_RESULT_SUCCESS)
    {
        printf("    [%d] UREE_RegisterSharedmem Error: %s\n", id, TZ_GetErrorString(ret));
        error_exit_1 ("do_add_mem_test");
    }
#ifndef minor_log    
    printf ("    [%d] shm handle = 0x%x\n", id, shm_handle);
#endif

    for (i = 0; i < (int) shm_size/4; i ++)
    {
        shm_p[i] = i;
    }

    param[0].memref.handle = (uint32_t) shm_handle;
    param[0].memref.offset = test_offset;
    param[0].memref.size = test_size;
    param[1].value.a = test_size / 4;
    ret = UREE_TeeServiceCall(session, TZCMD_TEST_ADD_MEM,
            TZ_ParamTypes3(TZPT_MEMREF_INPUT, TZPT_VALUE_INPUT, TZPT_VALUE_OUTPUT), param);   
    if (ret != TZ_RESULT_SUCCESS)
    {
        printf("    [%d] TZCMD_TEST_ADD_MEM Error: %s\n", id, TZ_GetErrorString(ret));
        error_exit_1 ("do_add_mem_test");
    }        
    ret = check_add_mem_test ((int *) shm_p, (int) param[2].value.a, test_offset, test_size, (int) test_size / 4);
#ifndef minor_log    
    printf ("    [%d] ADD MEM result = 0x%x\n", id, param[2].value.a);
#endif    
    if (ret != 0)
    {
        printf("    [%d] test add mem result NG: 0x%x\n", id, ret);
        error_exit ("do_add_mem_test");
    }

    ret = UREE_UnregisterSharedmem(mem_session, shm_handle);        
    if (ret != TZ_RESULT_SUCCESS)
    {
        printf("    [%d] UREE_UnregisterSharedmem Error: %s\n", id, TZ_GetErrorString(ret));
        error_exit_1 ("do_add_mem_test");
    }

    ret = UREE_CloseSession(session);
    if (ret != TZ_RESULT_SUCCESS)
    {
        printf("    [%d] CloseSeesion Error: 0x%x, %d\n", id, (uint32_t) session, ret);
        error_exit_1 ("do_add_mem_test");
    }

    free (shm_p);

#ifndef minor_log
    printf ("%d >>===> [%d] test add mem <====\n\n", pid, id);
#endif

    return 0;
}

int do_securefunc_test (pid_t pid, uint32_t id)
{
    UREE_SESSION_HANDLE session;
    MTEEC_PARAM param[4];
    TZ_RESULT ret;

#ifndef minor_log
    printf ("%d >>===> [%d] test secure function <====\n", pid, id);
#endif

    ret = UREE_CreateSession(TZ_TA_TEST_UUID, &session);
    if (ret != TZ_RESULT_SUCCESS)
    {
        printf("    [%d] CreateSession Error: 0x%x, %d\n", id, (uint32_t) session, ret);
        error_exit_1 ("do_securefunc_test");
    }

    ret = UREE_TeeServiceCall(session, TZCMD_TEST_SECUREFUNC,
            0, param);   
    if (ret != TZ_RESULT_SUCCESS)
    {
        printf("    [%d] TZCMD_TEST_SECUREFUNC Error: %s\n", id, TZ_GetErrorString(ret));
        error_exit_1 ("do_securefunc_test");
    }        
    
    ret = UREE_CloseSession(session);
    if (ret != TZ_RESULT_SUCCESS)
    {
        printf("    [%d] CloseSeesion Error: 0x%x, %d\n", id, (uint32_t) session, ret);
        error_exit_1 ("do_securefunc_test");
    }

#ifndef minor_log
    printf ("%d >>===> [%d] test secure function  <====\n\n", pid, id);
#endif

    return 0;
}

int do_thread_test (pid_t pid, uint32_t id)
{
    UREE_SESSION_HANDLE session;
    MTEEC_PARAM param[4];
    TZ_RESULT ret;
    uint32_t result;

#ifndef minor_log
    printf ("%d >>===> [%d] test thread <====\n", pid, id);
#endif

    ret = UREE_CreateSession(TZ_TA_TEST_UUID, &session);
    if (ret != TZ_RESULT_SUCCESS)
    {
        printf("    [%d] CreateSession Error: 0x%x, %d\n", id, (uint32_t) session, ret);
        error_exit_1 ("do_thread_test");
    }

    param[0].value.a = id;
    ret = UREE_TeeServiceCall(session, TZCMD_TEST_THREAD,
            TZ_ParamTypes2(TZPT_VALUE_INPUT, TZPT_VALUE_OUTPUT), param);   
    if (ret != TZ_RESULT_SUCCESS)
    {
        printf("    [%d] TZCMD_TEST_THREAD Error: %s\n", id, TZ_GetErrorString(ret));
        error_exit_1 ("do_thread_test");
    }    
    result = param[1].value.a;
    if (result != id)
    {
        printf("    [%d] result Error: NG = 0x%x, OK = 0x%x\n", id, (uint32_t) result, id);
        error_exit_1 ("do_thread_test");
    }
    
    ret = UREE_CloseSession(session);
    if (ret != TZ_RESULT_SUCCESS)
    {
        printf("    [%d] CloseSeesion Error: 0x%x, %d\n", id, (uint32_t) session, ret);
        error_exit_1 ("do_thread_test");
    }

#ifndef minor_log
    printf ("%d >>===> [%d] test thread done  <====\n\n", pid, id);
#endif

    return 0;
}



int do_do_A_B_cm_test (pid_t pid, uint32_t id, UREE_SESSION_HANDLE mem_session, uint32_t mem_size, uint32_t test_align, uint32_t test_size)
{
    TZ_RESULT ret;
    UREE_SECURECM_HANDLE mem_handle = 0;
    UREE_SESSION_HANDLE session;
    MTEEC_PARAM param[4];

#ifndef minor_log
    printf ("%d >>===> [%d] test do A B <====\n", pid, id);
#endif

    ret = UREE_CreateSession(TZ_TA_TEST_UUID, &session);
    if (ret != TZ_RESULT_SUCCESS)
    {
        printf("    [%d] CreateSession Error: 0x%x, %d\n", id, (uint32_t) session, ret);
        error_exit_1 ("do_do_A_B_test");
    }

    ret = UREE_AllocSecurechunkmem (mem_session, &mem_handle, test_align, mem_size);
    if (ret != TZ_RESULT_SUCCESS)
    {
        printf("    [%d] UREE_AllocSecureMem Error: %d\n", id, (uint32_t) ret);
        error_exit_1 ("do_do_A_B_test");
    }

    param[0].value.a = (uint32_t) mem_handle;
    ret = UREE_TeeServiceCall(session, TZCMD_TEST_DO_A,
            TZ_ParamTypes3(TZPT_VALUE_INPUT, TZPT_VALUE_OUTPUT, TZPT_VALUE_OUTPUT), param);
    if (ret != TZ_RESULT_SUCCESS)
    {
        printf("    [%d] do A Error: 0x%x, %d\n", id, (uint32_t) session, ret);
        error_exit_1 ("do_do_A_B_test");
    }        

#ifndef minor_log
    printf ("Do A = 0x%x, 0x%x (%d)\n", param[1].value.a, param[2].value.a, ret);
#endif

    param[0].value.a = (uint32_t) mem_handle;
    ret = UREE_TeeServiceCall(session, TZCMD_TEST_DO_B,
            TZ_ParamTypes3(TZPT_VALUE_INPUT, TZPT_VALUE_OUTPUT, TZPT_VALUE_OUTPUT), param);
    if (ret != TZ_RESULT_SUCCESS)
    {
        printf("    [%d] do B Error: 0x%x, %d\n", id, (uint32_t) param[2].value.a, ret);
        error_exit_1 ("do_do_A_B_test");
    }  

    if (param[2].value.a == 0)
    {
        printf("    [%d] do B result Error: 0x%x, %d\n", id, (uint32_t) param[2].value.a, ret);
        error_exit ("do_do_A_B_test");
    }  

#ifndef minor_log
    printf ("Do B = 0x%x, 0x%x (%d)\n", param[1].value.a, param[2].value.a, ret);
#endif

    ret = UREE_UnreferenceSecurechunkmem (mem_session, mem_handle);
    if (ret != TZ_RESULT_SUCCESS)
    {
        printf("    [%d] UREE_UnReferenceSecureMem Error: %d\n", id, ret);
        error_exit_1 ("do_do_A_B_test");
    }

    ret = UREE_CloseSession(session);
    if (ret != TZ_RESULT_SUCCESS)
    {
        printf("    [%d] CloseSeesion Error: 0x%x, %d\n", id, (uint32_t) session, ret);
        error_exit_1 ("do_do_A_B_test");
    }

#ifndef minor_log
    printf ("%d >>===> [%d] test do A B <====\n\n", pid, id);
#endif

    return 0;
}

int do_do_A_B_secure_test (pid_t pid, uint32_t id, UREE_SESSION_HANDLE mem_session, uint32_t mem_size, uint32_t test_align, uint32_t test_size)
{
    TZ_RESULT ret;
    UREE_SECUREMEM_HANDLE mem_handle = 0;
    UREE_SESSION_HANDLE session;
    MTEEC_PARAM param[4];

#ifndef minor_log
    printf ("%d >>===> [%d] test do A B <====\n", pid, id);
#endif

    ret = UREE_CreateSession(TZ_TA_TEST_UUID, &session);
    if (ret != TZ_RESULT_SUCCESS)
    {
        printf("    [%d] CreateSession Error: 0x%x, %d\n", id, (uint32_t) session, ret);
        error_exit_1 ("do_do_A_B_test");
    }

    ret = UREE_AllocSecuremem (mem_session, &mem_handle, test_align, mem_size);
    if (ret != TZ_RESULT_SUCCESS)
    {
        printf("    [%d] UREE_AllocSecureMem Error: %d\n", id, (uint32_t) ret);
        error_exit_1 ("do_do_A_B_test");
    }

    param[0].value.a = (uint32_t) mem_handle;
    ret = UREE_TeeServiceCall(session, TZCMD_TEST_DO_A,
            TZ_ParamTypes3(TZPT_VALUE_INPUT, TZPT_VALUE_OUTPUT, TZPT_VALUE_OUTPUT), param);
    if (ret != TZ_RESULT_SUCCESS)
    {
        printf("    [%d] do A Error: 0x%x, %d\n", id, (uint32_t) session, ret);
        error_exit_1 ("do_do_A_B_test");
    }        

#ifndef minor_log
    printf ("Do A = 0x%x, 0x%x (%d)\n", param[1].value.a, param[2].value.a, ret);
#endif

    param[0].value.a = (uint32_t) mem_handle;
    ret = UREE_TeeServiceCall(session, TZCMD_TEST_DO_B,
            TZ_ParamTypes3(TZPT_VALUE_INPUT, TZPT_VALUE_OUTPUT, TZPT_VALUE_OUTPUT), param);
    if (ret != TZ_RESULT_SUCCESS)
    {
        printf("    [%d] do B Error: 0x%x, %d\n", id, (uint32_t) param[2].value.a, ret);
        error_exit_1 ("do_do_A_B_test");
    }  

    if (param[2].value.a == 0)
    {
        printf("    [%d] do B result Error: 0x%x, %d\n", id, (uint32_t) param[2].value.a, ret);
        error_exit ("do_do_A_B_test");
    }  

#ifndef minor_log
    printf ("Do B = 0x%x, 0x%x (%d)\n", param[1].value.a, param[2].value.a, ret);
#endif

    ret = UREE_UnreferenceSecuremem (mem_session, mem_handle);
    if (ret != TZ_RESULT_SUCCESS)
    {
        printf("    [%d] UREE_UnReferenceSecureMem Error: %d\n", id, ret);
        error_exit_1 ("do_do_A_B_test");
    }

    ret = UREE_CloseSession(session);
    if (ret != TZ_RESULT_SUCCESS)
    {
        printf("    [%d] CloseSeesion Error: 0x%x, %d\n", id, (uint32_t) session, ret);
        error_exit_1 ("do_do_A_B_test");
    }

#ifndef minor_log
    printf ("%d >>===> [%d] test do A B <====\n\n", pid, id);
#endif

    return 0;
}

int do_do_C_D_test (pid_t pid, uint32_t id, uint32_t test_size)
{
    TZ_RESULT ret;
    uint32_t *buf;
    UREE_SESSION_HANDLE session;
    MTEEC_PARAM param[4];
    //int i;

#ifndef minor_log
    printf ("%d >>===> [%d] test do C D <====\n", pid, id);
#endif

    ret = UREE_CreateSession(TZ_TA_TEST_UUID, &session);
    if (ret != TZ_RESULT_SUCCESS)
    {
        printf("    [%d] CreateSession Error: 0x%x, %d\n", id, (uint32_t) session, ret);
        error_exit_1 ("do_do_C_D_test");
    }

    ret = UREE_TeeServiceCall(session, TZCMD_TEST_DO_C,
            TZ_ParamTypes1(TZPT_VALUE_OUTPUT), param);
    if (ret != TZ_RESULT_SUCCESS)
    {
        printf("    [%d] do C Error: 0x%x, %d\n", id, (uint32_t) session, ret);
        error_exit_1 ("do_do_C_D_test");
    }        
    buf = (uint32_t *) param[0].value.a;

    /*for (i = 0; i < 16; i ++)
    {
        printf ("--> %d\n", buf[i]);
    }*/

    param[0].value.a = (uint32_t) buf;
    ret = UREE_TeeServiceCall(session, TZCMD_TEST_DO_D,
            TZ_ParamTypes1(TZPT_VALUE_INPUT), param);
    if (ret != TZ_RESULT_SUCCESS)
    {
        printf("    [%d] do D Error: 0x%x, %d\n", id, (uint32_t) param[2].value.a, ret);
        error_exit_1 ("do_do_C_D_test");
    }  

    if (param[2].value.a == 0)
    {
        printf("    [%d] do C D result Error: 0x%x, %d\n", id, (uint32_t) param[2].value.a, ret);
        error_exit ("do_do_C_D_test");
    }  

    ret = UREE_CloseSession(session);
    if (ret != TZ_RESULT_SUCCESS)
    {
        printf("    [%d] CloseSeesion Error: 0x%x, %d\n", id, (uint32_t) session, ret);
        error_exit_1 ("do_do_C_D_test");
    }

#ifndef minor_log
    printf ("%d >>===> [%d] test do C D <====\n\n", pid, id);
#endif

    return 0;
}


#define TEST_MEM_LOOP_NUM 8

#define TEST_RELEASECM_THREAD_LOOP_NUM 64
#define TEST_RELEASECM_MEM_LOOP_NUM 64
#define TEST_RELEASECM_MEM_TEST_MAX_SIZE 64*1024


typedef struct {
    uint32_t id;
    int fd;
} RELEASECM_THREAD_INPUT_T;

void thread_releasecm_test (RELEASECM_THREAD_INPUT_T *input)
{
    uint32_t i, j, loop;
    uint32_t *cmTest;
    uint32_t test_size;
    int ret;
    
    for (loop = 0; loop < TEST_RELEASECM_THREAD_LOOP_NUM; loop ++)
    {        
		test_size = random() % TEST_RELEASECM_MEM_TEST_MAX_SIZE;
		printf (" tzmem loop %d, size = 0x%x\n", loop, test_size);

        ret = UREE_ReleaseTzmem (&input->fd);
        if (ret != TZ_RESULT_SUCCESS)
        {
            printf("    [%d] UREE_ReleaseTzmem Error: %d\n", input->id, (uint32_t) ret);
            error_exit_1 ("do_cm_release_test");
        }
    
        cmTest = malloc (test_size);
        if (cmTest == NULL)
        {
           printf("    [%d] do_cm_release_test Error: out of memory\n", input->id);
           error_exit_1 ("do_cm_release_test");
        }
    
        for (i = 0; i < TEST_RELEASECM_MEM_LOOP_NUM; i ++)
        {
            for (j = 0; j < test_size/4; j ++)
            {
                cmTest[j] = j;
            }
            write (input->fd, cmTest, test_size);    
        }
    
        lseek (input->fd, 0, SEEK_SET);
    
        for (i = 0; i < TEST_RELEASECM_MEM_LOOP_NUM; i ++)
        {
            memset (cmTest, 0, test_size);
            read (input->fd, cmTest, test_size);
            
            for (j = 0; j < test_size/4; j ++)
            {
                if (cmTest[j] != j)
                {
                     printf("    [%d] result Error: %d\n", input->id, (uint32_t) j);
                     error_exit_1 ("do_cm_release_test");
                }
                //printf ("0x%x ", cmTest[j]);
            }      
        }

        free (cmTest);
    
        ret = UREE_AppendTzmem (input->fd);
    	if (ret != TZ_RESULT_SUCCESS)
    	{
    		printf("	[%d] UREE_AppendTzmem Error: %d\n", input->id, (uint32_t) ret);
    		error_exit_1 ("do_cm_release_test");
    	}
	}
}

int do_cm_test (pid_t pid, uint32_t id, UREE_SESSION_HANDLE mem_session, uint32_t mem_size, uint32_t test_align, uint32_t test_size)
{
    TZ_RESULT ret;
    UREE_SECURECM_HANDLE mem_handle[TEST_MEM_LOOP_NUM] = {0};
    int i;
    UREE_SECUREMEM_INFO info;

#ifndef minor_log
    printf ("%d >>===> [%d] test cm <====\n", pid, id);
#endif

    for (i = 0; i < TEST_MEM_LOOP_NUM; i ++)
    {
        ret = UREE_AllocSecurechunkmem (mem_session, &mem_handle[i], test_align, mem_size);
        if (ret != TZ_RESULT_SUCCESS)
        {
            printf("    [%d] UREE_AllocSecurechunkmem Error: %d\n", id, (uint32_t) ret);
            error_exit_1 ("do_cm_test");
        }

        ret = UREE_GetSecurechunkmeminfo (dbg_session, &info);
        if (ret != TZ_RESULT_SUCCESS)
        {
            printf("    [%d] UREE_GetSecurememinfo Error: %d\n", id, (uint32_t) ret);
            error_exit_1 ("do_cm_test");
        }
#ifndef minor_log        
        printf ("===> (%d) 0x%x 0x%x (0x%x)\n", i, info.total_byte, info.used_byte, mem_size);
#endif  
    }

    for (i = 0; i < TEST_MEM_LOOP_NUM; i ++)
    {
        ret = UREE_UnreferenceSecurechunkmem (mem_session, mem_handle[i]);
        if (ret != TZ_RESULT_SUCCESS)
        {
            printf("    [%d] UREE_UnreferenceSecurechunkmem Error: %d\n", id, ret);
            error_exit_1 ("do_cm_test");
        }

        ret = UREE_GetSecurechunkmeminfo (dbg_session, &info);
        if (ret != TZ_RESULT_SUCCESS)
        {
            printf("    [%d] UREE_GetSecurememinfo Error: %d\n", id, (uint32_t) ret);
            error_exit_1 ("do_cm_test");
        }
#ifndef minor_log            
        printf ("===> free (%d) 0x%x 0x%x (0x%x)\n", i, info.total_byte, info.used_byte, mem_size);
#endif    
    }

#ifndef minor_log
    printf ("%d >>===> [%d] test cm <====\n\n", pid, id);
#endif

    return 0;
}

int do_secure_test (pid_t pid, uint32_t id, UREE_SESSION_HANDLE mem_session, uint32_t mem_size, uint32_t test_align, uint32_t test_size)
{
    TZ_RESULT ret;
    UREE_SECURECM_HANDLE mem_handle[TEST_MEM_LOOP_NUM] = {0};
    int i;
    UREE_SECUREMEM_INFO info;

#ifndef minor_log
    printf ("%d >>===> [%d] test secure <====\n", pid, id);
#endif

    for (i = 0; i < TEST_MEM_LOOP_NUM; i ++)
    {
        ret = UREE_AllocSecuremem (mem_session, &mem_handle[i], test_align, mem_size);
        if (ret != TZ_RESULT_SUCCESS)
        {
            printf("    [%d] UREE_AllocSecuremem Error: %d\n", id, (uint32_t) ret);
            error_exit_1 ("do_secure_test");
        }

        ret = UREE_GetSecurememinfo (dbg_session, &info);
        if (ret != TZ_RESULT_SUCCESS)
        {
            printf("    [%d] UREE_AllocSecuremem Error: %d\n", id, (uint32_t) ret);
            error_exit_1 ("do_secure_test");
        }
#ifndef minor_log        
        printf ("===> (%d) 0x%x 0x%x 0x%x (0x%x)\n", i, info.total_byte, info.used_byte, info.max_free_cont_mem_sz, mem_size);
#endif        
    }    

    for (i = 0; i < TEST_MEM_LOOP_NUM; i ++)
    {
        ret = UREE_UnreferenceSecuremem (mem_session, mem_handle[i]);
        if (ret != TZ_RESULT_SUCCESS)
        {
            printf("    [%d] UREE_UnreferenceSecuremem Error: %d\n", id, ret);
            error_exit_1 ("do_secure_test");
        }

        ret = UREE_GetSecurememinfo (dbg_session, &info);
        if (ret != TZ_RESULT_SUCCESS)
        {
            printf("    [%d] UREE_AllocSecuremem Error: %d\n", id, (uint32_t) ret);
            error_exit_1 ("do_secure_test");
        }
#ifndef minor_log            
        printf ("===> free (%d) 0x%x 0x%x 0x%x (0x%x)\n", i, info.total_byte, info.used_byte, info.max_free_cont_mem_sz, mem_size);
#endif        
    }

#ifndef minor_log
    printf ("%d >>===> [%d] test secure <====\n\n", pid, id);
#endif

    return 0;
}

#define SHM_LOOP_RND_RANGE_MAX (8)
#define SHM_SIZE_RND_RANGE_MAX (2*1024*1024)

int do_shm_test (pid_t pid, uint32_t id, UREE_SESSION_HANDLE mem_session)
{
    uint32_t shm_p[TEST_MEM_LOOP_NUM];
    UREE_SHAREDMEM_PARAM shm_param;
    TZ_RESULT ret;
    UREE_SHAREDMEM_HANDLE shm_handle[TEST_MEM_LOOP_NUM] = {0};
    int i;
    uint32_t shm_size;
    int loop_num;

    loop_num = random() % SHM_LOOP_RND_RANGE_MAX;

#ifndef minor_log
    printf ("%d >>===> [%d] test shm, loop = %d <====\n", pid, id, loop_num);
#endif

    for (i = 0; i < loop_num; i ++)
    {
        shm_size = random() % SHM_SIZE_RND_RANGE_MAX;
        shm_p[i] = (uint32_t)malloc(shm_size);
#ifndef minor_log        
        printf ("size = %d\n", shm_size);
#endif        
    
        shm_param.buffer = (void *) shm_p[i];
        shm_param.size = shm_size;
        ret = UREE_RegisterSharedmem(mem_session, &shm_handle[i], &shm_param);
        if (ret != TZ_RESULT_SUCCESS)
        {
            printf("    [%d] UREE_RegisterSharedmem Error: %s\n", id, TZ_GetErrorString(ret));
            error_exit_1 ("do_shm_test");
        }
    }    

    for (i = 0; i < loop_num; i ++)
    {
        ret = UREE_UnregisterSharedmem(mem_session, shm_handle[i]);        
        if (ret != TZ_RESULT_SUCCESS)
        {
            printf("    [%d] UREE_UnregisterSharedmem Error: %s\n", id, TZ_GetErrorString(ret));
            error_exit_1 ("do_shm_test");
        }
    
        free ((void *) shm_p[i]);
    }

#ifndef minor_log
    printf ("%d >>===> [%d] test shm <====\n\n", pid, id);
#endif

    return 0;
}

int main(int argc, char *argv[])
{
    TZ_RESULT ret;
    UREE_SESSION_HANDLE mem_session_A;
    int i = 0;
    pid_t pid;
    RELEASECM_THREAD_INPUT_T thread_input;
    pthread_t thread_id;

    pid = getpid ();

    
    printf ("<<%d>> thread A...\n", (uint32_t) pid);

    ret = UREE_CreateSession(TZ_TA_DBG_UUID, &dbg_session);
    if (ret != TZ_RESULT_SUCCESS)
    {
        // Should provide strerror style error string in UREE.
        printf("CreateSession TZ_TA_DBG_UUID Error: %s\n", TZ_GetErrorString(ret));
        return 1;
    }  

    ret = UREE_CreateSession(TZ_TA_MEM_UUID, &mem_session_A);
    if (ret != TZ_RESULT_SUCCESS)
    {
        // Should provide strerror style error string in UREE.
        printf("CreateSession TZ_TA_MEM_UUID Error: %s\n", TZ_GetErrorString(ret));
        return 1;
    }    

    // create thread for tzmem testing
    thread_input.id = 0;
    thread_input.fd = 0;
    ret = pthread_create (&thread_id, NULL, (void *) thread_releasecm_test, (void *) &thread_input);
    if (ret != 0)
    {
        printf("    [%d] pthread_create Error: %d\n", 0, (uint32_t) ret);
        error_exit_1 ("do_cm_release_test");
    }

#if 1
    for (i = 1; i < 8; i ++)
    {
        printf ("===> do %d\n", i);
        //do_cm_test (pid, i, mem_session_A, 4*8*i, 0, 8*i);
        do_secure_test (pid, i, mem_session_A, 4*8*i, 0, 8*i);
        do_shm_test (pid, i, mem_session_A);
    }
#else
    for (i = 1; i < 1024; i ++)
    {
        printf ("===> do %d\n", i);
        do_add_mem_test (pid, i, mem_session_A, 8*i, 0, 8*i);
        do_do_A_B_secure_test (pid, i, mem_session_A, 4*8*i, 0, 8*i);
        do_do_A_B_cm_test (pid, i, mem_session_A, 4*8*i, 0, 8*i);
        do_cm_test (pid, i, mem_session_A, 4*8*i, 0, 8*i);
        do_secure_test (pid, i, mem_session_A, 4*8*i, 0, 8*i);
        do_shm_test (pid, i, mem_session_A);
        do_do_C_D_test (pid, i, i);
        do_securefunc_test (pid, i);
        do_thread_test (pid, i);
    }
#endif    

    // close  thread for tzmem testing
	pthread_join (thread_id, NULL);

    ret = UREE_CloseSession(dbg_session);
    if (ret != TZ_RESULT_SUCCESS)
    {
        printf("CloseSeesion dbg_session Error: %d\n", ret);
    }

    ret = UREE_CloseSession(mem_session_A);
    if (ret != TZ_RESULT_SUCCESS)
    {
        printf("CloseSeesion mem_session_A Error: %d\n", ret);
    }

    printf ("<<%d>> A done!!!\n", pid);

    return 0;
}
