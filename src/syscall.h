/******************************************************************************
 *              syscall.h : system calls definition
 *****************************************************************************/
#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

#define SYSCALL(x) ( { __asm__("mov r0, %[type]" : : [type] "r" (x)); __asm__("swi #0"); } )

#define SETPARAM1(x) (\
	{ __asm__("mov r1, %[argument]" : : [argument]"r"(x));}\
		)

#define SETPARAM2(x) (\
	{ __asm__("mov r2, %[argument]" : : [argument]"r"(x));}\
		)
typedef enum syscall_type {
	REBOOT,
	SETTIME,
	GETTIME,
	EXIT,
	NOP,
	YIELD
} syscall_type;

void sys_reboot(void);

uint64_t sys_gettime(void);

void sys_nop(void);

void sys_settime(uint64_t date_ms);

void __attribute__((naked)) swi_handler(void);

#endif
