#ifndef SCHED_H
#define SCHED_H

#include <stdint.h>
#include "syscall.h"
#include "pcb.h"
#include "hw.h"
#include "asm_tools.h"

typedef int (func_t)(void);

typedef enum scheduler
{
	PRIORITY
} scheduler;


// ********************** PUBLIC **********************
// Créé un processus de priorité donné avec comme point
// d'entrée la fonction donnée 
pcb_s *create_process(func_t* entry, int priority);

// Modifie l'ordonnanceur utilisé
void setScheduler(scheduler s);

//Initialise l'ordonnanceur
void sched_init(scheduler s);

//Demande de mettre en pause le processus courant
void sys_wait(uint32_t miliseconds);

// ********************** PRIVEE **********************
//Appelé automatiquement à la fin d'un processus
void do_sys_exit();

//Met en pause le processus courant
void do_sys_wait();

//Inutile (utile pour l'ordonnanceur préemptif)
void do_sys_yieldto(pcb_s *ptr);

//Utile
void do_sys_yield();

//Handler pour les interruptions du timer
void __attribute__((naked)) irq_handler(void);

//Appelé automatiquement à la fin d'un processus
void sys_exit();

//Demande un changement de processus
void sys_yield();	

//Inutile (utile pour l'ordonnanceur préemptif)
void sys_yieldto(struct pcb_s* dest);	

#endif
