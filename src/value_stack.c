#include "value_stack.h"

#ifdef TEST
void test_ValueStack_initAndFree() {
  ValueStack stack;
  Value stackItems[10];
  Test_init(stackItems);

  ValueStack_init(&stack);

  assert(stack.height == 0);
  assert(stack.capacity == VALUE_STACK_INIT_ALLOCATE);
  assert(stack.items == stackItems);
  assert(Test_getAllocationCount() == 1);
  assert(Test_getAllocation(0) == sizeof(Value) * VALUE_STACK_INIT_ALLOCATE);

  ValueStack_free(&stack);

  assert(Test_getFreeCount() == 1);
  assert(Test_getFree(0) == stackItems);
}

void test_ValueStack_pushPop() {
  ValueStack stack;
  Value stackItems[10];
  Test_init(stackItems);

  ValueStack_init(&stack);

  ValueStack_push(&stack, Value_fromInt(42));
  assert(stack.height == 1);
  assert(Value_asInt(ValueStack_pop(&stack)) == 42);
  assert(stack.height == 0);

  ValueStack_free(&stack);
}
#endif
