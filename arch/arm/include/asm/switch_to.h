#ifndef __ASM_ARM_SWITCH_TO_H
#define __ASM_ARM_SWITCH_TO_H

#include <linux/thread_info.h>

/*
 * For v7 SMP cores running a preemptible kernel we may be pre-empted
 * during a TLB maintenance operation, so execute an inner-shareable dsb
 * to ensure that the maintenance completes in case we migrate to another
 * CPU.
 */
#if defined(CONFIG_PREEMPT) && defined(CONFIG_SMP) && defined(CONFIG_CPU_V7)
#define finish_arch_switch(prev)	dsb(ish)
#endif

/*
 * switch_to(prev, next) should switch from task `prev' to `next'
 * `prev' will never be the same as `next'.  schedule() itself
 * contains the memory barrier to tell GCC not to cache `current'.
 */
/*
 * JYW:
 * 该函数的实质就是将移出进程的寄存器上下文保存到移出进程的thread_info->cpu_context结构体中。
 * 并且将移入进程的thread_info->cpu_context结构体中的值restore到cpu的寄存器中
 * ，从而实现堆栈，pc指针和cpu通用寄存器都切换到新的进程上开始执行。
 */
extern struct task_struct *__switch_to(struct task_struct *, struct thread_info *, struct thread_info *);

/* JYW:
 * 负责从上一个进程的处理器状态切换到新进程的处理器状态
 *  包括保存、恢复栈信息和寄存器信息
 */
/* JYW: 进程切换只会发生在内核态，即进程切换只需要考虑内核态的寄存器上下文切换 */
#define switch_to(prev,next,last)					\
do {									\
	last = __switch_to(prev,task_thread_info(prev), task_thread_info(next));	\
} while (0)

#endif /* __ASM_ARM_SWITCH_TO_H */
