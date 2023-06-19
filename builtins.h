#ifndef BUILTINS_H
#define BUILTINS_H

#include <stdio.h>
#include <string.h>

#include "value.h"

static Value Builtin_print(uint8_t argc, Value* argv) {
  for(size_t i = 0; i < argc; i++) {
    if(i != 0) printf(" ");
    Value_print(argv[i]);
  }

  fflush(stdout);

  return NIL;
}

static Value Builtin_Bool(uint8_t argc, Value* argv) {
  assert(argc == 1);

  Value arg0 = argv[0];

  switch(arg0.type) {
    case VALUE_BOOLEAN:
      return arg0;

    case VALUE_NATIVE_FN:
      return TRUE;

    case VALUE_NIL:
      return FALSE;

    case VALUE_INTEGER:
      return Value_fromBoolean(Value_asInteger(arg0) != 0);
  }

  // Should never happen
  assert(false);
  return NIL;
}

static Value Builtin_Int(uint8_t argc, Value* argv) {
  assert(argc == 1);

  Value arg0 = argv[0];

  switch(arg0.type) {
    case VALUE_BOOLEAN:
      return Value_fromInteger(Value_asBoolean(arg0) ? 1 : 0);

    case VALUE_NATIVE_FN:
      // TODO Handle this better
      assert(false);
      return NIL;

    case VALUE_NIL:
      return Value_fromInteger(0);

    case VALUE_INTEGER:
      return arg0;
  }

  // Should never happen
  assert(false);
  return NIL;
}

static Value Builtin_println(uint8_t argc, Value* argv) {
  Builtin_print(argc, argv);
  printf("\n");
  return NIL;
}

typedef struct {
  const char* const name;
  const Value value;
} BuiltinValue;

#define BUILTINS_COUNT 4

static const BuiltinValue BUILTINS[BUILTINS_COUNT] = {
  { "Bool", { VALUE_NATIVE_FN, { .nativeFn=Builtin_Bool } } },
  { "Int", { VALUE_NATIVE_FN, { .nativeFn=Builtin_Int } } },
  { "print", { VALUE_NATIVE_FN, { .nativeFn=Builtin_print } } },
  { "println", { VALUE_NATIVE_FN, { .nativeFn=Builtin_println } } },
};

inline static int32_t Builtin_index(const char* name, size_t length) {
  for(int i = 0; i < BUILTINS_COUNT; i++) {
    if(!strncmp(BUILTINS[i].name, name, length)
        && strlen(BUILTINS[i].name) == length) {
      return i;
    }
  }
  return -1;
}

#endif
