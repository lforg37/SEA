#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "stdint.h"


void KeyboardLoop();

void getString(char *buffer, char delimiter);

void getLine(char *buffer);

void UsbInitialise();
char KeyboardGetChar();
void KeyboardUpdate();

#endif

