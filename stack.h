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
    assert(self->items != NULL);
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

inline static void Stack_unary(Stack* self, Value (*apply)(Value)) {
  assert(self->top >= 1);
  self->items[self->top - 1] = apply(self->items[self->top - 1]);
}

inline static void Stack_binary(Stack* self, Value (*apply)(Value, Value)) {
  assert(self->top >= 2);
  self->top--;
  self->items[self->top - 1] = apply(self->items[self->top - 1], self->items[self->top]);
}

#ifdef TEST

void test_Stack_startsEmpty();
void test_Stack_pushPop();
void test_Stack_peek();
void test_Stack_unary();
void test_Stack_binary();

#endif

#endif
