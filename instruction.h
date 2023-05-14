#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <stdint.h>
#include <stdlib.h>

typedef enum {
  OP_INTEGER,
  OP_NEGATE,
  OP_RETURN,
} Instruction;

typedef struct {
  size_t count;
  size_t capacity;
  uint8_t* items;
} InstructionList;

void InstructionList_init(InstructionList*);
void InstructionList_free(InstructionList*);
void InstructionList_append(InstructionList*, uint8_t);
void InstructionList_appendInt32(InstructionList*, int32_t);

#ifdef TEST

#endif

#endif
