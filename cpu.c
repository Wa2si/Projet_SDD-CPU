#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "cpu.h"

CPU *cpu_init(int memory_size) {
    CPU *cpu = malloc(sizeof(CPU));
    if (!cpu) {
        printf("Erreur : échec de l'allocation mémoire pour la structure CPU.\n");
        return NULL;
    }

    cpu->memory_handler = memory_init(memory_size);
    if (!cpu->memory_handler) {
        printf("Erreur : échec de l'initialisation du gestionnaire de mémoire.\n");
        free(cpu);
        return NULL;
    }

    cpu->context = hashmap_create();
    if (!cpu->context) {
        printf("Erreur : échec de la création de la table de hachage pour les registres.\n");
        free(cpu->memory_handler);
        free(cpu);
        return NULL;
    }

    // Initialiser les registres AX, BX, CX, DX à 0
    const char *registers[] = {"AX", "BX", "CX", "DX"};
    for (int i = 0; i < 4; ++i) {
        int *value = malloc(sizeof(int));
        if (!value) {
            printf("Avertissement : échec de l'allocation pour le registre %s.\n", registers[i]);
            continue;
        }
        *value = 0;
        hashmap_insert(cpu->context, registers[i], value);
    }

    return cpu;
}

void cpu_destroy(CPU *cpu) {
    if (!cpu) return;

    // Libérer les registres
    for (int i = 0; i < cpu->context->size; ++i) {
        if (cpu->context->table[i].key != NULL && cpu->context->table[i].value != TOMBSTONE) {
            free(cpu->context->table[i].value);
        }
    }

    hashmap_destroy(cpu->context);
    free(cpu->memory_handler);
    free(cpu);
}

void* store(MemoryHandler *handler, const char *segment_name, int pos, void *data) {
    if (!handler || !segment_name || !data) {
        printf("Erreur : paramètre invalide dans store().\n");
        return NULL;
    }

    Segment *segment = (Segment *)hashmap_get(handler->allocated, segment_name);
    if (!segment) {
        printf("Erreur : le segment '%s' n'existe pas.\n", segment_name);
        return NULL;
    }

    if (pos < 0 || pos >= segment->size) {
        printf("Erreur : position %d hors limites pour le segment '%s' (taille = %d).\n", pos, segment_name, segment->size);
        return NULL;
    }

    int index = segment->start + pos;
    handler->memory[index] = data;
    return data;
}

void* load(MemoryHandler *handler, const char *segment_name, int pos) {
    if (!handler || !segment_name) {
        printf("Erreur : paramètre invalide dans load().\n");
        return NULL;
    }

    Segment *segment = (Segment *)hashmap_get(handler->allocated, segment_name);
    if (!segment) {
        printf("Erreur : le segment '%s' n'existe pas.\n", segment_name);
        return NULL;
    }

    if (pos < 0 || pos >= segment->size) {
        printf("Erreur : position %d hors limites pour le segment '%s' (taille = %d).\n", pos, segment_name, segment->size);
        return NULL;
    }

    int index = segment->start + pos;
    return handler->memory[index];
}

void allocate_variables(CPU *cpu, Instruction **data_instructions, int data_count) {
    if (!cpu || !data_instructions || data_count <= 0) return;

    int total_size = 0;

    // Étape 1 : calculer la taille totale du segment
    for (int i = 0; i < data_count; i++) {
        Instruction *inst = data_instructions[i];
        if (strcmp(inst->operand1, "DB") == 0) {
            // Liste de valeurs séparées par des virgules
            char *token = strtok(inst->operand2, ",");
            while (token) {
                total_size++;
                token = strtok(NULL, ",");
            }
        } else if (strcmp(inst->operand1, "DW") == 0) {
            total_size++;
        }
    }

    // Étape 2 : allouer le segment "DS"
    if (create_segment(cpu->memory_handler, "DS", 0, total_size) != 0) {
        printf("Erreur : impossible d'allouer le segment DS.\n");
        return;
    }

    // Étape 3 : stocker les données dans le segment
    int pos = 0;
    for (int i = 0; i < data_count; i++) {
        Instruction *inst = data_instructions[i];

        if (strcmp(inst->operand1, "DB") == 0) {
            char *token = strtok(inst->operand2, ",");
            while (token) {
                int *value = malloc(sizeof(int));
                *value = atoi(token);
                store(cpu->memory_handler, "DS", pos++, value);
                token = strtok(NULL, ",");
            }
        } else if (strcmp(inst->operand1, "DW") == 0) {
            int *value = malloc(sizeof(int));
            *value = atoi(inst->operand2);
            store(cpu->memory_handler, "DS", pos++, value);
        }
    }
}

void print_data_segment(CPU *cpu){
    if (!cpu) return;

    Segment *segment = (Segment *)hashmap_get(cpu->memory_handler->allocated, "DS");
    if (!segment) {
        printf("Segment DS introuvable.\n");
        return;
    }

    printf("Contenu du segment DS :\n");
    for (int i = 0; i < segment->size; i++){
        int *val = (int *)load(cpu->memory_handler, "DS", i);
        if (val)
            printf("[DS + %d] = %d\n", i, *val);
        else
            printf("[DS + %d] = NULL\n", i);
    }
}