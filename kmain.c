#include "util.h"
#include "syscall.h"
#include "sched.h"

void user_process_1()
{
	int v1 = 5;
	while(v1 < 7)
	{
		v1++;
	}
}

void user_process_2()
{
	int v2 = -12;
	while(v2 > -17)
	{
		v2 -= 2;
	}
}

void user_process_3() 
{
	int v3 = 0;
	while(v3 > 15)
	{
		v3 += 5;
	}
}

void kmain( void )
{
	sched_init(PRIORITY);
	
	create_process((func_t *)&user_process_1, 3);
 	create_process((func_t *)&user_process_2, 2);
    create_process((func_t *)&user_process_3, 1);
	
	timer_init();
    ENABLE_IRQ();

	__asm("cps 0x10"); // switch CPU to USER mode
	
	// **********************************************************************
	while(1)
	{
		sys_yield();
	}
}
