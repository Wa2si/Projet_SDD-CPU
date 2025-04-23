#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <regex.h>
#include "cpu.h"

// Fonction fournie dans l'énoncé
int matches(const char *pattern , const char *string) {
    regex_t regex;
    int result = regcomp (&regex, pattern, REG_EXTENDED);
    if (result) {
        fprintf(stderr, "Regex compilation failed for pattern:%s\n", pattern);
        return 0;
    }
    result = regexec (&regex, string, 0, NULL, 0);
    regfree (&regex);
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
    const char *registers[] = {"AX", "BX", "CX", "DX", "IP", "ZF", "SF"};
    for (int i = 0; i < 7; ++i) {
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

    // Étape 2 : chercher une zone libre avec find_free_segment
    Segment *prev = NULL;
    Segment *free_seg = find_free_segment(cpu->memory_handler, 0, total_size, &prev);

    if (!free_seg) {
        printf("Erreur : aucun segment libre trouvé pour les données.\n");
        return;
    }

    int start = (free_seg->start >= 0) ? free_seg->start : 0;
    if (create_segment(cpu->memory_handler, "DS", start, total_size) != 0) {
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
                if (!value) {
                    printf("Erreur : allocation mémoire échouée.\n");
                    free(copy);
                    return;
                }
                *value = atoi(token);
                store(cpu->memory_handler, "DS", pos++, value);
                token = strtok(NULL, ";");
            }
            free(copy);
        } else if (strcmp(inst->operand1, "DW") == 0) {
            int *value = malloc(sizeof(int));
            if (!value) {
                printf("Erreur : allocation mémoire échouée.\n");
                return;
            }
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

void allocate_code_segment(CPU *cpu, Instruction **code_instructions, int code_count) {
    if (!cpu || !code_instructions || code_count <= 0) return;

    Segment *prev = NULL;
    // Chercher une zone libre dans la free_list
    Segment *free_seg = find_free_segment(cpu->memory_handler, 0, code_count, &prev);

    if (!free_seg) {
        printf("Erreur : aucun segment libre trouvé pour le code.\n");
        return;
    }

    // Créer le segment CS à l’endroit trouvé
    int start = (free_seg->start >= 0) ? free_seg->start : 0;
    if (create_segment(cpu->memory_handler, "CS", start, code_count) != 0) {
        printf("Erreur : allocation du segment CS échouée.\n");
        return;
    }

    // Stocker les instructions dans le segment CS
    for (int i = 0; i < code_count; i++) {
        store(cpu->memory_handler, "CS", i, code_instructions[i]);
    }

    // Initialiser le registre IP à 0
    int *ip = (int *) hashmap_get(cpu->context, "IP");
    if (ip) {
        *ip = 0;
    } else {
        printf("Erreur : registre IP introuvable.\n");
    }
}

int handle_instruction(CPU *cpu, Instruction *instr, void *src, void *dest) {
    if (!cpu || !instr) return 0;

    if (strcmp(instr->mnemonic, "MOV") == 0) {
        handle_MOV(cpu, src, dest);

        // Affichage de l'instruction exécutée
        printf("Instruction exécutée : MOV %s, %s\n", instr->operand1, instr->operand2);

        return 1;
    }

    printf("Instruction non supportée : %s\n", instr->mnemonic);
    return 0;
}

int execute_instruction(CPU *cpu, Instruction *instr) {
    if (!cpu || !instr || !instr->mnemonic) {
        printf("Erreur : instruction invalide.\n");
        return 0;
    }

    void *src = NULL;
    void *dest = NULL;

    if (instr->operand2) {
        src = register_addressing(cpu, instr->operand2);
        if (!src) src = register_indirect_addressing(cpu, instr->operand2);
        if (!src) src = memory_direct_addressing(cpu, instr->operand2);
        if (!src) src = immediate_addressing(cpu, instr->operand2);
    }

    if (instr->operand1) {
        dest = register_addressing(cpu, instr->operand1);
        if (!dest) dest = register_indirect_addressing(cpu, instr->operand1);
        if (!dest) dest = memory_direct_addressing(cpu, instr->operand1);
    }

    // Exécution
    return handle_instruction(cpu, instr, src, dest);
}

Instruction *fetch_next_instruction(CPU *cpu) {
    if (!cpu) return NULL;

    int *ip = hashmap_get(cpu->context, "IP");
    if (!ip) {
        printf("Erreur : registre IP introuvable.\n");
        return NULL;
    }

    Instruction *instr = (Instruction *)load(cpu->memory_handler, "CS", *ip);
    if (!instr) {
        printf("Fin du programme atteinte.\n");
        return NULL;
    }

    (*ip)++;
    return instr;
}

int run_program(CPU *cpu) {
    if (!cpu) return 0;

    printf("Début de l'exécution du programme...\n");

    // === ÉTAPE 1 : Affichage initial ===
    printf("\n--- État initial du CPU ---\n");
    const char *regs[] = {"AX", "BX", "CX", "DX", "IP", "SP", "BP"};
    for (int i = 0; i < 7; ++i) {
        int *val = hashmap_get(cpu->context, regs[i]);
        if (val) printf("  %s = %d\n", regs[i], *val);
    }

    printf("\n--- Mémoire initiale (segment DS) ---\n");
    print_data_segment(cpu);

    printf("\nAppuyez sur [Entrée] pour exécuter l'instruction suivante ou sur 'q' pour quitter.\n\n");

    int instructions_executed = 0;

    // === ÉTAPE 2 : Boucle d'exécution ===
    while (1) {
        char c = getchar();
        if (c == 'q') {
            printf("Exécution interrompue par l'utilisateur.\n");
            break;
        }

        Instruction *instr = fetch_next_instruction(cpu);
        if (!instr) {
            printf("Aucune instruction à exécuter (peut-être fin du programme).\n");
            break;
        }

        execute_instruction(cpu, instr);
        instructions_executed++;

        // Nettoyage du buffer clavier
        while ((c = getchar()) != '\n' && c != EOF);
    }

    // === ÉTAPE 3 : Affichage final ===
    printf("\n--- État final du CPU ---\n");
    for (int i = 0; i < 7; ++i) {
        int *val = hashmap_get(cpu->context, regs[i]);
        if (val) printf("  %s = %d\n", regs[i], *val);
    }

    printf("\n--- Mémoire finale (segment DS) ---\n");
    print_data_segment(cpu);

    printf("\nFin de l'exécution. Instructions exécutées : %d\n", instructions_executed);
    return instructions_executed;
}
