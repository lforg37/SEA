#include "util.h"
#include "syscall.h"
#include "sched.h"
#include "stdint.h"
#include "kb.h"
#include "hw.h"
#include "fb.h"

void test()
{
	while(1)
	{
		char buffer[10];
		getLine(buffer);
		drawString(buffer, 30, 50, 255, 255, 255);
	}
}

void kmain( void )
{
	
	sched_init(PRIORITY);
	FramebufferInitialize();
	
	UsbInitialise();
	
	create_process((func_t *)&KeyboardLoop, 3);
	create_process((func_t *)&test, 1);
	
	timer_init();
  	ENABLE_IRQ();

	__asm("cps 0x10"); // switch CPU to USER mode	
	
	// **********************************************************************
	while(1)
	{
		sys_yield();		
	}
}
