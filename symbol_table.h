#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "symbol.h"

typedef struct{
  Symbol** items;
  size_t load;
  size_t capacity;
} SymbolTable;

void SymbolTable_init(SymbolTable*);
void SymbolTable_free(SymbolTable*);

Symbol* SymbolTable_getOrCreate(SymbolTable*, const char*, size_t);

#ifdef TEST

void test_SymbolTable_init();
void test_SymbolTable_addFirst();
void test_SymbolTable_sameSymbolsSamePointers();
void test_SymbolTable_differentSymbolsDifferentPointers();
void test_SymbolTable_growth();

#endif

#endif
