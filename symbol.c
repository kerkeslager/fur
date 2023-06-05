#include <assert.h>
#include <stdbool.h>

#include "symbol.h"

bool SymbolList_find(SymbolList* self, size_t* out, Symbol* symbol) {
  for(size_t i = 0; i < self->length; i++) {
    if(self->items[i] == symbol) {
      *out = i;
      return true;
    }
  }

  return false;
}

bool SymbolList_append(SymbolList* self, Symbol* symbol) {
  size_t index;
  bool found = SymbolList_find(self, &index, symbol);
  if(found) return false;

  if(self->length == self->capacity) {
    if(self->capacity == 0) self->capacity = 16;
    else self->capacity *= 2;

    self->items = realloc(self->items, self->capacity * sizeof(Symbol*));

    // TODO Handle this better
    assert(self->items != false);
  }

  self->items[self->length++] = symbol;
  return true;
}

