/* -*- mode: c; tab-width: 8 -*-
 */
#include <alloca.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/crs.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

#include <lwip/tcpip.h>
#include "common.h"

extern void ecm_init(void *);
extern void usb_init(void);

/*
 *
 */
static void idle_cb(void *ctx) { (void)ctx; }
static unsigned idle_arg;

static void idle(void *ctx)
{
	unsigned wake;
	(void)ctx;

	tcpip_init(ecm_init, NULL);
	debugf("init done\n");

loop:	wake = systicks + 500ul;
sleep:  __asm__ __volatile__ ("wfe":::"memory");
	if (wake > systicks) goto sleep;
	gpio_toggle(GPIOC, GPIO13);
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

	rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE_240MHZ]);
	rcc_periph_clock_enable(RCC_AFIO);
	rcc_periph_clock_enable(RCC_CRS);
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);
	rcc_set_hsi_div(RCC_CFGR3_HSIDIV_NODIV);
        rcc_set_hsi_sclk(RCC_CFGR5_HSI_SCLK_HSIDIV);
        rcc_set_usb_clock_source(RCC_HSI);
        rcc_periph_clock_enable(RCC_USB);
        rcc_usb_alt_pma_enable();
        rcc_usb_alt_isr_enable();
        crs_autotrim_usb_enable();

	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
	systick_set_reload(rcc_ahb_frequency / 1000);
	systick_interrupt_enable();
	systick_counter_enable();

	gpio_set_mux(AFIO_GMUX_SWJ_NO_JTAG);
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_10_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL,
		      GPIO0|GPIO1);			/* DEBUG */
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
		      GPIO11|GPIO12);                   /* USBFS */
        gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL,
		      GPIO13);				/* PC13 LED */

	usb_init();

	idle_stack = alloca(BGRT_PROC_STACK_SIZE * sizeof(bgrt_stack_t));
	bgrt_proc_init_cs(&idle_proc, &idle, &idle_cb, &idle_cb,
			  &idle_arg, &idle_stack[BGRT_PROC_STACK_SIZE - 1],
			  BGRT_PRIO_LOWEST, 1, 0);
	bgrt_proc_run_cs(&idle_proc);
	bgrt_start();

	return 0;
}
