#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "parser.h"
#include "hash.h"

int main() {
    // --- Test parse_data_instruction ---
    HashMap *memory_locations = hashmap_create();
    const char *data_line = "arr DB 5,6,7";
    Instruction *data_inst = parse_data_instruction(data_line, memory_locations);

    printf("=== Test .DATA ===\n");
    if (data_inst != NULL) {
        printf("Instruction analysée :\n");
        printf("  mnemonic  : %s\n", data_inst->mnemonic);
        printf("  operand1  : %s\n", data_inst->operand1);
        printf("  operand2  : %s\n", data_inst->operand2);

        void *addr_ptr = hashmap_get(memory_locations, "arr");
        int addr = (int)(intptr_t)addr_ptr;
        printf("Adresse associée à 'arr' : %d\n", addr);
        
    } else {
        printf("Échec du parsing de .DATA.\n");
    }

    // --- Test parse_code_instruction ---
    HashMap *labels = hashmap_create();
    const char *code_line = "loop: MOV AX, 1";
    Instruction *code_inst = parse_code_instruction(code_line, labels, 0); // code_count = 0

    printf("\n=== Test .CODE ===\n");
    if (code_inst != NULL) {
        printf("Instruction analysée :\n");
        printf("  mnemonic  : %s\n", code_inst->mnemonic);
        printf("  operand1  : %s\n", code_inst->operand1);
        printf("  operand2  : %s\n", code_inst->operand2);

        void *label_addr = hashmap_get(labels, "loop");
        int index = (int)(intptr_t)label_addr;
        printf("Index associé au label 'loop' : %d\n", index);
    } else {
        printf("Échec du parsing de .CODE.\n");
    }

    // Libération mémoire
    free(data_inst->mnemonic);
    free(data_inst->operand1);
    free(data_inst->operand2);
    free(data_inst);

    free(code_inst->mnemonic);
    free(code_inst->operand1);
    free(code_inst->operand2);
    free(code_inst);

    hashmap_destroy(memory_locations);
    hashmap_destroy(labels);

    return 0;
}
