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

#endif
