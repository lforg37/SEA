#include "asm_tools.h"
#include "syscall.h"
#include "util.h"
#include "hw.h"
#include "sched.h"


/*************** System calls handlers *************************************/
static void do_sys_reboot(void)
{
#ifdef RASP_PI
	const int PM_RSTC = 0x2010001c;
	const int PM_WDOG = 0x20100024;
	const int PM_PASSWORD = 0x5a000000;
	const int PM_RSTC_WRCFG_FULL_RESET = 0x00000020;

	Set32(PM_WDOG, PM_PASSWORD | 1 );
	Set32(PM_RSTC, PM_PASSWORD | PM_RSTC_WRCFG_FULL_RESET);
	for(;;);
#else
	__asm__("b 0x8000");
#endif
}

static void do_sys_nop(void)
{
	__asm__("nop");
}

static void do_sys_settime(void* args)
{
	uint64_t date_ms;
	uint32_t *lower = (uint32_t*) &date_ms;
	uint32_t *upper = ((uint32_t*) &date_ms) + 1;

	*lower = *(uint32_t *) args;
	*upper = *(((uint32_t *) args) + 1);
	
	set_date_ms(date_ms);
}

static void do_sys_gettime(void* returnaddr)
{
	uint64_t retour = get_date_ms();
	uint32_t *lower = (uint32_t*) &retour;
	uint32_t *upper = lower + 1;

	__asm__("str %[what], [%[where]]" : : 
				[what]"r"(*lower),
				[where]"r"(returnaddr-4));

	__asm__("str %[what], [%[where]]" : : 
				[what]"r"(*upper),
				[where]"r"(returnaddr));
}


/********************* Public functions *************************************/
void sys_reboot(void)
{
	SYSCALL(REBOOT);	
}

void sys_nop(void)
{
	SYSCALL(NOP);
}

void sys_settime(uint64_t date_ms)
{
	uint32_t* lower = (uint32_t*) &date_ms;
	uint32_t *upper = ((uint32_t*) &date_ms) + 1;

	__asm__("mov r1, %[lower];" : : [lower]"r"(*lower));
	__asm__("mov r2, %[upper];" : : [upper]"r"(*upper));

	SYSCALL(SETTIME);
}

uint64_t sys_gettime(void)
{
	SYSCALL(GETTIME);
	uint64_t retour;
	
	__asm__("mov %[retour], r0" : [retour]"=r"(retour));
	__asm__("mov %[retour], r1" : [retour]"=r"(*(((uint32_t*)&retour)+1)));

	return retour;
}

void swi_handler(void) 
{
	__asm__("stmfd sp!, {r1-r12, lr}");
	void *args;
	reg_t lrsvc;
	__asm__("mov %[args], sp" : [args]"=r"(args) : : "sp");
	syscall_type type;
	__asm__("mov %[type], r0" : [type]"=r"(type));
	__asm__("add sp, sp, #-4");
	__asm__("mov %[lrsvc], lr" : 
				[lrsvc]"=r"(lrsvc)
			);

	switch(type) {
		case REBOOT:
			do_sys_reboot();
			break;
		case NOP:
			do_sys_nop();
			break;
		case SETTIME:
			do_sys_settime(args);
			break;
		case GETTIME:
			do_sys_gettime(args);
			break;
		case YIELDTO:
			do_sys_yieldto(args, lrsvc);
			break;
		case YIELD:
			do_sys_yield(args, lrsvc);
			break;
		default:
			PANIC();
			break;
	}
	__asm__("ldr r0, [sp], #4");

	__asm__("ldmfd sp!, {r1-r12, pc}^");
}

