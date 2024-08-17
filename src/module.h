#ifndef MODULE_H
#define MODULE_H

#include "namespace.h"
#include "symbol_list.h"

typedef struct {
  char* canonicalPath;
  Namespace exports;
  SymbolList symbols;
} Mod;

#endif

