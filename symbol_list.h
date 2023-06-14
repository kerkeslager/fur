#ifndef SYMBOL_LIST_H
#define SYMBOL_LIST_H

#include "symbol.h"

typedef struct {
  Symbol* symbol;
  size_t definedOnLine;
  bool isMutable;
} SymbolMetadata;

typedef struct {
  SymbolMetadata* items;
  uint16_t count;
  uint16_t capacity;
} SymbolList;

inline static void SymbolList_init(SymbolList* self) {
  self->items = NULL;
  self->count = 0;
  self->capacity = 0;
}

inline static void SymbolList_free(SymbolList* self) {
  free(self->items);
}

inline static size_t SymbolList_count(SymbolList* self) {
  return self->count;
}

inline static void SymbolList_rewind(SymbolList* self, size_t checkpoint) {
  self->count = checkpoint;
}

void SymbolList_append(SymbolList* self, Symbol* symbol, size_t definedOnLine, bool isMutable);
int32_t SymbolList_find(SymbolList* self, Symbol* symbol);

inline static bool SymbolList_isMutable(SymbolList* self, int32_t index) {
  return self->items[index].isMutable;
}

inline static size_t SymbolList_definedOnLine(SymbolList* self, int32_t index) {
  return self->items[index].definedOnLine;
}

#ifdef TEST

void test_SymbolList_init();
void test_SymbolList_append_many();
void test_SymbolList_append_allowsUpToUINT16_MAXsymbols();
void test_SymbolList_definedOnLine();
void test_SymbolList_isMutable();

#endif

#endif
