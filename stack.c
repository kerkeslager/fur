#include <stdlib.h>

#include "stack.h"

void Stack_init(Stack* self) {
  self->items = malloc(sizeof(Value) * 8);
  self->top = self->items - 1;
  self->maxTop = self->items + 7;
}

void Stack_free(Stack* self) {
  free(self->items);
}

#ifdef TEST

void test_Stack_init_empty() {
  Stack stack;
  Stack_init(&stack);

  assert(Stack_isEmpty(&stack));

  Stack_free(&stack);
}

void test_Stack_lifo() {
  Stack stack;
  Stack_init(&stack);

  for(int i = 999; i >= 0; i--) {
    Stack_push(&stack, Value_fromInteger(i));
  }

  for(int i = 0; i < 1000; i++) {
    assert(Value_asInteger(Stack_pop(&stack)) == i);
  }

  assert(Stack_isEmpty(&stack));

  Stack_free(&stack);
}

void test_Stack_pushIndex() {
  Stack stack;
  Stack_init(&stack);

  for(int i = 0; i < 1000; i++) {
    Stack_push(&stack, Value_fromInteger(i));
  }

  for(int i = 0; i < 1000; i++) {
    Stack_pushIndex(&stack, i);
    assert(Value_asInteger(Stack_pop(&stack)) == i);
  }

  assert(!Stack_isEmpty(&stack));

  Stack_free(&stack);
}

#endif
