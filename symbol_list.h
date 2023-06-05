#ifndef SYMBOL_LIST_H
#define SYMBOL_LIST_H

#include "symbol.h"

typedef struct {
  Symbol** items;
  size_t length;
  size_t capacity;
} SymbolList;

inline static void SymbolList_init(SymbolList* self) {
  self->items = NULL;
  self->length = 0;
  self->capacity = 0;
}

inline static void SymbolList_free(SymbolList* self) {
  free(self->items);
}

bool SymbolList_append(SymbolList* self, Symbol* symbol);
bool SymbolList_find(SymbolList* self, size_t* out, Symbol* symbol);

#ifdef TEST

#endif

#endif
