#include "util.h"
#include "syscall.h"
#include "sched.h"
#include "kheap.h"

pcb_s pcb1, pcb2;
pcb_s *p1, *p2;

int user_process_1()
{
	int v1 = 5;
	for(;;)
	{
		v1++;
		sys_yield();
	}
}

int user_process_2()
{
	int v2 = -12;
	for(;;)
	{
		v2-=2;
		sys_yield();
	}
}


int kmain( void )
{
	sched_init();

	p1 = create_process(user_process_1);
	p2 = create_process(user_process_2);

	__asm__("cps #0x10");
	
	for(;;)
		sys_yield();

	sys_reboot();
	return 0;
}
