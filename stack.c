#include "stack.h"

#ifdef TEST

#include <assert.h>

STACK_IMPLEMENT(int, 4);

void test_Stack_startsEmpty() {
  intStack stack;
  intStack_init(&stack);

  assert(intStack_isEmpty(&stack));

  intStack_free(&stack);
}

void test_Stack_pushPop() {
  intStack stack;
  intStack_init(&stack);

  int toPush = 42;

  intStack_push(&stack, toPush);

  int popped = intStack_pop(&stack);

  assert(toPush == popped);

  intStack_free(&stack);
}

void test_Stack_peek() {
  intStack stack;
  intStack_init(&stack);

  int toPush = 42;

  intStack_push(&stack, toPush);

  int peeked = intStack_peek(&stack);
  int popped = intStack_pop(&stack);

  assert(peeked == popped);

  intStack_free(&stack);
}

#endif
