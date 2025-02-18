#ifndef HASH_H
#define HASH_H

#include <stdlib.h> 
#include <stdio.h> 


#define TABLE_SIZE 128

#define TOMBSTONE ((void*) -1)

typedef struct hashentry {
    char* key;       
} HashEntry;

typedef struct hashmap {
    int size;        
    HashEntry* table; 
} HashMap;

unsigned long simple_hash(const char *str);

#endif 
