#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "symbol.h"

typedef struct{
  Symbol** items;
  size_t load;
  size_t capacity;
} SymbolTable;

inline static void SymbolTable_init(SymbolTable* self) {
  self->items = NULL;
  self->load = 0;
  self->capacity = 0;
}

inline static void SymbolTable_free(SymbolTable* self) {
  if(self->capacity == 0) return;

  assert(self->items != NULL);

  for(size_t i = 0; i < self->capacity; i++) {
    if(self->items[i] != NULL) Symbol_del(self->items[i]);
  }

  free(self->items);
}

Symbol* SymbolTable_getOrCreate(SymbolTable*, const char*, size_t);

#ifdef TEST

void test_SymbolTable_init();
void test_SymbolTable_addFirst();
void test_SymbolTable_sameSymbolsSamePointers();
void test_SymbolTable_differentSymbolsDifferentPointers();
void test_SymbolTable_growth();

#endif

#endif
