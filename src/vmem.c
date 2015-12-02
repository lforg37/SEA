#include <stdlib.h>
#include <stdbool.h>
#include "vmem.h"
#include "kheap.h"

#define DEV_ADDR_FLAG 0x20000000

static bool is_init_addr(uint32_t addr)
{
	return addr < __kernel_heap_end__ || ((addr & 0xFF000000) == DEV_ADDR_FLAG );
}

uint32_t init_kern_translation_table(void)
{
	uint8_t* base_table = kAlloc_aligned(FIRST_LVL_TT_SIZE, FIRST_LVL_BITSHIFT);
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

