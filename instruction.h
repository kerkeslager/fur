#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <stdint.h>
#include <stdlib.h>

typedef enum {
  OP_EXIT
} Instruction;

typedef struct {
  size_t count;
  size_t capacity;
  uint8_t* items;
} InstructionList;

void InstructionList_init(InstructionList*);
void InstructionList_free(InstructionList*);

#ifdef TEST

#endif

#endif
