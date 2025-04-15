#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"

// Fonction fournie dans l'énoncé
CPU* setup_test_environment () {
    CPU *cpu = cpu_init(1024);
    if (!cpu) {
        printf("Error: CPU initialization failed\n");
        return NULL;
    }

    int *ax = (int *) hashmap_get(cpu->context, "AX");
    int *bx = (int *) hashmap_get(cpu->context, "BX");
    int *cx = (int *) hashmap_get(cpu->context, "CX");
    int *dx = (int *) hashmap_get(cpu->context, "DX");

    *ax = 3;
    *bx = 6;
    *cx = 100;
    *dx = 0;

    if (!hashmap_get(cpu->memory_handler->allocated, "DS")) {
        create_segment(cpu->memory_handler, "DS", 0, 20);
        for (int i = 0; i < 10; i++) {
            int *value = malloc(sizeof(int));
            *value = i * 10 + 5; // 5, 15, 25, ...
            store(cpu->memory_handler, "DS", i, value);
        }
    }

    printf("Test environment initialized.\n");
    return cpu;
}

int main() {
    CPU *cpu = setup_test_environment();
    if (!cpu) return 1;

    printf("\n==== Test de handle_MOV avec les 4 types d'adressage ====\n");

    // 1. mémoire directe -> registre
    void *src1 = memory_direct_addressing(cpu, "[2]"); // DS[2] = 25
    void *dest1 = register_addressing(cpu, "AX");
    handle_MOV(cpu, src1, dest1);
    printf("AX = %d (expected 25)\n", *(int *)dest1);

    // 2. registre -> registre
    void *src2 = register_addressing(cpu, "AX");
    void *dest2 = register_addressing(cpu, "DX");
    handle_MOV(cpu, src2, dest2);
    printf("DX = %d (expected 25)\n", *(int *)dest2);

    // 3. indirect par registre -> registre
    void *src3 = register_indirect_addressing(cpu, "[BX]"); // BX = 6, DS[BX] = DS[6] = 65
    void *dest3 = register_addressing(cpu, "CX");
    handle_MOV(cpu, src3, dest3);
    printf("CX = %d (expected 65)\n", *(int *)dest3);

    // 4. registre -> indirect par registre
    void *src4 = register_addressing(cpu, "AX"); // AX = 25
    void *dest4 = register_indirect_addressing(cpu, "[BX]"); // BX = 6
    handle_MOV(cpu, src4, dest4);
    int *check = (int *)load(cpu->memory_handler, "DS", 6);
    printf("DS[6] = %d (expected 25)\n", *check);

    cpu_destroy(cpu);
    return 0;
}
