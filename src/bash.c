#include <stdint.h>
#include "string.h"
#include "fb.h"
#include "kb.h"

#define BUFFER_CMD 100
//Globals au programme

// **************************************** définition fonctions locales

char str_cmp(char *s1, char *s2);

void doEcho(char argv, char **args);
void doClear(char argv, char **args);
void doLorem(char argv, char **args);
void doPlusOuMoins(char argv, char **args);
void doDrawRect(char argv, char **args);
void doDrawText(char argv, char **args);

// ********************************************************************

void bash_process()
{
	char buffer[BUFFER_CMD];
	while (1)
	{
		printf("\n$ ");
		getLine(buffer, BUFFER_CMD);
	
		char item_bytes[BUFFER_CMD];
		char *cmdExploded[BUFFER_CMD];
		int size = explode(cmdExploded, " ", buffer, item_bytes);
		
		if (strcmp (cmdExploded[0], "echo") == 0)
			doEcho(size, cmdExploded);
		else if (strcmp (cmdExploded[0], "lorem")== 0)
			doLorem(0, NULL);
		else if (strcmp (cmdExploded[0], "clear") == 0)
			doClear(size, cmdExploded);
		else if (strcmp (cmdExploded[0], "pm") == 0)
			doPlusOuMoins(size, cmdExploded);
		else if (strcmp (cmdExploded[0], "drawRect") == 0)
			doDrawRect(size, cmdExploded);
		else if (strcmp (cmdExploded[0], "drawText") == 0)
			doDrawText(size, cmdExploded);
		else
			printf("Commande inconnue");
		buffer[0] = '\0';
	}
}

void doEcho(char argv, char **args)
{
	for (int i = 1 ; i < argv ; ++i)
	{
		printf(args[i]);
		printf(" ");
	}
}

void doClear(char argv, char **args)
{
	clear();
}

void doLorem(char argv, char **args)
{
	printf("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Morbi sit amet orci metus. Nam dictum efficitur lectus, sed aliquam dui gravida a. Mauris vel hendrerit metus. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vestibulum vulputate mattis tortor, at finibus mauris finibus ornare. Etiam vel congue magna. Morbi arcu urna, egestas eget erat at, suscipit pretium felis. Sed sed vestibulum purus. Fusce pulvinar luctus congue. Donec luctus dui nec elementum efficitur. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; Nullam at mi ligula. Vestibulum fermentum in nisl posuere tempor. Morbi maximus justo id nulla viverra ultricies vitae in leo. Phasellus tempus, lacus ac lacinia aliquam, dui tortor volutpat tortor, sit amet viverra purus mauris sit amet sem. Nunc at enim ut sapien tristique blandit sed et mi. \n Morbi turpis leo, auctor eget ullamcorper quis, rhoncus eget dui. Donec interdum mollis lacus vel dapibus. Vestibulum egestas tempor velit, sit amet rhoncus ligula tincidunt ut. Suspendisse justo mi, volutpat et dapibus et, lobortis in urna. Mauris tincidunt nulla ut augue bibendum, nec luctus elit posuere. Sed rhoncus sodales congue. Phasellus a libero nulla. Sed varius neque eu neque tempus, non ullamcorper lacus eleifend. \n Aenean auctor hendrerit dictum. Maecenas dignissim sapien non neque fringilla porttitor eu a metus. Nam fringilla scelerisque lectus, vitae placerat sapien ullamcorper nec. Quisque pretium nec sem a pretium. Vivamus vehicula ultricies ante eget vestibulum. Nullam ac arcu id orci porta sodales a at leo. Nulla pulvinar nisi vulputate nisl iaculis lobortis. Aenean eget mi vitae dolor venenatis vestibulum non sit amet tortor. Proin faucibus lobortis leo sed volutpat. Vivamus accumsan eros vitae congue euismod. Etiam dapibus dolor non volutpat gravida. Pellentesque ultricies nunc id volutpat lacinia. Etiam consectetur, turpis et gravida egestas, libero orci pellentesque ante, a euismod ante est quis nulla.");
}

void doPlusOuMoins(char argv, char **args)
{
	int nbInconnu = 756;
	int nb;
	char buffer[10];
	do
	{
		printf("Choisi un nombre : ");
		getLine(buffer, 10);
		
		nb = atoi(buffer);
		
		if (nb > nbInconnu)
			printf("C'est moins.\n");
		else if (nb < nbInconnu)
			printf("C'est plus.\n");
		else
			printf("Gagne.");
	}while(nb != nbInconnu);
}

void doDrawRect(char argv, char **args)
{
	if (argv < 8)
	{
		printf("Il n'y a pas assez d'argument.\nLa commande doit etre de la forme drawRect x y width height red green blue");
		return;
	}
	unsigned int x = atoi(args[1]);
	unsigned int y = atoi(args[2]);
	unsigned int width = atoi(args[3]);
	unsigned int height = atoi(args[4]);
	unsigned int red = atoi(args[5]);
	unsigned int green = atoi(args[6]);
	unsigned int blue = atoi(args[7]);
	
	draw(0, 0, 0);
	drawRect(x, y, width, height, red, green, blue);
	
	char buffer[2];
	getLine(buffer, 2);
	updateScreen();
}

void doDrawText(char argv, char **args)
{
	if (argv < 5)
	{
		printf("Il n'y a pas assez d'argument.\nLa commande doit etre de la forme drawText x y red green blue");
		return;
	}
	unsigned int x = atoi(args[1]);
	unsigned int y = atoi(args[2]);
	unsigned int red = atoi(args[3]);
	unsigned int green = atoi(args[4]);
	unsigned int blue = atoi(args[5]);
	
	draw(0, 0, 0);
	
	drawString("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Morbi sit amet orci metus. Nam dictum efficitur lectus, sed aliquam dui gravida a. Mauris vel hendrerit metus. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vestibulum vulputate mattis tortor, at finibus mauris finibus ornare. Etiam vel congue magna. Morbi arcu urna, egestas eget erat at, suscipit pretium felis. Sed sed vestibulum purus. Fusce pulvinar luctus congue. Donec luctus dui nec elementum efficitur. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; Nullam at mi ligula. Vestibulum fermentum in nisl posuere tempor. Morbi maximus justo id nulla viverra ultricies vitae in leo. Phasellus tempus, lacus ac lacinia aliquam, dui tortor volutpat tortor, sit amet viverra purus mauris sit amet sem. Nunc at enim ut sapien tristique blandit sed et mi. \n Morbi turpis leo, auctor eget ullamcorper quis, rhoncus eget dui. Donec interdum mollis lacus vel dapibus. Vestibulum egestas tempor velit, sit amet rhoncus ligula tincidunt ut. Suspendisse justo mi, volutpat et dapibus et, lobortis in urna. Mauris tincidunt nulla ut augue bibendum, nec luctus elit posuere. Sed rhoncus sodales congue. Phasellus a libero nulla. Sed varius neque eu neque tempus, non ullamcorper lacus eleifend. \n Aenean auctor hendrerit dictum. Maecenas dignissim sapien non neque fringilla porttitor eu a metus. Nam fringilla scelerisque lectus, vitae placerat sapien ullamcorper nec. Quisque pretium nec sem a pretium. Vivamus vehicula ultricies ante eget vestibulum. Nullam ac arcu id orci porta sodales a at leo. Nulla pulvinar nisi vulputate nisl iaculis lobortis. Aenean eget mi vitae dolor venenatis vestibulum non sit amet tortor. Proin faucibus lobortis leo sed volutpat. Vivamus accumsan eros vitae congue euismod. Etiam dapibus dolor non volutpat gravida. Pellentesque ultricies nunc id volutpat lacinia. Etiam consectetur, turpis et gravida egestas, libero orci pellentesque ante, a euismod ante est quis nulla.\n", 
				x, y, red, green, blue);

	char buffer[2];
	getLine(buffer, 2);
	updateScreen();
}
