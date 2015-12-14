#include "keyboard-nous.h"
#include "hw.h"
#include "fb.h"
#include "sched.h"

char KeysNormal[104] = 
	{0x0, 0x0, 0x0, 0x0, 'a', 'b', 'c', 'd',
	 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
	 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
	 'u', 'v', 'w', 'x', 'y', 'z', '1', '2',
	 '3', '4', '5', '6', '7', '8', '9', '0',
	 '\n', 0x0, '\b', '\t', ' ', '-', '=', '[',
	 ']', '\\', '#', ';', '\'', '`', ',', '.',
	 '/', 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	 0x0, 0x0, 0x0, 0x0, '/', '*', '-', '+',
	 '\n', '1', '2', '3', '4', '5', '6', '7',
	 '8', '9', '0', '.', '\\', 0x0, 0x0, '='};

// on a fait péter le £ et ¬ car constantes multi char
char KeysShift[104] = 
	{ 0x0, 0x0, 0x0, 0x0, 'A', 'B', 'C', 'D',
	'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
	'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
	'U', 'V', 'W', 'X', 'Y', 'Z', '!', '"',
	0x0, '$', '%', '^', '&', '*', '(', ')',
	'\n', 0x0, '\b', '\t', ' ', '_', '+', '{',
	'}', '|', '~', ':', '@', 0x0, '<', '>',
	'?', 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, '/', '*', '-', '+',
	'\n', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', '0', '.', '|', 0x0, 0x0, '='};


uint16_t keysDown[6];
uint16_t keysDownOld[6];

uint32_t ADDRESS;

extern char KeyboardGetChar();

void KeyboardUpdate2()
{	
	int position = 0;//, r = 0;
	
	for ( ; ; )
	{
		//led_blink();

		// Check if there is a new device on an usb port
		UsbCheckForChange();
	
		// Count keyboards
		uint32_t nbKeyboards = KeyboardCount();
		
				drawChar('0' + nbKeyboards, 0, 30 + position, 255, 0, 0);
				position += 15;
		// If there is no keyboard, just iterate
	//	if (nbKeyboards <= 0) 
		//	continue;
		
		// Else, get the address
		//ADDRESS = KeyboardGetAddress(nbKeyboards-1);

		// Check if keyoard well connected
		
			//char c = KeyboardGetChar2();
			//drawChar(c, 0, 300 , (r++ )% 255, 0, 0);
			
			//if (c > 'a' && c < 'z')
			//{
				drawChar('A', 0, 30 + position, 255, 0, 0);
				position += 15;
			//}
	}
}

void KeyboardBufferInitialize()
{
	for (int i = 0 ; i < 6 ; i++)
	{
		keysDown[i] = 0;
	}
	
	for (int i = 0 ; i < 6 ; i++)
	{
		keysDownOld[i] = 0;
	}
}

int KeyWasDown2(uint16_t scanCode)
{
	for (int i = 0 ; i < 6 ; i++)
	{
		if (keysDownOld[i] == scanCode)
		{
			return 1;
		}
	}
	return 0;
}

char KeyboardGetChar2()
{
	if (KeyboadGetKeyIsDown(ADDRESS, 4) != 0)
		return 'a';
	else if (KeyboadGetKeyIsDown(ADDRESS, 30) != 0)
		return 'z';
	return 0;
	
	/*for (int i = 0 ; i < 6 ; i++)
	{
		keysDownOld[i] = keysDown[i];
	}
	*/
	//uint32_t nbKeysDown = KeyboardGetKeyDownCount(ADDRESS);
	
	//if (nbKeysDown > 0)
	//	return KeyboardGetKeyDown(ADDRESS, 0);
	//return 0;
	
	//uint16_t key = 0;
	//char c_key;
	
	//char c = KeyboardGetChar();
	
	//return KeysNormal[KeyboardGetKeyDown(ADDRESS, 0)];
	
	/*for (int i = 0 ; i < nbKeysDown ; i++)
	{
		keysDown[i] = KeyboardGetKeyDown(ADDRESS, i);
	}
	for (int i = nbKeysDown ; i < 6 ; i++)
	{
		keysDown[i] = 0;
	}
	
	for (int i = 0 ; i < nbKeysDown ; i++)
	{
		if (!KeyWasDown(keysDown[i]))
		{
			key = keysDown[i];
			break;
		}
	}	
	
	int modifier = KeyboardGetModifiers(ADDRESS);
	if ((modifier & 0x01000000) == 0x01000000 
	 || (modifier & 0x00000100) == 0x00000100)
	{
		c_key = KeysShift[key];
	}
	else
	{
		c_key = KeysNormal[key];
	}
		
	return c_key;*/
}
