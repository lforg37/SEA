

#include "syscall.h"
#include "util.h"
#include "asm_tools.h"
#include "hw.h"
#include "sched.h"

static void do_sys_reboot();

static void do_sys_nop();

static void do_sys_settime();

static void do_sys_gettime();

uint32_t * g_spArg;

void do_sys_reboot()
{
#ifdef RP
	const int PM_RSTC = 0x2010001c;
	const int PM_WDOG = 0x20100024;
	const int PM_PASSWORD = 0x5a000000;
	const int PM_RSTC_WRCFG_FULL_RESET = 0x00000020;	
	
	Set32(PM_WDOG, PM_PASSWORD | 1);
	Set32(PM_RSTC, PM_PASSWORD | PM_RSTC_WRCFG_FULL_RESET);
	
	while(1);
#else
	__asm("b 0x8000");
#endif
}

void do_sys_nop()
{
	
}

void do_sys_settime()
{	
	uint64_t date_ms;
	
	uint32_t date1 = g_spArg[2];
	uint32_t date2 = g_spArg[3];
	
	date_ms = (uint64_t) date2 << 32 | date1;
		
	set_date_ms(date_ms);
}

void do_sys_gettime()
{
	uint64_t date_ms = get_date_ms();
	
	uint32_t date1 = date_ms & 0x00000000ffffffff;
	uint32_t date2 = (date_ms & 0xffffffff00000000) >> 32;
	
	g_spArg[1] = date1;
	g_spArg[2] = date2;
}

void sys_reboot()
{
	__asm("mov r0, %0" : : "r"(REBOOT));
	__asm("SWI #0");
	
}

void sys_nop()
{
	__asm("mov r0, %0" : : "r"(NOP): "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11");

	__asm("SWI #0");
}

void sys_settime(uint64_t date_ms)
{	
	uint32_t date1 = date_ms & 0x00000000ffffffff;
	uint32_t date2 = (date_ms & 0xffffffff00000000) >> 32;
		
	__asm("mov r1, %0" : : "r"(date1): "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11");
	__asm("mov r2, %0" : : "r"(date2): "r1", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11");

	__asm("mov r0, %0" : : "r"(SETTIME): "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11");

	__asm("SWI #0");

}

uint64_t sys_gettime()
{
	__asm("mov r0, %0" : : "r"(GETTIME): "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11");
	__asm("SWI #0");
	
	uint64_t date_ms;
	
	uint32_t date1, date2;
	__asm("mov %0, r0" : "=r"(date1));
	__asm("mov %0, r1" : "=r"(date2));
	
	date_ms = (uint64_t) date2 << 32 | date1;

	return date_ms;
}

void __attribute__((naked)) swi_handler(void)
{
	__asm("STMFD sp!, {r0-r12, lr}");
	__asm("MRS r4, spsr");
	__asm("STMFD sp!, {r4}");
	
	int action;

	__asm("mov %0, r0" : "=r"(action));
	__asm("mov %0, sp" : "=r"(g_spArg));

	switch (action)
	{
		case REBOOT :
			do_sys_reboot();
			break;
		case NOP :
			do_sys_nop();
			break;
		case SETTIME :
			do_sys_settime();
			break;
		case GETTIME :
			do_sys_gettime();
			break;
		case YIELDTO :
			 do_sys_yieldto();
			 break;
		case SYS_EXIT :
			do_sys_exit();
			break;
		default :
			PANIC();
			break;
	}

	__asm("LDMFD sp!, {r4}");	
	__asm("MSR spsr, r4");
	__asm("LDMFD sp!, {r0-r12, pc}^");	
}
