#ifndef VALUE_H
#define VALUE_H

#include <assert.h>
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

inline static Value Value_fromInteger(int32_t i) {
  Value result;
  result.type = VALUE_INTEGER;
  result.as.integer = i;
  return result;
}

inline static int32_t Value_asInteger(Value v) {
  assert(v.type == VALUE_INTEGER);
  return v.as.integer;
}

#ifdef TEST

#endif

#endif
