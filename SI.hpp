#ifndef SI_H
#define SI_H

#include <stdint.h>
#include "CPU/i8080.hpp"

#define MEMORY_SIZE 0x4000
#define VRAM_START 0x2400

/* 
(0x000 - 0x1FFF) ROM
(0x2000 - 0x23FF) RAM
(0x2400 - 0x3FFF) VRAM
(0x4000) not used
*/

typedef struct {
	uint8_t memory[MEMORY_SIZE];
	Device devs[6];
	uint8_t current_interrupt;
	uint16_t shift_reg;
	uint8_t shift_offset;
} SI;

#endif
