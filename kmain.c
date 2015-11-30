#include "util.h"
#include "syscall.h"
#include "sched.h"
#include "kheap.h"

int user_process(void)
{
	int v;
	for( v = 0 ; v < 2 ; v++ )
	{
		sys_yield();
	}

	return 8;
}

int kmain( void )
{
	sched_init();

	pcb_s* test = create_process(user_process);

	__asm__("cps #0x10");
	uint32_t compteur;
	for(compteur=0;;compteur++);

	uint32_t retour;
	retour = sys_wait(test);
	retour += 0;
	PANIC();

	return 0;
}
