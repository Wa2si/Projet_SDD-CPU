#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <regex.h>
#include "cpu.h"

int matches(const char *pattern , const char *string) {
    regex_t regex;
    int result = regcomp (&regex , pattern , REG_EXTENDED);
    if (result) {
        fprintf(stderr , "Regex compilation failed for pattern:%s\n" , pattern);
        return 0;
    }
    result = regexec (&regex , string , 0, NULL , 0);
    regfree (& regex);
    return result == 0;
}

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

    cpu->constant_pool = hashmap_create();
    if (!cpu->constant_pool) {
        printf("Erreur : échec de la création de la constant pool.\n");
        hashmap_destroy(cpu->context);
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

    // Libérer les registres (valeurs dans cpu->context)
    for (int i = 0; i < TABLE_SIZE; i++) {
        if (cpu->context->table[i].value != NULL &&
            cpu->context->table[i].value != TOMBSTONE) {
            free(cpu->context->table[i].value); // libère les int* des registres
        }
    }
    hashmap_destroy(cpu->context);

    // Libérer les constantes immédiates de la constant_pool
    for (int i = 0; i < TABLE_SIZE; i++) {
        if (cpu->constant_pool->table[i].value != NULL &&
            cpu->constant_pool->table[i].value != TOMBSTONE) {
            free(cpu->constant_pool->table[i].value);
        }
    }
    hashmap_destroy(cpu->constant_pool);

    // Libérer les données du segment "DS"
    Segment *segment = hashmap_get(cpu->memory_handler->allocated, "DS");
    if (segment) {
        for (int i = 0; i < segment->size; i++) {
            int index = segment->start + i;
            if (cpu->memory_handler->memory[index]) {
                free(cpu->memory_handler->memory[index]);
            }
        }
    }

    remove_segment(cpu->memory_handler, "DS");

    // Libérer tous les Segment* contenus dans allocated
    for (int i = 0; i < cpu->memory_handler->allocated->size; ++i) {
        void *val = cpu->memory_handler->allocated->table[i].value;
        if (val != NULL && val != TOMBSTONE) {
            free(val);
        }
    }

    hashmap_destroy(cpu->memory_handler->allocated);

    // Libérer la liste chaînée free_list
    Segment *current = cpu->memory_handler->free_list;
    while (current) {
        Segment *next = current->next;
        free(current);
        current = next;
    }

    // Libérer le tableau de mémoire
    free(cpu->memory_handler->memory);

    // Libérer la structure MemoryHandler
    free(cpu->memory_handler);

    // Libérer la structure CPU
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
            char *copy = strdup(inst->operand2);
            char *token = strtok(copy, ";");
            while (token) {
                total_size++;
                token = strtok(NULL, ";");
            }
            free(copy);
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
            char *copy = strdup(inst->operand2);
            char *token = strtok(copy, ";");
            while (token) {
                int *value = malloc(sizeof(int));
                *value = atoi(token);
                store(cpu->memory_handler, "DS", pos++, value);
                token = strtok(NULL, ";");
            }
            free(copy);
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

void *immediate_addressing(CPU *cpu, const char *operand) {
    if (!cpu || !operand) return NULL;

    // Vérifie si l'opérande est un entier valide
    if (!matches("^[-][0-9]+$", operand) && !matches("^[0-9]+$", operand)) {
        return NULL;
    }

    // Vérifie si la constante existe déjà dans la constant_pool
    void *existing = hashmap_get(cpu->constant_pool, operand);
    if (existing) {
        return existing;
    }

    // Sinon, alloue et insère dans le constant_pool
    int *val = malloc(sizeof(int));
    if (!val) {
        printf("Erreur : échec de l'allocation mémoire pour la constante immédiate '%s'\n", operand);
        return NULL;
    }

    *val = atoi(operand);
    hashmap_insert(cpu->constant_pool, operand, val);

    return val;
}

void *register_addressing(CPU *cpu, const char *operand){
    if (!cpu || !operand) return NULL;

    // Vérifie si l'opérande est un entier valide
    if (!matches("^(AX|BX|CX|DX)$", operand)) {
        return NULL;
    }

    // Récupère la valeur associée au registre dans le context
    void *val = hashmap_get(cpu->context, operand);
    return val;
}

void *memory_direct_addressing(CPU *cpu, const char *operand) {
    if (!cpu || !operand) return NULL;

    // Vérifie si l'opérande est une adresse mémoire directe au format [i]
    if (!matches("^\\[[0-9]+\\]$", operand)) {
        return NULL;
    }

    // Extraire le nombre entre les crochets
    char buffer[16];
    strncpy(buffer, operand + 1, strlen(operand) - 2);  // sauter le premier [ et le dernier ]
    buffer[strlen(operand) - 2] = '\0';

    // Convertir en entier
    int pos = atoi(buffer);

    // Charger la valeur à cette position dans le segment DS
    return load(cpu->memory_handler, "DS", pos);
}


void *register_indirect_addressing(CPU *cpu, const char *operand) {
    if (!cpu || !operand) return NULL;

    // Vérifie que l'opérande correspond à [AX], [BX], etc.
    if (!matches("^\\[(AX|BX|CX|DX)\\]$", operand)) {
        return NULL;
    }

    // Extraire le nom du registre entre les crochets
    char reg_name[4];
    strncpy(reg_name, operand + 1, 2);
    reg_name[2] = '\0';

    // Récupérer le pointeur vers le registre
    int *pos = (int *) hashmap_get(cpu->context, reg_name);

    // Charger la valeur dans le segment "DS"
    void *val = load(cpu->memory_handler, "DS", *pos);
    return val;
}

void handle_MOV(CPU *cpu, void *src, void *dest) {
    if (!cpu || !src || !dest) return;

    *(int *)dest = *(int *)src;
}
