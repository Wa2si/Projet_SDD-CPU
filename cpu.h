#ifndef CPU_H
#define CPU_H

#include "memory.h"
#include "hash.h"

// Définition de la structure CPU
typedef struct {
    MemoryHandler *memory_handler;
    HashMap *context;
} CPU;

// Initialise un CPU avec une mémoire de taille donnée
CPU *cpu_init(int memory_size);

// Libère les ressources allouées par le CPU
void cpu_destroy(CPU *cpu);

#endif