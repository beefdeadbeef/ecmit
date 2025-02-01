/* -*- mode: c; tab-width: 8 -*-
 */

#include <lwip/debug.h>
#include <lwip/err.h>
#include <lwip/memp.h>
#include <lwip/sys.h>

#ifndef SYS_ARCH_DEBUG
#define SYS_ARCH_DEBUG	LWIP_DBG_OFF
#endif

#define BGRT_SYNC_NR	12
#define BGRT_SYNC_SZ	(sizeof(bgrt_sync_t) + sizeof(void *))

#define BGRT_QUEUE_NR	6
#define BGRT_QUEUE_LEN	(1<<5)
#define BGRT_QUEUE_SZ	BGRT_Q_SIZEOF(BGRT_QUEUE_LEN)

#define BGRT_PROC_NR	4
#define BGRT_PROC_SZ	(sizeof(bgrt_proc_t))

LWIP_MEMPOOL_DECLARE(bgrt_sync, BGRT_SYNC_NR, BGRT_SYNC_SZ, "BGRT_SYNC");
LWIP_MEMPOOL_DECLARE(bgrt_queue, BGRT_QUEUE_NR, BGRT_QUEUE_SZ, "BGRT_QUEUE");
LWIP_MEMPOOL_DECLARE(bgrt_proc, BGRT_PROC_NR, BGRT_PROC_SZ, "BGRT_PROC");

/*
 *
 */
void sys_init(void)
{
	LWIP_MEMPOOL_INIT(bgrt_sync);
	LWIP_MEMPOOL_INIT(bgrt_queue);
	LWIP_MEMPOOL_INIT(bgrt_proc);
}

uint32_t sys_now()
{
	return bgrt_kernel.timer.val;
}

void sys_arch_msleep(u32_t ms)
{
	bgrt_wait_time(ms);
}

/*
 * Mutexes
 */
#define MUTEX_PRIO (DEFAULT_THREAD_PRIO - 1)

err_t sys_mutex_new(sys_mutex_t *mutex)
{
	LWIP_ASSERT("mutex != NULL", mutex != NULL);
	mutex->mtx = LWIP_MEMPOOL_ALLOC(bgrt_sync);
	LWIP_ERROR("mutex->mtx", (mutex->mtx != NULL), return ERR_MEM);
	bgrt_mtx_init(mutex->mtx, MUTEX_PRIO);

	LWIP_DEBUGF(SYS_ARCH_DEBUG, ("mutex=%p\n", mutex->mtx));

	return ERR_OK;
}

void sys_mutex_lock(sys_mutex_t *mutex)
{
	bgrt_st_t st;

	LWIP_ASSERT("mutex != NULL", mutex != NULL);
	LWIP_ASSERT("mutex->mtx != NULL", mutex->mtx != NULL);
	st = bgrt_mtx_lock(mutex->mtx);
	LWIP_ASSERT("st == BGRT_ST_OK", st == BGRT_ST_OK);
}

void sys_mutex_unlock(sys_mutex_t *mutex)
{
	bgrt_st_t st;

	LWIP_ASSERT("mutex != NULL", mutex != NULL);
	LWIP_ASSERT("mutex->mtx != NULL", mutex->mtx != NULL);
	st = bgrt_mtx_free(mutex->mtx);
	LWIP_ASSERT("st == BGRT_ST_OK", st == BGRT_ST_OK);
}

void sys_mutex_free(sys_mutex_t *mutex)
{
	LWIP_ASSERT("mutex != NULL", mutex != NULL);
	LWIP_ASSERT("mutex->mtx != NULL", mutex->mtx != NULL);
	LWIP_MEMPOOL_FREE(bgrt_sync, mutex->mtx);

	mutex->mtx = NULL;
}

/*
 * Semaphores
 */
err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
	LWIP_ASSERT("sem != NULL", sem != NULL);
	sem->sem = LWIP_MEMPOOL_ALLOC(bgrt_sync);
	LWIP_ERROR("sem->sem", (sem->sem != NULL), return ERR_MEM);
	bgrt_sem_init(sem->sem, count);

	LWIP_DEBUGF(SYS_ARCH_DEBUG, ("sem=%p count=%d\n", sem->sem, count));

	return ERR_OK;
}

void sys_sem_signal(sys_sem_t *sem)
{
	bgrt_st_t st;

	LWIP_ASSERT("sem != NULL", sem != NULL);
	LWIP_ASSERT("sem->sem != NULL", sem->sem != NULL);
	st = bgrt_sem_free(sem->sem);
	LWIP_ASSERT("st == BGRT_ST_OK", st == BGRT_ST_OK);
}

u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
	bgrt_st_t st;

	LWIP_ASSERT("sem != NULL", sem != NULL);
	LWIP_ASSERT("sem->sem != NULL", sem->sem != NULL);

	if (!timeout) {
		st = bgrt_sem_lock(sem->sem);
		LWIP_ASSERT("st == BGRT_ST_OK", st == BGRT_ST_OK);
		return 0;
	}

	while(timeout--) {
		st = bgrt_sem_try_lock(sem->sem);
		if (st == BGRT_ST_OK) return 0;
		LWIP_ASSERT("st == BGRT_ST_ROLL", st == BGRT_ST_ROLL);
		bgrt_sched_proc_yield();
	}

	return SYS_ARCH_TIMEOUT;
}

void sys_sem_free(sys_sem_t *sem)
{
	LWIP_ASSERT("sem != NULL", sem != NULL);
	LWIP_ASSERT("sem->sem != NULL", sem->sem != NULL);

	LWIP_MEMPOOL_FREE(bgrt_sync, sem->sem);
	sem->sem = NULL;
}

/*
 * Mailboxes
 */
err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
	LWIP_ASSERT("mbox != NULL", mbox != NULL);
	LWIP_ASSERT("size <= BGRT_QUEUE_LEN", !size || size <= BGRT_QUEUE_LEN);

	mbox->queue = LWIP_MEMPOOL_ALLOC(bgrt_queue);
	LWIP_ERROR("mbox->queue", (mbox->queue != NULL), goto errmem);

	mbox->queue->enq.sem = LWIP_MEMPOOL_ALLOC(bgrt_sync);
	LWIP_ERROR("mbox->queue->sem", (mbox->queue->enq.sem != NULL), goto free1);

	mbox->queue->deq = LWIP_MEMPOOL_ALLOC(bgrt_sync);
	LWIP_ERROR("mbox->queue->sem", (mbox->queue->deq != NULL), goto free0);

	bgrt_queue_init(mbox->queue, size ? size : BGRT_QUEUE_LEN, 0);

	LWIP_DEBUGF(SYS_ARCH_DEBUG, ("mbox=%p size=%d\n", mbox->queue, size));

	return ERR_OK;

free0:	LWIP_MEMPOOL_FREE(bgrt_sync, mbox->queue->enq.sem);
free1:	LWIP_MEMPOOL_FREE(bgrt_queue, mbox->queue);
errmem:
	return ERR_MEM;
}

void sys_mbox_post(sys_mbox_t *mbox, void *msg)
{
	LWIP_ASSERT("mbox != NULL", mbox != NULL);
	LWIP_ASSERT("mbox->queue != NULL", mbox->queue != NULL);

	bgrt_queue_post(mbox->queue, msg);
}

err_t sys_mbox_trypost (sys_mbox_t *mbox, void *msg)
{
	bgrt_st_t st;

	LWIP_ASSERT("mbox != NULL", mbox != NULL);
	LWIP_ASSERT("mbox->queue != NULL", mbox->queue != NULL);

	st = bgrt_queue_trypost(mbox->queue, msg);
	if (st == BGRT_ST_OK) return ERR_OK;
	LWIP_ASSERT("st == BGRT_ST_ROLL", st == BGRT_ST_ROLL);
	return ERR_MEM;
}

u32_t sys_arch_mbox_fetch (sys_mbox_t *mbox, void **msg, u32_t timeout)
{
	bgrt_st_t st;

	LWIP_ASSERT("mbox != NULL", mbox != NULL);
	LWIP_ASSERT("mbox->queue != NULL", mbox->queue != NULL);

	if (!timeout) {
		st = bgrt_queue_fetch(mbox->queue, msg);
		LWIP_ASSERT("st == BGRT_ST_OK", st == BGRT_ST_OK);
		return 0;
	}

	while(timeout--) {
		st = bgrt_queue_try_fetch(mbox->queue, msg);
		if (st == BGRT_ST_OK) return 0;
		LWIP_ASSERT("st == BGRT_ST_ROLL", st == BGRT_ST_ROLL);
		bgrt_sched_proc_yield();
	}

	*msg = NULL;
	return SYS_ARCH_TIMEOUT;
}

u32_t sys_arch_mbox_tryfetch (sys_mbox_t *mbox, void **msg)
{
	bgrt_st_t st;

	LWIP_ASSERT("mbox != NULL", mbox != NULL);
	LWIP_ASSERT("mbox->queue != NULL", mbox->queue != NULL);

	st = bgrt_queue_try_fetch(mbox->queue, msg);
	if (st == BGRT_ST_OK) return 0;
	LWIP_ASSERT("st == BGRT_ST_ROLL", st == BGRT_ST_ROLL);

	return SYS_MBOX_EMPTY;
}

void sys_mbox_free (sys_mbox_t *mbox)
{
	LWIP_ASSERT("mbox != NULL", mbox != NULL);
	LWIP_ASSERT("mbox->queue != NULL", mbox->queue != NULL);

	LWIP_MEMPOOL_FREE(bgrt_sync, mbox->queue->enq.sem);
	LWIP_MEMPOOL_FREE(bgrt_sync, mbox->queue->deq);
	LWIP_MEMPOOL_FREE(bgrt_queue, mbox->queue);
	mbox->queue = NULL;
}

/*
 * Threads
 */
static void sys_arch_dummy_cb(void *ctx) {(void)ctx;}
static uint32_t sys_arch_dummy_arg;

sys_thread_t sys_thread_create(const char *name,
			       void (*pmain)(void *),
			       void *arg,
			       int ssize,	/* stackwords */
			       int prio,
			       unsigned timeslice,
			       bool is_rt)
{
	sys_thread_t thread;
	bgrt_stack_t *stack;
	bgrt_st_t st;
	(void)name;

	ssize = ssize ? ssize : DEFAULT_THREAD_STACKSIZE;
	LWIP_ASSERT("sane stacksize", ssize <= MAX_THREAD_STACKSIZE);

	stack = mem_malloc(ssize * sizeof(bgrt_stack_t));
	LWIP_ASSERT("stack != NULL", stack);

	thread.proc = LWIP_MEMPOOL_ALLOC(bgrt_proc);
	LWIP_ASSERT("thread.proc != NULL", thread.proc);

	st = bgrt_proc_init(thread.proc,
			    pmain,
			    &sys_arch_dummy_cb,
			    &sys_arch_dummy_cb,
			    arg ? arg : &sys_arch_dummy_arg,
			    &stack[ssize - 1],
			    prio, timeslice, is_rt);
	LWIP_ASSERT("st == BGRT_ST_OK", st == BGRT_ST_OK);

	st = bgrt_proc_run(thread.proc);
	LWIP_ASSERT("st == BGRT_ST_OK", st == BGRT_ST_OK);

	LWIP_DEBUGF(SYS_ARCH_DEBUG, ("[%s] fn=%p proc=%p stack=%d@%p\n",
				     name, pmain, thread.proc, ssize, stack));
	return thread;
}

sys_thread_t sys_thread_new(const char *name, lwip_thread_fn pmain,
			    void *arg, int ssize, int prio)
{
	return sys_thread_create(name, pmain, arg, ssize, prio, 1, 0);
}
