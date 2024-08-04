#ifndef VALUE_H
#define VALUE_H

#include <stdbool.h>

#include "obj.h"

typedef enum {
  VALUE_BOOLEAN,
  VALUE_INTEGER
} ValueType;

typedef struct {
  ValueType type;
  union {
    bool boolean;
    int32_t integer;
    Obj* obj;
  } as;
} Value;

#endif
