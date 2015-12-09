#include "sched.h"
#include "kheap.h"

#define STACK_SIZE 10240

extern uint32_t * g_spArg;

//Globals au programme
pcb_s *g_current_process;
pcb_s g_kmain_process;

//Globals au fichier
static scheduler g_current_scheduler;
static uint64_t g_dateForWaitingProcess;
static pcb_s *g_ready_processes;
static pcb_s *g_waiting_processes;

// **************************************** définition fonctions locales

//Choisi le prochain processus à utiliser suivant l'ordonnanceur actuel
static pcb_s *elect();

//Choisi le prochain processus à utiliser avec l'ordonnanceur à priorité fixe
static pcb_s *electBestPriority();

//Ajoute à la bonne place de la liste donnée l'élement donné.
//Si usePriority == 1 alors la fonction utilise l'argument priority de la 
//pcb, sinon, il utilise l'argument wakingTime
static void insertInList(pcb_s ** list, pcb_s *e, uint8_t usePriority);

//Démarre le processus en cours
static void start_current_process();

//Suprime de la liste donnée l'élement donné.
static void removeFromList(pcb_s ** list, pcb_s *e);

// ********************************************************************
pcb_s *create_process(func_t* entry, int priority)
{
	pcb_s * newPcb = (pcb_s*) kAlloc(sizeof(pcb_s));
	newPcb->lr_user = (int32_t)entry;
	newPcb->stack = (uint32_t *)kAlloc(STACK_SIZE);
	newPcb->sp = newPcb->stack + STACK_SIZE/4;
	newPcb->CPSR_user = 0x60000150;
	newPcb->priority = priority;
	newPcb->lr_svc = (int32_t)&start_current_process;
	newPcb->state = READY;
	newPcb->wakingTime = 0;
	
	insertInList(&g_ready_processes, newPcb, 1);

	return newPcb;//Utile que pour les tests du chapitre 5 du prof	
}

void do_sys_exit()
{
	pcb_s *processToDelete = g_current_process;
	g_current_process->state = TERMINATED;
	g_spArg[2] = (uint32_t)elect();

	do_sys_yieldto();

	int terminateKernel = 0;
	if (g_ready_processes == NULL)//Si on supprime le dernier processus...
		terminateKernel = 1;
		
	processToDelete->next_process->prev_process = processToDelete->prev_process;
	processToDelete->prev_process->next_process = processToDelete->next_process;

	if (processToDelete != &g_kmain_process)
	{
		kFree((void *)processToDelete, STACK_SIZE);
		kFree((void *)processToDelete, sizeof(pcb_s));
	}

	if (terminateKernel)
		terminate_kernel();
}

void do_sys_wait()
{
	uint32_t milisec = g_spArg[2];
	
	//Ajout dans la liste des processus en attente
	g_current_process->wakingTime = g_dateForWaitingProcess + milisec;
	g_current_process->state = WAITING;
	insertInList(&g_waiting_processes, g_current_process, 0);
}

void do_sys_yieldto()
{
	pcb_s * dest = (pcb_s*)g_spArg[2];
	
	if (dest == g_current_process)
		return;
	
	//Récupération du lr et du sp du processus en cours
	__asm("cps 0x1F");
    __asm("mov %0, lr " : "=r"(g_current_process->lr_user));
	__asm("mov %0, sp " : "=r"(g_current_process->sp));
	__asm("cps 0x13");

	for(int i = 0 ; i < 13 ; i ++)
	{
		g_current_process->r[i] = g_spArg[i + 1];// Sauvgarde de l'ancien contexte
		g_spArg[i + 1] = dest->r[i];//recuperation du prochain contexte
	}
	
	if (g_current_process->state == RUNNING)
	{
		g_current_process->state = READY;
		insertInList(&g_ready_processes, g_current_process, 1);
	}
	dest->state = RUNNING; 
	removeFromList(&g_ready_processes, dest);
	
	g_current_process->CPSR_user = g_spArg[0];
	g_spArg[0] = dest->CPSR_user;
	
	g_current_process->lr_svc = g_spArg[14];
	g_spArg[14] = dest->lr_svc;
	
	g_current_process = dest;
	
	//Restauration du lr et du sp du nouveau processus
	__asm("cps 0x1F"); 
    __asm("mov lr, %0" : : "r"(g_current_process->lr_user));
    __asm("mov sp, %0" : : "r"(g_current_process->sp));
    __asm("cps 0x13");
}

pcb_s *elect()
{
	switch (g_current_scheduler)
	{
		case PRIORITY :
			return electBestPriority();
		default :
			return electBestPriority();
	}
}

pcb_s *electBestPriority()
{
	if (g_ready_processes == NULL || 
		(g_current_process->state == RUNNING && g_current_process->priority > g_ready_processes->priority))
		return g_current_process;
	else
		return g_ready_processes;	
}

static void insertInList(pcb_s ** list, pcb_s *e, uint8_t usePriority)
{
	//Si la liste est vide
	if (*list == NULL)
	{
		*list = e;
		e->next_process = e;
		e->prev_process = e;
		return;
	}
	pcb_s * tmp = NULL;
	
	//Si le premier processus est inférieur au processus à ajouter
	if ((usePriority && (*list)->priority < e->priority) ||
		(!usePriority && (*list)->wakingTime < e->wakingTime))
	{
		tmp = *list;
		*list = e;
	}
	else //Sinon, on trouve la place où l'inserer
	{	
		tmp = *list;
		do
		{    
			if ((usePriority && tmp->priority >= e->priority) ||
				(!usePriority && tmp->wakingTime >= e->wakingTime))
				tmp = tmp->next_process;
			else 
				break;
		}while(tmp != *list);  
		
		
	}
		
	tmp->prev_process->next_process = e;
		
	e->next_process = tmp;
	e->prev_process = tmp->prev_process;
	
	tmp->prev_process = e;
}

void __attribute__((naked)) irq_handler(void)
{
	__asm("STMFD sp!, {r0-r12, lr}");
    __asm("MRS r4, spsr");
    __asm("STMFD sp!, {r4}");
    
    
	//On vérifie si on doit réveiller des processus
	g_dateForWaitingProcess += 10;
	while (g_waiting_processes != NULL && g_waiting_processes->wakingTime <= g_dateForWaitingProcess)
	{
		g_waiting_processes->state = READY;
		g_waiting_processes->wakingTime = 0;
		pcb_s *tmp = g_waiting_processes;
		removeFromList(&g_waiting_processes, tmp);
		insertInList(&g_ready_processes, tmp, 1);
	}

	//selection du prochain processus
	__asm("mov %0, sp" : "=r"(g_spArg));
	g_spArg[2] = (uint32_t) elect();
	
	//Changement de contexte
	__asm("cps 0x13");
	do_sys_yieldto();
	__asm("cps 0x12");
	
	//reearmement du timer
	set_next_tick_default();
	ENABLE_TIMER_IRQ();

	__asm("LDMFD sp!, {r4}");
    __asm("MSR spsr, r4");
    __asm("LDMFD sp!, {r0-r12, pc}^");	
}

static void removeFromList(pcb_s ** list, pcb_s *e)
{
	if (e == *list)//Si jamais c'est le permier élément
	{
		if (e->next_process == e)//Si la liste n'en contient qu'un
			*list = NULL;//La liste devient nulle
		else
			*list = e->next_process;
	}
	
	if (e->next_process != NULL && e->prev_process != NULL)
	{
		e->next_process->prev_process = e->prev_process;
		e->prev_process->next_process = e->next_process;
	}
	
	e->next_process = NULL;
	e->prev_process = NULL;
	
}

void setScheduler(scheduler s)
{
	g_current_scheduler = s;
}

void sched_init(scheduler s)
{
	kheap_init();	
	setScheduler(s);
	g_kmain_process.CPSR_user = 0x60000150;
	g_kmain_process.priority = 0;
	g_kmain_process.state = RUNNING;

	g_current_process = &g_kmain_process;
	g_current_process->next_process = NULL;
	g_current_process->prev_process = NULL;
	g_ready_processes = NULL;
	g_waiting_processes = NULL;
	
	g_dateForWaitingProcess = 0;
}

void start_current_process()
{
	((func_t *)g_current_process->lr_user)();
	sys_exit();
}

void sys_exit()
{
	__asm("mov r0, %0" : : "r"(SYS_EXIT): "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11");
	__asm("SWI #0");
}

void sys_wait(uint32_t miliseconds)
{
	__asm("mov r1, %0" : : "r"(miliseconds): "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11");
	__asm("mov r0, %0" : : "r"(WAIT): "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11");
	__asm("SWI #0");
	
	sys_yield();
}

void sys_yield()
{
	sys_yieldto(elect());
}

void sys_yieldto(pcb_s * dest)
{	
	__asm("mov r1, %0" : : "r"(dest): "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11");
	__asm("mov r0, %0" : : "r"(YIELDTO): "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11");
	__asm("SWI #0");
}
