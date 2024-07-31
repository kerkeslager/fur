#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdint.h>
#include <stdlib.h>

typedef struct {
  const char* text;
  size_t length;
  uint32_t hash;
} Symbol;

Symbol* Symbol_new(const char* text, size_t length, uint32_t hash);
void Symbol_del(Symbol* self);

#ifdef TEST

#endif

#endif
