#include "kb.h"
#include "hw.h"
#include "fb.h"
#include "sched.h"
#include "string.h"

void KeyboardLoop()
{	
	while (1)
	{
		KeyboardUpdate();
		//wait(10);
	}
}

void getLine(char *buffer)
{
	getString(buffer, '\n');
}

void getString(char *buffer, char delimiter)
{
	int size = strlen(buffer);
	if (size == 0) 
		return;
		
	char c;
	int i;
	for (i = 0 ; i < size && buffer[i] != delimiter ; ++i)
	{
		c = KeyboardGetChar();
		while (c != 0)
			buffer[i] = c;
	}
	buffer[i] = '\0';
}
