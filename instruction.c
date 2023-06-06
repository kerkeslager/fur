#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "instruction.h"

void ByteCode_init(ByteCode* self) {
  self->count = 0;
  self->capacity = 256;
  self->items = malloc(sizeof(uint8_t) * self->capacity);

  /*
   * If there was a possibility of the lineRuns list being empty, we would have
   * to check for an empty list every time we checked the line. To avoid that,
   * we initialize the list with a single empty LineRun.
   */
  self->lineRunCount = 1;
  self->lineRunCapacity = 8;
  self->lineRuns = malloc(sizeof(LineRun) * self->lineRunCapacity);

  LineRun lineRun;
  lineRun.line = 1;
  lineRun.run = 0;

  self->lineRuns[0] = lineRun;
}

void ByteCode_free(ByteCode* self) {
  free(self->items);
  free(self->lineRuns);
}

inline static bool ByteCode_canInsert(ByteCode* self, size_t i) {
  // TODO Handle different item sizes before using this function
  // for other lists.
  return self->capacity > self->count + i - 1;
}

inline static void ByteCode_grow(ByteCode* self) {
#define GROWTH_FACTOR 2
  self->capacity *= GROWTH_FACTOR;
  self->items = realloc(self->items, self->capacity * sizeof(uint8_t));

  // TODO Handle this
  assert(self->items != NULL);
#undef GROWTH_FACTOR
}

inline static bool ByteCode_canInsertLineRun(ByteCode* self) {
  return self->lineRunCapacity > self->lineRunCount;
}

inline static void ByteCode_growLineRuns(ByteCode* self) {
#define GROWTH_FACTOR 2
  self->lineRunCapacity *= GROWTH_FACTOR;
  self->lineRuns = realloc(self->lineRuns, self->lineRunCapacity * sizeof(LineRun));

  // TODO Handle this
  assert(self->lineRuns != NULL);
#undef GROWTH_FACTOR
}

inline static void ByteCode_updateLineRuns(ByteCode* self, size_t line, size_t count) {
  /*
   * We don't have to check for lineRunCount == 0 here because we initialize
   * lineRuns with a single LineRun.
   */
  LineRun* currentLineRun = &(self->lineRuns[self->lineRunCount - 1]);

  if(currentLineRun->line == line) {
    currentLineRun->run += count;
    return;
  }

  if(!ByteCode_canInsertLineRun(self)) {
    ByteCode_growLineRuns(self);
  }

  LineRun newLineRun;
  newLineRun.line = line;
  newLineRun.run = count;

  self->lineRuns[self->lineRunCount] = newLineRun;
  self->lineRunCount++;
}

// TODO Can we inline this?
void ByteCode_append(ByteCode* self, uint8_t item, size_t line) {
  if(!ByteCode_canInsert(self, sizeof(uint8_t))) {
    ByteCode_grow(self);
  }
  self->items[self->count] = item;
  self->count++;

  ByteCode_updateLineRuns(self, line, 1);
}

void ByteCode_appendUInt16(ByteCode* self, uint16_t i, size_t line) {
  if(!ByteCode_canInsert(self, sizeof(uint16_t))) {
    ByteCode_grow(self);
  }

  *((uint16_t*)&(self->items[self->count])) = i;
  self->count += sizeof(uint16_t);

  ByteCode_updateLineRuns(self, line, sizeof(uint16_t) / sizeof(uint8_t));
}

void ByteCode_appendInt32(ByteCode* self, int32_t i, size_t line) {
  if(!ByteCode_canInsert(self, sizeof(int32_t))) {
    ByteCode_grow(self);
  }

  *((int32_t*)&(self->items[self->count])) = i;
  self->count += sizeof(int32_t);

  ByteCode_updateLineRuns(self, line, sizeof(int32_t) / sizeof(uint8_t));
}

size_t ByteCode_getLine(ByteCode* self, uint8_t* instruction) {
  assert(instruction >= self->items);
  assert(instruction < self->items + self->count);

  uint8_t* instructionCounter = self->items;

  for(size_t i = 0; i < self->lineRunCount; i++) {
    instructionCounter += self->lineRuns[i].run;

    /*
     * If we pass the end of the list, that means we somehow emitted too many
     * or too long of line runs for the instructions.
     */
    assert(instructionCounter <= self->items + self->count);

    if(instructionCounter > instruction) {
      return self->lineRuns[i].line;
    }
  }

  /*
   * If we don't find the instruction before we run out of lineRuns, that means
   * we somehow didn't emit a lineRun for one of the instructions we inserted
   * into the list.
   */
  assert(false);
}

void ByteCode_rewind(ByteCode* self, size_t count) {
  size_t run = 0;

  for(size_t i = 0; i < self->lineRunCount; i++) {
    run += self->lineRuns[i].run;

    if(run >= count) {
      self->count = count;
      self->lineRuns[i].run -= run - count;
      self->lineRunCount = i + 1;
      return;
    }
  }

  assert(count == self->count);
}

#ifdef TEST

void test_ByteCode_append_basic() {
  ByteCode byteCode;
  ByteCode_init(&byteCode);

  ByteCode_append(&byteCode, OP_RETURN, 1);

  assert(ByteCode_count(&byteCode) == 1);
  assert(*ByteCode_pc(&byteCode, 0) == OP_RETURN);

  ByteCode_free(&byteCode);
}

void test_ByteCode_append_lines() {
  ByteCode byteCode;
  ByteCode_init(&byteCode);

  ByteCode_append(&byteCode, OP_RETURN, 42);

  assert(ByteCode_getLine(&byteCode, ByteCode_pc(&byteCode, 0)) == 42);

  ByteCode_free(&byteCode);
}

void test_ByteCode_append_many() {
  ByteCode byteCode;
  ByteCode_init(&byteCode);

  for(int i = 0; i < 1000; i++) {
    ByteCode_append(&byteCode, i % 256, i / 10);
  }

  assert(ByteCode_count(&byteCode) == (size_t)1000);

  for(int i = 0; i < 1000; i++) {
    uint8_t* instruction = ByteCode_pc(&byteCode, i);
    assert(*instruction == i % 256);
    assert(ByteCode_getLine(&byteCode, instruction) == (size_t)(i / 10));

    /*
     * This could be a separate test--tests that instruction/pc are inverses
     * of each other.
     */
    assert(ByteCode_index(&byteCode, instruction) == (size_t)i);
  }

  ByteCode_free(&byteCode);
}

void test_ByteCode_rewind_toLineBoundary() {
  ByteCode byteCode;
  ByteCode_init(&byteCode);

  for(int i = 0; i < 10; i++) {
    ByteCode_append(&byteCode, i, 1);
  }

  size_t checkpoint = ByteCode_count(&byteCode);

  for(int i = 0; i < 10; i++) {
    ByteCode_append(&byteCode, i + 10, 2);
  }

  assert(ByteCode_count(&byteCode) == (size_t)20);

  ByteCode_rewind(&byteCode, checkpoint);

  assert(ByteCode_count(&byteCode) == (size_t)10);

  for(int i = 0; i < 10; i++) {
    ByteCode_append(&byteCode, i + 20, 3);
  }

  assert(ByteCode_count(&byteCode) == (size_t)20);

  uint8_t* instruction = ByteCode_pc(&byteCode, checkpoint);

  for(int i = 0; i < 10; i++) {
    assert(*instruction == i + 20);
    assert(ByteCode_getLine(&byteCode, instruction) == 3);
    instruction++;
  }

  ByteCode_free(&byteCode);
}

void test_ByteCode_rewind_toMiddleOfLine() {
  ByteCode byteCode;
  ByteCode_init(&byteCode);

  for(int i = 0; i < 10; i++) {
    ByteCode_append(&byteCode, i, 1);
  }

  for(int i = 0; i < 5; i++) {
    ByteCode_append(&byteCode, i + 10, 2);
  }

  size_t checkpoint = ByteCode_count(&byteCode);

  for(int i = 5; i < 10; i++) {
    ByteCode_append(&byteCode, i + 10, 2);
  }

  assert(ByteCode_count(&byteCode) == (size_t)20);

  ByteCode_rewind(&byteCode, checkpoint);

  assert(ByteCode_count(&byteCode) == (size_t)15);

  for(int i = 0; i < 10; i++) {
    ByteCode_append(&byteCode, i + 20, 3);
  }

  assert(ByteCode_count(&byteCode) == (size_t)25);

  uint8_t* instruction = ByteCode_pc(&byteCode, checkpoint);

  for(int i = 0; i < 10; i++) {
    assert(*instruction == i + 20);
    assert(ByteCode_getLine(&byteCode, instruction) == 3);
    instruction++;
  }

  ByteCode_free(&byteCode);
}

#endif
