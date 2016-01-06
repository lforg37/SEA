#include "kb.h"
#include "hw.h"
#include "fb.h"
#include "sched.h"
#include "string.h"

void getLine(char *buffer, int tailleBuf)
{
	getString(buffer, tailleBuf, '\n');
}

void getString(char *buffer, int tailleBuf, char delimiter)
{
	int i;
		
	if (tailleBuf == 0) 
		return;
		
		
	char c;
	for (i = 0 ; i < tailleBuf - 1 && buffer[i - 1] != delimiter ; ++i)
	{
		c = 0;
		while (c == 0)
		{
			KeyboardUpdate();
			addToBuffer(c);
		}
		buffer[i] = c;
	}
}
