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

void test_Value_fromIntAndAsInt() {
  Value v = Value_fromInt(42);
  assert(v.type == VALUE_INTEGER);
  assert(v.as.integer == 42);
  assert(Value_asInt(v) == 42);
}
#endif
