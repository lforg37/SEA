#ifndef VMEM_H
#define VMEM_H

/* Taille d'une page en nombre d'adresse */
#define PAGE_SIZE 4096 
#define SECON_LVL_TT_COUN 256
#define SECON_LVL_TT_SIZE 1024
#define FIRST_LVL_TT_COUN 4096
#define FIRST_LVL_TT_SIZE 16384

#include <stdint.h>

uint32_t init_kern_translation_table(void);

#endif
