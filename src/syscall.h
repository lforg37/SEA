#ifndef SYS_CALL_H
#define SYS_CALL_H

#include <stdint.h>

enum action
{
	DEFAULT, REBOOT, NOP, SETTIME, GETTIME, YIELDTO, SYS_EXIT, WAIT
};

//Handler pour les changements de mode 
void __attribute__((naked)) swi_handler(void);

//Retourne l'heure du système (Retourne une constante avec l'émulateur)
uint64_t sys_gettime();

//Inutile
void sys_nop();

//Redémarre le RPI
void sys_reboot();

//Modifie l'heure du système (ne fonctionne pas avec l'émulateur)
void sys_settime(uint64_t date);
#endif
