#ifndef SI_H
#define SI_H

#include <stdint.h>
#include "CPU/i8080.h"

#define MEMORY_SIZE 0x4000

typedef struct {
	uint8_t memory[MEMORY_SIZE];
	Device devs[6];
	uint8_t current_interrupt;
} SI;

#endif
