#ifndef MEMORY_H
#define MEMORY_H

#include <stdlib.h>
#include <stdio.h>
#include "hash.h"

typedef struct segment {
    int start;
    int size;
    struct segment *next;
} Segment;

typedef struct memoryHandler {
    void **memory;
    int total_size;
    Segment *free_list;
    HashMap *allocated;
} MemoryHandler;

MemoryHandler *memory_init(int size);

#endif
