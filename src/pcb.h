#ifndef PCB_H
#define PCB_H

#include <stdlib.h>
#include <stdint.h>

typedef enum process_state
{
	READY, RUNNING, TERMINATED, WAITING
} process_state;

typedef struct page_element
{
	uint8_t* address;
	size_t nb_pages;
	struct page_element* next;
} page_element;

typedef struct pcb_s
{
	uint32_t r[13];
	uint32_t lr_svc;
	uint32_t lr_user;
	uint32_t* stack;
	uint32_t* sp;
	uint32_t CPSR_user;
	uint32_t priority;
	process_state state;
	struct pcb_s* next_process;
	struct pcb_s* prev_process;	
	uint64_t wakingTime;

	page_element* free_list;
	page_element* occupied_list;
	uint8_t* page_table_addr;
} pcb_s;
#endif
