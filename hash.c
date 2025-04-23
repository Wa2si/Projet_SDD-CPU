#include"hash.h"

/* Calcule un hash simple pour une chaîne de caractères */
unsigned long simple_hash(const char *str) {
    unsigned long hash = 0;
    int i = 0;
    
    while (str[i] != '\0') {
        hash = (hash * 31) + str[i];  
        i++;  
    }
    
    return hash % TABLE_SIZE;  
}

/* Crée et initialise une table de hachage vide */
HashMap* hashmap_create(){
    HashMap* new_hash = malloc(sizeof(HashMap));
    if(new_hash == NULL){
        printf("Erreur d'allocation <hashmap_create> \n");
        return NULL;
    }

    new_hash->size = 0;
    
    new_hash->table = calloc(TABLE_SIZE, sizeof(HashEntry));

    if(new_hash->table == NULL){
        printf("Erreur d'allocation <hashmap_create> \n");
        free(new_hash);
        return NULL;
    }

    return new_hash;
}


/* Insère une paire clé/valeur dans la table de hachage */
int hashmap_insert(HashMap *map, const char *key, void *value) {
    if (map == NULL) return -1;  

    if (map->size >= TABLE_SIZE) {
        printf("Table pleine, insertion impossible\n");
        return -1;
    }
    
    unsigned long clef = simple_hash(key);

    while (map->table[clef].key != NULL && map->table[clef].key != TOMBSTONE) {
        clef = (clef + 1) % TABLE_SIZE;
    }

    map->table[clef].key = strdup(key);
    if(map->table[clef].key == NULL){
        printf("Erreur d'allocation mémoire <hashmap_insert> \n");
        return -1;
    }
    map->table[clef].value = value;
    map->size++;

    return 0;  
}

/* Récupère une valeur associée à une clé dans la table */
void *hashmap_get(HashMap *map, const char *key){

    unsigned long clef = simple_hash(key);

    while (map->table[clef].key != NULL && map->table[clef].key != TOMBSTONE) {
        if (strcmp(map->table[clef].key, key) == 0) {
            return map->table[clef].value;
        }
        clef = (clef + 1) % TABLE_SIZE;
    }

    return NULL;
}

/* Supprime une entrée de la table de hachage */
int hashmap_remove(HashMap *map, const char *key) {
    if (map == NULL || key == NULL)
        return 0;

    unsigned long index = simple_hash(key);
    unsigned long start = index;

    while (map->table[index].key != NULL) {
        if (map->table[index].key != TOMBSTONE && strcmp(map->table[index].key, key) == 0) {
            free(map->table[index].key);
            map->table[index].key = NULL;
            map->table[index].value = TOMBSTONE;
            map->size--;
            return 1;
        }
        index = (index + 1) % TABLE_SIZE;
        if (index == start)
            break;
    }
    return 0;
}

/* Libère toute la mémoire occupée par la table de hachage */
void hashmap_destroy(HashMap *map) {
    if (map == NULL)
        return;

    for (int i = 0; i < TABLE_SIZE; i++) {
        if (map->table[i].key != NULL && map->table[i].key != TOMBSTONE) {
            free(map->table[i].key);
        }
    }
    free(map->table);
    free(map);
}