#include "sched.h"
#include "kheap.h"
#include "hw.h"

#define STACK_SIZE 10240

extern uint32_t * g_spArg;

pcb_s *g_current_process;
pcb_s g_kmain_process;

static pcb_s *elect();

void start_current_process()
{
	((func_t *)g_current_process->lr_user)();
	sys_exit();
}

pcb_s * create_process(func_t* entry)
{
	pcb_s * newPcb = (pcb_s*) kAlloc(sizeof(pcb_s));
	newPcb->lr_user = (int32_t)entry;
	newPcb->stack = (uint32_t *)kAlloc(STACK_SIZE);
	newPcb->sp = newPcb->stack + STACK_SIZE/4;
	newPcb->CPSR_user = 0x200001d0;
	newPcb->lr_svc = (int32_t)&start_current_process;
	
	pcb_s * tmp = g_current_process->next_process;
	g_current_process->next_process = newPcb;
	newPcb->prev_process = g_current_process;
	newPcb->next_process = tmp;
	tmp->prev_process = newPcb;

	return newPcb;//Utile que pour les tests du chapitre 5 du prof	
}

pcb_s *elect()
{
	return g_current_process->next_process;
}

void sys_yield()
{
	sys_yieldto(elect());
}

void sys_exit()
{
	__asm("mov r0, %0" : : "r"(SYS_EXIT): "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11");
	__asm("SWI #0");
}

void sys_yieldto(pcb_s * dest)
{	
	__asm("mov r1, %0" : : "r"(dest): "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11");
	__asm("mov r0, %0" : : "r"(YIELDTO): "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11");
	__asm("SWI #0");
}

void do_sys_yieldto()
{
	pcb_s * dest = (pcb_s*)g_spArg[2];
	
	__asm("cps 0x1F");
    __asm("mov %0, lr " : "=r"(g_current_process->lr_user));
	__asm("mov %0, sp " : "=r"(g_current_process->sp));
	__asm("cps 0x13");

	for(int i = 0 ; i < 13 ; i ++)
	{
		g_current_process->r[i] = g_spArg[i + 1];// Sauvgarde de l'ancien contexte
		g_spArg[i + 1] = dest->r[i];//recuperation du prochain contexte
	}

	g_current_process->CPSR_user = g_spArg[0];
	g_spArg[0] = dest->CPSR_user;
	
	g_current_process->lr_svc = g_spArg[14];
	g_spArg[14] = dest->lr_svc;
	//g_spArg[14] = (dest->lr_svc == 0x0) ?  dest->lr_user : dest->lr_svc;
	g_current_process = dest;
	
	__asm("cps 0x1F"); 
    __asm("mov lr, %0" : : "r"(g_current_process->lr_user));
    __asm("mov sp, %0" : : "r"(g_current_process->sp));
    __asm("cps 0x13");
}

void do_sys_exit()
{
	pcb_s *processToDelete = g_current_process;
	g_spArg[2] = (uint32_t)elect();

	do_sys_yieldto();

	int terminateKernel = 0;
    if (processToDelete->next_process->next_process == processToDelete)//Si on supprime le dernier processus...
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

void  sched_init()
{
	kheap_init();	
    g_kmain_process.CPSR_user = 0x200001d0;

	g_current_process = &g_kmain_process;
	g_current_process->next_process = g_current_process;
	g_current_process->prev_process = g_current_process;
}