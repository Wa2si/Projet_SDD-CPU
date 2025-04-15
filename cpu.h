#ifndef CPU_H
#define CPU_H

#include "memory.h"
#include "hash.h"
#include "parser.h"

// Définition de la structure CPU
typedef struct {
    MemoryHandler *memory_handler;
    HashMap *context;
    HashMap *constant_pool; // Table de hachage pour stocker les valeurs immédiates
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

// Traite l'adressage immédiat
void *immediate_addressing(CPU *cpu, const char *operand);

// Traite l'adressage par registre
void *register_addressing(CPU *cpu, const char *operand);

// Traite l'adressage direct
void *memory_direct_addressing(CPU *cpu, const char *operand);

// Traite l'adressage indirect par registre
void *register_indirect_addressing(CPU *cpu, const char *operand);

// Effectue l'instruction MOV
void handle_MOV(CPU* cpu, void* src, void* dest);

#endif