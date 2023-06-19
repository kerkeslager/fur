#ifndef VALUE_H
#define VALUE_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef enum {
  VALUE_BOOLEAN,
  VALUE_NATIVE_FN,
  VALUE_NIL,
  VALUE_INTEGER
} ValueType;

struct Value;
typedef struct Value Value;

typedef Value (*NativeFn)(uint8_t argc, Value* argv);

struct Value {
  ValueType type;
  union {
    bool boolean;
    NativeFn nativeFn;
    int32_t integer;
  } as;
};

static const Value NIL = { VALUE_NIL, { 0 } };
static const Value TRUE = { VALUE_BOOLEAN, { true } };
static const Value FALSE = { VALUE_BOOLEAN, { false } };

inline static Value Value_fromBoolean(bool b) {
  return b ? TRUE : FALSE;
}

inline static bool Value_asBoolean(Value v) {
  assert(v.type == VALUE_BOOLEAN);
  return v.as.boolean;
}

inline static Value Value_fromNativeFn(NativeFn nativeFn) {
  Value result;
  result.type = VALUE_NATIVE_FN;
  result.as.nativeFn = nativeFn;
  return result;
}

inline static NativeFn Value_asNativeFn(Value v) {
  assert(v.type == VALUE_NATIVE_FN);
  return v.as.nativeFn;
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

inline static void Value_print(Value v) {
  switch(v.type) {
    case VALUE_BOOLEAN:
      if(Value_asBoolean(v)) {
        printf("true");
      } else {
        printf("false");
      }
      return;

    case VALUE_NATIVE_FN:
      printf("<NativeFn@%p>", Value_asNativeFn(v));
      return;

    case VALUE_NIL:
      printf("nil");
      return;

    case VALUE_INTEGER:
      printf("%i", v.as.integer);
      return;
  }

  assert(false);
}

inline static void Value_println(Value v) {
  printf("  ");
  Value_print(v);
  printf("\n");
}

inline static Value Value_copy(Value* self) {
  /*
   * None of these types require updating reference counts, but future types
   * will.
   */
  switch(self->type) {
    case VALUE_BOOLEAN:
    case VALUE_NATIVE_FN:
    case VALUE_NIL:
    case VALUE_INTEGER:
      return *self;
  }

  // Should never get here
  assert(false);
  return NIL; // Silence warnings
}

inline static void Value_unreference(Value* self) {
  /*
   * None of these types require updating reference counts, but future types
   * will.
   */
  switch(self->type) {
    case VALUE_BOOLEAN:
    case VALUE_NATIVE_FN:
    case VALUE_NIL:
    case VALUE_INTEGER:
      break;
  }
}

#ifdef TEST

#endif

#endif
