#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdbool.h>
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

inline static void Symbol_del(Symbol* self) {
  assert(self != NULL);
  free(self);
}

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

#endif
