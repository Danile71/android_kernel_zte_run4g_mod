#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include "nl80211_copy.h"
#include "event_loop.h"
#include "nl80211.h"

struct nl80211_cfg { 
	struct nl_cb *nl_cb;
	struct nl_sock *nl;
	struct nl_sock *nl_event;
	s32 nl80211_id;
	s32 ifindex;
	struct wifi_callback_funcs agps_cb;
}; 

struct family_data {
	const s8 *group;
	s32 id;
};

struct driver_ap_info {
	u8 bssid[6];
    s16 ap_rssi; //-127..128
    u16 ap_channel;   //0..256
    wifi2agps_ap_device_type_e phy_type;
};

struct driver_ap_list {
	u8 num;
	struct driver_ap_info ap_info[32];
};

static struct nl80211_cfg nl_cfg;
static int android_genl_ctrl_resolve(struct nl_sock *handle, const s8 *name);
static struct nl_sock * nl_create_handle(struct nl_cb *cb, const s8 *dbg)
{
	struct nl_sock *handle;

	handle = nl_socket_alloc_cb(cb);
	if (handle == NULL) {
		wifi2agps_log(MSG_ERROR, "nl80211: Failed to allocate netlink "
			   "callbacks (%s)", dbg);
		return NULL;
	}

	if (genl_connect(handle)) {
		wifi2agps_log(MSG_ERROR, "nl80211: Failed to connect to generic "
			   "netlink (%s)", dbg);
		nl_socket_free(handle);
		return NULL;
	}

	return handle;
}

static void nl_destroy_handles(struct nl_sock **handle)
{
	if (*handle == NULL)
		return;
	nl_socket_free(*handle);
	*handle = NULL;
}

static int ack_handler(struct nl_msg *msg, void *arg)
{
	int *err = arg;
	*err = 0;
	return NL_STOP;
}

static int finish_handler(struct nl_msg *msg, void *arg)
{
	int *ret = arg;
	*ret = 0;
	return NL_SKIP;
}

static int error_handler(struct sockaddr_nl *nla, struct nlmsgerr *err,
			 void *arg)
{
	int *ret = arg;
	*ret = err->error;
	return NL_SKIP;
}

static int no_seq_check(struct nl_msg *msg, void *arg)
{
	return NL_OK;
}

static int send_and_recv_msgs(struct nl80211_cfg *nl_cfg,
			 struct nl_sock *nl_sock, struct nl_msg *msg,
			 int (*valid_handler)(struct nl_msg *, void *),
			 void *valid_data)
{
	struct nl_cb *cb;
	int err = -ENOMEM;

	cb = nl_cb_clone(nl_cfg->nl_cb);
	if (!cb)
		goto out;

	err = nl_send_auto_complete(nl_sock, msg);
	if (err < 0)
		goto out;

	err = 1;

	nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &err);
	nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &err);
	nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &err);

	if (valid_handler)
		nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM,
			  valid_handler, valid_data);

	while (err > 0)
		nl_recvmsgs(nl_sock, cb);
 out:
	nl_cb_put(cb);
	nlmsg_free(msg);
	return err;
}

static int family_handler(struct nl_msg *msg, void *arg)
{
	struct family_data *res = arg;
	struct nlattr *tb[CTRL_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *mcgrp;
	int i;

	nla_parse(tb, CTRL_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);
	if (!tb[CTRL_ATTR_MCAST_GROUPS])
		return NL_SKIP;

	nla_for_each_nested(mcgrp, tb[CTRL_ATTR_MCAST_GROUPS], i) {
		struct nlattr *tb2[CTRL_ATTR_MCAST_GRP_MAX + 1];
		nla_parse(tb2, CTRL_ATTR_MCAST_GRP_MAX, nla_data(mcgrp),
			  nla_len(mcgrp), NULL);
		if (!tb2[CTRL_ATTR_MCAST_GRP_NAME] ||
		    !tb2[CTRL_ATTR_MCAST_GRP_ID] ||
		    strncmp(nla_data(tb2[CTRL_ATTR_MCAST_GRP_NAME]),
			       res->group,
			       nla_len(tb2[CTRL_ATTR_MCAST_GRP_NAME])) != 0)
			continue;
		res->id = nla_get_u32(tb2[CTRL_ATTR_MCAST_GRP_ID]);
		break;
	};

	return NL_SKIP;
}


static int nl_get_multicast_id(struct nl80211_cfg *nl_cfg,
			       const s8 *family, const s8 *group)
{
	struct nl_msg *msg;
	int ret = -1;
	struct family_data res = { group, -ENOENT };

	msg = nlmsg_alloc();
	if (!msg)
		return -ENOMEM;
	genlmsg_put(msg, 0, 0, genl_ctrl_resolve(nl_cfg->nl, "nlctrl"),
		    0, 0, CTRL_CMD_GETFAMILY, 0);
	NLA_PUT_STRING(msg, CTRL_ATTR_FAMILY_NAME, family);

	ret = send_and_recv_msgs(nl_cfg, nl_cfg->nl, msg, family_handler, &res);
	msg = NULL;
	if (ret == 0)
		ret = res.id;

nla_put_failure:
	nlmsg_free(msg);
	return ret;
}


static void * nl80211_cmd(struct nl80211_cfg *nl_cfg,
			  struct nl_msg *msg, int flags, uint8_t cmd)
{
	return genlmsg_put(msg, 0, 0, nl_cfg->nl80211_id,
			   0, flags, cmd, 0);
}

static void mlme_event_connect(struct nl80211_cfg *nl_cfg,
			       enum nl80211_commands cmd, struct nlattr *status,
			       struct nlattr *addr)
{
	wifi2agps_ap_info assoc_ap;	
	if (cmd == NL80211_CMD_CONNECT &&
	    nla_get_u16(status) != 0) {
		wifi2agps_log(MSG_DEBUG, "associate reject with status: %d", nla_get_u16(status));
		return;
	}
	memset(&assoc_ap, 0, sizeof(assoc_ap));
	memcpy(assoc_ap.ap_mac_addr, nla_data(addr), 6);
	//wifi2agps_log(MSG_DEBUG, "wifi associated");

	nl_cfg->agps_cb.wifi_associated(&assoc_ap);
	
}

static u8 checkIfIndex(struct nl80211_cfg *nl_cfg, struct nlattr *tb) {		
	if (!tb)
		return 0;
	
	//wifi2agps_log(MSG_DEBUG, "ifidx=%d, saved ifidx=%d", nla_get_u32(tb), nl_cfg->ifindex);
	if (nla_get_u32(tb) != nl_cfg->ifindex) {
		s8 if_name[16];
		s8 *return_if = if_indextoname(nla_get_u32(tb), if_name);

		if (return_if && strncmp(if_name, "wlan", 4) == 0)
			nl_cfg->ifindex = nla_get_u32(tb);
		else {
			wifi2agps_log(MSG_DEBUG, "return if=%s, errno=%s", 
				return_if==NULL ? "null":return_if, strerror(errno));
			return 0;
		}
	}
	return 1;
}

static int process_nl_event(struct nl_msg *msg, void *arg)
{
	struct nl80211_cfg *nl_cfg = arg;
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	int ifidx = -1;

	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);
	
	//wifi2agps_log(MSG_DEBUG, "new event %d", gnlh->cmd);
	switch (gnlh->cmd) {
	case NL80211_CMD_CONNECT:
	case NL80211_CMD_ROAM:
		if (!checkIfIndex(nl_cfg, tb[NL80211_ATTR_IFINDEX]))
			break;
		mlme_event_connect(nl_cfg, gnlh->cmd,
				   tb[NL80211_ATTR_STATUS_CODE],
				   tb[NL80211_ATTR_MAC]);
		break;
	case NL80211_CMD_DISCONNECT:
		if (!checkIfIndex(nl_cfg, tb[NL80211_ATTR_IFINDEX]))
			break;
		nl_cfg->agps_cb.wifi_disassociated();
		break;
	case NL80211_CMD_TESTMODE:
	{
		if (!tb[NL80211_ATTR_TESTDATA])
			break;
		struct nlattr *notify_tb[MTK_NL80211_ATTR_MAX];
		nla_parse_nested(notify_tb, MTK_NL80211_ATTR_MAX, tb[NL80211_ATTR_TESTDATA], NULL);

		if (!checkIfIndex(nl_cfg, notify_tb[MTK_NL80211_ATTR_IFINDEX]))
			break;
		/* wifi enable/disable */
		wifi2agps_log(MSG_DEBUG, "agps event=%d", nla_get_u8(notify_tb[MTK_NL80211_ATTR_CMD]));
		switch (nla_get_u8(notify_tb[MTK_NL80211_ATTR_CMD])) {
			case AGPS_EVENT_WIFI_ON:
				nl_cfg->agps_cb.wifi_enabled();
				break;
			case AGPS_EVENT_WIFI_OFF:
				nl_cfg->agps_cb.wifi_disabled();
				break;
			case AGPS_EVENT_WIFI_AP_INFO:
			{
				wifi2agps_ap_info_list ap_list;
				struct driver_ap_list drv_ap_list;
				struct nlattr *ap_list_data = notify_tb[MTK_NL80211_ATTR_DATA];
				wifi2agps_ap_info *ap_info = &ap_list.list[0];
				struct driver_ap_info *drv_ap_info = &drv_ap_list.ap_info[0];
				u8 i = 0;
				memcpy(&drv_ap_list, nla_data(ap_list_data), nla_len(ap_list_data));
				memset(&ap_list, 0, sizeof(ap_list));
				ap_list.num = drv_ap_list.num;
				for (; i<ap_list.num; i++) {
					/*wifi2agps_log(MSG_INFO, "ap_mac_addr_%d=%02x:%02x:%02x:%02x:%02x:%02x\n", i,
				        drv_ap_info->bssid[0],
				        drv_ap_info->bssid[1],
				        drv_ap_info->bssid[2],
				        drv_ap_info->bssid[3],
				        drv_ap_info->bssid[4],
				        drv_ap_info->bssid[5]);*/
				//wifi2agps_log(MSG_INFO, "rssi_%d=%d, freq_%d=%d", i, drv_ap_info->ap_rssi, i,drv_ap_info->ap_channel);
					memcpy(ap_info->ap_mac_addr, drv_ap_info->bssid, 6);
					ap_info->ap_signal_strength_used = 1;
					ap_info->ap_signal_strength = drv_ap_info->ap_rssi;
					ap_info->ap_channel_frequency_used = 1;
					ap_info->ap_channel_frequency = drv_ap_info->ap_channel;
					/*ap_info->ap_device_type = drv_ap_info->phy_type;
					ap_info->ap_device_type_used = 1;*/
					ap_info++;
					drv_ap_info++;
				}
				
				nl_cfg->agps_cb.wifi_scan_results(&ap_list);
				break;
			}
		}
	}
		break;
	default:
		break;
	}
	return NL_SKIP;
}

static void nl80211_event_receive(void *eloop_ctx,
					     void *handle)
{
	nl_recvmsgs(handle, (struct nl_cb*)eloop_ctx);
}

static int android_genl_ctrl_resolve(struct nl_sock *handle,
				     const s8 *name)
{
	struct nl_cache *cache = NULL;
	struct genl_family *nl80211 = NULL;
	int id = -1;

	if (genl_ctrl_alloc_cache(handle, &cache) < 0) {
		wifi2agps_log(MSG_ERROR, "nl80211: Failed to allocate generic "
			   "netlink cache");
		goto fail;
	}

	nl80211 = genl_ctrl_search_by_name(cache, name);
	if (nl80211 == NULL)
		goto fail;

	id = genl_family_get_id(nl80211);

fail:
	if (nl80211)
		genl_family_put(nl80211);
	if (cache)
		nl_cache_free(cache);

	return id;
}


/* nl80211_init_nl: initialize the netlink interface to nl80211, to listen the multicast 
	event from cfg80211 in driver */
static int nl80211_init_nl(struct nl80211_cfg *nl_cfg)
{
	s32 ret;
	
	nl_cfg->nl_cb = nl_cb_alloc(NL_CB_DEFAULT);
	if (nl_cfg->nl_cb == NULL) {
		wifi2agps_log(MSG_ERROR, "nl80211: Failed to allocate netlink "
			   "callbacks");
		return -1;
	}

	nl_cfg->nl = nl_create_handle(nl_cfg->nl_cb, "nl");
	if (nl_cfg->nl == NULL)
		goto err;

	nl_cfg->nl80211_id = android_genl_ctrl_resolve(nl_cfg->nl, "nl80211");
	if (nl_cfg->nl80211_id < 0) {
		wifi2agps_log(MSG_ERROR, "nl80211: 'nl80211' generic netlink not "
			   "found");
		goto err;
	}

	nl_cfg->nl_event = nl_create_handle(nl_cfg->nl_cb, "event");
	if (nl_cfg->nl_event == NULL)
		goto err;

	ret = nl_get_multicast_id(nl_cfg, "nl80211", "testmode");
	if (ret >= 0)
		ret = nl_socket_add_membership(nl_cfg->nl_event, ret);
	if (ret < 0) {
		wifi2agps_log(MSG_ERROR, "nl80211: Could not add multicast "
			   "membership for testmode events: %d (%s)",
			   ret, strerror(-ret));
		goto err;
	}

	ret = nl_get_multicast_id(nl_cfg, "nl80211", "mlme");
	if (ret >= 0)
		ret = nl_socket_add_membership(nl_cfg->nl_event, ret);
	if (ret < 0) {
		wifi2agps_log(MSG_ERROR, "nl80211: Could not add multicast "
			   "membership for mlme events: %d (%s)",
			   ret, strerror(-ret));
		goto err;
	}

	
	nl_cb_set(nl_cfg->nl_cb, NL_CB_SEQ_CHECK, NL_CB_CUSTOM,
		  no_seq_check, NULL);
	nl_cb_set(nl_cfg->nl_cb, NL_CB_VALID, NL_CB_CUSTOM,
		  process_nl_event, nl_cfg);

	event_loop_add_event(nl_socket_get_fd(nl_cfg->nl_event), nl_cfg->nl_cb,
				 nl_cfg->nl_event, nl80211_event_receive);
	
	return 0;

err:
	nl_destroy_handles(&nl_cfg->nl_event);
	nl_destroy_handles(&nl_cfg->nl);
	nl_cb_put(nl_cfg->nl_cb);
	nl_cfg->nl_cb = NULL;
	return -1;
}

int wifi2agps_init(struct wifi_callback_funcs *callback) {
	memcpy(&nl_cfg.agps_cb, callback, sizeof(struct wifi_callback_funcs));
	return nl80211_init_nl(&nl_cfg);
}

int wifi2agps_deinit() {
	event_loop_terminate();
	return 0;
}
