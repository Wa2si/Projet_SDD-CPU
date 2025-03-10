#include "memory.h"

MemoryHandler *memory_init(int size){

    MemoryHandler *handler = malloc(sizeof(MemoryHandler));
    if (handler == NULL){
        printf("Erreur d'allocation <memory_init>\n");
        return NULL;
    }

    handler->memory = calloc(size, sizeof(void *));
    if (handler->memory == NULL){
        printf("Erreur d'allocation <memory_init>\n");
        free(handler);
        return NULL;
    }

    handler->total_size = size;
    handler->allocated = hashmap_create();

    handler->free_list = malloc(sizeof(Segment));
    if (handler->free_list == NULL){
        printf("Erreur d'allocation <memory_init>\n");
        free(handler->memory);
        free(handler);
        return NULL;
    }

    handler->free_list->start = 0;
    handler->free_list->size = size;
    handler->free_list->next = NULL;

    return handler;
}

Segment* find_free_segment(MemoryHandler* handler, int start, int size, Segment** prev) {
    if (handler == NULL || handler->free_list == NULL)
        return NULL;

    Segment *current = handler->free_list;
    *prev = NULL;

    while (current != NULL) {
        if (current->start <= start && (current->start + current->size) >= (start + size)) {
            return current;
        }

        *prev = current;
        current = current->next;
    }

    return NULL;
}
