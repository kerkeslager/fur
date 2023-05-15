#ifndef VALUE_H
#define VALUE_H

#include <stdint.h>

typedef enum {
  VALUE_INTEGER
} ValueType;

typedef struct {
  ValueType type;
  union {
    int32_t integer;
  } as;
} Value;

#ifdef TEST

#endif

#endif
