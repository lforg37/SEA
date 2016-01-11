#include <stdbool.h>
#include "vmem.h"
#include "util.h"
#include "kheap.h"
#include "sched.h"

#define DEV_ADDR_FLAG 0x20000000
#define IO_START 0x20000000
#define idx_first_ram_page 0x200
#define idx_last_ram_page 0x3FFF
#define NB_FRAME_STACK 10
#define STACK_START 0xFFFFFFFF


uint32_t mmu_base;
static void init_frame_table();
static uint8_t* frame_table;
static uint16_t idx_last_frame_allocated;
static void map_table(pcb_s* process, uint32_t* frame_addr, uint32_t process_addr);

static page_list* insert_list(page_list* list, uint8_t* adress,
        uint8_t nb_pages, bool merge);
static uint8_t* get_contiguous_addr(pcb_s* process, size_t size);
static void free_addr(uint8_t* vAddress, pcb_s* process);
static uint32_t* get_free_frame();

#ifdef verifINIT
static uint32_t vmem_translate(uint32_t va, pcb_s* process);
static void verify_init();
static void verify_process_table(pcb_s *pcb);
#endif

static bool is_init_addr(uint32_t addr)
{
	return (addr <= (uint32_t) &__kernel_heap_end__)|| ((addr & 0xFF000000) == DEV_ADDR_FLAG );
}

void data_handler()
{
	uint32_t DFSR;
	__asm__ volatile("MCR p15, 0, r3, c5, c0, 0");
	__asm__ volatile("mov %[dfsr], r3" : [dfsr] "=r"(DFSR));

	uint32_t FAR; // adresse virtuelle responsable de la faute
	__asm__ volatile("MCR p15, 0, r3, c6, c0, 0");
	__asm__ volatile("mov %[far], r3" : [far] "=r"(FAR));

	kernel_panic("DATA ABORT", FAR);
}

void clean_stack(pcb_s *pcb)
{
	uint32_t addr = STACK_START - NB_FRAME_STACK * PAGE_SIZE + 1;
	vmem_free((uint8_t*) addr, pcb, NB_FRAME_STACK);
}

static void start_mmu_c(void)
{
	register unsigned int control;
	/*Desactivation du cache*/
	__asm__("mcr p15, 0, %[zero], c1, c0, 0" :: [zero]"r"(0));

	/*Invalidation du cache*/
	__asm__("mcr p15, 0, r0, c7, c7, 0");

	/*Invalidation de la TLB*/
	__asm__("mcr p15, 0, r0, c8, c7, 0");

	/*Activation des fonctionnalités de la MMU ARMv6*/
	control = (1 << 23) | (1 << 15) | (1 << 4) | 1;

	/*Invalidation du buffer de la TLB*/
	__asm__ volatile("mcr p15, 0, %[data], c8, c7, 0" :: [data]"r"(0));

	/*Écriture du registre de contrôle*/
	__asm__ volatile("mcr p15, 0, %[control], c1, c0, 0" :: [control]"r"(control));
}

void init_pcb_table(pcb_s *pcb)
{
	uint32_t* pcb_table = (uint32_t*) kAlloc_aligned(FIRST_LVL_TT_SIZE, FIRST_LVL_INDEX_WIDTH);
	size_t nb_pages_code = (((uint32_t)&__kernel_heap_start__) - 1) / PAGE_SIZE + 1;	
	pcb->page_table_addr = pcb_table;	

	size_t cur_frame_idx;
	for (cur_frame_idx = 0 ; cur_frame_idx < nb_pages_code ; cur_frame_idx++) {
		map_table(pcb, (uint32_t*) (cur_frame_idx * PAGE_SIZE), cur_frame_idx * PAGE_SIZE);
	}

	uint32_t stack_limit = STACK_START - NB_FRAME_STACK * PAGE_SIZE + 1;
	uint32_t current_frame = stack_limit;
	
	size_t i ;
	for(i = 0 ; i < NB_FRAME_STACK ; i++)
	{
		uint32_t* free_frame_addr = get_free_frame();
		map_table(pcb, free_frame_addr, current_frame);
		current_frame+= PAGE_SIZE;
	}

	pcb->sp = (uint32_t*) STACK_START;

	page_element* free_list = (page_element*) kAlloc(sizeof(page_element));
	free_list->address = (uint8_t*) &__kernel_heap_start__;
	free_list->nb_pages = &__kernel_heap_end__ - &__kernel_heap_start__ + 1;
	free_list->next = NULL;

	pcb->free_list = free_list;
	pcb->occupied_list = NULL;

#ifdef verifINIT
	verify_process_table(pcb);
#endif
}

void handle_vmem(pcb_s* pcb)
{
	uint32_t addr = (uint32_t) pcb->page_table_addr;
	__asm__ volatile("mcr p15, 0, %[addr], c2, c0, 0" :: [addr]"r"(addr));
	__asm__ volatile("mcr p15, 0, %[addr], c2, c0, 1" :: [addr]"r"(addr));
	__asm__ volatile("MCR p15,0,R2,c8, c6,0");
}

static void configure_mmu_c(void)
{
	register unsigned int pt_addr = mmu_base;

	/*Table de traduction 0*/
	__asm__ volatile("mcr p15, 0, %[addr], c2, c0, 0" :: [addr]"r"(pt_addr));
	/*Table de traduction 1*/
	__asm__ volatile("mcr p15, 0, %[addr], c2, c0, 1" :: [addr]"r"(pt_addr));

	/*Ne pas utiliser la table 1*/
	__asm__ volatile("mcr p15, 0, %[n], c2, c0, 2" :: [n]"r"(0));

	/*Gestion des accès domaines simple*/
	__asm__ volatile ("mcr p15, 0, %[r], c3, c0, 0" :: [r]"r"(0x3));
}

void vmem_init(void)
{
	init_kern_translation_table();
	init_frame_table();
	configure_mmu_c();
#ifdef verifINIT
	verify_init();
#endif
	//Interruptions
	start_mmu_c();
}

static uint32_t* get_free_frame()
{
	size_t iter_idx;
	for (iter_idx = idx_last_frame_allocated ; 
			(frame_table[iter_idx] == 0xFF && iter_idx < idx_last_ram_page);
			iter_idx++
		);

	if(frame_table[iter_idx] ==  0xFF)
	{
		for (iter_idx = idx_first_ram_page; 
			(frame_table[iter_idx] == 0xFF && iter_idx < idx_last_frame_allocated -1);
			iter_idx++
		);

		if(frame_table[iter_idx] == 0xFF)
		{
			return NULL;
		}
	}

	uint8_t shift;
	uint8_t mask = frame_table[iter_idx];
	for (shift = 0 ; shift < 8 && (((mask >> shift) & 0x01) != 0) ; shift++);
	
	uint32_t address = ((iter_idx * 8) + shift) * PAGE_SIZE;
	return (uint32_t *) address;	
}

static void map_table(pcb_s* process, uint32_t* frame_addr, uint32_t process_addr)
{
	uint32_t shift = process_addr >> FIRST_LVL_BITSHIFT;
	uint32_t* addr_ptr = process->page_table_addr + shift;

	uint32_t* second_lvl_tt;
	if((*addr_ptr & 0x00000003) == 0)
	{
		second_lvl_tt = (uint32_t*) kAlloc_aligned(SECON_LVL_TT_SIZE,
				SECON_LVL_INDEX_WIDTH);

		*addr_ptr = ((uint32_t)second_lvl_tt & 0xFFFFFC00) + 1;
	} else {
		second_lvl_tt = (uint32_t*) (*addr_ptr & 0xFFFFFC00);
	}
	
	shift = ((process_addr << 12) >> 24);
	uint32_t* frame_entry = (uint32_t*)(second_lvl_tt) + shift;

	*frame_entry = ((uint32_t) frame_addr & 0xFFFFF000 ) + 0x0000044E;
}

uint8_t* vmem_alloc_for_userland(pcb_s* process, size_t size)
{
	size_t nb_pages = size / (PAGE_SIZE * 4);
	if (nb_pages % (4 * PAGE_SIZE) > 0) {
		nb_pages++;
	}
	
	uint8_t* page_start = get_contiguous_addr(process, nb_pages);
	uint32_t page_addr = (uint32_t) page_start;
	size_t i;
	uint32_t *frame;
	for(i = 0 ; i < nb_pages ; i++)
	{
		frame = get_free_frame();
		map_table(process, frame, page_addr);	
		page_addr += PAGE_SIZE;
	}

	return page_start;
}

void vmem_free(uint8_t* vAddress, pcb_s* process, size_t size) 
{
	uint32_t* addr_ptr = (uint32_t*)( process->page_table_addr + 
		((uint32_t) vAddress)/(PAGE_SIZE * SECON_LVL_TT_COUN));
	
	uint32_t* second_lvl_tt = (uint32_t*) (*addr_ptr & 0xFFFFFC00);

	uint32_t* frame_entry = (uint32_t*) ((uint32_t) second_lvl_tt + 
		(
		 ((uint32_t) vAddress >> 10) & 0x000003FF
		));
	
	uint32_t frame_addr = *frame_entry & 0xFFFFF000;
	//Effacement de l'entrée
	*frame_entry = 0;
	
	uint32_t* second_lvl_iter;
	size_t non_empty = 0;
	for( 	second_lvl_iter = second_lvl_tt ; 
			second_lvl_iter < second_lvl_tt + SECON_LVL_TT_COUN ;
			second_lvl_iter++) {
		if (*second_lvl_iter != 0) {
			non_empty++;
			break;
		}
	}
	
	if (non_empty == 0) {
		//On supprime aussi la table
		*addr_ptr = 0;
		kFree((uint8_t*) second_lvl_tt, SECON_LVL_TT_SIZE);
	} 

	uint32_t frame_idx = frame_addr / PAGE_SIZE;	
	frame_table[frame_idx / 8] &= 0xFF - (1 << (frame_idx % 8 ));
	
	free_addr(vAddress, process);
}

void free_all(pcb_s* process)
{
	page_element* current = process->occupied_list;
	
	while(current != NULL) {
		process->free_list = insert_list(process->free_list,
			current->address, current->nb_pages, true);
		kFree((uint8_t*) current, sizeof(page_element));
		
		current = current->next;
	}
	
	process->occupied_list = NULL;
}

void free_addr(uint8_t* vAddress, pcb_s* process)
{
	page_list* occupied_list = process->occupied_list;
	page_list* free_list = process->free_list;
	page_element* current_element = occupied_list;
	page_element* previous_element = NULL;
	
	while(current_element != NULL)
	{
		if(current_element->address == vAddress) {
			
			process->free_list = insert_list(free_list, vAddress,
				current_element->nb_pages, true);
			
			if(previous_element == NULL) {
				process->occupied_list = current_element->next;
			}
			else {
				previous_element->next = current_element->next;
			}
			
			return;
		}
		
		previous_element = current_element;
		current_element = current_element->next;
	}
	
	if(occupied_list == NULL) {
		// la liste est vide, rien d'alloué
		return;
	}
	
}

// size_t size : taille en nombre de page
uint8_t* get_contiguous_addr(pcb_s* process, size_t size)
{
	page_element* free_list = process->free_list;
	page_element* occupied_list = process->occupied_list;
	page_element* current_element;
	page_element* previous_element = NULL;

	current_element = free_list;

	do {
		if (current_element->nb_pages >= size) {
			// assez de pages consécutives

			occupied_list = insert_list(occupied_list, current_element->address,
					size, false);
					
			if (current_element->nb_pages != size) {
				
				// on rabote l'élément	   
				current_element->nb_pages -= size;
				
				current_element->address = (uint8_t*)
					((size * PAGE_SIZE)
					+ (size_t) current_element->address);

			} else if (previous_element != NULL) {
				
				previous_element->next = current_element->next;
				
			} else if (previous_element == NULL) {
				process->free_list = NULL;
				
				return current_element->address;
			}
			process->occupied_list = occupied_list;
			process->free_list = free_list;

			return current_element->address;
		}

		previous_element = current_element;
		current_element = current_element->next;

	} while(current_element != NULL);

	return NULL; // code d'erreur à définir
}

void switch_os()
{
	__asm__ volatile("mcr p15, 0, %[addr], c2, c0, 0" :: [addr]"r"(mmu_base));
	__asm__ volatile("MCR p15,0,R4,c8, c6,0");
}

// bool merge : vaut vrai lorsqu'on insère dans la free_list
page_list* insert_list(page_list* list, uint8_t* address,
		uint8_t nb_pages, bool merge)
{
	page_element* current = list;
	page_element* previous = NULL;

	bool previous_merge;

	while(current != NULL) {
		previous_merge = false;

		if(address < current->address) {

			if(merge) {
				
				uint8_t* next_address;

				// fusion avec le bloc d'avant ?
				if(previous != NULL) {
					next_address =
						(uint8_t*)((size_t) previous->address +
						(PAGE_SIZE * previous->nb_pages));

					if(next_address == address) {
						previous->nb_pages += nb_pages;
						previous_merge = true;
					}
				}

				//fusion avec le bloc d'après ?
				next_address =
					(uint8_t*)((size_t) address + (PAGE_SIZE * nb_pages));

				if(next_address == current->address) {
					if(!previous_merge) {
						// fusion avec le bloc d'après uniquement
						current->address = address;
						current->nb_pages += nb_pages;
					}
					else {
						// fusion avec le bloc d'après et d'avant
						previous->next = current->next;
						previous->nb_pages += current->nb_pages;

						kFree((uint8_t*) current, sizeof(page_element));
					}

					return list;
				} else if(previous_merge) {
					return list;
				}
			}

			page_element* new_element =
					(page_element*) kAlloc(sizeof(page_element));
			
			if(previous != NULL) {
				previous->next = new_element;
			} else {
				list = new_element;
			}
			
			new_element->next = current;
			new_element->address = address;
			new_element->nb_pages = nb_pages;

			return list;
		}

		previous = current;
		current = current->next;
	}
	
	page_element* last_element = (page_element*) kAlloc(sizeof(page_element));
		
	last_element->next = NULL;
	last_element->address = address;
	last_element->nb_pages = nb_pages;
	
	if(list == NULL) {
		// la liste est vide
		list = last_element;
	}
	else {
		previous->next = last_element;
	}

	return list;
}

uint32_t init_kern_translation_table(void)
{
	kheap_init();
	uint8_t* base_table = kAlloc_aligned(FIRST_LVL_TT_SIZE, FIRST_LVL_INDEX_WIDTH);
	mmu_base = (uint32_t) base_table;
	uint32_t device_flags = 0x437; 
	uint32_t normal_flags = 0x44E;
	uint32_t coarse_page_flag = 1;
	
	//size_t j;
	uint32_t* second_lvl_tt;
	uint32_t virtual_addr = 0;
	uint32_t* second_lvl_tt_entry;
	uint32_t* first_lvl_tt = (uint32_t*) base_table;

	uint32_t flag = 0;
	uint32_t virtual_addr_lvl_2;

	for (virtual_addr = 0 ;
			virtual_addr <  FIRST_LVL_START(0xFFFFFFFF);
			virtual_addr += SECON_LVL_TT_COUN * PAGE_SIZE
		) {
		if (virtual_addr <= FIRST_LVL_START((uint32_t) (&__kernel_heap_end__))) {
			flag = coarse_page_flag;
			second_lvl_tt = (uint32_t*) kAlloc_aligned(
					SECON_LVL_TT_SIZE,
					SECON_LVL_INDEX_WIDTH );
			second_lvl_tt_entry = second_lvl_tt;
			for (virtual_addr_lvl_2 = virtual_addr ; 
					virtual_addr_lvl_2 < virtual_addr + SECON_LVL_TT_COUN * PAGE_SIZE ;
					virtual_addr_lvl_2 += PAGE_SIZE
				)
			{
				if(is_init_addr(virtual_addr_lvl_2)) {
					*second_lvl_tt_entry = (virtual_addr_lvl_2 & 0xFFFFF000)+ normal_flags;	
				} else {
					*second_lvl_tt_entry = 0;	
				}
				second_lvl_tt_entry++;
			}
		} else if (
				virtual_addr >= 0x20000000 && 
				virtual_addr <= FIRST_LVL_START(0x20FFFFFF)
			) {
			flag = coarse_page_flag;	
			second_lvl_tt = (uint32_t*) kAlloc_aligned(
					SECON_LVL_TT_SIZE,
					SECON_LVL_INDEX_WIDTH );
			second_lvl_tt_entry = second_lvl_tt;
			for (virtual_addr_lvl_2 = virtual_addr ; 
					virtual_addr_lvl_2 < virtual_addr + SECON_LVL_TT_COUN * PAGE_SIZE ;
					virtual_addr_lvl_2 += PAGE_SIZE
				)
			{
				if(is_init_addr(virtual_addr_lvl_2)) {
					*second_lvl_tt_entry = (virtual_addr_lvl_2 & 0xFFFFF000) + device_flags;	
				} else {
					*second_lvl_tt_entry = 0;	
				}
				second_lvl_tt_entry++;
			}
		} else {
			second_lvl_tt = 0;
			flag = 0;
		}

		*first_lvl_tt = ((uint32_t)second_lvl_tt & 0xFFFFFC00) + flag;
		first_lvl_tt++;
	}

	return 0;
}

static void init_frame_table()
{
	size_t nb_frames = IO_START / (PAGE_SIZE * 8);
	frame_table = (uint8_t*) kAlloc(nb_frames/8);
	uint8_t* frame_table_ptr = frame_table;
	size_t i;
	for(i = 0 ; i < nb_frames ; i++)
	{
		if (i < 0x200) {
			*frame_table_ptr++ = 0xFF;
		} else {
			*frame_table_ptr++ =  0;
		}
	}
	
	idx_last_frame_allocated = 0x200;
}

#ifdef verifINIT
static uint32_t vmem_translate(uint32_t va, struct pcb_s* process)
{
	uint32_t pa; /* The result */

	/* 1st and 2nd table addresses */
	uint32_t table_base;
	uint32_t second_level_table;

	/* Indexes */
	uint32_t first_level_index;
	uint32_t second_level_index;
	uint32_t page_index;

	/* Descriptors */
	uint32_t first_level_descriptor;
	uint32_t* first_level_descriptor_address;
	uint32_t second_level_descriptor;
	uint32_t* second_level_descriptor_address;

	if(process == NULL)
	{
		__asm("mrc p15, 0, %[tb], c2, c0, 0" : [tb] "=r"(table_base));
	}
	else
	{
		table_base = (uint32_t) process->page_table_addr;
	}
	table_base = table_base & 0xFFFFC000;

	/* Indexes*/
	first_level_index = (va >> 20);
	second_level_index = ((va << 12) >> 24);
	page_index = (va & 0x00000FFF);

	/* First level descriptor */
	first_level_descriptor_address = (uint32_t*) (table_base | (first_level_index << 2));
	first_level_descriptor = *(first_level_descriptor_address);

	/* Translation fault*/
	if (! (first_level_descriptor & 0x3)) {
		return (uint32_t) FORBIDDEN_ADDRESS;
	}

	/* Second level descriptor */
	second_level_table = first_level_descriptor & 0xFFFFFC00;
	second_level_descriptor_address = (uint32_t*) (second_level_table | (second_level_index << 2));
	second_level_descriptor = *((uint32_t*) second_level_descriptor_address);

	/* Translation fault*/
	if (! (second_level_descriptor & 0x3)) {
		return (uint32_t) FORBIDDEN_ADDRESS;
	}

	/* Physical address */
	pa = (second_level_descriptor & 0xFFFFF000) | page_index;
	return pa;
}



static void verify_init() 
{
	uint32_t adresse;	
	for(adresse = 0 ; adresse < 0xFFFFFF00; adresse+= (1 << 7))
	{
		if(is_init_addr(adresse))
		{
			if (vmem_translate(adresse, NULL) != adresse)
				kernel_panic("verify_init allowed", adresse);
		
		} else {
			if (vmem_translate(adresse, NULL) != (uint32_t) FORBIDDEN_ADDRESS)
				kernel_panic("verify_init forbid", adresse);
		}
	}

	if(vmem_translate(0, NULL) != 0)
		PANIC();
	
}

static void verify_process_table(pcb_s* pcb)
{
	uint32_t adresse;	
	for(adresse = 0 ; adresse < (uint32_t)(&__kernel_heap_start__ - 1); adresse+= 1)
	{
			if (vmem_translate(adresse, pcb) != adresse)
				kernel_panic("verify_init allowed", adresse);
	}

	size_t i;
	adresse = STACK_START - NB_FRAME_STACK * PAGE_SIZE + 1 ;
	for ( i = 0 ; 
		  i < NB_FRAME_STACK ;	
		  i++ ) {
			if (vmem_translate(adresse, pcb) == (uint32_t) FORBIDDEN_ADDRESS)
				kernel_panic("verify_init allowed", adresse);
			adresse += PAGE_SIZE;
	}

	//kernel_panic("OK !", 42);
}
#endif
