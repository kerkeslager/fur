#include "symbol.h"

#include <assert.h>

Symbol* Symbol_new(const char* text, size_t length, uint32_t hash) {
  Symbol* result = malloc(sizeof(Symbol));
  result->text = text;
  result->length = length;
  result->hash = hash;
  return result;
}

void Symbol_del(Symbol* self) {
  assert(self != NULL);
  free(self);
}

#if TEST

#endif
