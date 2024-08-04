#ifndef MODULE_DICTIONARY_H
#define MODULE_DICTIONARY_H

#include "module.h"

typedef struct {
  size_t capacity;
  size_t load;
  Module* items;
} ModuleDictionary;

#endif
