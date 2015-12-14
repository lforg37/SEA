#include <stdlib.h>
#include <stdbool.h>
#include "vmem.h"
#include "util.h"
#include "kheap.h"
#include "sched.h"

#define DEV_ADDR_FLAG 0x20000000
#define NB_FRAME 0x21000

static uint32_t mmu_base;
static void init_frame_table();
static uint8_t* frame_table;
static uint16_t idx_last_frame_allocated;
#ifdef verifINIT
static uint32_t vmem_translate(uint32_t va, struct pcb_s* process);
static void verify_init();
#endif

static bool is_init_addr(uint32_t addr)
{
	return (addr <= (uint32_t) &__kernel_heap_end__)|| ((addr & 0xFF000000) == DEV_ADDR_FLAG );
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

uint8_t* vmem_alloc_for_userland(pcb_s* process, size_t size)
{
	size_t nb_pages = size / PAGE_SIZE;
	if (nb_pages % PAGE_SIZE > 0) {
		nb_pages++;
	}

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

	/* Initialisation for kernel heap 
	while(virtual_addr <= FIRST_LVL_START(__kernel_heap_end__)) {
		second_lvl_tt = (uint32_t*) kAlloc_aligned(SECON_LVL_TT_SIZE, SECON_LVL_INDEX_WIDTH);	
		second_lvl_tt_entry = second_lvl_tt;
		*first_lvl_tt++ = ((uint32_t)second_lvl_tt & 0xFFFFFC00) + coarse_page_flag; 

		for (j = 0; j < SECON_LVL_TT_COUN ; j++) {
			if (is_init_addr(virtual_addr)) {
				*second_lvl_tt_entry = (virtual_addr & 0xFFFFF000)+ normal_flags;	
			} else {
				*second_lvl_tt_entry = 0;	
			}
			second_lvl_tt_entry++;
			virtual_addr += PAGE_SIZE;
		}
	}

	while (virtual_addr < 0x20000000) {
		*first_lvl_tt++ = 0;
		virtual_addr += SECON_LVL_TT_COUN * PAGE_SIZE;
	}

	while (virtual_addr <= FIRST_LVL_START(0x20FFFFFF)) {
		second_lvl_tt = (uint32_t*) kAlloc_aligned(SECON_LVL_TT_SIZE, SECON_LVL_BITSHIFT);	
		second_lvl_tt_entry = second_lvl_tt;
		*first_lvl_tt++ = ((uint32_t)second_lvl_tt & 0xFFFFFC00) + coarse_page_flag; 

		for (j = 0; j < SECON_LVL_TT_COUN ; j++) {
			if (is_init_addr(virtual_addr)) {
				*second_lvl_tt_entry = (virtual_addr & 0xFFFFF000)+ device_flags;	
			} else {
				*second_lvl_tt_entry = 0;	
			}
			second_lvl_tt_entry++;
			virtual_addr += PAGE_SIZE;
		}
	}

	uint32_t test = FIRST_LVL_START(0XFFFFFFFF);
	while(virtual_addr < test) { 
		*first_lvl_tt++ = 0;
		virtual_addr += SECON_LVL_TT_COUN * PAGE_SIZE;
	}*/

	uint32_t flag = 0;
	uint32_t virtual_addr_lvl_2;
	uint32_t test = FIRST_LVL_START(0xFFFFFFFF);
	for (virtual_addr = 0 ;
			virtual_addr < test ;
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

//Adresse reelle : frameNumber 0x

static void init_frame_table()
{
	frame_table = (uint8_t*) kAlloc(NB_FRAME/8);
	size_t i;
	for(i = 0 ; i < NB_FRAME / 8 ; i++)
	{
		if (i < 0x2000) {
			frame_table[i] = 0xFF;
		} else if (i == 0x2000 ) {
			
		}
		
		else if ( i < 0x4000 && i > 2

	}
	
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
		//table_base = (uint32_t) process->page_table;
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
#endif