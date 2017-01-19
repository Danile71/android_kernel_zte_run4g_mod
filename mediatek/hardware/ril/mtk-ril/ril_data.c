/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

/* //hardware/ril/reference-ril/ril_data.c
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include <telephony/ril.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <alloca.h>
#include "atchannels.h"
#include "at_tok.h"
#include "misc.h"
#include <getopt.h>
#include <sys/socket.h>
#include <cutils/sockets.h>
#include <termios.h>
#include "ril_callbacks.h"
#include <utils/Log.h>

#include <cutils/properties.h>
#include <netdb.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <linux/route.h>

#define PROPERTY_RIL_SPECIFIC_SM_CAUSE "ril.specific.sm_cause"
#define PROPERTY_SIM_OPERATOR_NUMERIC "gsm.sim.operator.numeric"

#define DATA_CHANNEL_CTX getRILChannelCtxFromToken(t)

#define PPP_PPP_RUNNING_PROPERTY "init.svc.pppd_gprs"
#define PPP_CSD_RUNNING_PROPERTY "init.svc.pppd_csd"
#define PPP_DEF_RUNNING "running"
#define PPP_DEF_STOPPING "stopping"

static RILChannelCtx g_CsdDataChannel = {0};

int sock_fd[MAX_CCMNI_NUMBER] = {0};
int sock6_fd[MAX_CCMNI_NUMBER] = {0};

extern int max_pdn_support;
extern int pdnInfoSize;
extern int isCid0Support;

/* Refer to system/core/libnetutils/ifc_utils.c */
extern int ifc_disable(const char *ifname);
extern int ifc_remove_default_route(const char *ifname);
extern int ifc_reset_connections(const char *ifname);

extern void initialCidTable();
extern void clearPdnInfo(PdnInfo* info);
extern int get_protocol_type(const char* protocol);
extern int getAvailableCid();
extern void configureNetworkInterface(int interfaceId, int isUp);
extern void requestSetFDMode(void * data, size_t datalen, RIL_Token t);
extern void onNetworkBearerUpdate(void* param);
extern void onPdnDeactResult(void* param);
extern int handleLastPdnDeactivation (const int isEmergency, const int lastPDNCid, RILChannelCtx* rilchnlctx);
extern int deactivateDataCall(int cid, RILChannelCtx* rilchnlctx);
extern int definePdnCtx(const RIL_Default_Bearer_VA_Config_Struct *pDefaultBearerVaConfig, const char *requestedApn,
                    int protocol, int availableCid, int authType, const char *username, const char* password, RILChannelCtx *pChannel);
extern int queryMatchedPdnWithSameApn(const char* apn, int* matchedCidList, RILChannelCtx* rilchnlctx);
extern int getAttachApnSupport();
void getIaCache(char* cache);
void setIaCache(const char* iccid, const char* protocol, const int authtype, const char* username, const int canHandleIms, const char* apn);
int doPsAttach(RILChannelCtx *pChannel);
int isAlwaysAttach();

//Fucntion prototype
void ril_data_ioctl_init(int index);
void ril_data_setflags(int s, struct ifreq *ifr, int set, int clr);
void ril_data_setaddr(int s, struct ifreq *ifr, const char *addr);

int receivedSCRI_RAU = 0;
int sendSCRI_RAU = 0;

//Global variables/strcuture
static int disableFlag = 1;
int gprs_failure_cause = 0;

extern int gcf_test_mode;
extern int s_md_off;

extern PdnInfo* pdn_info;

static const struct timeval TIMEVAL_0 = {0,0};
// MMDC
static const struct timeval MMDC_TIMEVAL = {0,500000};

int disableIpv6Interface(char* filepath) {
    int fd = open(filepath, O_WRONLY);
    if (fd < 0) {
        LOGE("failed to open file (%s)", strerror(errno));
        return -1;
    }
    if (write(fd, "1", 1) != 1) {
        LOGE("failed to write property file (%s)",strerror(errno));
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

void ril_data_ioctl_init(int index)
{
    disableFlag = 0;

    if(disableFlag || (gcf_test_mode > 0 )) {
        LOGD("Disable IPv6 inteface for CCNMI");
#ifdef MTK_RIL_MD1
        disableIpv6Interface("/proc/sys/net/ipv6/conf/ccmni0/disable_ipv6");
        disableIpv6Interface("/proc/sys/net/ipv6/conf/ccmni1/disable_ipv6");
        disableIpv6Interface("/proc/sys/net/ipv6/conf/ccmni2/disable_ipv6");
#else
        disableIpv6Interface("/proc/sys/net/ipv6/conf/cc2mni0/disable_ipv6");
        disableIpv6Interface("/proc/sys/net/ipv6/conf/cc2mni1/disable_ipv6");
        disableIpv6Interface("/proc/sys/net/ipv6/conf/cc2mni2/disable_ipv6");
#endif
    }

    if (sock_fd[index] > 0)
        close(sock_fd[index]);
    sock_fd[index] = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd[index] < 0)
        LOGD("Couldn't create IP socket: errno=%d", errno);
    else
        LOGD("Allocate sock_fd=%d, for cid=%d", sock_fd[index], index+1);

#ifdef INET6
    if (sock6_fd[index] > 0)
        close(sock6_fd[index]);
    sock6_fd[index] = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sock6_fd[index] < 0) {
        sock6_fd[index] = -errno;	/* save errno for later */
        LOGD("Couldn't create IPv6 socket: errno=%d", errno);
    } else {
        LOGD("Allocate sock6_fd=%d, for cid=%d", sock6_fd[index], index+1);
    }
#endif
}

/* For setting IFF_UP: ril_data_setflags(s, &ifr, IFF_UP, 0) */
/* For setting IFF_DOWN: ril_data_setflags(s, &ifr, 0, IFF_UP) */
void ril_data_setflags(int s, struct ifreq *ifr, int set, int clr)
{
    if(ioctl(s, SIOCGIFFLAGS, ifr) < 0)
        goto terminate;
    ifr->ifr_flags = (ifr->ifr_flags & (~clr)) | set;
    if(ioctl(s, SIOCSIFFLAGS, ifr) < 0)
        goto terminate;
    return;
terminate:
    LOGD("Set SIOCSIFFLAGS Error!");
    return;
}

inline void ril_data_init_sockaddr_in(struct sockaddr_in *sin, const char *addr)
{
    sin->sin_family = AF_INET;
    sin->sin_port = 0;
    sin->sin_addr.s_addr = inet_addr(addr);
}

void ril_data_setaddr(int s, struct ifreq *ifr, const char *addr)
{
    LOGD("Configure IPv4 adress :%s", addr);
    ril_data_init_sockaddr_in((struct sockaddr_in *) &ifr->ifr_addr, addr);
    if(ioctl(s, SIOCSIFADDR, ifr) < 0)
        LOGD("Set SIOCSIFADDR Error");
}

void initialDataCallResponse(RIL_Data_Call_Response_v6* responses, int length) {
    int i = 0;
    for (i=0; i<length; i++) {
        memset(&responses[i], 0, sizeof(RIL_Data_Call_Response_v6));
        responses[i].status = PDP_FAIL_ERROR_UNSPECIFIED;
        responses[i].cid = INVALID_CID;
    }
}

int getAuthType(const char* authTypeStr) {
    int  authType = atoi(authTypeStr);
    //Application 0->none, 1->PAP, 2->CHAP, 3->PAP/CHAP;
    //Modem 0->PAP, 1->CHAP, 2->NONE, 3->PAP/CHAP;
    switch (authType) {
        case 0:
            return AUTHTYPE_NONE;
        case 1:
            return AUTHTYPE_PAP;
        case 2:
            return AUTHTYPE_CHAP;
        case 3:
            return AUTHTYPE_CHAP;
        default:
            return AUTHTYPE_NOT_SET;
    }
}

int getLastDataCallFailCause()
{
    if (gprs_failure_cause == SM_OPERATOR_BARRED ||
            gprs_failure_cause == SM_MBMS_CAPABILITIES_INSUFFICIENT ||
            gprs_failure_cause == SM_LLC_SNDCP_FAILURE ||
            gprs_failure_cause == SM_INSUFFICIENT_RESOURCES ||
            gprs_failure_cause == SM_MISSING_UNKNOWN_APN ||
            gprs_failure_cause == SM_UNKNOWN_PDP_ADDRESS_TYPE ||
            gprs_failure_cause == SM_USER_AUTHENTICATION_FAILED ||
            gprs_failure_cause == SM_ACTIVATION_REJECT_GGSN ||
            gprs_failure_cause == SM_ACTIVATION_REJECT_UNSPECIFIED ||
            gprs_failure_cause == SM_SERVICE_OPTION_NOT_SUPPORTED ||
            gprs_failure_cause == SM_SERVICE_OPTION_NOT_SUBSCRIBED ||
            gprs_failure_cause == SM_SERVICE_OPTION_OUT_OF_ORDER ||
            gprs_failure_cause == SM_NSAPI_IN_USE ||
            gprs_failure_cause == SM_REGULAR_DEACTIVATION ||
            gprs_failure_cause == SM_QOS_NOT_ACCEPTED ||
            gprs_failure_cause == SM_NETWORK_FAILURE ||
            gprs_failure_cause == SM_REACTIVATION_REQUESTED ||
            gprs_failure_cause == SM_FEATURE_NOT_SUPPORTED ||
            gprs_failure_cause == SM_SEMANTIC_ERROR_IN_TFT ||
            gprs_failure_cause == SM_SYNTACTICAL_ERROR_IN_TFT ||
            gprs_failure_cause == SM_UNKNOWN_PDP_CONTEXT ||
            gprs_failure_cause == SM_SEMANTIC_ERROR_IN_PACKET_FILTER ||
            gprs_failure_cause == SM_SYNTACTICAL_ERROR_IN_PACKET_FILTER ||
            gprs_failure_cause == SM_PDP_CONTEXT_WITHOU_TFT_ALREADY_ACTIVATED ||
            gprs_failure_cause == SM_MULTICAST_GROUP_MEMBERSHIP_TIMEOUT ||
            gprs_failure_cause == SM_BCM_VIOLATION ||
            gprs_failure_cause == SM_ONLY_IPV4_ALLOWED ||
            gprs_failure_cause == SM_ONLY_IPV6_ALLOWED ||
            gprs_failure_cause == SM_ONLY_SINGLE_BEARER_ALLOWED ||
            gprs_failure_cause == SM_COLLISION_WITH_NW_INITIATED_REQUEST ||
            gprs_failure_cause == SM_BEARER_HANDLING_NOT_SUPPORT ||
            gprs_failure_cause == SM_MAX_PDP_NUMBER_REACHED ||
            gprs_failure_cause == SM_APN_NOT_SUPPORT_IN_RAT_PLMN ||
            gprs_failure_cause == SM_INVALID_TRANSACTION_ID_VALUE ||
            gprs_failure_cause == SM_SEMENTICALLY_INCORRECT_MESSAGE ||
            gprs_failure_cause == SM_INVALID_MANDATORY_INFO ||
            gprs_failure_cause == SM_MESSAGE_TYPE_NONEXIST_NOT_IMPLEMENTED ||
            gprs_failure_cause == SM_MESSAGE_TYPE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE ||
            gprs_failure_cause == SM_INFO_ELEMENT_NONEXIST_NOT_IMPLEMENTED ||
            gprs_failure_cause == SM_CONDITIONAL_IE_ERROR ||
            gprs_failure_cause == SM_MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE ||
            gprs_failure_cause == SM_PROTOCOL_ERROR ||
            gprs_failure_cause == SM_APN_RESTRICTION_VALUE_INCOMPATIBLE_WITH_PDP_CONTEXT) {
        return gprs_failure_cause;
    }else if (gprs_failure_cause == 0x0D19){
        //SM_LOCAL_REJECT_ACT_REQ_DUE_TO_GPRS_ATTACH_FAILURE
        //Modem will retry attach
        LOGD("getLastDataCallFailCause(): GMM error %X", gprs_failure_cause);
    } else {
       gprs_failure_cause = 14;  //FailCause.Unknown
    }

    return gprs_failure_cause;
}

/* Change name from requestOrSendPDPContextList to requestOrSendDataCallList */
static void requestOrSendDataCallList(RIL_Token *t, RILId rilid)
{
    /* Because the RIL_Token* t may be NULL passed due to receive URC: Only t is NULL, 2nd parameter rilid is used */
    RILChannelCtx* rilchnlctx = NULL;
    if (t != NULL)
        rilchnlctx = getRILChannelCtxFromToken(*t);
    else
        rilchnlctx = getChannelCtxbyProxy(rilid);

    requestOrSendDataCallListIpv6(rilchnlctx, t, rilid);
}

/* 27:RIL_REQUEST_SETUP_DATA_CALL/RIL_REQUEST_SETUP_DEFAULT_PDP */
/* ril_commands.h : {RIL_REQUEST_SETUP_DATA_CALL, dispatchStrings, responseStrings} */
/* ril_commands.h : {RIL_REQUEST_SETUP_DEFAULT_PDP, dispatchStrings, responseStrings, RIL_CMD_PROXY_3} */
/* Change name from requestSetupDefaultPDP to requestSetupDataCall */
void requestSetupDataCall(void * data, size_t datalen, RIL_Token t)
{
    /* In GSM with CDMA version: DOUNT - data[0] is radioType(GSM/UMTS or CDMA), data[1] is profile,
     * data[2] is apn, data[3] is username, data[4] is passwd, data[5] is authType (added by Android2.1)
     * data[6] is cid field added by mtk for Multiple PDP contexts setup support 2010-04
     */
    const char* profile = ((const char **)data)[1];
    const char* apn = ((const char **)data)[2];
    const char* username = ((const char **)data)[3];
    const char* passwd = ((const char **)data)[4];
    const int authType = getAuthType(((const char **)data)[5]);
    const int protocol = get_protocol_type(((const char **)data)[6]);
    const int interfaceId = atoi(((const char **)data)[7]) - 1;
    RIL_Default_Bearer_VA_Config_Struct defaultBearerVaConfig;
    int requestParamNumber = (datalen/sizeof(char*));

    memset(&defaultBearerVaConfig, 0, sizeof(RIL_Default_Bearer_VA_Config_Struct));
    LOGD("requestSetupData with datalen=%d and parameter number=%d", datalen, requestParamNumber);

#if MTK_VOLTE_SUPPORT
    if (requestParamNumber == 17) { // TODO: need to check this magic number
        defaultBearerVaConfig.isValid = atoi(((const char **)data)[8]);
    }
#endif

    LOGD("requestSetupData profile=%s, apn=%s, username=%s, password=xxxx, authType=%d, protocol=%d, interfaceId=%d, defaultBearerVaConfig=%d",
        profile, apn, username, authType, protocol, interfaceId, defaultBearerVaConfig.isValid);

    if(defaultBearerVaConfig.isValid) {
        defaultBearerVaConfig.qos.qci = atoi(((const char **) data)[9]);
        defaultBearerVaConfig.qos.dlGbr = atoi(((const char **) data)[10]);
        defaultBearerVaConfig.qos.ulGbr = atoi(((const char **) data)[11]);
        defaultBearerVaConfig.qos.dlMbr = atoi(((const char **) data)[12]);
        defaultBearerVaConfig.qos.ulMbr = atoi(((const char **) data)[13]);
        defaultBearerVaConfig.emergency_ind = atoi(((const char **) data)[14]);
        defaultBearerVaConfig.pcscf_discovery_flag = atoi(((const char **) data)[15]);
        defaultBearerVaConfig.signalingFlag = atoi(((const char **) data)[16]);

        LOGD("requestSetupData defaultBearerVaConfig QOS[qci=%d, dlGbr=%d, ulGbr=%d, dlMbr=%d, ulMbr=%d] flag[emergency_ind=%d, pcscf_discovery_flag=%d, signalingFlag=%d]",
            defaultBearerVaConfig.qos.qci, defaultBearerVaConfig.qos.dlGbr, defaultBearerVaConfig.qos.ulGbr, defaultBearerVaConfig.qos.dlMbr, defaultBearerVaConfig.qos.ulMbr,
            defaultBearerVaConfig.emergency_ind, defaultBearerVaConfig.pcscf_discovery_flag, defaultBearerVaConfig.signalingFlag);
    }

#if MTK_VOLTE_SUPPORT
    if (requestParamNumber != 17) { // TODO: need to check this magic number
#else
    if (requestParamNumber != 8) {
#endif        /* Last parm is the authType instaed of cid */
        LOGD("requestSetupData with incorrect parameters");
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    }

#if defined(PURE_AP_USE_EXTERNAL_MODEM) && !defined(MT6280_SUPER_DONGLE)
    if (strcmp(profile, "1002") == 0) {
        // Always request for CID 1 for default connection
        requestSetupDataCallOverIPv6(apn, username, passwd, authType, protocol, 0, 1, profile, t);
    } else {
        requestSetupDataCallOverIPv6(apn, username, passwd, authType, protocol, 1, 2, profile, t);
    }
#else
    //genenal project
    if (defaultBearerVaConfig.emergency_ind == 1) { // Emergency
        requestSetupDataCallEmergency(apn, username, passwd, authType, protocol, interfaceId, profile, (void *) &defaultBearerVaConfig, t);
    } else {
        requestSetupDataCallOverIPv6(apn, username, passwd, authType, protocol, interfaceId, profile, (void *) &defaultBearerVaConfig, t);
    }
#endif
}

/* 41:RIL_REQUEST_DEACTIVATE_DATA_CALL/RIL_REQUEST_DEACTIVATE_DEFAULT_PDP */
/* ril_commands.h : {RIL_REQUEST_DEACTIVATE_DATA_CALL, dispatchStrings, responseVoid} */
/* ril_commands.h : {RIL_REQUEST_DEACTIVATE_DEFAULT_PDP, dispatchStrings, responseVoid, RIL_CMD_PROXY_3} */
/* Change name from requestDeactiveDefaultPdp to requestDeactiveDataCall */
void requestDeactiveDataCall(void * data, size_t datalen, RIL_Token t)
{
#ifdef MTK_RIL_MD2
    if (getExternalModemSlot() >= 0) {
        LOGD("[%s] PPP shutdown", __FUNCTION__);
        shutdownPPP(t);
        int cidArray[1] = {1};
        RIL_onRequestComplete(t, RIL_E_SUCCESS, cidArray, sizeof(cidArray));
        return;
    }
#endif

    const char *cid;
    char *cmd;
    int err;
    ATResponse *p_response = NULL;
    int i = 0, lastPdnCid = INVALID_CID;
	int deactiveDataCallCid[pdnInfoSize];    //using [0] for primary cid, dedicate cid following index 0
    int responseLen = 0;
    int *pResponse = NULL;
    int interfaceId = atoi(((const char **)data)[0]);
    int responseDataSize = 0;
    LOGD("[%s] interfaceId=%d X", __FUNCTION__, interfaceId);

    int needHandleLastPdn = 0;
    int lastPdnState = DATA_STATE_INACTIVE;
	int isEmergency = 0;
    // AT+CGACT=<state>,<cid>;  <state>:0-deactivate;1-activate
    for(i = 0; i < pdnInfoSize; i++) {
        deactiveDataCallCid[i] = INVALID_CID;
        if (pdn_info[i].interfaceId == interfaceId) {
            if (!pdn_info[i].isDedicateBearer) {
                int err = deactivateDataCall(pdn_info[i].cid, DATA_CHANNEL_CTX);
                switch (err) {
                case CME_SUCCESS:
                    break;
                case CME_L4C_CONTEXT_CONFLICT_DEACT_ALREADY_DEACTIVATED:
                    LOGD("[%s] deactivateDataCall cid%d already deactivated", __FUNCTION__, i);
                    break;
                case CME_LAST_PDN_NOT_ALLOW:
                    if (needHandleLastPdn == 0) {
                        needHandleLastPdn = 1;
                        lastPdnState = pdn_info[i].active;
                        isEmergency = pdn_info[i].isEmergency;
                        lastPdnCid = pdn_info[i].cid;
                        LOGD("[%s] deactivateDataCall cid%d is the last PDN, state: %d", __FUNCTION__, i, lastPdnState);
                    }
                    break;
                case CME_LOCAL_REJECTED_DUE_TO_PS_SWITCH:
                    //in this case, we make the deactivation fail
                    //after switch, framework will check data call list and to deactivation again
                    LOGD("[%s] receive LOCAL_REJECTED_DUE_TO_PS_SWITCH", __FUNCTION__);
                    goto error;
                default:
                    goto error;
                };
                deactiveDataCallCid[responseLen++] = i; //primary cid
            }
        }
    }

    configureNetworkInterface(interfaceId, 0);
    for(i = 0; i < pdnInfoSize; i++) { //fixed network interface not disable issue while deactivating)
        if (pdn_info[i].interfaceId == interfaceId && !pdn_info[i].isDedicateBearer) {
            // clear only default bearer
            clearPdnInfo(&pdn_info[i]);
        }
    }

    if (responseLen > 0) {
        responseDataSize = sizeof(int) * responseLen;
        pResponse = alloca(responseDataSize);
        memcpy(pResponse, deactiveDataCallCid, responseDataSize);
    }

    if (needHandleLastPdn) {
        if (lastPdnState != DATA_STATE_LINKDOWN) {
        if(0 != handleLastPdnDeactivation(isEmergency, lastPdnCid, DATA_CHANNEL_CTX)) {
            goto error;
        }
        } else {
            LOGD("[%s] last pdn alread linkdown", __FUNCTION__);
        }
        pdn_info[lastPdnCid].active = DATA_STATE_LINKDOWN;
        pdn_info[lastPdnCid].cid = lastPdnCid;
        pdn_info[lastPdnCid].primaryCid = lastPdnCid;
    }

#if defined(PURE_AP_USE_EXTERNAL_MODEM)
    updateAvailablePdpNum();
#endif

    LOGD("[%s] response data size: %d, deactivate cid num: %d E", __FUNCTION__, responseDataSize,
            responseLen);
    //response deactivation result first then do re-attach
    RIL_onRequestComplete(t, RIL_E_SUCCESS, pResponse, responseDataSize);
    AT_RSP_FREE(p_response);
    return;
error:
    LOGE("[%s] error E", __FUNCTION__);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    AT_RSP_FREE(p_response);
}

/* 56:RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE/RIL_REQUEST_LAST_PDP_FAIL_CAUSE */
/* ril_commands.h : {RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE, dispatchVoid, responseInts} */
/* ril_commands.h : {RIL_REQUEST_LAST_PDP_FAIL_CAUSE, dispatchVoid, responseInts, RIL_CMD_PROXY_3} */
/* Change name from requestLastPdpFailCause to requestLastDataCallFailCause */
// TODO: requestLastDataCallFailCause/requestLastPdpFailCause
void requestLastDataCallFailCause(void * data, size_t datalen, RIL_Token t)
{
    int lastPdpFailCause = 14;
    lastPdpFailCause = getLastDataCallFailCause();
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &lastPdpFailCause, sizeof(lastPdpFailCause));
}

/* 57:RIL_REQUEST_DATA_CALL_LIST/RIL_REQUEST_PDP_CONTEXT_LIST */
/* ril_commands.h : {RIL_REQUEST_DATA_CALL_LIST, dispatchVoid, responseDataCallList} */
/* ril_commands.h : {RIL_REQUEST_PDP_CONTEXT_LIST, dispatchVoid, responseContexts, RIL_CMD_PROXY_3} */
/* Change name from requestPdpContetList to requestDataCallList */
void requestDataCallList(void * data, size_t datalen, RIL_Token t)
{
    requestOrSendDataCallList(&t, getRILIdByChannelCtx(getRILChannelCtxFromToken(t)));
}

void requestSetInitialAttachApn(void * data, size_t datalen, RIL_Token t)
{
    RIL_InitialAttachApn* param = data;
    LOGD("IA: requestSetInitialAttachApn [apn=%s, protocol=%s, auth_type=%d, username=%s, password=%s, operatorNumeric=%s, canHandleIms=%d], attachApnSupport=%d, isCid0Support=%d",
        param->apn, param->protocol, param->authtype, param->username, "XXXXX"/*param->password*/, param->operatorNumeric, param->canHandleIms, getAttachApnSupport(), isCid0Support);

    RILId rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    char operatorNumeric[PROPERTY_VALUE_MAX] = {0};
    int mappedSimId = getMappingSIMByCurrentMode(rid);
    if (mappedSimId == GEMINI_SIM_1) {
        property_get(PROPERTY_SIM_OPERATOR_NUMERIC, operatorNumeric, "");
    } else {
        char operatorProperty[100] = {0};
        sprintf(operatorProperty, "%s.%d", PROPERTY_SIM_OPERATOR_NUMERIC, mappedSimId+1);
        property_get(operatorProperty, operatorNumeric, "");
        LOGD("IA: requestSetInitialAttachApn current SIM operator numeric key=%s, value=%s", operatorProperty, operatorNumeric);
    }

    if (strcmp(operatorNumeric, param->operatorNumeric) != 0) {
        LOGE("IA: requestSetInitialAttachApn current SIM operator numeric is different from the parameter, skip setting attach APN [%s, %s]", operatorNumeric, param->operatorNumeric);
        goto error;
    } else if (isCid0Support) {
        if (rid != MTK_RIL_SOCKET_1) {
            LOGD("IA: requestSetInitialAttachApn but not to execute current SIM");
            goto error;
        } else if (getAttachApnSupport() == ATTACH_APN_NOT_SUPPORT) {
            LOGD("IA: requestSetInitialAttachApn but attach apn is not support");
            goto error;
        } else {

            if (isDualTalkMode()) {
                #ifdef MTK_RIL_MD2
                    LOGD("IA: requestSetInitialAttachApn but dual talk MD2 is not support LTE");
                    goto error;
                #endif
            }

            int triggerPsAttach = 1;
            if (getMtkShareModemCurrent() > 1) {
                //This is for gemini load that attach apn should be set on data SIM
                char dataSettings[PROPERTY_VALUE_MAX] = {0};
                int simId3G = RIL_get3GSIM();
                property_get("persist.ril.gprs.setting", dataSettings, "0");

                if (atoi(dataSettings) == 0) {
                    LOGD("IA: requestSetInitialAttachApn data is disabled, trigger attach on 4G SIM");
                } else if (simId3G != atoi(dataSettings)) {
                    LOGD("IA: requestSetInitialAttachApn data is not enabled on 4G SIM,defind CID0 only");
                    triggerPsAttach = 0;
                }
            }

            char iccid[PROPERTY_VALUE_MAX] = {0};
            char iaProperty[PROPERTY_VALUE_MAX*2] = {0};

            property_get(PROPERTY_ICCID_SIM[getMappingSIMByCurrentMode(0)], iccid, "");
            getIaCache(iaProperty);

            if (strlen(iaProperty) != 0) {
                //here we need to check if current IA property is different than what we want to set
                //if different, need to clear cache and do PS detach
                //this scenario happens when attach APN is changed
                char iaParameter[PROPERTY_VALUE_MAX*2] = {0};
                if (strlen(param->password) == 0) {
                    sprintf(iaParameter, "%s,%s,%d,%s,%d,%s", iccid, param->protocol, param->authtype, param->username, param->canHandleIms, param->apn);
                } else {
                    //when password is set, iccid is not recorded
                    //so we do not need to compare iccid
                    sprintf(iaParameter, "%s,%s,%d,%s,%d,%s", "", param->protocol, param->authtype, param->username, param->canHandleIms, param->apn);
                }

                if (strcmp(iaParameter, iaProperty) != 0) {
                    LOGD("IA: requestSetInitialAttachApn the parameter is different than cache [%s][%s]", iaParameter, iaProperty);
                    if (strlen(iccid) > 0) {
                        if (triggerPsAttach) {
                            //when AT+EGREA=1 is set, EUTRAN would not be disabled after PS detached
                            at_send_command("AT+EGREA=1", NULL, DATA_CHANNEL_CTX);
                            requestDetachPS((void*)1, 1, t); //the 1st & 2nd parameters are used to prevent response is sent
                        } else {
                            LOGD("IA: requestSetInitialAttachApn not trigger re-attach");
                        }
                        memset(iaProperty, 0, sizeof(iaProperty));
                    } else {
                        LOGD("IA: requestSetInitialAttachApn no SIM detected");
                    }
                }
            }

            if (strlen(iaProperty) == 0) {
                RIL_Default_Bearer_VA_Config_Struct* defaultBearer = NULL;
                if (param->canHandleIms) {
                    defaultBearer = malloc(sizeof(RIL_Default_Bearer_VA_Config_Struct));
                    defaultBearer->signalingFlag = 1;
                    defaultBearer->pcscf_discovery_flag  =1;
                    defaultBearer->emergency_ind = 0;
                }
                if (!definePdnCtx(defaultBearer, param->apn, get_protocol_type(param->protocol), 0, getAuthTypeInt(param->authtype), param->username, param->password, DATA_CHANNEL_CTX)) {
                    LOGE("IA: requestSetInitialAttachApn define CID0 failed");
                    at_send_command("AT+EGREA=0", NULL, DATA_CHANNEL_CTX);
                    goto error;
                }

                if (defaultBearer != NULL) {
                    free(defaultBearer);
                    defaultBearer = NULL;
                }

                if (triggerPsAttach) {
                    //when AT+EGREA=0 is set, EUTRAN would be disabled after PS detached
                    at_send_command("AT+EGREA=0", NULL, DATA_CHANNEL_CTX);
                    if (!doPsAttach(DATA_CHANNEL_CTX)) {
                        LOGE("IA: requestSetInitialAttachApn trigger attach failed");
                        //we sill continue to update cache even if attach is failed
                    }
                    //Trigger query apn to update network assigned apn
                    int matchCount = 0; //use to prevent queryMatchedPdnWithSameApn exception
                    queryMatchedPdnWithSameApn("", &matchCount, DATA_CHANNEL_CTX);

                    if (strlen(param->password) == 0) {
                        setIaCache(iccid, param->protocol, param->authtype, param->username, param->canHandleIms, param->apn);
                    } else {
                        LOGD("IA: requestSetInitialAttachApn initial attach APN contain password, set patch to invalid value");
                        setIaCache("", "", 0, "", 0, "");
                    }
                } else {
                    LOGD("IA: requestSetInitialAttachApn not to do PS attach");
                }
            } else {
                LOGD("IA: requestSetInitialAttachApn valid IA property is set, not to do attach again");
            }
        }
    } else {
        LOGD("IA: requestSetInitialAttachApn but CID0 is not support");
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void requestDialUpCsd(void* data, size_t datalen, RIL_Token t)
{
    const char *dialUpNumber, *slotId;
    int request_parm_num = (datalen/sizeof(char*));
    char* command;
    int err = 0;
    int isPPPSucceed = 0;

#ifndef MTK_CSD_DIALER_SUPPORT
    LOGE("CSD is not support");
    goto error;
#endif

    RIL_Data_Call_Response_v6* response = alloca(sizeof(RIL_Data_Call_Response_v6));
    initialDataCallResponse(response, 1);

    LOGD("requestCsdDialUp() with datalen=%d,parm_num=%d",datalen,request_parm_num);
    dialUpNumber = ((const char**)data)[0];
    slotId = ((const char **)data)[1];

    err = openDataChannel("/dev/pttycsd", 1, &g_CsdDataChannel, g_CsdDataChannel.id);
    if (err < 0) {
        LOGE("requestCsdDialUp(): fail to open channel.");
        goto error;
    }

    // Select CSD dial up SIM.
    switch (slotId[0]) {
        case '0':
            command = "AT+ESUO=4";
            break;
        case '1':
            command = "AT=ESUO=5";
            break;
        default:
            command = "AT+ESUO=4";
            break;
    }
    err = at_send_command_to_data_channel(command, NULL, &g_CsdDataChannel);
    if (err < 0) {
        LOGE("requestCsdDialUp(): fail to select dial up SIM.");
        goto error;
    }

    // CSD dial up.
    asprintf(&command, "ATD%s", dialUpNumber);
    err = at_send_command_to_data_channel(command, NULL, &g_CsdDataChannel);
    free(command);
    if (err < 0) {
        LOGE("requestCsdDialUp(): fail to dial up.");
        goto error;
    }
    closeDataChannel(&g_CsdDataChannel);

    isPPPSucceed = startPPPCall(response, "pppd_csd");


    if (!isPPPSucceed)
        goto error;

    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(RIL_Data_Call_Response_v6));
    if (response != NULL)
        freeDataResponse(response);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    if (response != NULL)
        freeDataResponse(response);
}

void requestSetupPPPDataCall(void * data, size_t datalen, RIL_Token t) {
    const char *apn;
    //const char *username, *passwd, *authType, *protocol;
    char *req_dns1="", *req_dns2="";
    char *cmd;
    int err;
    int isPPPSucceed = 0;
    ATResponse *p_response = NULL;
    RIL_Data_Call_Response_v6 *responses = alloca(sizeof(RIL_Data_Call_Response_v6));
    initialDataCallResponse(responses, 1);

    LOGD("Shutdown PPP before setup data call");
    shutdownPPP(t);

    RILChannelCtx* p_channel = openPPPDataChannel(1);
    if (p_channel == NULL) {
        goto error;
    }

    int response_cid = 0;
    /* response[0]:cid, response[1]:interface name, response[2]:ipaddr, response[3]:dns1, response[4]:dns2 [5]: remote-ip*/
    //char *response[6] = {0};

    /* In GSM with CDMA version: DOUNT - data[0] is radioType(GSM/UMTS or CDMA), data[1] is profile,
     * data[2] is apn, data[3] is username, data[4] is passwd: modified 2009-0908 by mtk01411
     */
    apn = ((const char **)data)[2];

    LOGD("requesting data connection to APN '%s'", apn);

    asprintf(&cmd, "AT+CGDCONT=1,\"IP\",\"%s\",,0,0", apn);
    //FIXME check for error here
    err = at_send_command_to_ppp_data_channel(cmd, NULL, p_channel);
    free(cmd);

    // packet-domain event reporting
    err = at_send_command_to_ppp_data_channel("AT+CGEREP=1,0", NULL, p_channel);

    // Hangup anything that's happening there now
    // 20120508: we shutdown ppp at the begining of this function, so remove this
    //err = at_send_command_to_ppp_data_channel("AT+CGACT=0,1", NULL, p_channel);

    // Start data on PDP context 1
    err = at_send_command_to_ppp_data_channel("ATD*99***1#", &p_response, p_channel);

    if (err < 0 || p_response->success == 0) {
        goto error;
    }

    closePPPDataChannel();

    p_channel = openPPPDataChannel(0);
    purge_data_channel(p_channel);
    closePPPDataChannel();

    //TODO: Check
    isPPPSucceed = startPPPCall(responses, "pppd_gprs");


    if (!isPPPSucceed)
        goto error;

    RIL_onRequestComplete(t, RIL_E_SUCCESS, responses, sizeof(RIL_Data_Call_Response_v6));
    at_response_free(p_response);
    if (responses != NULL)
        freeDataResponse(responses);
    return;
error:
    closePPPDataChannel();
    LOGD("Shutdown PPP before return fail");
    shutdownPPP(t);

    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
    if (responses != NULL)
        freeDataResponse(responses);
}

int isPPPRunning(char* runningProperty) {
    char runningCode[PROPERTY_VALUE_MAX] = {0};
    LOGD("isPPPRunning:%s", runningProperty);
    property_get(runningProperty, runningCode, "");
    if (strcmp(runningCode, PPP_DEF_RUNNING) == 0 || strcmp(runningCode, PPP_DEF_STOPPING) == 0)
        return 1;
    else
        return 0;
}

int startPPPCall(RIL_Data_Call_Response_v6* response, char* port) {

    property_set("net.gprs.ppp-exit", "");
    // Start pppd
    property_set("ctl.start", port);

    char *cmd;
    asprintf(&cmd, "init.svc.%s", port);

    int count = 0;
    while (count < 10) {
        if (isPPPRunning(cmd)) {
            LOGD("pppd started [%d]", count);
            break;
        } else {
            LOGD("Wait pppd started [%d]", count);
            sleep(1);
        }
        ++count;
    }
    free(cmd);

    int isPolling = 1;
    char* exitCode = malloc(PROPERTY_VALUE_MAX);
    char* runningCode = malloc(PROPERTY_VALUE_MAX);
    count = 0;
    int isPPPSucceed = 0;
    while (isPolling && count < 300) {
        memset(exitCode, 0, PROPERTY_VALUE_MAX);
        memset(runningCode, 0, PROPERTY_VALUE_MAX);
        property_get("net.gprs.ppp-running", runningCode, "");
        property_get("net.gprs.ppp-exit", exitCode, "");

        if (strcmp(exitCode, "") != 0) {
            LOGD("pppd exited [%s] during starting", exitCode);
            isPolling = 0;
        } else if (strcmp(runningCode, "yes") == 0) {
            //PPP Parameter
            char local[PROPERTY_VALUE_MAX] = {0};
            char remote[PROPERTY_VALUE_MAX] = {0};
            char dns1[PROPERTY_VALUE_MAX] = {0};
            char dns2[PROPERTY_VALUE_MAX] = {0};

            LOGD("polling pppd done");

            property_get("net.ppp0.local-ip", local, "0.0.0.0");
            property_get("net.ppp0.remote-ip", remote, "0.0.0.0");
            property_get("net.ppp0.dns1", dns1, "0.0.0.0");
            property_get("net.ppp0.dns2", dns2, "0.0.0.0");

            response->status = PDP_FAIL_NONE;
            response->suggestedRetryTime = 0;
            response->cid = 0;
            response->active = 2;
            asprintf(&response->type, "%s", "PPP");
            asprintf(&response->ifname, "%s", "ppp0");
            //address, local-ip
            asprintf(&response->addresses, "%s", local);
            //gateway, remote-ip
            asprintf(&response->gateways, "%s", remote);
            //dns
            asprintf(&response->dnses, "%s %s", dns1, dns2);

            LOGD("PPP Data call response: status=%d, suggestedRetryTime=%d, cid=%d, active=%d, \
                type=%s, ifname=%s, addresses=%s, dnses=%s, gateways=%s",
                response->status, response->suggestedRetryTime,
                response->cid, response->active, response->type,
                response->ifname, response->addresses,
                response->dnses, response->gateways);

            isPPPSucceed = 1;
            isPolling = 0;
        } else {
            LOGD("polling pppd not finished [%d]", count);
            sleep(1);
        }
        ++count;
    }
    free(exitCode);
    free(runningCode);
    return isPPPSucceed;

}

void shutdownPPP(RIL_Token t) {
    int isPPPOriginRunning = isPPPRunning(PPP_PPP_RUNNING_PROPERTY);
    int count = 0;

    if (isPPPOriginRunning) {
        do {
            LOGD("Shutdown ppp is retring [%d]", count);
            //property_set("ctl.stop", "pppd_gprs");
            at_send_command("AT+CGACT=0,1", NULL, DATA_CHANNEL_CTX);
            ++count;
            sleep(1);
        } while (count < 60 && isPPPRunning(PPP_PPP_RUNNING_PROPERTY));

        LOGD("Shutdown ppp is done [%d]", count);

        RILChannelCtx* p_channel = openPPPDataChannel(0);
        //waitForTargetPPPStopped(p_channel);
        closePPPDataChannel();
    }

    RILChannelCtx* p_channel = openPPPDataChannel(0);
    purge_data_channel(p_channel);
    closePPPDataChannel();
}

void onGPRSDeatch(char* urc, RILId rid)
{

    LOGD("onGPRSDeatch:%s", urc);
    RIL_onUnsolicitedResponse(RIL_UNSOL_GPRS_DETACH, NULL, 0, rid);
}

/* Change name from onPdpContextListChanged to onDataCallListChanged */
/* It can be called in onUnsolicited() mtk-ril\ril_callbacks.c */
void onDataCallListChanged(void* param)
{
    RILId rilid = *((RILId *) param);
    requestOrSendDataCallList(NULL, rilid);
}

void onMePdnActive(void* param)
{
    int activatedCid = *((int *) param);

    LOGD("onMePdnActive CID%d is activated and current state is %d", activatedCid, pdn_info[activatedCid].active);
    if (pdn_info[activatedCid].active == DATA_STATE_INACTIVE) {
        pdn_info[activatedCid].active = DATA_STATE_LINKDOWN; // Update with link down state.
        pdn_info[activatedCid].cid = activatedCid;
        pdn_info[activatedCid].primaryCid = activatedCid;
    }
    free(param);
}

int isAlwaysAttach()
{
    return 1;
}

extern int rilDataMain(int request, void *data, size_t datalen, RIL_Token t)
{
    switch (request) {
        case RIL_REQUEST_SETUP_DATA_CALL:
        case RIL_REQUEST_SETUP_EMERGENCY_DATA_CALL:
            if (getExternalModemSlot() >= 0) {
#ifdef MTK_RIL_MD2
                MTK_REQUEST_SETUP_PPP_CALL(data, datalen, t);
#else
                MTK_REQUEST_SETUP_DATA_CALL(data, datalen, t);
#endif
            } else {
                MTK_REQUEST_SETUP_DATA_CALL(data, datalen, t);
            }
            break;
        case RIL_REQUEST_DATA_CALL_LIST:
            MTK_REQUEST_DATA_CALL_LIST(data, datalen, t);
            break;
        case RIL_REQUEST_DEACTIVATE_DATA_CALL:
            MTK_REQUEST_DEACTIVATE_DATA_CALL(data, datalen, t);
            break;
        case RIL_REQUEST_SET_INITIAL_ATTACH_APN:
            MTK_REQUEST_SET_INITIAL_ATTACH_APN(data, datalen, t);
            break;
        case RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE:
            MTK_REQUEST_LAST_DATA_CALL_FAIL_CAUSE(data, datalen, t);
            break;
        case RIL_REQUEST_SET_SCRI:
            MTK_REQUEST_SET_SCRI(data, datalen, t);
            break;
        //[New R8 modem FD]
        case RIL_REQUEST_SET_FD_MODE:
             MTK_REQUEST_FD_MODE(data, datalen, t);
            break;
        case RIL_REQUEST_DETACH_PS:
            MTK_REQUEST_DETACH_PS(data, datalen, t);
            break;
        //[MMDC]
        case RIL_REQUEST_GET_PSDM_DECISION:
            MTK_REQUEST_GET_PSDM_DECISION(data, datalen, t);
            break;
        case RIL_REQUEST_CONFIRM_PS_SWITCH:
            MTK_REQUEST_CONFIRM_PS_SWITCH(data, datalen, t);
            break;
        case RIL_REQUEST_CONFIRM_PS_SWITCH_ATTACH:
            MTK_REQUEST_CONFIRM_PS_SWITCH_ATTACH(data, datalen, t);
            break;
        case RIL_REQUEST_RESET_MODE_AFTER_SWITCH:
            MTK_REQUEST_RESET_MODE_AFTER_SWITCH(data, datalen, t);
            break;
        case RIL_REQUEST_PS_REATTACH:
            MTK_REQUEST_PS_REATTACH(data, datalen, t);
            break;
        case RIL_REQUEST_DIAL_UP_CSD:
            MTK_REQUEST_DIAL_UP_CSD(data, datalen, t);
            break;
        //VoLTE
        case RIL_REQUEST_SETUP_DEDICATE_DATA_CALL:
            requestSetupDedicateDataCall(data, datalen, t);
            break;
        case RIL_REQUEST_DEACTIVATE_DEDICATE_DATA_CALL:
            requestDeactivateDedicateDataCall(data, datalen, t);
            break;
        case RIL_REQUEST_MODIFY_DATA_CALL:
            requestModifyDataCall(data, datalen, t);
            break;
        case RIL_REQUEST_ABORT_SETUP_DATA_CALL:
            requestAbortSetupDataCall(data, datalen, t);
            break;
        case RIL_REQUEST_PCSCF_DISCOVERY_PCO:
            requestPcscfPco(data, datalen, t);
            break;
        case RIL_REQUEST_CLEAR_DATA_BEARER:
            requestClearDataBearer(data,datalen,t);
            break;
        case RIL_REQUEST_SET_DATA_ON_TO_MD:
            MTK_REQUEST_SET_DATA_ON_TO_MD(data, datalen, t);
            break;
        case RIL_REQUEST_SET_REMOVE_RESTRICT_EUTRAN_MODE:
            MTK_REQUEST_SET_REMOVE_RESTRICT_EUTRAN_MODE(data, datalen, t);
            break;
        default:
            return 0; /* no matched request */
    }

    return 1; /* request found and handled */

}

extern int rilDataUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel)
{
    int rilid = getRILIdByChannelCtx(p_channel);
    RILChannelCtx* pDataChannel = getChannelCtxbyId(sCmdChannel4Id[rilid]);
    //[MMDC]
    char property_isSwitching[PROPERTY_VALUE_MAX] = { 0 };
    char property_protocol[PROPERTY_VALUE_MAX] = {0};

    if (strStartsWith(s, "+CGEV: NW DEACT") || strStartsWith(s, "+CGEV: ME DEACT") ||
        strStartsWith(s, "+CGEV: NW REACT") ||
        strStartsWith(s, "+CGEV: NW PDN DEACT") || strStartsWith(s, "+CGEV: ME PDN DEACT"))
    {
        char* urc = NULL;
        asprintf(&urc, "%s", s);
        TimeCallbackParam* param = (TimeCallbackParam*)malloc(sizeof(TimeCallbackParam));
        param->rid = rilid;
        param->urc = urc;

        RIL_requestProxyTimedCallback (onPdnDeactResult,
               param,
               &TIMEVAL_0,
               pDataChannel->id,
               "onPdnDeactResult");
        return 1;
    } else if (strStartsWith(s, "+CGEV: NW ACT") || strStartsWith(s, "+CGEV: NW MODIFY")) {
        //+CGEV: NW ACT <p_cid>, <cid>, <event_type>
        //+CGEV: NW MODIFY <cid>, <change_reason>, <event_type>
        LOGD("rilDataUnsolicited receive bearer activation/modification by network [%s]", s);
        char* urc = NULL;
        asprintf(&urc, "%s", s);

        TimeCallbackParam* param = (TimeCallbackParam*)malloc(sizeof(TimeCallbackParam));
        param->rid = rilid;
        param->urc = urc;
        if (strStartsWith(s, "+CGEV: NW MODIFY"))
            param->isModification = 1;
        else
            param->isModification = 0;

        //be aware that the param->urc & param should be free in onNetworkBearerUpdate
        RIL_requestProxyTimedCallback (onNetworkBearerUpdate,
               param,
               &TIMEVAL_0,
               pDataChannel->id,
               "onNetworkBearerUpdate");
        return 1;
    } else if (strStartsWith(s, "+CGEV: ME")) {
        if (strStartsWith(s, "+CGEV: ME PDN ACT")) {
            char *p = strstr(s, ME_PDN_URC);
            if( p == NULL) {
                return 1;
            }
            p = p + strlen(ME_PDN_URC) + 1;
            int *activatedCid = malloc(sizeof(int));
            *activatedCid = atoi(p);

            LOGD("rilDataUnsolicited CID%d is activated and current state is %d", *activatedCid, pdn_info[*activatedCid].active);

            RIL_requestProxyTimedCallback (onMePdnActive,
                (void *) activatedCid,
                &TIMEVAL_0,
                pDataChannel->id,
                "onMePdnActive");

        } else {
            //+CGEV: ME related cases should be handled in setup data call request handler
            LOGD("rilDataUnsolicited ignore +CGEV: ME cases (%s)", s);
        }
        return 1;
    } else if (strStartsWith(s, "+CGEV:")) {
        /* Really, we can ignore NW CLASS and ME CLASS events here,
         * but right now we don't since extranous
         * RIL_UNSOL_DATA_CALL_LIST_CHANGED calls are tolerated
         */
        /* can't issue AT commands here -- call on main thread */
        // + CGEV:DETACH only means modem begin to detach gprs.
        //if(strstr(s, "DETACH")){
            //onGPRSDeatch((char*) s,(RILId) rilid);
        //}else{
        //RIL_requestTimedCallback (onDataCallListChanged, &s_data_ril_cntx[rilid], NULL);
        //}
        if (s_md_off)	{
            LOGD("rilDataUnsolicited(): modem off!");
            RIL_onUnsolicitedResponse(RIL_UNSOL_DATA_CALL_LIST_CHANGED, NULL, 0, rilid);
         } else {

            //[MMDC]
            #ifdef MTK_LTE_DC_SUPPORT
            property_get("ril.epsd.protocol", property_protocol, "-1");
            property_get("ril.epsd.switching", property_isSwitching, "0");
            LOGD("rilid: %d , switching: %s , protocol: %s" , rilid, property_isSwitching, property_protocol );
            if ( (atoi(property_isSwitching) == 1) && (atoi(property_protocol) == rilid) ) {
                LOGD("ignore NW DETACH when switching");
                return 1;
            }
            #endif

            #ifdef MTK_LTE_DC_SUPPORT
            RIL_requestProxyTimedCallback (onDataCallListChanged,
                &s_data_ril_cntx[rilid],
                &MMDC_TIMEVAL,
                pDataChannel->id,
                "onDataCallListChanged");
            #else
            RIL_requestProxyTimedCallback (onDataCallListChanged,
                &s_data_ril_cntx[rilid],
                &TIMEVAL_0,
                pDataChannel->id,
                "onDataCallListChanged");
            #endif
         }

        return 1;

#ifdef WORKAROUND_FAKE_CGEV
    } else if (strStartsWith(s, "+CME ERROR: 150")) {

        if (s_md_off)	{
            LOGD("rilDataUnsolicited(): modem off!");
            RIL_onUnsolicitedResponse(RIL_UNSOL_DATA_CALL_LIST_CHANGED, NULL, 0, rilid);
         } else {
            RIL_requestProxyTimedCallback (onDataCallListChanged,
                &s_data_ril_cntx[rilid],
                &TIMEVAL_0,
                pDataChannel->id,
                "onDataCallListChanged");
         }

        return 1;

#endif /* WORKAROUND_FAKE_CGEV */
    }

#ifdef PACKETS_FLUSH_INDICATION
    else if(strStartsWith(s,"+EPKTFI:")){
        onPacketsFlushIndication((char*)s, (RILId)rilid);
        return 1;
    }
#endif/*Packets flush indication(+EPKTFI:cid)*/

    else if(strStartsWith(s, "+ESCRI:")) {
        onScriResult((char*) s,(RILId) rilid);
        return 1;
    } else if (strStartsWith(s, "+EPSD:")){
        onPsdmDecisionChanged((char*) s,(RILId) rilid);
        return 1;
    } else if (strStartsWith(s, "+EGIND: 1")){
        #ifdef MTK_LTE_DC_SUPPORT
        RIL_onUnsolicitedResponse(RIL_UNSOL_RESPONSE_PS_NETWORK_STATE_CHANGED, NULL, 0, (RILId) rilid);
        #endif
        return 1;
    } else if (strStartsWith(s, "+EGIND: 0")){
        #ifdef MTK_LTE_DC_SUPPORT
        onSpecialBandSupport((char*) s,(RILId) rilid);
        #endif
        return 1;
    } else if (strStartsWith(s, "+ECODE33")){
        //M: CC33
        RIL_onUnsolicitedResponse(RIL_UNSOL_REMOVE_RESTRICT_EUTRAN, NULL, 0, (RILId) rilid);
        return 1;
    }

    return 0;
}


