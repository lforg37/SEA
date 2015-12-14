#ifndef VMEM_H
#define VMEM_H

/* Taille d'une page en nombre d'adresse */
#define PAGE_SIZE 4096 
#define SECON_LVL_TT_COUN 256
#define SECON_LVL_BITSHIFT 12
#define SECON_LVL_INDEX_WIDTH 10
#define SECON_LVL_TT_SIZE 1024
#define FIRST_LVL_TT_COUN 4096
#define FIRST_LVL_BITSHIFT 20
#define FIRST_LVL_TT_SIZE 16384
#define FIRST_LVL_INDEX_WIDTH 14
#define FIRST_LVL_START(ADDR) (ADDR & (0xFFFFFFFF - (1 << FIRST_LVL_BITSHIFT) +1))

#include <stdint.h>
#include <stdlib.h>

typedef struct page_element
{
	uint8_t* adress;
	size_t nb_pages;
	struct page_element* next;

} page_element;

typedef page_element page_list;

uint32_t init_kern_translation_table(void);
void vmem_init(void);

#endif
