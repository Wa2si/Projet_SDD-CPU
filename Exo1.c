#include<stdio.h>
#include<stdlib.h>
#include"hash.h"


unsigned long simple_hash(const char *str) {
    unsigned long hash = 0;
    int i = 0;
    
    while (str[i] != '\0') {
        hash = (hash * 31) + str[i];  
        i++;  
    }
    
    return hash % TABLE_SIZE;  
}

HashMap* hashmap_create(){
    HashMap* new_hash = malloc(sizeof(HashMap));
    if(new_hash == NULL){
        printf("Erreur d'allocation <hashmap_create> \n");
        return;
    }

    new_hash->size = TABLE_SIZE;
    
    new_hash->table = calloc(TABLE_SIZE, sizeof(HashEntry*));

    if(new_hash->table == NULL){
        printf("Erreur d'allocation <hashmap_create> \n");
        return;
    }

    return new_hash;

}

int hashmap_insert(HashMap *map, const char *key, void *value) {
    if (map == NULL) return -1;  

    unsigned long clef = simple_hash(key);

    while (map->table[clef].key != NULL && map->table[clef].key != TOMBSTONE) {
        clef = (clef + 1);
    }

    map->table[clef].key = strdup(key);  
    map->table[clef].value = value;

    return 0;  
}


void *hashmap_get(HashMap *map, const char *key){

    unsigned long clef = simple_hash(key);

    while (map->table[clef].key != NULL && map->table[clef].key != TOMBSTONE) {
        if (strcmp(map->table[clef].key, key) == 0) {
            return map->table[clef].value;
        }
        clef = (clef + 1);
    }

    return;

}