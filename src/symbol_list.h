#ifndef SYMBOL_LIST_H
#define SYMBOL_LIST_H

#include "symbol.h"

typedef struct {
  size_t length;
  size_t capacity;
  Symbol** items;
} SymbolList;

#endif
