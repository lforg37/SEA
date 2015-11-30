#ifndef SCHED_H
#define SCHED_H

#include <stdint.h>
#include "syscall.h"

typedef int (func_t)(void);

typedef struct pcb_s
{
	uint32_t r[13];
	uint32_t lr_svc;
	uint32_t lr_user;
	uint32_t * stack;
	uint32_t * sp;
	uint32_t CPSR_user;
	struct pcb_s * next_process;
	struct pcb_s * prev_process;
} pcb_s;

pcb_s *create_process(func_t* entry);

void sys_yieldto(struct pcb_s* dest);	

void sys_yield();	

void sys_exit();

void do_sys_yieldto();

void do_sys_exit();

void sched_init();
#endif