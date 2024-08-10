#include "value.h"

#ifdef TEST
void test_Value_isNil() {
  Value v;
  v.type = VALUE_OBJ;

  v.as.obj = NULL;
  assert(Value_isNil(v));

  v.as.obj = (Obj*)(&v);
  assert(!Value_isNil(v));
}

void test_Value_fromBoolAndAsBool() {
  Value trueValue = Value_fromBool(true);
  assert(trueValue.type == VALUE_BOOLEAN);
  assert(trueValue.as.boolean == true);
  assert(Value_asBool(trueValue));

  Value falseValue = Value_fromBool(false);
  assert(falseValue.type == VALUE_BOOLEAN);
  assert(falseValue.as.boolean == false);
  assert(!Value_asBool(falseValue));
}

void test_Value_fromIntAndAsInt() {
  Value v = Value_fromInt(42);
  assert(v.type == VALUE_INTEGER);
  assert(v.as.integer == 42);
  assert(Value_asInt(v) == 42);
}
#endif
