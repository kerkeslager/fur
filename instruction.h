#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <stdint.h>
#include <stdlib.h>

typedef enum {
  OP_NIL,
  OP_TRUE,
  OP_FALSE,
  OP_INTEGER,
  OP_GET,
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
  OP_JUMP,
  OP_JUMP_TRUE,
  OP_JUMP_FALSE,
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
} ByteCode;

void ByteCode_init(ByteCode*);
void ByteCode_free(ByteCode*);
void ByteCode_append(ByteCode*, uint8_t, size_t line);
void ByteCode_appendInt16(ByteCode*, int16_t, size_t line);
void ByteCode_appendUInt16(ByteCode*, uint16_t, size_t line);
void ByteCode_appendInt32(ByteCode*, int32_t, size_t line);
size_t ByteCode_getLine(ByteCode*, uint8_t* instruction);

inline static size_t ByteCode_count(ByteCode* self) {
  return self->count;
}

void ByteCode_rewind(ByteCode* self, size_t count);

/*
 * The following functions allow us to maintain program counters when compiling
 * onto the end of an ByteCode that is already being referenced by a
 * thread. If ByteCode_append() resizes the ByteCode, the thread
 * pc is likely no longer valid. To solve this, we save the pc off as an index before
 * compilation, and then restore the pc using the index after compilation.
 */
inline static uint8_t* ByteCode_pc(ByteCode* self, size_t index) {
  return &(self->items[index]);
}

inline static size_t ByteCode_index(ByteCode* self, uint8_t* pc) {
  return pc - (self->items);
}

#ifdef TEST

void test_ByteCode_append_basic();
void test_ByteCode_append_lines();
void test_ByteCode_append_many();
void test_ByteCode_rewind_toLineBoundary();
void test_ByteCode_rewind_toMiddleOfLine();

#endif

#endif
