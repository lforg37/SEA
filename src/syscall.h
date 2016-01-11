#ifndef SYS_CALL_H
#define SYS_CALL_H

#include <stdint.h>
#include "types.h"

enum action
{
	DEFAULT, 
	REBOOT, 
	NOP, 
	SETTIME, 
	GETTIME, 
	YIELDTO, 
	SYS_EXIT, 
	WAIT,
	MMAP,
	MUMAP,
	GMALLOC,
	GFREE
};

//Handler pour les changements de mode 
void __attribute__((naked)) swi_handler(void);

//Retourne l'heure du système (Retourne une constante avec l'émulateur)
uint64_t sys_gettime();

//Récupérer un pointeur sur une zone de size octets contigus;
void* sys_mmap(size_t size); 

//Détacher size nombre de page à partir de l'adresse addresse
void sys_munmap(void* addr, size_t size);

// Alloue size octets dans le tas du processus courant
void* gmalloc(size_t size);

// Libère la zone mémoire du tas associée à l'adresse ptr
void gfree(void* ptr);

//Inutile
void sys_nop();

//Redémarre le RPI
void sys_reboot();

//Modifie l'heure du système (ne fonctionne pas avec l'émulateur)
void sys_settime(uint64_t date);
#endif
