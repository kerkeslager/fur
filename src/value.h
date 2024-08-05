#ifndef VALUE_H
#define VALUE_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

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

inline static Value Value_fromInt(int32_t v) {
  Value result = {
    .type = VALUE_INTEGER,
    .as.integer = v
  };
  return result;
}

inline static int32_t Value_asInt(Value v) {
  assert(v.type == VALUE_INTEGER);
  return v.as.integer;
}

#ifdef TEST
void test_Value_fromIntAndAsInt();
#endif

#endif
