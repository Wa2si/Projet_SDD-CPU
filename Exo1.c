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
