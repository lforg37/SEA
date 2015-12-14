#include <stdint.h>
#include "string.h"
#include "fb.h"

#define BUFFER_CMD 100
//Globals au programme

// **************************************** définition fonctions locales

char str_cmp(char *s1, char *s2);

void doHello(char argv, char **args);

// ********************************************************************

void bash_process()
{
	char buffer[BUFFER_CMD] = "hello";
	while (1)
	{
		//getString(buffer, BUFFER_CMD);
		
		char item_bytes[BUFFER_CMD];
		char *cmdExploded[BUFFER_CMD];
		int size = explode(cmdExploded, " ", buffer, item_bytes);
		
		if (strcmp (cmdExploded[0], "hello") == 0)
			doHello(size, cmdExploded);
		buffer[0] = '\0';
	}
}


void doHello(char argv, char **args)
{
	draw(0, 0, 0);
	drawString("Hello", 0, 30, 255, 255, 255);
		
}
