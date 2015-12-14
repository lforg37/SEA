#include "util.h"
#include "syscall.h"
#include "sched.h"
#include "stdint.h"
#include "keyboard-nous.h"
#include "usbd.h"
#include "hw.h"
#include "fb.h"

//extern void KeyboardUpdate();
//void UsbInitialise();

//extern char KeyboardGetChar();
/*
void blink_leds(int fois, int adresse) 
{
	int eteint = 0x00000000;
	int leds = 0;
	
	__asm("mov r0, %0" : : "r"(adresse)); //address
	__asm("bl KeyboardGetLedSupport");
	__asm("mov %0, r0" : "=r"(leds));

	for (int i = 0; i<fois ; i++)
	{
		__asm("mov r0, %0" : : "r"(adresse)); //address
		__asm("mov r1, %0" : : "r"(leds));
		__asm("bl KeyboardSetLeds");
		
		__asm("mov r0, %0" : : "r"(adresse)); //address
		__asm("mov r1, %0" : : "r"(eteint));
		__asm("bl KeyboardSetLeds");
	}
}
*/
void user_process_1()
{
	while(1)
	{
		
	}
}


void init_clavier()
{	
	/*int nbClavier = -1;

	__asm("bl UsbInitialise");
	do {
		__asm("bl UsbCheckForChange");
		__asm("bl KeyboardCount");
		__asm("mov %0, r0" : "=r"(nbClavier));
	} while (nbClavier <= 0);
	
	int adresse = -1;
	int index = 0;

	__asm("mov r0, %0" : : "r"(index));
	__asm("bl KeyboardGetAddress");
	__asm("mov %0, r0" : "=r"(adresse));*/




	/*extern uint32_t KeyboardAddress;
	
	KeyboardAddress = -1;
	
	do {
		KeyboardUpdate();
	} while (KeyboardAddress == -1);
		
	blink_leds(KeyboardAddress, adresse);*/
	
	/*char lastChar = 0;
	
	do {
		lastChar = KeyboardGetChar();
	} while(lastChar == 0);

	blink_leds(200, adresse);
	*/
	
	__asm("bl UsbInitialise");
	
	while (1)
	{
		int nbClavier = -1;
		
		do {
			__asm("bl UsbCheckForChange");
			__asm("bl KeyboardCount");
			__asm("mov %0, r0" : "=r"(nbClavier));
		} while (nbClavier < 0);

		int adresse = -1;
		int index = 0;
		int nbKeysDown = 0;
		//int keyCode = -1;
		//int status = 0;
		
		//int keysPushed[50];
			

				
		__asm("mov r0, %0" : : "r"(index));
		__asm("bl KeyboardGetAddress");
		__asm("mov %0, r0" : "=r"(adresse));

		/*__asm("mov r0, %0" : : "r"(adresse)); //address
		__asm("mov r1, %0" : : "r"(0)); //key number
		__asm("bl KeyboardGetKeyDown");
		__asm("mov %0, r0" : "=r"(keyCode)); //scan code*/

		__asm("mov r0, %0" : : "r"(adresse)); //address
		__asm("bl KeyboardGetKeyDownCount");
		__asm("mov %0, r0" : "=r"(nbKeysDown));

		int eteint = 0x00000000;
		int leds = 0;
		
		__asm("mov r0, %0" : : "r"(adresse)); //address
		__asm("bl KeyboardGetLedSupport");
		__asm("mov %0, r0" : "=r"(leds));

		for (int i = 0 ; i<nbKeysDown ; i++)
		{
			__asm("mov r0, %0" : : "r"(adresse)); //address
			__asm("mov r1, %0" : : "r"(leds));
			__asm("bl KeyboardSetLeds");
			
			__asm("mov r0, %0" : : "r"(adresse)); //address
			__asm("mov r1, %0" : : "r"(eteint));
			__asm("bl KeyboardSetLeds");
		}
			
			/*keysPushed[0] = keyCode;*/
			//blink_leds(keyCode, adresse);

		/*}
		
		for (int i = 0 ; i<nbKeysDown ; i++)
		{
			__asm("mov r0, %0" : : "r"(adresse)); //address
			__asm("mov r1, %0" : : "r"(keysPushed[i])); //key number
			__asm("bl KeyboadGetKeyIsDown");
			__asm("mov %0, r0" : "=r"(status)); //scan code
			
			if (status != 0)
			{
				blink_leds(1, adresse);
			}
		}*/
	}
}

void kmain( void )
{
	//led_blink();
	
	sched_init(PRIORITY);
	FramebufferInitialize();
	
	UsbInitialise();
	//KeyboardBufferInitialize();
	
	draw (0, 0, 0);
	create_process((func_t *)&KeyboardUpdate2, 3);

/*
	create_process((func_t *)&user_process_1);
 	create_process((func_t *)&user_process_2);
    create_process((func_t *)&user_process_3);
*/
	//create_process((func_t *)&user_process_1, 3);
	
	timer_init();
    ENABLE_IRQ();
	
	__asm("cps 0x10"); // switch CPU to USER mode

	sys_yield();	
	
	// **********************************************************************
	volatile int i=0;
	while(1)
	{
		i++;
	}
}
