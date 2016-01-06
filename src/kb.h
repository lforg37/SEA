#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "stdint.h"


void KeyboardLoop();

void getString(char *buffer, int tailleBuf, char delimiter);

void getLine(char *buffer, int tailleBuf);

void UsbInitialise();
char KeyboardGetChar();
void KeyboardUpdate();

#endif

