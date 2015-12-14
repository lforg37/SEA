#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "stdint.h"

#include "keyboard.h"
#include "usbd.h"

/*
* The address of the keyboard we're reading from.
* C++ Signautre: u32 KeyboardAddress;
*/

/*
* The scan codes that were down before the current set on the keyboard.
* C++ Signautre: u16* KeyboardOldDown;
*/

/*
* KeysNoShift contains the ascii representations of the first 104 scan codes
* when the shift key is up. Special keys are ignored.
* C++ Signature: char* KeysNoShift;
*/

/*
* KeysShift contains the ascii representations of the first 104 scan codes
* when the shift key is held. Special keys are ignored.
* C++ Signature: char* KeysShift;
*/

/*
* Updates the keyboard pressed and released data.
* C++ Signature: void KeyboardUpdate();
*/
void KeyboardUpdate2();

/*
* Returns r0=0 if a in r1 key was not pressed before the current scan, and r0
* not 0 otherwise.
* C++ Signature bool KeyWasDown(u16 scanCode)
*/
int KeyWasDown2(uint16_t scanCode);

/*
* Returns the ascii character last typed on the keyboard, with r0=0 if no 
* character was typed.
* C++ Signature char KeyboardGetChar()
*/
char KeyboardGetChar2();

/*
void UsbCheckForChange();
uint16_t KeyboardGetAddress(uint16_t);
uint16_t KeyboardGetKeyIsDown(uint32_t, uint16_t);
uint16_t KeyboardCount();
void KeyboardBufferInitialize();
uint16_t KeyboardPoll(uint16_t);
uint16_t KeyboardGetKeyDownCount(uint16_t);
uint16_t KeyboardGetKeyDown(uint16_t, uint16_t);
uint16_t KeyboardGetModifiers(uint16_t);*/

#endif
