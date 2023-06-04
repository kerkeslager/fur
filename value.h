#ifndef VALUE_H
#define VALUE_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef enum {
  VALUE_BOOLEAN,
  VALUE_NIL,
  VALUE_INTEGER
} ValueType;

typedef struct {
  ValueType type;
  union {
    bool boolean;
    int32_t integer;
  } as;
} Value;

const static Value NIL = { VALUE_NIL, { 0 } };
const static Value TRUE = { VALUE_BOOLEAN, { true } };
const static Value FALSE = { VALUE_BOOLEAN, { false } };

inline static Value Value_fromBoolean(bool b) {
  return b ? TRUE : FALSE;
}

inline static bool Value_asBoolean(Value v) {
  assert(v.type == VALUE_BOOLEAN);
  return v.as.boolean;
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
    case VALUE_BOOLEAN:
      if(Value_asBoolean(v)) {
        printf("  true\n");
      } else {
        printf("  false\n");
      }
      return;

    case VALUE_NIL:
      printf("  nil\n");
      return;

    case VALUE_INTEGER:
      printf("  %i\n", v.as.integer);
      return;
  }

  assert(false);
}

inline static Value Value_copy(Value* self) {
  /*
   * None of these types require updating reference counts, but future types
   * will.
   */
  switch(self->type) {
    case VALUE_BOOLEAN:
    case VALUE_NIL:
    case VALUE_INTEGER:
      return *self;
  }
}

#ifdef TEST

#endif

#endif
