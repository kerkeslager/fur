#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdlib.h>

typedef struct {
  const char* text;
  size_t length;
  uint32_t hash;
} Symbol;

inline static Symbol* Symbol_new(const char* text, size_t length, uint32_t hash) {
  Symbol* result = malloc(sizeof(Symbol));
  result->text = text;
  result->length = length;
  result->hash = hash;
  return result;
}

inline static void Symbol_destroy(Symbol* self) {
  assert(self != NULL);
  free(self);
}

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
    if(self->items[i] != NULL) Symbol_destroy(self->items[i]);
  }

  free(self->items);
}

Symbol* SymbolTable_getOrCreate(SymbolTable*, const char*, size_t);


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

void test_SymbolTable_init();
void test_SymbolTable_addFirst();
void test_SymbolTable_sameSymbolsSamePointers();
void test_SymbolTable_differentSymbolsDifferentPointers();
void test_SymbolTable_growth();

#endif

#endif
