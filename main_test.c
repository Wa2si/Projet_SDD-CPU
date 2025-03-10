#include "memory.h"

int main(){

    MemoryHandler *handler = memory_init(1024);
    if(handler != NULL){
        printf("Mémoire initialisée avec %d unités\n",handler->total_size);
    }

    
    return 0;
}