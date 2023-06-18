#ifndef SYMBOL_LIST_H
#define SYMBOL_LIST_H

#include "symbol.h"

typedef struct {
  Symbol* symbol;
  size_t definedOnLine;
  bool isMutable;
} SymbolMetadata;

typedef enum {
  SCOPE_GENERIC,
  SCOPE_BREAKABLE
} ScopeType;

typedef struct {
  size_t start;
  uint16_t checkpoint;
  ScopeType type;
} CompilationScope;

typedef struct {
  SymbolMetadata* items;
  uint16_t count;
  uint16_t capacity;

  CompilationScope scopes[256];
  uint8_t scopeDepth;
} SymbolList;

inline static void SymbolList_init(SymbolList* self) {
  self->items = NULL;
  self->count = 0;
  self->capacity = 0;

  self->scopeDepth = 0;
}

inline static void SymbolList_free(SymbolList* self) {
  free(self->items);
}

inline static size_t SymbolList_count(SymbolList* self) {
  return self->count;
}

inline static void SymbolList_rewind(SymbolList* self, size_t checkpoint) {
  self->count = checkpoint;

  /*
   * Rewind is only called from "in the global scope", i.e. Compiler_compile
   * assumes we're parsing an outermost statement. Therefore, we can do this:
   */
  self->scopeDepth = 0;
}

inline static void SymbolList_openScope(SymbolList* self, ScopeType type, size_t start) {
  // TODO Handle this
  assert(self->scopeDepth < UINT8_MAX);

  CompilationScope scope;
  scope.start = start;
  scope.checkpoint = self->count;
  scope.type = type;

  self->scopes[self->scopeDepth] = scope;
  self->scopeDepth++;
}

inline static void SymbolList_closeScope(SymbolList* self) {
  // This would mean we're closing a scope we never opened
  assert(self->scopeDepth > 0);

  self->scopeDepth--;
  CompilationScope scope = self->scopes[self->scopeDepth];
  self->count = scope.checkpoint;
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
