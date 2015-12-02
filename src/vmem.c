#include <stdlib.h>
#include <stdbool.h>
#include "vmem.h"
#include "kheap.h"

#define DEV_ADDR_FLAG 0x20000000

static uint32_t mmu_base;

static bool is_init_addr(uint32_t addr)
{
	return addr < __kernel_heap_end__ || ((addr & 0xFF000000) == DEV_ADDR_FLAG );
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
	__asm__ volatile("mcr p15, 0, %[control], c1, c0, O" :: [control]"r"(control));
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
	configure_mmu_c();
	//Interruptions
	start_mmu_c();
}

uint32_t init_kern_translation_table(void)
{
	uint8_t* base_table = kAlloc_aligned(FIRST_LVL_TT_SIZE, FIRST_LVL_BITSHIFT);
	mmu_base = base_table;
	uint32_t device_flags = 0x437; 
	uint32_t normal_flags = 0x44E;
	uint32_t coarse_page_flag = 1;
	
	size_t kheap_page_count = __kernel_heap_end__ / PAGE_SIZE;

	if(__kernel_heap_end__ % PAGE_SIZE > 0)
		kheap_page_count++;

	size_t kheap_second_lvl_tt_count = kheap_page_count / SECON_LVL_TT_COUN;
	if(kheap_page_count % SECON_LVL_TT_COUN > 0)
		kheap_second_lvl_tt_count++;

	size_t kheap_first_lvl_tt_count = kheap_second_lvl_tt_count / FIRST_LVL_TT_COUN;
	if(kheap_second_lvl_tt_count % FIRST_LVL_TT_COUN > 0)
		kheap_first_lvl_tt_count++;
	
	size_t i, j;
	uint32_t* second_lvl_tt;
	uint32_t virtual_addr = 0;
	uint32_t* second_lvl_tt_entry;
	uint32_t* first_lvl_tt = (uint32_t*) base_table;

	/* Initialisation for kernel heap */
	for (i = 0 ; i < kheap_first_lvl_tt_count; i++) {
		second_lvl_tt = (uint32_t*) kAlloc_aligned(SECON_LVL_TT_SIZE, SECON_LVL_BITSHIFT);	
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

	for ( ; virtual_addr < 0x20000000 ; virtual_addr += SECON_LVL_TT_COUN * PAGE_SIZE ) {
		*first_lvl_tt++ = 0;
	}
	
	for ( ; virtual_addr <= FIRST_LVL_START(0x20FFFFFF) ; ) {
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
	
	for ( ; virtual_addr <= FIRST_LVL_START(0xFFFFFFFF) ; 
			virtual_addr += SECON_LVL_TT_COUN * PAGE_SIZE ) {
		*first_lvl_tt++ = 0;
	}

	return 0;
}

