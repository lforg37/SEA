#ifndef SYS_CALL_H
#define SYS_CALL_H

#include <stdint.h>

void sys_reboot();

void sys_nop();

void sys_settime(uint64_t date);

uint64_t sys_gettime();

void __attribute__((naked)) swi_handler(void);

enum action
{
	DEFAULT, REBOOT, NOP, SETTIME, GETTIME, YIELDTO, SYS_EXIT
};
#endif
