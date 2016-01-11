#include "util.h"
#include "syscall.h"
#include "sched.h"
#include "stdint.h"
#include "kb.h"
#include "hw.h"
#include "fb.h"
#include "bash.h"
#include "kb.h"



void kmain( void )
{
	
	sched_init(PRIORITY);
	FramebufferInitialize();
	
	UsbInitialise();
	
	create_process((func_t *)&bash_process, 3);
	
	timer_init();
    	ENABLE_IRQ();

	__asm("cps 0x10"); // switch CPU to USER mode		
	
	// **********************************************************************
	sys_yield();
}
