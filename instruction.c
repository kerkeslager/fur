#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "instruction.h"

void InstructionList_init(InstructionList* self) {
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

void InstructionList_free(InstructionList* self) {
  free(self->items);
  free(self->lineRuns);
}

inline static bool InstructionList_canInsert(InstructionList* self, size_t i) {
  // TODO Handle different item sizes before using this function
  // for other lists.
  return self->capacity > self->count + i - 1;
}

inline static void InstructionList_grow(InstructionList* self) {
#define GROWTH_FACTOR 2
  self->capacity *= GROWTH_FACTOR;
  self->items = realloc(self->items, self->capacity * sizeof(uint8_t));

  // TODO Handle this
  assert(self->items != NULL);
#undef GROWTH_FACTOR
}

inline static bool InstructionList_canInsertLineRun(InstructionList* self) {
  return self->lineRunCapacity > self->lineRunCount;
}

inline static void InstructionList_growLineRuns(InstructionList* self) {
#define GROWTH_FACTOR 2
  self->lineRunCapacity *= GROWTH_FACTOR;
  self->lineRuns = realloc(self->lineRuns, self->lineRunCapacity * sizeof(LineRun));

  // TODO Handle this
  assert(self->lineRuns != NULL);
#undef GROWTH_FACTOR
}

inline static void InstructionList_updateLineRuns(InstructionList* self, size_t line, size_t count) {
  /*
   * We don't have to check for lineRunCount == 0 here because we initialize
   * lineRuns with a single LineRun.
   */
  LineRun* currentLineRun = &(self->lineRuns[self->lineRunCount - 1]);

  if(currentLineRun->line == line) {
    currentLineRun->run += count;
    return;
  }

  if(!InstructionList_canInsertLineRun(self)) {
    InstructionList_growLineRuns(self);
  }

  LineRun newLineRun;
  newLineRun.line = line;
  newLineRun.run = count;

  self->lineRuns[self->lineRunCount] = newLineRun;
  self->lineRunCount++;
}

// TODO Can we inline this?
void InstructionList_append(InstructionList* self, uint8_t item, size_t line) {
  if(!InstructionList_canInsert(self, sizeof(uint8_t))) {
    InstructionList_grow(self);
  }
  self->items[self->count] = item;
  self->count++;

  InstructionList_updateLineRuns(self, line, 1);
}

void InstructionList_appendInt32(InstructionList* self, int32_t i, size_t line) {
  if(InstructionList_canInsert(self, sizeof(int32_t))) {
    InstructionList_grow(self);
  }

  // TODO Handle different item sizes before using this function
  // for other lists.
  *((int32_t*)&(self->items[self->count])) = i;
  self->count += sizeof(int32_t);

  InstructionList_updateLineRuns(self, line, sizeof(int32_t) / sizeof(uint8_t));
}

size_t InstructionList_getLine(InstructionList* self, uint8_t* instruction) {
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

#ifdef TEST

// TODO Oof, why didn't I write these originally?

#endif
