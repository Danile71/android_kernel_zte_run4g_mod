/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2012. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "MET_DAEMON"

#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <sched.h>

#include <linux/limits.h>
#include <sys/resource.h>
#include <sys/stat.h>

#include "met_tag.h"

#define TIMER_SIG SIGRTMIN
static int met_daemonize(void)
{

    pid_t pid;
    struct sched_param sched_param;

    //Mask hangup signal for let child process going on
//    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    pid = fork();

    if(0 > pid)
    {
        printf("Unable to fork a process:%d\n" , pid);
        return -1;
    }

    if(0 < pid)
    {
        //Sleep 100 us to let child go first
        printf("The child PID is %d, from parent %d\n" , pid , getpid());
        exit(EXIT_SUCCESS);
    }

    printf("Child is forked\n");

    umask(0);
    chdir("/");

    //Boost thread priority since the loop is time critical and light weight
    sched_param.sched_priority = sched_get_priority_max(SCHED_RR);
    if(sched_setscheduler(0 , SCHED_RR , &sched_param))
    {
        printf("Boost piroirty failed : %d\n" , errno);
    }

    //Deattach from TTY
    if(isatty(STDIN_FILENO))
    {
        close(STDIN_FILENO);
    }

    if(isatty(STDOUT_FILENO))
    {
        close(STDOUT_FILENO);
    }

    if(isatty(STDERR_FILENO))
    {
        close(STDERR_FILENO);
    }

    return 0;
}

static void TimerHandler(int signum)
{
//Nothing to do with
}

typedef struct timerID{
    sigset_t sigMask;
    timer_t TimerID;
} sigTimer_t;

static int CreateTimer(sigTimer_t * pTimerID , unsigned long u4TimeIntervalms)
{
    struct sigaction sigAct;
    struct sigevent sigE;
    struct itimerspec timerSpec;
    siginfo_t sigInfo;

    if(NULL == pTimerID)
    {
        printf("pTimerID is NULL\n");
        return -1;
    }

    //Create the timer
    sigAct.sa_flags = SA_SIGINFO;
    sigAct.sa_handler = TimerHandler;
    sigemptyset(&sigAct.sa_mask);
    if (sigaction(TIMER_SIG, &sigAct, NULL) == -1)
    {
        printf("sigaction failed\n");
        return -1;
    }

    sigemptyset(&(pTimerID->sigMask));
    sigaddset(&(pTimerID->sigMask), TIMER_SIG);
    if (sigprocmask(SIG_SETMASK, &(pTimerID->sigMask), NULL) == -1)
    {
        printf("sigproc set mask failed\n");
        return -1;
    }

    sigE.sigev_notify = TIMER_SIG;
    sigE.sigev_signo = SIGRTMIN;
    sigE.sigev_value.sival_ptr = &(pTimerID->TimerID);
    if(timer_create(CLOCK_REALTIME , &sigE , &(pTimerID->TimerID)))
    {
        printf("Create timer failed\n");
        return -1;
    }

    timerSpec.it_value.tv_sec = u4TimeIntervalms/1000;
    timerSpec.it_value.tv_nsec = u4TimeIntervalms*1000000;
    timerSpec.it_interval.tv_sec = timerSpec.it_value.tv_sec;
    timerSpec.it_interval.tv_nsec = timerSpec.it_value.tv_nsec;
    if(timer_settime(pTimerID->TimerID , 0 , &timerSpec , NULL))
    {
        printf("set timer failed\n");
        timer_delete(pTimerID->TimerID);
        return -1;
    }

    if (sigprocmask(SIG_UNBLOCK, &(pTimerID->sigMask), NULL) == -1)
    {
        printf("sigprocmask unblock failed\n");
        timer_delete(pTimerID->TimerID);
        return -1;
    }

    return 0;
}

static int WaitTimer(sigTimer_t * pTimerID)
{
    int retSigNumber;
    return sigwait(&(pTimerID->sigMask) , &retSigNumber);
}

static void DestroyTimer(sigTimer_t * pTimerID)
{
    timer_delete(pTimerID->TimerID);
}

#define STRING_SIZE 4096
#define STRING_PERLINE_SIZE 64
int main(int argc, char *argv[])
{

    char str[STRING_SIZE];
    char strTmp[STRING_PERLINE_SIZE];
    struct timeval t1;
    struct timezone tz;
    unsigned long u4KeepRunning = 1;
    sigTimer_t timerID;
    FILE *fp;
    int ret = 0;

    //Daemonize
    if(met_daemonize())
    {
        printf("fail to fork a daemon\n");
        return -1;
    }

    //initialize met tag
    if(met_tag_init())
    {
        printf("met tag init failed\n");
        return -1;
    }

    //open file for check MET running
    fp = fopen("/sys/class/misc/met/run" , "r+");
    if (fp == NULL)
    {
        printf("Open /sys/class/misc/met/run failed\n");
        return -1;
    }

    //Create timer
    if(CreateTimer(&timerID , 10))
    {
        printf("Create timer failed\n");
        return -1;
    }

    //Thread Loop
    while(u4KeepRunning)
    {
        if(0 > WaitTimer(&timerID))
        {
            met_tag_userdata("sigwait failed");
            ret = -1;
            break;
        }
        gettimeofday(&t1 , &tz);
        snprintf(str , 128 , "GPU:%u,%u,%u" , (unsigned int)t1.tv_sec , (unsigned int)t1.tv_usec , 100);
        met_tag_userdata(str);

        fflush(fp);
        rewind(fp);
        if(NULL == fgets(str , 16 , fp))
        {
            met_tag_userdata("fgets failed");
            ret = -1;
            break;
        }
        u4KeepRunning = strcmp(str , "-1\n");

    }

DELETE_TIMER:
    DestroyTimer(&timerID);

ERROR_OUT:
    fclose(fp);

    return ret;
}

