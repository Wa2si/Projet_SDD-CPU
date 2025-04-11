#ifndef CPU_H
#define CPU_H

#include "memory.h"
#include "hash.h"
#include "parser.h"

// Définition de la structure CPU
typedef struct {
    MemoryHandler *memory_handler;
    HashMap *context;
} CPU;

// Initialise un CPU avec une mémoire de taille donnée
CPU *cpu_init(int memory_size);

// Libère les ressources allouées par le CPU
void cpu_destroy(CPU *cpu);

// Stocke une donnée à une position donnée dans un segment
void* store(MemoryHandler *handler, const char *segment_name, int pos, void *data);

// Récupère une donnée depuis une position dans un segment
void* load(MemoryHandler *handler, const char *segment_name, int pos);

// Alloue dynamiquement les variables selon les instructions .DATA
void allocate_variables(CPU *cpu, Instruction **data_instructions, int data_count);

// Affiche le contenu du segment "DS"
void print_data_segment(CPU *cpu);

#endif