#ifndef PARSER_H
#define PARSER_H

#include <stdlib.h>
#include <stdio.h>
#include "hash.h"
#include "memory.h"

typedef struct {
    char *mnemonic;
    char *operand1;
    char *operand2;
} Instruction;

typedef struct {
    Instruction **data_instructions;
    int data_count;
    Instruction **code_instructions;
    int code_count;
    HashMap *labels;
    HashMap *memory_locations;
} ParserResult;

Instruction *parse_data_instruction(const char *line, HashMap *memory_locations);

#endif