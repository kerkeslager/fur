#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <stdint.h>
#include <stdlib.h>

typedef enum {
  OP_NIL,
  OP_TRUE,
  OP_FALSE,
  OP_INTEGER,
  OP_NEGATE,
  OP_NOT,
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_IDIVIDE,
  OP_LESS_THAN,
  OP_LESS_THAN_EQUAL,
  OP_GREATER_THAN,
  OP_GREATER_THAN_EQUAL,
  OP_EQUAL,
  OP_NOT_EQUAL,
  OP_DROP,
  OP_RETURN,
} Instruction;

typedef struct {
  size_t line;
  size_t run;
} LineRun;

typedef struct {
  size_t count;
  size_t capacity;
  uint8_t* items;
  size_t lineRunCount;
  size_t lineRunCapacity;
  LineRun* lineRuns;
} InstructionList;

void InstructionList_init(InstructionList*);
void InstructionList_free(InstructionList*);
void InstructionList_append(InstructionList*, uint8_t, size_t line);
void InstructionList_appendInt32(InstructionList*, int32_t, size_t line);
size_t InstructionList_getLine(InstructionList*, uint8_t* instruction);

inline static size_t InstructionList_count(InstructionList* self) {
  return self->count;
}

/*
 * The following functions allow us to maintain program counters when compiling
 * onto the end of an InstructionList that is already being referenced by a
 * thread. If InstructionList_append() resizes the InstructionList, the thread
 * pc is likely no longer valid. To solve this, we save the pc off as an index before
 * compilation, and then restore the pc using the index after compilation.
 */
inline static uint8_t* InstructionList_pc(InstructionList* self, size_t index) {
  return &(self->items[index]);
}

inline static size_t InstructionList_index(InstructionList* self, uint8_t* pc) {
  return pc - (self->items);
}

#ifdef TEST

void test_InstructionList_append_basic();
void test_InstructionList_append_lines();

#endif

#endif
