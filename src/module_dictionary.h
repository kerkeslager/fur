#ifndef MODULE_DICTIONARY_H
#define MODULE_DICTIONARY_H

#include "module.h"

typedef struct {
  size_t capacity;
  size_t load;
  Mod* items;
} ModDictionary;

#endif
