/* -*- mode: c; tab-width: 8 -*-
 */
#include <alloca.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/crs.h>
#include <libopencm3/stm32/rcc.h>

#include <lwip/api.h>
#include <lwip/inet.h>
#include <lwip/tcp.h>
#include <lwip/tcpip.h>

#include "common.h"

extern void ecm_init(void *);
extern void usb_init(void);

#define LISTEN_PORT	7
#define WORKERS_MAX	2

typedef struct {
	struct netconn *conn;
	sys_thread_t thread;
	sys_sem_t *sem;
	void *ctx;
} worker_t;

/*
 *
 */

static worker_t workers[WORKERS_MAX];

static void worker_proc(void *arg)
{
	worker_t *w = arg;
	struct netbuf *buf;
	ip_addr_t addr;
	uint16_t port;

	if (w->conn == NULL)
		goto out;

	netconn_peer(w->conn, &addr, &port);
	debugf("client: %s %d\n", ip4addr_ntoa(&addr), port);
	tcp_nagle_disable(w->conn->pcb.tcp);

	while (netconn_recv(w->conn, &buf) == ERR_OK) {
		uint16_t len;
		void *p;

		do {
			netbuf_data(buf, &p, &len);
			netconn_write(w->conn, p, len, NETCONN_COPY);

		} while (netbuf_next(buf) >= 0);

		netbuf_delete(buf);
	}

	debugf("client: bye\n");
	netconn_close(w->conn);
	netconn_delete(w->conn);
	w->conn = NULL;
out:
	sys_sem_signal(w->sem);
}

static void start_workers(void *ctx, sys_sem_t *sem)
{
	worker_t *w = workers;

	for (; w < &workers[WORKERS_MAX]; w++) {
		w->ctx = ctx;
		w->sem = sem;
		w->thread = sys_thread_new("worker", worker_proc, w,
					   DEFAULT_THREAD_STACKSIZE,
					   DEFAULT_THREAD_PRIO);
	}
}

static void wake_worker(struct netconn *conn)
{
	int i = 0;

	while (i < WORKERS_MAX && workers[i].conn != NULL) i++;
	LWIP_ASSERT("WORKERS_MAX", i < WORKERS_MAX);

	workers[i].conn = conn;
	sys_thread_restart(workers[i].thread);
}

static void app_proc(void *arg)
{
	struct netconn *conn, *listen;
	sys_sem_t sem;
	err_t err;

	err = sys_sem_new(&sem, 0);
	LWIP_ASSERT("sem_new", err == ERR_OK);

	start_workers(arg, &sem);

	listen = netconn_new(NETCONN_TCP);
	netconn_bind(listen, IP_ADDR_ANY, LISTEN_PORT);
	netconn_listen(listen);

loop:
	if (netconn_accept(listen, &conn) != ERR_OK)
		goto loop;

	if ((sys_arch_sem_wait(&sem, 500)) == 0) {
		wake_worker(conn);
	} else { /* out of free workers */
		netconn_close(conn);
		netconn_delete(conn);
	}

	goto loop;
}

static void app_init(void *app_arg)
{
	sys_thread_new("listener", app_proc, app_arg,
		       DEFAULT_THREAD_STACKSIZE,
		       DEFAULT_THREAD_PRIO);

	debugf("app_init: %p\n", app_arg);
}

/*
 *
 */
static void idle_cb(void *ctx) { (void)ctx; }
static unsigned idle_arg, late_arg;

static void late_init(void *arg)
{
	ecm_init(arg);
	app_init(arg);
}

static void idle(void *ctx)
{
	unsigned wake;
	(void)ctx;

	tcpip_init(late_init, &late_arg);
	debugf("init done\n");

loop:	wake = systicks + 500ul;
sleep:  __asm__ __volatile__ ("wfe":::"memory");
	if (wake > systicks) goto sleep;
	gpio_toggle(GPIOD, GPIO15);
	goto loop;
}

/*
 *
 */
int main()
{
	bgrt_proc_t idle_proc;
	bgrt_stack_t *idle_stack;

	bgrt_init();

	rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE_288MHZ]);

	rcc_set_hsi_div(RCC_CFGR3_HSIDIV_NODIV);        /* HSI48M */
	rcc_set_hsi_sclk(RCC_CFGR3_HSI_SCLK);           /* HSI48M to SYSCLK */
	rcc_set_usb_clock_source(RCC_HSI);              /* HSI48M to OTG */

	rcc_set_mco_source(MCO1, RCC_CFGR_MCO1_PLL);
	rcc_set_mcopre(MCO1, RCC_CFGR_MCOPRE_NODIV);
	rcc_set_mcodiv(MCO1, RCC_CFGR3_MCO_DIV64);

	rcc_set_mco_source(MCO2, RCC_CFGR_MCO2_MUX);
	rcc_set_mcopre(MCO2, RCC_CFGR_MCOPRE_NODIV);
	rcc_set_mcodiv(MCO2, RCC_CFGR3_MCO_DIV8);

	rcc_periph_clock_enable(RCC_CRS);
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOC);
	rcc_periph_clock_enable(RCC_GPIOD);
	rcc_periph_clock_enable(RCC_GPIOE);

	crs_autotrim_usb_enable();

	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
	systick_set_reload(rcc_ahb_frequency / 1000);
	systick_interrupt_enable();
	systick_counter_enable();

	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE,
                        GPIO8|GPIO11|GPIO12);		/* OTGFS1 */
	gpio_set_af(GPIOA, GPIO_AF10, GPIO8|GPIO11|GPIO12);

	gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9);
	gpio_set_af(GPIOE, GPIO_AF0, GPIO9);		/* MCO2 */

	gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
                        GPIO13|GPIO14|GPIO15);          /* PD LEDS */
	gpio_set_output_options(GPIOD, GPIO_OTYPE_OD, GPIO_OSPEED_LOW,
				GPIO13|GPIO14|GPIO15);

	gpio_mode_setup(GPIOE, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
			GPIO0|GPIO1|GPIO2|GPIO3);       /* DEBUG */

	gpio_mode_setup(GPIOE, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO4);
	gpio_set_af(GPIOE, GPIO_AF0, GPIO4);		/* MCO1 */

	gpio_set(GPIOD, GPIO13|GPIO14|GPIO15);

	usb_init();

	idle_stack = alloca(BGRT_PROC_STACK_SIZE * sizeof(bgrt_stack_t));
	bgrt_proc_init_cs(&idle_proc, &idle, &idle_cb, &idle_cb,
			  &idle_arg, &idle_stack[BGRT_PROC_STACK_SIZE - 1],
			  BGRT_PRIO_LOWEST, 1, 0);
	bgrt_proc_run_cs(&idle_proc);
	bgrt_start();

	return 0;
}
