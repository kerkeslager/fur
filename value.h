#ifndef VALUE_H
#define VALUE_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef enum {
  VALUE_NIL,
  VALUE_INTEGER
} ValueType;

typedef struct {
  ValueType type;
  union {
    int32_t integer;
  } as;
} Value;

inline static Value Value_nil() {
  Value result;
  result.type = VALUE_NIL;
  return result;
}

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

inline static void Value_println(Value v) {
  switch(v.type) {
    case VALUE_NIL:
      printf("  nil\n");
      return;

    case VALUE_INTEGER:
      printf("  %i\n", v.as.integer);
      return;
  }

  assert(false);
}

#ifdef TEST

#endif

#endif
