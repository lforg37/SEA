#include "util.h"
#include "syscall.h"
#include "sched.h"

void user_process_1()
{
	int v1 = 5;
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

void init_clavier()
{
	__asm("bl UsbInitialise");
	
	while (1)
	{
		int nbClavier = -1;
		
		do {
			__asm("bl UsbCheckForChange");
			__asm("bl KeyboardCount");
			__asm("mov %0, r0" : "=r"(nbClavier));
		} while (nbClavier == 0);

		int adresse = -1;
		int index = 0;
		int nbKeysDown = -1;
		int keyCode = -1;

		__asm("mov r0, %0" : : "r"(index));
		__asm("bl KeyboardGetAddress");
		__asm("mov %0, r0" : "=r"(adresse));

		while (nbKeysDown <= 0)
		{
			__asm("bl KeybordGetKeyDownCount");
			__asm("mov %0, r0" : "=r"(nbKeysDown));

			//wait();
		}
		
		__asm("mov r0, %0" : : "r"(adresse));
		__asm("mov r1, %0" : : "r"(index+1));
		__asm("bl KeyboardGetKeyDown");
		__asm("mov %0, r0" : "=r"(keyCode));
	}
}

void kmain( void )
{
	sched_init(PRIORITY);
	
	create_process((func_t *)&init_clavier);

	create_process((func_t *)&user_process_1);
 	create_process((func_t *)&user_process_2);
    create_process((func_t *)&user_process_3);
	
	timer_init();
    ENABLE_IRQ();

	__asm("cps 0x10"); // switch CPU to USER mode
	
	// **********************************************************************
	while(1)
	{
		sys_yield();
	}
}
