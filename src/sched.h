#ifndef SCHED_H
#define SCHED_H

#include <stdint.h>
#include "syscall.h"
#include "hw.h"
#include "asm_tools.h"

typedef int (func_t)(void);

typedef enum scheduler
{
	PRIORITY
} scheduler;

typedef enum process_state
{
	READY, RUNNING, TERMINATED, WAITING
} process_state;


typedef struct pcb_s
{
	uint32_t r[13];
	uint32_t lr_svc;
	uint32_t lr_user;
	uint32_t * stack;
	uint32_t * sp;
	uint32_t CPSR_user;
	uint32_t priority;
	process_state state;
	struct pcb_s * next_process;
	struct pcb_s * prev_process;
	
	uint64_t wakingTime;
} pcb_s;

pcb_s *create_process(func_t* entry, int priority);

void setScheduler(scheduler s);

void sys_yieldto(struct pcb_s* dest);	

void sys_yield();	

void sys_exit();

void sys_wait(uint32_t miliseconds);

void do_sys_yieldto();

void do_sys_wait();

void do_sys_exit();

void sched_init(scheduler s);

void __attribute__((naked)) irq_handler(void);

#endif
