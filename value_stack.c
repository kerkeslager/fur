#include <stdlib.h>

#include "value_stack.h"

STACK_IMPLEMENT(Value, 8);

#ifdef TEST

#include <assert.h>

Value _unaryFunction(Value v) {
  assert(v.type == VALUE_INTEGER);
  assert(v.as.integer == 42);

  Value testValue;
  testValue.type = VALUE_INTEGER;
  testValue.as.integer = 100;
  return testValue;
}

void test_ValueStack_unary() {
  ValueStack stack;
  ValueStack_init(&stack);

  Value toPush;
  toPush.type = VALUE_INTEGER;
  toPush.as.integer = 42;

  ValueStack_push(&stack, toPush);

  ValueStack_unary(&stack, _unaryFunction);

  Value popped = ValueStack_pop(&stack);

  assert(popped.type == VALUE_INTEGER);
  assert(popped.as.integer == 100);

  ValueStack_free(&stack);
}

Value _binaryFunction(Value v0, Value v1) {
  assert(v0.type == VALUE_INTEGER);
  assert(v0.as.integer == 42);
  assert(v1.type == VALUE_INTEGER);
  assert(v1.as.integer == 43);

  Value testValue;
  testValue.type = VALUE_INTEGER;
  testValue.as.integer = 100;
  return testValue;
}

void test_ValueStack_binary() {
  ValueStack stack;
  ValueStack_init(&stack);

  Value toPush;
  toPush.type = VALUE_INTEGER;
  toPush.as.integer = 42;
  ValueStack_push(&stack, toPush);

  toPush.type = VALUE_INTEGER;
  toPush.as.integer = 43;
  ValueStack_push(&stack, toPush);

  ValueStack_binary(&stack, _binaryFunction);

  Value popped = ValueStack_pop(&stack);

  assert(popped.type == VALUE_INTEGER);
  assert(popped.as.integer == 100);

  ValueStack_free(&stack);
}

#endif
