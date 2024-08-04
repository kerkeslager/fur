#ifndef MODULE_H
#define MODULE_H

#include "namespace.h"
#include "symbol_list.h"

typedef struct {
  char* path;
  Namespace exports;
  SymbolList symbols;
} Module;

#endif

