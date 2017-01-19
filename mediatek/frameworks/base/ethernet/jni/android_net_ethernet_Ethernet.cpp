/*
 * Copyright 2010, The Android-x86 Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: Yi Sun <beyounn@gmail.com>
 */

#define LOG_TAG "ethernet"

#include "jni.h"
#include <inttypes.h>
#include <utils/misc.h>
#include <android_runtime/AndroidRuntime.h>
#include <utils/Log.h>
#include <cutils/xlog.h>
#include <asm/types.h>
#include <linux/if_arp.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <net/if_arp.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include "JNIHelp.h"


#define ETH_PKG_NAME     "android/net/ethernet/EthernetNative"
#define NL_SOCK_INV      -1
#define RET_STR_SZ       4096
#define NL_POLL_MSG_SZ   8*1024
#define SYSFS_PATH_MAX   256


static struct fieldIds {
    jclass dhcpInfoClass;
    jmethodID constructorId;
    jfieldID ipaddress;
    jfieldID gateway;
    jfieldID netmask;
    jfieldID dns1;
    jfieldID dns2;
    jfieldID serverAddress;
    jfieldID leaseDuration;
} dhcpInfoFieldIds;

struct interface_info_t {
    unsigned int i;                   /* interface index        */
    char *name;                       /* name (eth0, eth1, ...) */
    struct interface_info_t *next;
};

interface_info_t *interfaces = NULL;
int total_int = 0; 
static const char SYSFS_CLASS_NET[] = "/sys/class/net";
static int nl_socket_msg = NL_SOCK_INV;
static struct sockaddr_nl addr_msg;
static int nl_socket_poll = NL_SOCK_INV;
static struct sockaddr_nl addr_poll;
static int getinterfacename(int index, char *name, size_t len);

static interface_info_t *find_info_by_index(unsigned int index)
{
    interface_info_t *info = interfaces;
    while (info) {
        if (info->i == index)
            break;
        info = info->next;
    }
    return info;
}

static jstring android_net_ethernet_waitForEvent(JNIEnv *env, jobject clazz)
{
    char *buff;
    struct nlmsghdr *nh;
    struct ifinfomsg *einfo;
    struct iovec iov;
    struct msghdr msg;
    char *result = NULL;
    char rbuf[4096];
    unsigned int left;
    interface_info_t *info;
    int len;
	
    ALOGV("Poll events from ethernet devices");
    /*
     *wait on uevent netlink socket for the ethernet device
     */
    buff = (char *)malloc(NL_POLL_MSG_SZ);
    if (!buff) {
        ALOGE("Allocate poll buffer failed");
        goto error;
    }

    iov.iov_base = buff;
    iov.iov_len = NL_POLL_MSG_SZ;
    msg.msg_name = (void *)&addr_msg;
    msg.msg_namelen =  sizeof(addr_msg);
    msg.msg_iov =  &iov;
    msg.msg_iovlen =  1;
    msg.msg_control =  NULL;
    msg.msg_controllen =  0;
    msg.msg_flags =  0;

    if ((len = recvmsg(nl_socket_poll, &msg, 0)) >= 0) {
        ALOGV("recvmsg get data");
        result = rbuf;
        left = 4096;
        rbuf[0] = '\0';
        for (nh = (struct nlmsghdr *) buff; NLMSG_OK (nh, len);
             nh = NLMSG_NEXT (nh, len)) {

            if (nh->nlmsg_type == NLMSG_DONE) {
                ALOGE("Did not find useful eth interface information");
                goto error;
            }

            if (nh->nlmsg_type == NLMSG_ERROR) {
                /* Do some error handling. */
                ALOGE("Read device name failed");
                goto error;
            }

            ALOGV(" event :%d  found", nh->nlmsg_type);	


            einfo = (struct ifinfomsg *)NLMSG_DATA(nh);
            ALOGV("the device flag :%X", einfo->ifi_flags);
            if (nh->nlmsg_type == RTM_DELLINK ||
                nh->nlmsg_type == RTM_NEWLINK ||
                nh->nlmsg_type == RTM_DELADDR ||
                nh->nlmsg_type == RTM_NEWADDR) {
                int type = nh->nlmsg_type;
                if (type == RTM_NEWLINK &&
                    (!(einfo->ifi_flags & IFF_LOWER_UP))) {
                    type = RTM_DELLINK;
                }
                if ((info = find_info_by_index
                      (((struct ifinfomsg*) NLMSG_DATA(nh))->ifi_index)) != NULL)
                    snprintf(result,left, "%s:%d:",info->name,type);
                left = left - strlen(result);
                result =(char *)(result+ strlen(result));
            }

        }
        ALOGV("Done parsing");
        rbuf[4096 - left] = '\0';
        ALOGV("poll state :%s, left:%d", rbuf, left);
    }


error:
    free(buff);
    return env->NewStringUTF(rbuf);
}

static int netlink_send_dump_request(int sock, int type, int family)
{
    int ret;
    char buf[4096];
    struct sockaddr_nl snl;
    struct nlmsghdr *nlh;
    struct rtgenmsg *g;

    memset(&snl, 0, sizeof(snl));
    snl.nl_family = AF_NETLINK;

    memset(buf, 0, sizeof(buf));
    nlh = (struct nlmsghdr *)buf;
    g = (struct rtgenmsg *)(buf + sizeof(struct nlmsghdr));

    nlh->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtgenmsg));
    nlh->nlmsg_flags = NLM_F_REQUEST|NLM_F_DUMP;
    nlh->nlmsg_type = type;
    g->rtgen_family = family;

    ret = sendto(sock, buf, nlh->nlmsg_len, 0, (struct sockaddr *)&snl,
                 sizeof(snl));
    if (ret < 0) {
        perror("netlink_send_dump_request sendto");
        return -1;
    }

    return ret;
}

static void free_int_list()
{
    interface_info_t *tmp = interfaces;
    while (tmp) {
        if (tmp->name)
            free(tmp->name);
        interfaces = tmp->next;
        free(tmp);
        tmp = interfaces;
        total_int--;
    }
    if (total_int) {
        ALOGE("Wrong interface count found");
        total_int = 0;
    }
}

static void add_int_to_list(interface_info_t *node)
{
    /* Todo: Lock here!!!! */
    node->next = interfaces;
    interfaces = node;
    total_int++;
}

static int netlink_init_interfaces_list(void)
{
    int ret = -1;
    DIR  *netdir;
    struct dirent *de;
    char path[SYSFS_PATH_MAX];
    interface_info_t *intfinfo;
    int index;
    FILE *fp;
    #define MAX_FGETS_LEN 4
    char idx[MAX_FGETS_LEN + 1];

    if ((netdir = opendir(SYSFS_CLASS_NET)) != NULL) {
         while ((de = readdir(netdir))) {
            if (!strstr(de->d_name, "eth"))
                continue;
            snprintf(path, SYSFS_PATH_MAX,"%s/%s/type", SYSFS_CLASS_NET, de->d_name);
            if ((fp = fopen(path, "r")) != NULL) {
                memset(idx, 0, MAX_FGETS_LEN + 1);
                if (fgets(idx, MAX_FGETS_LEN, fp) != NULL) {
                    fclose(fp);
                    if (strtoimax(idx, NULL, 10) != ARPHRD_ETHER)
                        continue;
                } else {
                    fclose(fp);
                    continue;
                }
            } else {
                continue;
            }

            snprintf(path, SYSFS_PATH_MAX,"%s/%s/ifindex", SYSFS_CLASS_NET, de->d_name);
            if ((fp = fopen(path, "r")) != NULL) {
                memset(idx, 0, MAX_FGETS_LEN + 1);
                if (fgets(idx, MAX_FGETS_LEN, fp) != NULL) {
                    fclose(fp);
                    index = strtoimax(idx, NULL, 10);
                } else {
                    fclose(fp);
                    ALOGE("Can not read %s", path);
                    continue;
                }
            } else {
                ALOGE("Can not open %s for read", path);
                continue;
            }
            /* make some room! */
            intfinfo = (interface_info_t *)malloc(sizeof(interface_info_t));
            if (intfinfo == NULL) {
                ALOGE("malloc in netlink_init_interfaces_table");
                goto error;
            }
            /* copy the interface name (eth0, eth1, ...) */
            intfinfo->name = strndup((char *) de->d_name, SYSFS_PATH_MAX);
            intfinfo->i = index;
            ALOGI("interface %s:%d found", intfinfo->name, intfinfo->i);
            add_int_to_list(intfinfo);
        }
        closedir(netdir);
    }
    ret = 0;

error:
    return ret;
}

/*
 * The netlink socket
 */

static jint android_net_ethernet_initEthernetNative(JNIEnv *env,
                                                    jobject clazz)
{
    int ret = -1;

    memset(&addr_msg, 0, sizeof(sockaddr_nl));
    addr_msg.nl_family = AF_NETLINK;
    memset(&addr_poll, 0, sizeof(sockaddr_nl));
    addr_poll.nl_family = AF_NETLINK;
    addr_poll.nl_pid = 0;//getpid();
    addr_poll.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR;

    /*
     *Create connection to netlink socket
     */
    nl_socket_msg = socket(AF_NETLINK,SOCK_RAW,NETLINK_ROUTE);
    if (nl_socket_msg <= 0) {
        ALOGE("Can not create netlink msg socket");
        goto error;
    }
    if (bind(nl_socket_msg, (struct sockaddr *)(&addr_msg),
             sizeof(struct sockaddr_nl))) {
        ALOGE("Can not bind to netlink msg socket");
        goto error;
    }

    nl_socket_poll = socket(AF_NETLINK,SOCK_RAW,NETLINK_ROUTE);
    if (nl_socket_poll <= 0) {
        ALOGE("Can not create netlink poll socket");
        goto error;
    }

    errno = 0;
    if (bind(nl_socket_poll, (struct sockaddr *)(&addr_poll),
            sizeof(struct sockaddr_nl))) {
        ALOGE("Can not bind to netlink poll socket,%s", strerror(errno));

        goto error;
    }

    if ((ret = netlink_init_interfaces_list()) < 0) {
        ALOGE("Can not collect the interface list");
        goto error;
    }
    ALOGE("%s exited with success", __FUNCTION__);
    return ret;

error:
    ALOGE("%s exited with error", __FUNCTION__);
    if (nl_socket_msg > 0)
        close(nl_socket_msg);
    if (nl_socket_poll > 0)
        close(nl_socket_poll);
    return ret;
}

static jstring android_net_ethernet_getInterfaceName(JNIEnv *env,
                                                     jobject clazz,
                                                     jint index)
{
    int i = 0;
    interface_info_t *info;
    ALOGI("User ask for device name on %d, list:%X, total:%d",
         index, (unsigned int)interfaces, total_int);
    info = interfaces;
    if (total_int != 0 && index <= (total_int - 1)) {
        while (info) {
            if (index == i) {
                ALOGV("Found: %s", info->name);
                return env->NewStringUTF(info->name);
            }
            info = info->next;
            i++;
        }
    }
    ALOGI("No device name found");
    return env->NewStringUTF(NULL);
}


static jint android_net_ethernet_getInterfaceCnt()
{
    return total_int;
}

static int eth_get_mac_addr(unsigned char *pmac_addr)
{
	int socket_id = socket (AF_INET, SOCK_DGRAM, 0);
int nret;
  char eth0_ifname[10] = {"eth0"};
struct ifreq req;
strncpy (req.ifr_name, eth0_ifname, sizeof(req.ifr_name));
nret = ioctl(socket_id, SIOCGIFHWADDR, &req);
close(socket_id);
if (nret != -1)
{
	memcpy (pmac_addr, req.ifr_hwaddr.sa_data, 6); // 
	return 0 ;
}
return -1;
}

static jstring android_net_ethernet_getMacAddressCommand(JNIEnv* env, jobject clazz)
{
    char reply[256];
    char buf[256];
    int ret;
 int test; 	
    int mac_len = strlen("00:00:00:00:00:00");
    unsigned char _mac[6] = {0};
    ret = eth_get_mac_addr( _mac );
    if(ret == -1 )
    {	     
       ALOGD(">>> DRIVER MACADDR get mac addr return error!\n");
       return env->NewStringUTF(NULL);
    }
    sprintf(reply ,"%02x:%02x:%02x:%02x:%02x:%02x",_mac[0],_mac[1],_mac[2],_mac[3],_mac[4],_mac[5]);
    ALOGD(">>>>>> DRIVER MACADDR --> mac = %s\n",reply);
    if (sscanf(reply, "%255s", buf) == 1)
    {
        return env->NewStringUTF(buf);
    }
    else
    {
        return env->NewStringUTF(NULL);
    }
}

static jint android_net_ethernet_getLinkStatus(JNIEnv* env, jobject clazz)
{	
FILE *ifidx;
int linkstatus = 0;
char path[SYSFS_PATH_MAX];
#define MAX_FGETS_LEN 4
char idx[MAX_FGETS_LEN+1];
snprintf(path, SYSFS_PATH_MAX,"%s/eth0/carrier", SYSFS_CLASS_NET);
    if ((ifidx = fopen(path, "r")) != NULL) {
        memset(idx, 0, MAX_FGETS_LEN+1);
        if (fgets(idx,MAX_FGETS_LEN, ifidx) != NULL) {
            linkstatus = strtoimax(idx, NULL, 10);
        } else {
            ALOGE("Can not read %s", path);
        }
        fclose(ifidx);
    } else {
        ALOGE("Can not open %s for read", path);
    }
return linkstatus;
}

//--------------------------------------------------------------------------
static JNINativeMethod gEthernetMethods[] = {
    {"waitForEvent", "()Ljava/lang/String;",
     (void *)android_net_ethernet_waitForEvent},
    {"getInterfaceName", "(I)Ljava/lang/String;",
     (void *)android_net_ethernet_getInterfaceName},
    {"initEthernetNative", "()I",
     (void *)android_net_ethernet_initEthernetNative},
    {"getInterfaceCnt","()I",
     (void *)android_net_ethernet_getInterfaceCnt},
    { "getMacAddressCommand", "()Ljava/lang/String;", 
     (void*) android_net_ethernet_getMacAddressCommand },
    { "getLinkStatus", "()I",
     (void*) android_net_ethernet_getLinkStatus}
};

jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env = NULL;

    ALOGI("[Ethernet]JNI_OnLoad ++");
    if (vm->GetEnv((void**) &env, JNI_VERSION_1_6)) {
        ALOGE("GetEnv failed!");
        return JNI_ERR;
    }

    int ret = jniRegisterNativeMethods(
    env, "android/net/ethernet/EthernetNative", gEthernetMethods, NELEM(gEthernetMethods));
    

    if (ret) {
        ALOGE("[Ethernet] call jniRegisterNativeMethods() failed, ret:%d\n", ret);
    }
    
    return JNI_VERSION_1_6;
}




