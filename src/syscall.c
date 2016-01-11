#include "syscall.h"
#include "util.h"
#include "asm_tools.h"
#include "hw.h"
#include "sched.h"
#include "vmem.h"


//Globals au programme
uint32_t * g_spArg;
extern pcb_s *g_current_process; 

// **************************************** définition fonctions locales
//Retourne l'heure du système (mode privilégié)
static void do_sys_gettime();

//inutile
static void do_sys_nop();

//Redémarre le RPI (mode privilégié)
static void do_sys_reboot();

//Modifie l'heure du système mode privilégié)
static void do_sys_settime();

// ********************************************************************

void do_sys_gettime()
{
	uint64_t date_ms = get_date_ms();
	
	uint32_t date1 = date_ms & 0x00000000ffffffff;
	uint32_t date2 = (date_ms & 0xffffffff00000000) >> 32;
	
	g_spArg[1] = date1;
	g_spArg[2] = date2;
}

static void do_sys_mmap()
{
	size_t size = g_spArg[2];
	uint8_t* start_addr = vmem_alloc_for_userland(g_current_process, size);	
	g_spArg[1] = (uint32_t) start_addr;
}

static void do_sys_munmap()
{
	size_t size = g_spArg[3];
	uint32_t addr = g_spArg[2];
	
	vmem_free((uint8_t*) addr, g_current_process, size);
}

void do_gmalloc()
{
	size_t size = g_spArg[2];
	uint8_t* addr;
	
	addr = get_contiguous_addr(g_current_process, size);
	
	g_spArg[1] = (uint32_t) addr;
}

void do_gfree()
{
	uint32_t address = g_spArg[2];
	uint8_t* ptr = (uint8_t*) address;
	
	free_addr(ptr, g_current_process);
}

void do_sys_nop()
{	
}

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

void do_sys_settime()
{	
	uint64_t date_ms;
	
	uint32_t date1 = g_spArg[2];
	uint32_t date2 = g_spArg[3];
	
	date_ms = (uint64_t) date2 << 32 | date1;
		
	set_date_ms(date_ms);
}

void __attribute__((naked)) swi_handler(void)
{
	__asm("STMFD sp!, {r0-r12, lr}");
	__asm("MRS r4, spsr");
	__asm("STMFD sp!, {r4}");
	
	int action;

	__asm("mov %0, r0" : "=r"(action));
	__asm("mov %0, sp" : "=r"(g_spArg));

	switch_os();

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
		case WAIT :
			do_sys_wait();
			break;
		case MMAP :
			do_sys_mmap();
			break;
		case MUMAP :
			do_sys_munmap();
			break;
		case GFREE :
			do_gfree();
			break;
		case GMALLOC :
			do_gmalloc();
			break;
		default :
			PANIC();
			break;
	}

	handle_vmem(g_current_process);

	__asm("LDMFD sp!, {r4}");	
	__asm("MSR spsr, r4");
	__asm("LDMFD sp!, {r0-r12, pc}^");	
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

void* sys_mmap(size_t size)
{
	__asm("mov r1, %0" : : "r"(size));
	__asm("mov r0, %0" : : "r"(MMAP));
	__asm("SWI #0");

	void* address; 
	__asm("mov %0, r0" : "=r"(address));

	return address;
}


void sys_munmap(void* addr, size_t size)
{
	__asm("mov r2, %0" : : "r"(size));
	__asm("mov r1, %0" : : "r"(addr));
	__asm("mov r0, %0" : : "r"(MUMAP));
	
	__asm("SWI #0");
}


void* gmalloc(size_t size)
{
	__asm("mov r1, %[size]" : : [size]"r"(size));
	__asm("mov r0, %0" : : "r"(GMALLOC));
	
	__asm("SWI #0");
	
	void* ptr;

	__asm("mov r0, %[ptr]" : [ptr]"=r"(ptr));

	return ptr;	
}

void gfree(void* ptr)
{
	uint32_t address = (uint32_t) ptr;
	
	__asm("mov r1, %[address]" : : [address]"r"(address));
	__asm("mov r0, %0" : : "r"(GFREE));
	
	__asm("SWI #0");
}

void sys_nop()
{
	__asm("mov r0, %0" : : "r"(NOP): "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11");

	__asm("SWI #0");

}

void sys_reboot()
{
	__asm("mov r0, %0" : : "r"(REBOOT));
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
