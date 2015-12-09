#include "util.h"
#include "syscall.h"
#include "sched.h"
#include "fb.h"

void user_process_1()
{
	while(1)
	{
		
	}
}


void kmain( void )
{
	sched_init(PRIORITY);
	FramebufferInitialize();
	
	//create_process((func_t *)&user_process_1, 3);
	
	timer_init();
    ENABLE_IRQ();

	__asm("cps 0x10"); // switch CPU to USER mode
	
	draw(0, 0, 0);
	drawString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est\n laborum.", 0, 30, 255, 0, 0);
		
	
	// **********************************************************************
	while(1)
	{
		
	}
}
