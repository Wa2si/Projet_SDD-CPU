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