#ifndef SYMBOL_SET_H
#define SYMBOL_SET_H

#include "symbol.h"

typedef struct {
  size_t capacity;
  size_t load;
  Symbol** items;
} SymbolSet;

#endif
