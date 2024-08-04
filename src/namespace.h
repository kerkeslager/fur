#ifndef NAMESPACE_H
#define NAMESPACE_H

#include "value.h"

typedef struct {
  Symbol* symbol;
  Value* value;
} NamespaceKeyValuePair;

typedef struct {
  size_t capacity;
  size_t load;
  NamespaceKeyValuePair* items;
} Namespace;

#endif
