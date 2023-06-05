#ifndef SYMBOL_LIST_H
#define SYMBOL_LIST_H

#include "symbol.h"

typedef struct {
  Symbol** items;
  uint16_t length;
  uint16_t capacity;
} SymbolList;

inline static void SymbolList_init(SymbolList* self) {
  self->items = NULL;
  self->length = 0;
  self->capacity = 0;
}

inline static void SymbolList_free(SymbolList* self) {
  free(self->items);
}

void SymbolList_append(SymbolList* self, Symbol* symbol);
int32_t SymbolList_find(SymbolList* self, Symbol* symbol);

#ifdef TEST

void test_SymbolList_init();
void test_SymbolList_append_many();
void test_SymbolList_append_allowsUpToUINT16_MAXsymbols();

#endif

#endif
