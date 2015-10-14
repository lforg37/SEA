/**********************************************************************
 *          sched.h system calls related to scheduling
 *
 * *******************************************************************/
#ifndef SCHED_H
#define SCHED_H

#include <stdint.h>

#define STACK_SIZE 10240
#define R0 context[0]
#define R1 context[1]
#define R2 context[2]
#define R3 context[3]
#define R4 context[4]
#define R5 context[5]
#define R6 context[6]
#define R7 context[7]
#define R8 context[8]
#define R9 context[9]
#define R10 context[10]
#define R11 context[11]
#define R12 context[12]

typedef uint32_t reg_t;

typedef struct pcb_s {
	 reg_t context[13];
	 reg_t lr_user;
	 reg_t sp;
	 reg_t cpsr;

	 pcb_s* next_task;
} pcb_s;

void sys_yield(void);
void sys_yieldto(pcb_s* dest);

typedef int (func_t)(void);

void sched_init(void);

void do_sys_yieldto(void *args, reg_t lrsvc);

pcb_s* create_process(func_t entry);
#endif
