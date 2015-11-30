#include "sched.h"
#include "kheap.h"
#include "syscall.h"

pcb_s* current_process;
pcb_s kmain_process;

void sched_init(void)
{
	kheap_init();
	kmain_process.next_task = kmain_process.prev_task = &kmain_process;
	current_process = &kmain_process;
}

void sys_yield(void)
{
	SYSCALL(YIELD);
}

pcb_s* create_process(func_t entry)
{
	pcb_s* retour = (pcb_s*) kAlloc(sizeof(pcb_s));
	retour->context[12] = (reg_t) entry;
	retour->sp = (reg_t)(kAlloc(STACK_SIZE)+STACK_SIZE);
	retour->cpsr = 0x190;

	retour->next_task = current_process->next_task;
	retour->next_task->prev_task = retour;
	retour->prev_task = current_process;
	current_process->next_task = retour;

	return retour;
}

static void elect(void)
{
	current_process = current_process->next_task;
}

void do_sys_yield(void *args, reg_t lrsvc)
{
	pcb_s *actuel = current_process;
	reg_t* courant = current_process->context;
	reg_t* pile =  args;
	reg_t* current_lr = &(current_process->lr_user);	

	elect();
	pcb_s *pcb = current_process;
	reg_t *suivant = pcb->context;
	

	/*Stockage des registres courants */
	__asm__("ldm %[fromwhere], {r6-r12}" : : 
				[fromwhere]"r"(pile)
		   );

	__asm__("stm %[where], {r6-r12}" : :
				[where]"r"(courant)
		   );

	__asm__("ldm %[fromwhere], {r5-r12}" : : 
				[fromwhere]"r"(pile + 6)
		   );

	__asm__("stm %[where], {r5-r12}" : :
				[where]"r"(courant+6)
		   );

	/*Gestion cpsr*/
	__asm__("mrs r6, SPSR" );

	__asm__("mov %[courantcpsr], r6" :
				[courantcpsr]"=r"(actuel->cpsr)
			);

	__asm__("mov r6, %[prochaincpsr]" : :
				[prochaincpsr]"r"(pcb->cpsr)
		   );

	__asm__("msr SPSR, r6");

	/*Gestion des lr et sp utilisateur*/
	__asm__("mov r6, %[lrnext]" : :
				[lrnext]"r"(pcb->lr_user)
		   );

	__asm__("mov r7, %[pcnext]" : :
				[pcnext]"r"(pcb->sp)
		   );


/******** Zone mode système **************************************/	
	__asm__("cps #0b11111");
	__asm__("mov r5, lr");
	__asm__("mov lr, r6");
	__asm__("mov r8, sp");
	__asm__("mov sp, r7");
	__asm__("cps #0b10011");
/****** Fin zone mode système ***********************************/
	
	__asm__("str r5, [%[where]]" : :
				[where]"r"(current_lr)
			);

	__asm__("str r8, [%[where]]" : :
				[where]"r"(&(actuel->sp))
		   );

	/*Chargement des registres du contexte à restaurer */
	__asm__("ldm %[fromwhere], {r6-r12}" : : 
				[fromwhere]"r"(suivant)
		   );

	/*Mise en pile des registres à restaurer*/
	__asm__("stm %[where], {r6-r12}" : :
				[where]"r"(pile)
		   );

	pile = pile + 6;

	/*Chargement des registres du contexte à restaurer */
	__asm__("ldm %[fromwhere], {r5-r12}" : : 
				[fromwhere]"r"(suivant + 6)
		   );

	/*Mise en pile des registres à restaurer*/
	__asm__("stm %[where], {r5-r12}" : :
				[where]"r"(pile)
		   );
	
	/* Mémorisation du contexte actuel */
	*(courant + 12) = lrsvc;
}
