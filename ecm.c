/* -*- mode: c; tab-width: 8 -*-
 */

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/desig.h>
#include <libopencm3/usb/cdc.h>
#include <libopencm3/usb/usbd.h>

#include <lwip/debug.h>
#include <lwip/dhcp.h>
#include <lwip/err.h>
#include <lwip/netif.h>
#include <lwip/netifapi.h>
#include <lwip/pbuf.h>
#include <lwip/tcpip.h>

#include "common.h"

#define __usb_isr usb_lp_isr
#define __usb_irq NVIC_USB_LP_IRQ
#define __usb_driver st_usbfs_v1_usb_driver

#define PKTSIZE0 16
#define MIN_PACKET_SIZE 32
#define ECM_PACKET_SIZE 64
#define ECM_OUT_ENDP_ADDR 0x03
#define ECM_IN_ENDP_ADDR 0x84
#define ECM_CTRL_ENDP_ADDR 0x85

#define ECM_SEGSZ 1514		/* MTU 1500 */

#ifndef ECM_DEBUG
#define ECM_DEBUG	LWIP_DBG_OFF
#endif

LWIP_MEMPOOL_PROTOTYPE(bgrt_sync);
LWIP_MEMPOOL_PROTOTYPE(bgrt_queue);

static char mac[2 * NETIF_MAX_HWADDR_LEN + 1];
static const char * usb_strings[] = { "Acme Corp", "ECMIT", mac };

static const struct usb_device_descriptor dev = {
	.bLength = USB_DT_DEVICE_SIZE,
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = 0,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize0 = PKTSIZE0,
	.idVendor = 0x6666,
	.idProduct = 0x2702,
	.bcdDevice = 0x0200,
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 0,
	.bNumConfigurations = 1
};

#define USB_CDC_SUBCLASS_ECM    0x06
#define USB_CDC_TYPE_ENET       0x0F

/* ECM120 Tab 3 */
struct usb_cdc_enet_functional_descriptor {
	uint8_t bFunctionLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubtype;
	uint8_t iMACAddress;
	uint8_t bmEthernetStatistics[4];
	uint16_t wMaxSegmentSize;
	uint16_t wNumberMCFilters;
	uint8_t bNumberPowerFilters;
} __attribute__((packed));

struct usb_cdc_ecm_function {
	struct usb_cdc_header_descriptor header_desc;
	struct usb_cdc_union_descriptor union_desc;
	struct usb_cdc_enet_functional_descriptor enet_desc;
} __attribute__((packed));

static const struct {
	struct usb_config_descriptor cdesc;

	struct usb_interface_descriptor ecm_ctrl_iface;
	struct usb_cdc_ecm_function ecm_function;
	struct usb_endpoint_descriptor ecm_ctrl_ep;

	struct usb_interface_descriptor ecm_data_iface_0;
	struct usb_interface_descriptor ecm_data_iface_1;
	struct usb_endpoint_descriptor ecm_in_ep;
	struct usb_endpoint_descriptor ecm_out_ep;

} __attribute__((packed)) config = {
	.cdesc = {
		.bLength = USB_DT_CONFIGURATION_SIZE,
		.bDescriptorType = USB_DT_CONFIGURATION,
		.wTotalLength = sizeof(config),
		.bNumInterfaces = 2,
		.bConfigurationValue = 1,
		.iConfiguration = 0,
		.bmAttributes = 0x80,
		.bMaxPower = 0x32,
	},
	.ecm_ctrl_iface = {
		.bLength = sizeof(struct usb_interface_descriptor),
		.bDescriptorType = USB_DT_INTERFACE,
		.bInterfaceNumber = 0,
		.bAlternateSetting = 0,
		.bNumEndpoints = 1,
		.bInterfaceClass = USB_CLASS_CDC,
		.bInterfaceSubClass = USB_CDC_SUBCLASS_ECM,
		.bInterfaceProtocol = 0,
		.iInterface = 0,
	},
	.ecm_function = {
		.header_desc = {
			.bFunctionLength = sizeof(struct usb_cdc_header_descriptor),
			.bDescriptorType = CS_INTERFACE,
			.bDescriptorSubtype = 0,
			.bcdCDC = 0x120
		},
		.union_desc = {
			.bFunctionLength = sizeof(struct usb_cdc_union_descriptor),
			.bDescriptorType = CS_INTERFACE,
			.bDescriptorSubtype = USB_CDC_TYPE_UNION,
			.bControlInterface = 0,
			.bSubordinateInterface0 = 1
		},
		.enet_desc = {
			.bFunctionLength = sizeof(struct usb_cdc_enet_functional_descriptor),
			.bDescriptorType = CS_INTERFACE,
			.bDescriptorSubtype = USB_CDC_TYPE_ENET,
			.iMACAddress = 3,
			.bmEthernetStatistics = {0, 0, 0, 0},
			.wMaxSegmentSize = ECM_SEGSZ,
			.wNumberMCFilters = 0,
			.bNumberPowerFilters = 0
		},
	},
	.ecm_ctrl_ep = {
		.bLength = USB_DT_ENDPOINT_SIZE,
		.bDescriptorType = USB_DT_ENDPOINT,
		.bEndpointAddress = ECM_CTRL_ENDP_ADDR,
		.bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
		.wMaxPacketSize = MIN_PACKET_SIZE,
		.bInterval = 255
	},
	.ecm_data_iface_0 = {
		.bLength = USB_DT_INTERFACE_SIZE,
		.bDescriptorType = USB_DT_INTERFACE,
		.bInterfaceNumber = 1,
		.bAlternateSetting = 0,
		.bNumEndpoints = 0,
		.bInterfaceClass = USB_CLASS_DATA,
		.bInterfaceSubClass = 0,
		.bInterfaceProtocol = 0,
		.iInterface = 0
	},
	.ecm_data_iface_1 = {
		.bLength = USB_DT_INTERFACE_SIZE,
		.bDescriptorType = USB_DT_INTERFACE,
		.bInterfaceNumber = 1,
		.bAlternateSetting = 1,
		.bNumEndpoints = 2,
		.bInterfaceClass = USB_CLASS_DATA,
		.bInterfaceSubClass = 0,
		.bInterfaceProtocol = 0,
		.iInterface = 0
	},
	.ecm_in_ep = {
		.bLength = USB_DT_ENDPOINT_SIZE,
		.bDescriptorType = USB_DT_ENDPOINT,
		.bEndpointAddress = ECM_IN_ENDP_ADDR,
		.bmAttributes = USB_ENDPOINT_ATTR_BULK,
		.wMaxPacketSize = ECM_PACKET_SIZE,
		.bInterval = 1
	},
	.ecm_out_ep = {
		.bLength = USB_DT_ENDPOINT_SIZE,
		.bDescriptorType = USB_DT_ENDPOINT,
		.bEndpointAddress = ECM_OUT_ENDP_ADDR,
		.bmAttributes = USB_ENDPOINT_ATTR_BULK,
		.wMaxPacketSize = ECM_PACKET_SIZE,
		.bInterval = 1
	}
};

static const struct usb_config_descriptor *configs[] = {
	&config.cdesc
};

static usbd_device * usbdev;
static uint8_t usbd_control_buffer[64];
static bgrt_vint_t usbd_vint;

enum { TX_REQ = (1 << 0), TX_RDY = (1 << 1), EV_LNK = (1 << 2) };
static bgrt_map_t ev;

static struct netif ecmif = {
	.name = { 'e', 'n' },
	.hwaddr_len = NETIF_MAX_HWADDR_LEN
};

static struct ifstate {
	bgrt_queue_t *rxqueue;
	bgrt_queue_t *txqueue;
} ecmstate;

#define IF_Q_SZ		(1<<5)
#define RXQUEUE		(ecmstate.rxqueue)
#define TXQUEUE		(ecmstate.txqueue)

typedef struct {
	uint16_t flags;
	uint16_t len;
	uint8_t payload[ECM_PACKET_SIZE];
} ecm_sg_t;

static ecm_sg_t sgbuf[IF_Q_SZ + 1];

/*
 *
 */
static void  __attribute__((constructor)) uid_to_macs()
{
	uint8_t uid[12];
	char *p;
	int i;

	desig_get_unique_id((uint32_t *)&uid);

	/* LAA bit */
	uid[0] |= 2; uid[11] |= 2;

	for (i=0; i<6; i++)
		ecmif.hwaddr[i] = uid[i];

	for (i=6, p=&mac[11]; i<12; i++) {
		uint8_t hi = (uid[i] & 0xf0) >> 4;
		uint8_t lo = uid[i] & 0x0f;
		*p-- =  lo < 10 ? '0' + lo : 'A' + lo - 10;
		*p-- =  hi < 10 ? '0' + hi : 'A' + hi - 10;
	}

	debugf("UID: %08x %08x %08x\n",
	       *(uint32_t *)&uid[0],
	       *(uint32_t *)&uid[4],
	       *(uint32_t *)&uid[8]);

	debugf("MAC0: %02X%02X%02X%02X%02X%02X\n",
	       ecmif.hwaddr[0], ecmif.hwaddr[1],
	       ecmif.hwaddr[2], ecmif.hwaddr[3],
	       ecmif.hwaddr[4], ecmif.hwaddr[5]);

	debugf("MAC1: %s\n", mac);
}

/*
 * USB ISR context
 */
static void ecm_tx_q_cb(void **p, void *msg)
{
	ecm_sg_t *q = *p;
	(void)msg;

	usbd_ep_write_packet(usbdev, ECM_IN_ENDP_ADDR, q->payload, q->len);
}

static void ecm_tx_cb(usbd_device *udev, uint8_t ep)
{
	(void)ep;
	(void)udev;
	bgrt_st_t st;

	st = bgrt_queue_trypost_cb_cs(TXQUEUE, NULL, ecm_tx_q_cb);
	if (st == BGRT_ST_ROLL)
		bgrt_atm_bset(&ev, TX_RDY);
}

static void ecm_rx_q_cb(void **p, void *msg)
{
	struct pbuf *q = *p;
	uint16_t len;
	(void)msg;

	len = usbd_ep_read_packet(usbdev, ECM_OUT_ENDP_ADDR,
				  q->payload, ECM_PACKET_SIZE);
	q->len = q->tot_len = len;
}

static void ecm_rx_cb(usbd_device *udev, uint8_t ep)
{
	(void)ep;
	(void)udev;
	bgrt_st_t st;

	st = bgrt_queue_trypost_cb_cs(RXQUEUE, NULL, ecm_rx_q_cb);
	LWIP_ASSERT("st == BGRT_ST_OK", st == BGRT_ST_OK);
}

static void ecm_sof_cb(void)
{
	if (bgrt_atm_bget(&ev, TX_REQ|TX_RDY) == (TX_REQ|TX_RDY)) {
		bgrt_atm_bclr(&ev, TX_REQ);
		ecm_tx_cb(usbdev, ECM_IN_ENDP_ADDR);
	}
}

static void ecm_altset_cb(usbd_device *usbd_dev,
			  uint16_t wIndex, uint16_t wValue)
{
	(void) usbd_dev;
	LWIP_DEBUGF(ECM_DEBUG, ("altset_cb: wIndex: %d wValue: %d\n", wIndex, wValue));

	if(wIndex != 1) return;			/* wIndex: iface # */

	if (wValue) {				/* wValue: alt setting # */
		bgrt_atm_bset(&ev, TX_RDY);
	} else {
		bgrt_atm_bclr(&ev, TX_RDY);
	}

	bgrt_atm_bset(&ev, EV_LNK);
}

static enum usbd_request_return_codes ecm_cs_cb(
	usbd_device *usbd_dev,
	struct usb_setup_data *req,
	uint8_t **buf,
	uint16_t *len,
	usbd_control_complete_callback *complete)
{
	(void) usbd_dev;
	(void) complete;
	(void) len;

	LWIP_DEBUGF(ECM_DEBUG, ("cs_cb: bRequest: %02x wValue: %04x wIndex: %04x len: %d data=%02x\n",
				req->bRequest, req->wValue, req->wIndex, *len, *buf[0]));
	/*
	 * TODO
	 * ip li set dev <link> down
	 * bRequest: 43 wValue: 000e wIndex: 0002 len: 0 data=80
	 * ip li set dev <link> up
	 * bRequest: 43 wValue: 000c wIndex: 0002 len: 0 data=80
	 *
	 * wValue: SET_ETHERNET_PACKET_FILTER [4-0]
	 * 4: PACKET_TYPE_MULTICAST
	 * 3: PACKET_TYPE_BROADCAST
	 * 2: PACKET_TYPE_DIRECTED
	 * 1: PACKET_TYPE_ALL_MULTICAST
	 * 0: PACKET_TYPE_PROMISCUOUS
	 */
	if (req->bRequest == 0x43)
		return USBD_REQ_HANDLED;
	else
		return USBD_REQ_NOTSUPP;
}

static void usbd_set_config(usbd_device *usbd_dev, uint16_t wValue)
{
	switch (wValue) {			/* configuration # */
	case 1:
		usbd_ep_setup(
			usbd_dev,
			ECM_OUT_ENDP_ADDR,
			USB_ENDPOINT_ATTR_BULK,
			ECM_PACKET_SIZE,
			ecm_rx_cb);

		usbd_ep_setup(
			usbd_dev,
			ECM_IN_ENDP_ADDR,
			USB_ENDPOINT_ATTR_BULK,
			ECM_PACKET_SIZE,
			ecm_tx_cb);

		usbd_register_control_callback(
			usbd_dev,
			USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
			USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
			ecm_cs_cb);

		usbd_register_set_altsetting_callback(usbd_dev,
						      ecm_altset_cb);

		usbd_register_sof_callback(usbd_dev, ecm_sof_cb);
	};
}

static void usbd_intr(usbd_device *udev)
{
	usbd_poll(udev);
	nvic_enable_irq(__usb_irq);
}

void usb_init()
{
	usbdev = usbd_init(&__usb_driver, &dev, configs,
			   usb_strings, sizeof(usb_strings)/sizeof(usb_strings[0]),
			   usbd_control_buffer, sizeof(usbd_control_buffer));
	usbd_register_set_config_callback(usbdev, usbd_set_config);
	bgrt_vint_init(&usbd_vint, 1, (bgrt_code_t)usbd_intr, usbdev);
	nvic_enable_irq(__usb_irq);
}

BGRT_ISR(__usb_isr)
{
	nvic_disable_irq(__usb_irq);
	bgrt_vint_push_isr(&usbd_vint, &bgrt_kernel.kblock.vic);
}

/*
 * LWIP context
 */
static void ecm_input(void *ctx)
{
	struct netif *iface = ctx;
	bgrt_queue_t *q = ((struct ifstate *)iface->state)->rxqueue;
	struct pbuf *p, *pp = NULL;
	bgrt_st_t st;

loop:	if (bgrt_atm_bget(&ev, EV_LNK)) {
		bgrt_atm_bclr(&ev, EV_LNK);
		if (bgrt_atm_bget(&ev, TX_RDY))
			netifapi_netif_set_link_up(&ecmif);
		else
			netifapi_netif_set_link_down(&ecmif);
	}

	p = pbuf_alloc(PBUF_RAW, ECM_PACKET_SIZE, PBUF_RAM);
	LWIP_ASSERT("p != NULL", p);

	st = bgrt_queue_swap(q, (void **)&p);
	LWIP_ASSERT("st == BGRT_ST_OK", st == BGRT_ST_OK);

	if (pp)
		pbuf_cat(pp, p);
	else
		pp = p;

	if (p->len == ECM_PACKET_SIZE)
		goto loop;

	p = pbuf_coalesce(pp, PBUF_RAW);
	pp = NULL;

	if (iface->input(p, iface) != ERR_OK) {
		LWIP_DEBUGF(ECM_DEBUG, ("ecmif.input\n"));
		pbuf_free(p);
	}

	goto loop;
}

static err_t ecm_output(struct netif *iface, struct pbuf *p)
{
	static ecm_sg_t *sg = &sgbuf[IF_Q_SZ];
	bgrt_queue_t *q = ((struct ifstate *)iface->state)->txqueue;
	uint16_t len, offset, total;
	bgrt_st_t st;

	offset = 0;
	total = p->tot_len;

	while (total) {
		len = MIN(ECM_PACKET_SIZE, total);
		sg->len = pbuf_copy_partial(p, sg->payload, len, offset);
 		LWIP_ASSERT("sg->len == len\n", sg->len == len);

		st = bgrt_queue_swap(q, (void **)&sg);
		LWIP_ASSERT("st == BGRT_ST_OK", st == BGRT_ST_OK);

		total -= len;
		offset += len;
	}

	bgrt_atm_bset(&ev, TX_REQ);

	return ERR_OK;
}

#if LWIP_NETIF_STATUS_CALLBACK
static void ecm_status_cb(struct netif *iface)
{
	LWIP_DEBUGF(ECM_DEBUG, ("address: %s\n", ip4addr_ntoa(netif_ip4_addr(iface))));
}
#endif

#if LWIP_NETIF_LINK_CALLBACK
static void ecm_link_cb(struct netif *iface)
{
	LWIP_DEBUGF(ECM_DEBUG, ("iface is %s\n", netif_is_link_up(iface) ? "up" : "down"));
}
#endif

static err_t ecmif_init(struct netif *iface)
{
	struct ifstate *ifs = iface->state;
	bgrt_queue_t *q;
	void **p;

	/* allocate queues */
	q = LWIP_MEMPOOL_ALLOC(bgrt_queue);
	LWIP_ASSERT("rxqueue != NULL", q != NULL);
	q->deq = LWIP_MEMPOOL_ALLOC(bgrt_sync);
	LWIP_ASSERT("rxqueue->sem != NULL", q->deq != NULL);
	bgrt_queue_init(q, IF_Q_SZ, Q_SW|Q_CS);
	/* populate rx queue with pbufs */
	p = q->queue;
	for (int i=0; i<IF_Q_SZ; i++, p++) {
		*p = (void *)pbuf_alloc(PBUF_RAW, ECM_PACKET_SIZE, PBUF_RAM);
		LWIP_ASSERT("pbuf != NULL", *p != NULL);
	}
	ifs->rxqueue = q;

	q = LWIP_MEMPOOL_ALLOC(bgrt_queue);
	LWIP_ASSERT("txqueue != NULL", q != NULL);
	q->deq = LWIP_MEMPOOL_ALLOC(bgrt_sync);
	LWIP_ASSERT("txqueue->sem != NULL", q->deq != NULL);
	bgrt_queue_init(q, IF_Q_SZ, Q_SW|Q_CS|Q_REV);
	/* populate tx queue with sgbufs */
	p = q->queue;
	for (int i=0; i<IF_Q_SZ; i++, p++)
		*p = &sgbuf[i];
	ifs->txqueue = q;

	iface->linkoutput = ecm_output;
	iface->output = etharp_output;
	iface->mtu = 1500;
	iface->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

	sys_thread_new("ecm_input", ecm_input, iface,
		       BGRT_PROC_STACK_SIZE, DEFAULT_THREAD_PRIO);

	return ERR_OK;
}

void ecm_init(void *arg)
{
	(void)arg;

	netif_add(&ecmif, NULL, NULL, NULL, &ecmstate, ecmif_init, tcpip_input);
	netif_set_default(&ecmif);
	netif_set_up(&ecmif);

#if LWIP_NETIF_STATUS_CALLBACK
	netif_set_status_callback(&ecmif, ecm_status_cb);
#endif
#if LWIP_NETIF_LINK_CALLBACK
	netif_set_link_callback(&ecmif, ecm_link_cb);
#endif
	dhcp_start(&ecmif);
}
