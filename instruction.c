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

#ifdef TEST

#endif
