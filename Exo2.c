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

int create_segment(MemoryHandler *handler, const char *name, int start, int size){
    if(handler == NULL || handler->free_list == NULL)
        return -1;

    // Etape 1
    Segment *prev = NULL;
    Segment *free_segment = find_free_segment(handler, start, size, &prev);
    if(free_segment == NULL){
        printf("Erreur : Pas assez de mémoire disponible pour %s\n",name);
        return -1;
    }

    // Etape 2
    Segment *new_seg = malloc(sizeof(Segment));
    if (new_seg == NULL){
        printf("Erreur d'allocation <create_segment>\n");
        return -1;
    }

    new_seg->start = start;
    new_seg->size = size;
    new_seg->next = NULL;

    hashmap_insert(handler->allocated, name, new_seg);

    // Etape 3
    if (free_segment->start == start){
        free_segment->start += size;
        free_segment->size -= size;
        
        if (free_segment->size == 0){
            if(prev == NULL){
                handler->free_list = free_segment->next;
            }
            else{
                prev->next = free_segment->next;
            }
            free(free_segment);
        }
    }
    else{
        Segment *new_free_segment = malloc(sizeof(Segment));
        if (new_free_segment == NULL){
            printf("Erreur d'allocation <create_segment>\n");
            return -1;
        }

        new_free_segment->start = start + size;
        new_free_segment->size = free_segment->size - size;
        new_free_segment->next = free_segment->next;

        free_segment->size = start - free_segment->start;
        free_segment->next = new_free_segment;
    }

    return 0;
}