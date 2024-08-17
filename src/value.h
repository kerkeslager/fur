#ifndef VALUE_H
#define VALUE_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "common.h"
#include "module.h"
#include "obj.h"

typedef enum {
  VALUE_BOOLEAN,
  VALUE_INTEGER,
  VALUE_MODULE,
  VALUE_OBJ
} ValueType;

struct Value {
  ValueType type;
  union {
    bool boolean;
    int32_t integer;
    Mod* mod;
    Obj* obj;
  } as;
};

inline static Value Value_fromBool(bool b) {
  Value result = {
    .type = VALUE_BOOLEAN,
    .as.boolean = b
  };
  return result;
}

inline static bool Value_asBool(Value v) {
  assert(v.type == VALUE_BOOLEAN);
  return v.as.boolean;
}

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

inline static bool Value_isNil(Value v) {
  assert(v.type == VALUE_OBJ);
  return v.as.obj == NULL;
}

#ifdef TEST
void test_Value_isNil();
void test_Value_fromBoolAndAsBool();
void test_Value_fromIntAndAsInt();
#endif

#endif
