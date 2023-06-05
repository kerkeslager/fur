#ifndef STACK_H
#define STACK_H

#include <assert.h>
#include <stdbool.h>

#include "value.h"

typedef struct {
  Value* maxTop;
  Value* items;
  Value* top;
} Stack;

void Stack_init(Stack*);
void Stack_free(Stack*);

inline static bool Stack_isEmpty(Stack* self) {
  return self->top < self->items;
}

inline static void Stack_push(Stack* self, Value item) {
  if(self->top == self->maxTop) {
    size_t topIndex = self->top - self->items;
    size_t capacity = (topIndex + 1) * 2;

    self->items = realloc(self->items, sizeof(Value) * capacity);
    assert(self->items != NULL);
    self->top = self->items + topIndex;
    self->maxTop = self->items + capacity - 1;
  }

  *(++(self->top)) = item;
}

inline static Value Stack_pop(Stack* self) {
  assert(!Stack_isEmpty(self));

  return *(self->top--);
}

inline static void Stack_pushIndex(Stack* self, size_t index) {
  assert(self->items + index <= self->top);
  Stack_push(self, Value_copy(self->items + index));
}

#ifdef TEST

void test_Stack_init_empty();
void test_Stack_lifo();
void test_Stack_pushIndex();

#endif

#endif
