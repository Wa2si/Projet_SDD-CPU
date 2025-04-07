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
