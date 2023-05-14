#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "instruction.h"

void InstructionList_init(InstructionList* self) {
  self->count = 0;
  self->capacity = 256;
  self->items = malloc(sizeof(uint8_t) * 256);
}

void InstructionList_free(InstructionList* self) {
  free(self->items);
}

inline static bool InstructionList_canInsert(InstructionList* self, size_t i) {
  // TODO Handle different item sizes before using this function
  // for other lists.
  return self->capacity <= self->count + i - 1;
}

inline static void InstructionList_grow(InstructionList* self) {
#define GROWTH_FACTOR 2
  self->capacity *= GROWTH_FACTOR;
  self->items = realloc(self->items, self->capacity * sizeof(uint8_t));

  // TODO Handle this
  assert(self->items != false);
#undef GROWTH_FACTOR
}

// TODO Can we inline this?
void InstructionList_append(InstructionList* self, uint8_t item) {
  if(InstructionList_canInsert(self, sizeof(uint8_t))) {
    InstructionList_grow(self);
  }
  self->items[self->count] = item;
  self->count++;
}

void InstructionList_appendInt32(InstructionList* self, int32_t i) {
  if(InstructionList_canInsert(self, sizeof(int32_t))) {
    InstructionList_grow(self);
  }

  // TODO Handle different item sizes before using this function
  // for other lists.
  *((int32_t*)&(self->items[self->count])) = i;
  self->count += sizeof(int32_t);
}

#ifdef TEST

#endif
