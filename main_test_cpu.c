#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"
#include "parser.h"

int main() {
    printf("=== Test du module CPU (avec parsing d'un fichier) ===\n");

    // Parser un fichier contenant .DATA et .CODE
    ParserResult *result = parse("test_parser.txt");
    if (!result) {
        printf("Erreur lors du parsing du fichier.\n");
        return 1;
    }

    // Initialiser le CPU avec une mémoire de taille 64
    CPU *cpu = cpu_init(64);
    if (!cpu) {
        printf("Échec de l'initialisation du CPU.\n");
        free_parser_result(result);
        return 1;
    }

    // Allouer les variables à partir des instructions .DATA
    allocate_variables(cpu, result->data_instructions, result->data_count);

    // Afficher le contenu du segment DS
    print_data_segment(cpu);

    // Nettoyage
    cpu_destroy(cpu);
    free_parser_result(result);

    printf("=== Fin du test CPU ===\n");
    return 0;
}
