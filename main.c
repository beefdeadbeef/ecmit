/* -*- mode: c; tab-width: 8 -*-
 */
#include <alloca.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
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
	gpio_toggle(GPIOA, GPIO1);
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

	rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOD);

	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
	systick_set_reload(rcc_ahb_frequency / 1000);
	systick_interrupt_enable();
	systick_counter_enable();

	gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
			GPIO0|GPIO1);			/* DEBUG */
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE,
			GPIO11|GPIO12);			/* USBFS */
	gpio_set_af(GPIOA, GPIO_AF10, GPIO11|GPIO12);

	gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT,
			GPIO_PUPD_NONE, GPIO15);	/* BLUE LED */

	usb_init();

	idle_stack = alloca(BGRT_PROC_STACK_SIZE * sizeof(bgrt_stack_t));
	bgrt_proc_init_cs(&idle_proc, &idle, &idle_cb, &idle_cb,
			  &idle_arg, &idle_stack[BGRT_PROC_STACK_SIZE - 1],
			  BGRT_PRIO_LOWEST, 1, 0);
	bgrt_proc_run_cs(&idle_proc);
	bgrt_start();

	return 0;
}
