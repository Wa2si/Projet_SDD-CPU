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
        if ((current->start + current->size) >= (start + size)) {
            if (current->start > start) {
                start = current->start;
            }
            return current;
        }

        *prev = current;
        current = current->next;
    }

    return NULL;
}

int create_segment(MemoryHandler *handler, const char *name, int start, int size) {
    if (handler == NULL || handler->free_list == NULL)
        return -1;

    Segment *prev = NULL;
    Segment *free_segment = find_free_segment(handler, start, size, &prev);
    if (free_segment == NULL) {
        printf("Erreur : Pas assez de mémoire disponible pour %s\n", name);
        return -1;
    }

    Segment *new_seg = malloc(sizeof(Segment));
    if (new_seg == NULL) {
        printf("Erreur d'allocation <create_segment>\n");
        return -1;
    }
    new_seg->start = start;
    new_seg->size = size;
    new_seg->next = NULL;
    hashmap_insert(handler->allocated, name, new_seg);

    if (free_segment->start == start) {
        free_segment->start += size;
        free_segment->size -= size;
        if (free_segment->size == 0) {
            if (prev)
                prev->next = free_segment->next;
            else
                handler->free_list = free_segment->next;
            free(free_segment);
        }
    } else {
        Segment *new_free = malloc(sizeof(Segment));
        new_free->start = start + size;
        new_free->size = free_segment->start + free_segment->size - new_free->start;
        new_free->next = free_segment->next;
        free_segment->size = start - free_segment->start;
        free_segment->next = new_free;
    }
    return 0;
}

int remove_segment(MemoryHandler *handler, const char *name) {
    if (handler == NULL || name == NULL)
        return -1;

    Segment *segment = hashmap_get(handler->allocated, name);
    if (segment == NULL) {
        printf("Erreur : Le segment %s n'existe pas\n", name);
        return -1;
    }

    int start = segment->start;
    int size = segment->size;
    hashmap_remove(handler->allocated, name);
    free(segment);

    Segment *prev = NULL;
    Segment *current = handler->free_list;
    
    while (current != NULL && current->start < start) {
        prev = current;
        current = current->next;
    }
    
    Segment *new_free = malloc(sizeof(Segment));
    new_free->start = start;
    new_free->size = size;
    new_free->next = current;
    
    if (prev == NULL) {
        handler->free_list = new_free;
    } else {
        prev->next = new_free;
        if (prev->start + prev->size == new_free->start) {
            prev->size += new_free->size;
            prev->next = new_free->next;
            free(new_free);
            new_free = prev;
        }
    }
    
    if (current && new_free->start + new_free->size == current->start) {
        new_free->size += current->size;
        new_free->next = current->next;
        free(current);
    }
    
    return 0;
}