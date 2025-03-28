#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>  // pour intptr_t
#include "parser.h"
#include "hash.h"

int main() {
    // Initialisation de la table des emplacements mémoire
    HashMap *memory_locations = hashmap_create();

    // Ligne d'exemple à parser
    const char *line = "arr DB 5,6,7";

    // Appel de la fonction à tester
    Instruction *inst = parse_data_instruction(line, memory_locations);

    // Vérification du résultat
    if (inst != NULL) {
        printf("Instruction analysée :\n");
        printf("  mnemonic  : %s\n", inst->mnemonic);
        printf("  operand1  : %s\n", inst->operand1);
        printf("  operand2  : %s\n", inst->operand2);
    } else {
        printf("Échec du parsing de la ligne.\n");
    }

    // Vérification de la mémoire associée à "arr"
    void *addr_ptr = hashmap_get(memory_locations, "arr");
    int addr = (int)(intptr_t)addr_ptr;
    printf("Adresse associée à 'arr' : %d\n", addr);

    // Libération mémoire
    free(inst->mnemonic);
    free(inst->operand1);
    free(inst->operand2);
    free(inst);
    hashmap_destroy(memory_locations);

    return 0;
}
