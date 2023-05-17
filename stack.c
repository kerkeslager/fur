#include <stdlib.h>

#include "stack.h"

void Stack_init(Stack* self) {
  self->capacity = 8;
  self->items = malloc(sizeof(Value) * self->capacity);
  self->top = 0;
}

void Stack_free(Stack* self) {
  free(self->items);
}

#ifdef TEST

#include <assert.h>

void test_Stack_startsEmpty() {
  Stack stack;
  Stack_init(&stack);

  assert(Stack_isEmpty(&stack));

  Stack_free(&stack);
}

void test_Stack_pushPop() {
  Stack stack;
  Stack_init(&stack);

  Value toPush;
  toPush.type = VALUE_INTEGER;
  toPush.as.integer = 42;

  Stack_push(&stack, toPush);

  Value popped = Stack_pop(&stack);

  assert(toPush.type == popped.type);
  assert(toPush.as.integer == popped.as.integer);

  Stack_free(&stack);
}

void test_Stack_peek() {
  Stack stack;
  Stack_init(&stack);

  Value toPush;
  toPush.type = VALUE_INTEGER;
  toPush.as.integer = 42;

  Stack_push(&stack, toPush);

  Value peeked = Stack_peek(&stack);
  Value popped = Stack_pop(&stack);

  assert(peeked.type == popped.type);
  assert(peeked.as.integer == popped.as.integer);

  Stack_free(&stack);
}

Value _unaryFunction(Value v) {
  assert(v.type == VALUE_INTEGER);
  assert(v.as.integer == 42);

  Value testValue;
  testValue.type = VALUE_INTEGER;
  testValue.as.integer = 100;
  return testValue;
}

void test_Stack_unary() {
  Stack stack;
  Stack_init(&stack);

  Value toPush;
  toPush.type = VALUE_INTEGER;
  toPush.as.integer = 42;

  Stack_push(&stack, toPush);

  Stack_unary(&stack, _unaryFunction);

  Value popped = Stack_pop(&stack);

  assert(popped.type == VALUE_INTEGER);
  assert(popped.as.integer == 100);

  Stack_free(&stack);
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

void test_Stack_binary() {
  Stack stack;
  Stack_init(&stack);

  Value toPush;
  toPush.type = VALUE_INTEGER;
  toPush.as.integer = 42;
  Stack_push(&stack, toPush);

  toPush.type = VALUE_INTEGER;
  toPush.as.integer = 43;
  Stack_push(&stack, toPush);

  Stack_binary(&stack, _binaryFunction);

  Value popped = Stack_pop(&stack);

  assert(popped.type == VALUE_INTEGER);
  assert(popped.as.integer == 100);

  Stack_free(&stack);
}

#endif
