#include "parser.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>

static int current_address = 0; // compteur mémoire global

Instruction *parse_data_instruction(const char *line, HashMap *memory_locations) {
    if (line == NULL || memory_locations == NULL) return NULL;

    Instruction *inst = malloc(sizeof(Instruction));
    if (!inst) {
        printf("Erreur d'allocation mémoire : parse_data_instruction (Instruction)\n");
        return NULL;
    }

    char *line_copy = strdup(line);
    if (!line_copy) {
        printf("Erreur d'allocation mémoire : parse_data_instruction (line_copy)\n");
        free(inst);
        return NULL;
    }

    char *token = strtok(line_copy, " ");
    if (!token) {
        free(line_copy);
        free(inst);
        return NULL;
    }
    inst->mnemonic = strdup(token); // nom de la variable
    if (!inst->mnemonic) {
        printf("Erreur d'allocation mémoire : parse_data_instruction (mnemonic)\n");
        free(line_copy);
        free(inst);
        return NULL;
    }

    token = strtok(NULL, " ");
    if (!token) {
        free(line_copy);
        free(inst->mnemonic);
        free(inst);
        return NULL;
    }
    inst->operand1 = strdup(token); // type (DW ou DB)
    if (!inst->operand1) {
        printf("Erreur d'allocation mémoire : parse_data_instruction (operand1)\n");
        free(line_copy);
        free(inst->mnemonic);
        free(inst);
        return NULL;
    }

    token = strtok(NULL, "\n");
    if (!token) {
        free(line_copy);
        free(inst->mnemonic);
        free(inst->operand1);
        free(inst);
        return NULL;
    }

    while (*token == ' ') token++; // Enlève les espaces inutiles devant les valeurs

    // Convertit les virgules en points-virgules pour faciliter le stockage
    char *cleaned_operand2 = malloc(strlen(token) + 1);
    if (!cleaned_operand2) {
        printf("Erreur d'allocation mémoire : parse_data_instruction (operand2)\n");
        free(line_copy);
        free(inst->mnemonic);
        free(inst->operand1);
        free(inst);
        return NULL;
    }

    int j = 0;
    int count = 1;
    for (int i = 0; token[i] != '\0'; i++) {
        if (token[i] == ',') {
            cleaned_operand2[j++] = ';';
            count++;
        } else {
            cleaned_operand2[j++] = token[i];
        }
    }
    cleaned_operand2[j] = '\0';
    inst->operand2 = cleaned_operand2; // valeurs

    // Associe l'adresse à la variable dans la table
    hashmap_insert(memory_locations, inst->mnemonic, (void *)(intptr_t)current_address);
    current_address += count;

    free(line_copy);
    return inst;
}

Instruction *parse_code_instruction(const char *line, HashMap *labels, int code_count) {
    if (line == NULL || labels == NULL) return NULL;

    Instruction *inst = malloc(sizeof(Instruction));
    if (!inst) {
        printf("Erreur d'allocation mémoire : parse_code_instruction (Instruction)\n");
        return NULL;
    }

    char *line_copy = strdup(line);
    if (!line_copy) {
        printf("Erreur d'allocation mémoire : parse_code_instruction (line_copy)\n");
        free(inst);
        return NULL;
    }

    char *label = NULL;
    char *instruction_part = line_copy;

    // Vérifie s’il y a un label (présence de ':')
    char *colon = strchr(line_copy, ':');
    if (colon != NULL) {
        *colon = '\0';
        label = line_copy;
        instruction_part = colon + 1;
        while (*instruction_part == ' ') instruction_part++;

        // Enregistrer le label avec code_count comme valeur
        hashmap_insert(labels, label, (void *)(intptr_t)code_count);
    }

    // Extraction du mnemonic
    char *token = strtok(instruction_part, " ");
    if (!token) {
        printf("Erreur : ligne sans mnemonic\n");
        free(line_copy);
        free(inst);
        return NULL;
    }
    inst->mnemonic = strdup(token);

    // Extraction de l’opérande 1
    token = strtok(NULL, ",");
    if (token != NULL) {
        while (*token == ' ') token++;
        inst->operand1 = strdup(token);
    } else {
        inst->operand1 = strdup("");
    }

    // Extraction de l’opérande 2
    token = strtok(NULL, "\n");
    if (token != NULL) {
        while (*token == ' ') token++;
        inst->operand2 = strdup(token);
    } else {
        inst->operand2 = strdup("");
    }

    free(line_copy);
    return inst;
}


ParserResult *parse(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Erreur : impossible d'ouvrir le fichier %s\n", filename);
        return NULL;
    }

    ParserResult *result = malloc(sizeof(ParserResult));
    if (!result) {
        printf("Erreur d'allocation mémoire : ParserResult\n");
        fclose(file);
        return NULL;
    }

    result->data_instructions = NULL;
    result->data_count = 0;
    result->code_instructions = NULL;
    result->code_count = 0;
    result->memory_locations = hashmap_create();
    result->labels = hashmap_create();

    char buffer[256];
    int mode = 0; // 1 = .DATA, 2 = .CODE

    while (fgets(buffer, sizeof(buffer), file)) {
        // Supprimer saut de ligne (\n ou \r)
        buffer[strcspn(buffer, "\r\n")] = '\0';

        // Ligne vide ou commentaire (optionnel à ajouter)
        if (buffer[0] == '\0') continue;

        if (strcmp(buffer, ".DATA") == 0) {
            mode = 1;
            continue;
        }
        if (strcmp(buffer, ".CODE") == 0) {
            mode = 2;
            continue;
        }

        if (mode == 1) {
            Instruction *inst = parse_data_instruction(buffer, result->memory_locations);
            if (inst) {
                Instruction **temp = realloc(result->data_instructions, sizeof(Instruction *) * (result->data_count + 1));
                if (!temp) {
                    printf("Erreur realloc pour data_instructions\n");
                    fclose(file);
                    return NULL;
                }
                result->data_instructions = temp;
                result->data_instructions[result->data_count++] = inst;
            }
        } else if (mode == 2) {
            Instruction *inst = parse_code_instruction(buffer, result->labels, result->code_count);
            if (inst) {
                Instruction **temp = realloc(result->code_instructions, sizeof(Instruction *) * (result->code_count + 1));
                if (!temp) {
                    printf("Erreur realloc pour code_instructions\n");
                    fclose(file);
                    return NULL;
                }
                result->code_instructions = temp;
                result->code_instructions[result->code_count++] = inst;
            }
        }
    }

    fclose(file);
    return result;
}


void free_parser_result(ParserResult *result) {
    if (!result) return;

    // Libérer les instructions .DATA
    for (int i = 0; i < result->data_count; i++) {
        Instruction *inst = result->data_instructions[i];
        free(inst->mnemonic);
        free(inst->operand1);
        free(inst->operand2);
        free(inst);
    }
    free(result->data_instructions);

    // Libérer les instructions .CODE
    for (int i = 0; i < result->code_count; i++) {
        Instruction *inst = result->code_instructions[i];
        free(inst->mnemonic);
        free(inst->operand1);
        free(inst->operand2);
        free(inst);
    }
    free(result->code_instructions);

    // Libérer les hashmaps
    hashmap_destroy(result->memory_locations);
    hashmap_destroy(result->labels);

    // Libérer la structure globale
    free(result);
}
