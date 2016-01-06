#include <stdint.h>
#include "string.h"
#include "fb.h"

#define BUFFER_CMD 100
#define BUFFER_HEIGHT 10
#define BUFFER_WIDTH fb_x/8 - 20
//Globals au programme

static int g_iCol;
static int g_iLin;
static char g_bufferScreen[BUFFER_HEIGHT + 1][BUFFER_WIDTH];

// **************************************** définition fonctions locales

char str_cmp(char *s1, char *s2);

void doHello(char argv, char **args);

// ********************************************************************

void bash_process()
{
	char buffer[BUFFER_CMD] = "lorem";
	while (1)
	{
		//getString(buffer, BUFFER_CMD);
		
		char item_bytes[BUFFER_CMD];
		char *cmdExploded[BUFFER_CMD];
		int size = explode(cmdExploded, " ", buffer, item_bytes);
		
		if (strcmp (cmdExploded[0], "hello") == 0)
			doHello(size, cmdExploded);
		if (strcmp (cmdExploded[0], "lorem") == 0)
			bufferFill = doLorem(size, cmdExploded);
		if (strcmp (cmdExploded[0], "clear") == 0)
			doClear(size, cmdExploded);
		buffer[0] = '\0';
	}
}

void doHello(char argv, char **args)
{
	draw(0, 0, 0);
	drawString("Hello", 0, 30, 255, 255, 255);
		
}

void doClear(char argv, char **args)
{
	draw(0, 0, 0);
	//vider buffer
}

void doLorem(char argv, char **args)
{
	printf("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Morbi sit amet orci metus. Nam dictum efficitur lectus, sed aliquam dui gravida a. Mauris vel hendrerit metus. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vestibulum vulputate mattis tortor, at finibus mauris finibus ornare. Etiam vel congue magna. Morbi arcu urna, egestas eget erat at, suscipit pretium felis. Sed sed vestibulum purus. Fusce pulvinar luctus congue. Donec luctus dui nec elementum efficitur. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; Nullam at mi ligula. Vestibulum fermentum in nisl posuere tempor. Morbi maximus justo id nulla viverra ultricies vitae in leo. Phasellus tempus, lacus ac lacinia aliquam, dui tortor volutpat tortor, sit amet viverra purus mauris sit amet sem. Nunc at enim ut sapien tristique blandit sed et mi. \n Morbi turpis leo, auctor eget ullamcorper quis, rhoncus eget dui. Donec interdum mollis lacus vel dapibus. Vestibulum egestas tempor velit, sit amet rhoncus ligula tincidunt ut. Suspendisse justo mi, volutpat et dapibus et, lobortis in urna. Mauris tincidunt nulla ut augue bibendum, nec luctus elit posuere. Sed rhoncus sodales congue. Phasellus a libero nulla. Sed varius neque eu neque tempus, non ullamcorper lacus eleifend. \n Aenean auctor hendrerit dictum. Maecenas dignissim sapien non neque fringilla porttitor eu a metus. Nam fringilla scelerisque lectus, vitae placerat sapien ullamcorper nec. Quisque pretium nec sem a pretium. Vivamus vehicula ultricies ante eget vestibulum. Nullam ac arcu id orci porta sodales a at leo. Nulla pulvinar nisi vulputate nisl iaculis lobortis. Aenean eget mi vitae dolor venenatis vestibulum non sit amet tortor. Proin faucibus lobortis leo sed volutpat. Vivamus accumsan eros vitae congue euismod. Etiam dapibus dolor non volutpat gravida. Pellentesque ultricies nunc id volutpat lacinia. Etiam consectetur, turpis et gravida egestas, libero orci pellentesque ante, a euismod ante est quis nulla. ");
}

void addToBuffer(char c)
{
	int drawAll = 0;
	
	if (g_iCol +1 > BUFFER_WIDTH || c == '\n')
	{
		g_iLin++;
		g_iCol = 0;
	}
	
	if (g_iLin > BUFFER_HEIGHT)
	{
		int j;
		for(j = 0; j < g_iLin - 1; j++)
		{
			g_bufferScreen[j] = g_bufferScreen[j+1];
		}
		drawAll = 1;
	}
	if (c != '\n')
	{
		g_bufferScreen[g_iLin][g_iCol] = c;
	}
	
	if (drawAll == 1)
		drawBuffer(10, 10, 255, 255, 255, g_bufferScreen, BUFFER_WIDTH, g_iLin);
	else
		drawChar(g_bufferScreen[g_iLin][g_iCol], 10 + g_iCol * 8, 10 + g_iLin * 16, 255, 255, 255);
}

void printf(char * string)
{
	int size = strlen(string);
	for (int i = 0 ; i< size ; i++)
	{
		addToBuffer(string[i]);
	}
}
