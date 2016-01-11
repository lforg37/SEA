#include "util.h"
#include "syscall.h"
#include "sched.h"

void user_process_1()
{
	int v1 = 5;
	
	uint32_t* tab = (uint32_t*)gmalloc(sizeof(uint32_t)*2);
	uint32_t* tab2 = (uint32_t*)gmalloc(sizeof(uint32_t)*3);
	*tab = 0xffffff1f;
	*(tab + 1) = 0xffffff2f;

	*tab2 = 0;
	*(tab2 + 1) = 1;
	*(tab2 + 2) = 2;
	
	gfree(tab);
	gfree(tab2);

	*tab = 0xffffff3f; /* doit generer une erreur */

	while(1)
	{
		v1++;
	}

	
}

void user_process_2()
{
	int v2 = -12;
	while(1)
	{
		v2 -= 2;
		sys_wait(30);
	}
}

void user_process_3() 
{
	int v3 = 0;
	while(1)
	{
		v3 += 5;
	}
}

void kmain( void )
{
	sched_init(PRIORITY);
	
	create_process((func_t *)&user_process_1, 3);
 	create_process((func_t *)&user_process_2, 3);
	
	timer_init();
    ENABLE_IRQ();

	__asm("cps 0x10"); // switch CPU to USER mode
	
	// **********************************************************************
	while(1)
	{
		sys_yield();
	}
}
