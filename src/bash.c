#include <stdint.h>
#include "string.h"
#include "fb.h"
#include "kb.h"

#define BUFFER_CMD 100
//Globals au programme

// **************************************** définition fonctions locales

char str_cmp(char *s1, char *s2);

void doHello(char argv, char **args);
void doClear(char argv, char **args);
void doLorem(char argv, char **args);

// ********************************************************************

void bash_process()
{
	char buffer[BUFFER_CMD];
	while (1)
	{
		getLine(buffer, BUFFER_CMD);
		draw(0, 0, 0);
		drawString(buffer, 0, 30, 255, 255, 255);
		
		char item_bytes[BUFFER_CMD];
		char *cmdExploded[BUFFER_CMD];
		int size = explode(cmdExploded, " ", buffer, item_bytes);
		
		//if (strcmp (cmdExploded[0], "hello") == 0)
		//	doHello(size, cmdExploded);
		if (cmdExploded[0][0] == 'l')
			doLorem(0, NULL);
		//if (strcmp (cmdExploded[0], "clear") == 0)
			//doClear(size, cmdExploded);
		//buffer[0] = '\0';
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
	
	drawString("Hello", 100, 130, 255, 255, 255);
	//printf("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Morbi sit amet orci metus. Nam dictum efficitur lectus, sed aliquam dui gravida a. Mauris vel hendrerit metus. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vestibulum vulputate mattis tortor, at finibus mauris finibus ornare. Etiam vel congue magna. Morbi arcu urna, egestas eget erat at, suscipit pretium felis. Sed sed vestibulum purus. Fusce pulvinar luctus congue. Donec luctus dui nec elementum efficitur. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; Nullam at mi ligula. Vestibulum fermentum in nisl posuere tempor. Morbi maximus justo id nulla viverra ultricies vitae in leo. Phasellus tempus, lacus ac lacinia aliquam, dui tortor volutpat tortor, sit amet viverra purus mauris sit amet sem. Nunc at enim ut sapien tristique blandit sed et mi. \n Morbi turpis leo, auctor eget ullamcorper quis, rhoncus eget dui. Donec interdum mollis lacus vel dapibus. Vestibulum egestas tempor velit, sit amet rhoncus ligula tincidunt ut. Suspendisse justo mi, volutpat et dapibus et, lobortis in urna. Mauris tincidunt nulla ut augue bibendum, nec luctus elit posuere. Sed rhoncus sodales congue. Phasellus a libero nulla. Sed varius neque eu neque tempus, non ullamcorper lacus eleifend. \n Aenean auctor hendrerit dictum. Maecenas dignissim sapien non neque fringilla porttitor eu a metus. Nam fringilla scelerisque lectus, vitae placerat sapien ullamcorper nec. Quisque pretium nec sem a pretium. Vivamus vehicula ultricies ante eget vestibulum. Nullam ac arcu id orci porta sodales a at leo. Nulla pulvinar nisi vulputate nisl iaculis lobortis. Aenean eget mi vitae dolor venenatis vestibulum non sit amet tortor. Proin faucibus lobortis leo sed volutpat. Vivamus accumsan eros vitae congue euismod. Etiam dapibus dolor non volutpat gravida. Pellentesque ultricies nunc id volutpat lacinia. Etiam consectetur, turpis et gravida egestas, libero orci pellentesque ante, a euismod ante est quis nulla. ");
}
