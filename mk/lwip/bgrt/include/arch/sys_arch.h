#ifndef LWIP_ARCH_SYS_ARCH_H
#define LWIP_ARCH_SYS_ARCH_H

#include <bugurt.h>
#include <native.h>

/*
 * critsec
 */
#define SYS_ARCH_DECL_PROTECT(lev)
#define SYS_ARCH_PROTECT(lev)		bgrt_crit_sec_enter()
#define SYS_ARCH_UNPROTECT(lev)		bgrt_crit_sec_exit()

/*
 * Mutexes
 */
typedef struct {
	bgrt_mtx_t *mtx;
} sys_mutex_t;

#define sys_mutex_valid_val(mutex)	((mutex).mtx != NULL)
#define sys_mutex_valid(mutex)		(((mutex) != NULL) && sys_mutex_valid_val(*(mutex)))
#define sys_mutex_set_invalid(mutex)	((mutex)->mtx = NULL)

/*
 * Semaphores
 */
typedef struct {
	bgrt_sem_t *sem;
} sys_sem_t;

#define sys_sem_valid_val(sema)		((sema).sem != NULL)
#define sys_sem_valid(sema)		(((sema) != NULL) && sys_sem_valid_val(*(sema)))
#define sys_sem_set_invalid(sema)	((sema)->sem = NULL)

/*
 * Mailboxes
 */
typedef struct {
	bgrt_queue_t *queue;
} sys_mbox_t;

#define sys_mbox_valid_val(mbox)	((mbox).queue != NULL)
#define sys_mbox_valid(mbox)		(((mbox) != NULL) && sys_mbox_valid_val(*(mbox)))
#define sys_mbox_set_invalid(mbox)	((mbox)->queue = NULL)

/*
 *
 */
typedef struct {
	bgrt_proc_t *proc;
} sys_thread_t;

sys_thread_t
sys_thread_create(const char *name, void (*fn)(void *),
		  void *arg, int stacksize, int prio,
		  unsigned timeslice, bool is_rt);

#endif // LWIP_ARCH_SYS_ARCH_H
