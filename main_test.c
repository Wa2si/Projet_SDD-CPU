#include <stdio.h>
#include "memory.h"

void print_free_list(MemoryHandler *handler) {
    Segment *current = handler->free_list;
    printf("\n--- Free List ---\n");
    while (current != NULL) {
        printf("[start: %d, size: %d] -> ", current->start, current->size);
        current = current->next;
    }
    printf("NULL\n");
}

void memory_destroy(MemoryHandler *handler) {
    if (handler) {
        Segment *current = handler->free_list;
        while (current) {
            Segment *temp = current;
            current = current->next;
            free(temp);
        }
        hashmap_destroy(handler->allocated);
        free(handler->memory);
        free(handler);
    }
}

int main() {
    printf("===== Test du gestionnaire de mémoire =====\n");

    // Initialisation
    MemoryHandler *handler = memory_init(1024);
    if (!handler) {
        printf("Erreur lors de l'initialisation de la mémoire.\n");
        return 1;
    }
    print_free_list(handler);

    // Allocation de segments
    printf("\n>>> Allocation de 'seg1' (200 unités à 100)...\n");
    create_segment(handler, "seg1", 100, 200);
    print_free_list(handler);

    printf("\n>>> Allocation de 'seg2' (300 unités à 400)...\n");
    create_segment(handler, "seg2", 400, 300);
    print_free_list(handler);

    printf("\n>>> Allocation de 'seg3' (100 unités à 800)...\n");
    create_segment(handler, "seg3", 800, 100);
    print_free_list(handler);

    // Suppression de segments
    printf("\n>>> Suppression de 'seg2'...\n");
    remove_segment(handler, "seg2");
    print_free_list(handler);

    printf("\n>>> Suppression de 'seg1'...\n");
    remove_segment(handler, "seg1");
    print_free_list(handler);

    printf("\n>>> Suppression de 'seg3'...\n");
    remove_segment(handler, "seg3");
    print_free_list(handler);

    printf("\n>>> Nettoyage de la mémoire...\n");
    memory_destroy(handler);
    return 0;
}
