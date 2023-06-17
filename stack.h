#ifndef STACK_H
#define STACK_H

#include <assert.h>
#include <stdbool.h>

#include "value.h"

typedef struct {
  Value* maxTop;
  Value* items;
  Value* top;

  Value* currentScope;
  Value** scopes;
  uint8_t scopeCount;
  uint8_t scopeCapacity;
} Stack;

void Stack_init(Stack*);
void Stack_free(Stack*);

void Stack_openScope(Stack*);
void Stack_closeScope(Stack*);

inline static bool Stack_isEmpty(Stack* self) {
  return self->top < self->items;
}

inline static void Stack_push(Stack* self, Value item) {
  if(self->top == self->maxTop) {
    size_t topIndex = self->top - self->items;
    size_t capacity = (topIndex + 1) * 2;
    Value* oldItems = self->items;

    self->items = realloc(self->items, sizeof(Value) * capacity);
    assert(self->items != NULL);
    self->top = self->items + topIndex;
    self->maxTop = self->items + capacity - 1;

    self->currentScope = self->items + (self->currentScope - oldItems);

    for(size_t i = 0; i < self->scopeCount; i++) {
      self->scopes[i] = self->items + (self->scopes[i] - oldItems);
    }
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

inline static void Stack_popToIndex(Stack* self, size_t index) {
  assert(self->items + index <= self->top);
  Value_unreference(self->items + index);
  self->items[index] = Stack_pop(self);
}

inline static void Stack_println(Stack* self) {
  printf("[");
  bool first = true;
  for(Value* v = self->items; v <= self->top; v++) {
    if(first) {
      first = false;
    } else {
      printf(", ");
    }

    Value_print(*v);
  }

  printf("] top\n");
}

#ifdef TEST

void test_Stack_init_empty();
void test_Stack_lifo();
void test_Stack_pushIndex();
void test_Stack_scopes();

#endif

#endif
