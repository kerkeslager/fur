#ifndef STACK_H
#define STACK_H

#include <assert.h>
#include <stdbool.h>

#include "value.h"

typedef struct {
  size_t capacity;
  Value* items;
  size_t top;
} Stack;

void Stack_init(Stack*);
void Stack_free(Stack*);

inline static bool Stack_isEmpty(Stack* self) {
  return self->top == 0;
}

inline static void Stack_push(Stack* self, Value value) {
  if(self->top == self->capacity) {
    self->capacity *= 2;
    self->items = realloc(self->items, sizeof(Value) * self->capacity);
  }

  self->items[self->top++] = value;
}

inline static Value Stack_pop(Stack* self) {
  assert(!Stack_isEmpty(self));
  return self->items[--(self->top)];
}

inline static Value Stack_peek(Stack* self) {
  assert(!Stack_isEmpty(self));
  return self->items[self->top - 1];
}

#ifdef TEST

void test_Stack_startsEmpty();
void test_Stack_pushPop();

#endif

#endif
