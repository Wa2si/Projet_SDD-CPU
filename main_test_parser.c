#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "parser.h"
#include "hash.h"

int main() {
    printf("=== Test du fichier test_parser.txt ===\n");

    ParserResult *result = parse("test_parser.txt");

    if (result) {
        printf("\n--- .DATA ---\n");
        for (int i = 0; i < result->data_count; i++) {
            Instruction *inst = result->data_instructions[i];
            printf("  [%d] %s %s %s\n", i, inst->mnemonic, inst->operand1, inst->operand2);
        }

        printf("\n--- .CODE ---\n");
        for (int i = 0; i < result->code_count; i++) {
            Instruction *inst = result->code_instructions[i];
            printf("  [%d] %s %s %s\n", i, inst->mnemonic, inst->operand1, inst->operand2);
        }

        printf("\n--- LABELS ---\n");
        const char *labels[] = {"start", "loop"};
        for (int i = 0; i < 2; i++) {
            void *addr = hashmap_get(result->labels, labels[i]);
            printf("  %s → %d\n", labels[i], (int)(intptr_t)addr);
        }

        printf("\n--- MEMORY LOCATIONS ---\n");
        const char *vars[] = {"x", "arr", "y"};
        for (int i = 0; i < 3; i++) {
            void *addr = hashmap_get(result->memory_locations, vars[i]);
            printf("  %s → %d\n", vars[i], (int)(intptr_t)addr);
        }

        free_parser_result(result);

    } else {
        printf("Échec du parsing de test_parser.txt\n");
    }

    return 0;
}
