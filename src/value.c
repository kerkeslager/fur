#include "value.h"

#ifdef TEST
void test_Value_fromIntAndAsInt() {
  Value v = Value_fromInt(42);
  assert(v.type == VALUE_INTEGER);
  assert(v.as.integer == 42);
  assert(Value_asInt(v) == 42);
}
#endif
