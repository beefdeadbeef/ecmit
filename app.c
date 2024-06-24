/* -*- mode: c; tab-width: 8 -*-
 */

#include <lwip/apps/http_client.h>
#include <lwip/netif.h>
#include <lwip/sys.h>

#include "common.h"

#ifndef APP_DEBUG
#define APP_DEBUG	LWIP_DBG_OFF
#endif

#ifndef HOST
#define HOST	"example.org"
#endif
#ifndef URI
#define URI		"/boot/uImage"
#endif
#define PORT		80

NETIF_DECLARE_EXT_CALLBACK(status_cb_hook)

static void httpc_input(void);

static void status_cb(struct netif *iface, netif_nsc_reason_t reason,
		      const netif_ext_callback_args_t *args)
{
	(void)iface;
	(void)args;

	if (reason & LWIP_NSC_IPV4_ADDR_VALID) {
		LWIP_DEBUGF(APP_DEBUG, ("address: %s\n",
					ip4addr_ntoa(netif_ip4_addr(iface))));
		httpc_input();
	}
}

static err_t start_cb(httpc_state_t *state, void *ctx,
		      struct pbuf *p, u16_t hdr_len, u32_t body_len)
{
	(void)state;
	(void)ctx;
	LWIP_DEBUGF(APP_DEBUG, ("hdr=%d body=%d\n", hdr_len, body_len));

	return ERR_OK;
}

static err_t recv_cb(void *ctx, struct tcp_pcb *pcb,
		     struct pbuf *p, err_t err)
{
	(void)ctx;

	altcp_recved(pcb, p->tot_len);
	if (p) pbuf_free(p);

	return ERR_OK;
}

static void done_cb(void *ctx, httpc_result_t httpc_result,
		    u32_t rx_content_len,
		    u32_t srv_res,
		    err_t err)
{
	(void)ctx;
	LWIP_DEBUGF(APP_DEBUG, ("done: rx=%d res=%d srv=%d err=%d\n",
				rx_content_len, httpc_result, srv_res, err));
	httpc_input();
}

static httpc_connection_t settings = {
	.headers_done_fn = start_cb,
	.result_fn = done_cb
};

static void httpc_input()
{
	err_t ret;

	ret = httpc_get_file_dns(HOST, PORT, URI,
				 &settings, recv_cb, NULL, NULL);
	LWIP_DEBUGF(APP_DEBUG, ("req: %d\n", ret));
}

void app_init(void *ctx)
{
	debugf("app init: %p\n", ctx);
	netif_add_ext_callback(&status_cb_hook, status_cb);
}
