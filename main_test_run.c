#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "cpu.h"

int main() {
    // === Étape 1 : Parsing du fichier ===
    ParserResult *result = parse("test_run.txt");
    if (!result) {
        printf("Erreur lors du parsing du fichier.\n");
        return 1;
    }

    // === Étape 2 : Création du CPU ===
    CPU *cpu = cpu_init(1024);
    if (!cpu) {
        printf("Erreur d'initialisation du CPU.\n");
        free_parser_result(result);
        return 1;
    }

    // === Étape 3 : Résolution des labels et constantes ===
    resolve_constants(result);

    // === Étape 4 : Allocation des segments ===
    allocate_variables(cpu, result->data_instructions, result->data_count);
    allocate_code_segment(cpu, result->code_instructions, result->code_count);

    // === Étape 5 : Exécution interactive ===
    run_program(cpu);

    // === Étape 6 : Nettoyage ===
    cpu_destroy(cpu);
    free_parser_result(result);

    return 0;
}
