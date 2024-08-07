#ifndef VALUE_STACK_H
#define VALUE_STACK_H

#include "memory.h"
#include "value.h"

#define VALUE_STACK_INIT_ALLOCATE 8

typedef struct {
  size_t height;
  size_t capacity;
  Value* items;
} ValueStack;

inline static void ValueStack_init(ValueStack* self) {
  self->height = 0;
  self->capacity = VALUE_STACK_INIT_ALLOCATE;
  self->items = Memory_allocate(sizeof(Value) * self->capacity);
}

inline static void ValueStack_free(ValueStack* self) {
  Memory_free(self->items);
}

inline static void ValueStack_push(ValueStack* self, Value v) {
  if(self->height == self->capacity) {
    assert(false); // TODO Implement resizing
  }

  assert(self->height < self->capacity);

  self->items[self->height++] = v;
}

inline static Value ValueStack_pop(ValueStack* self) {
  assert(self->height > 0);
  return self->items[--self->height];
}

inline static Value ValueStack_peek(ValueStack* self) {
  assert(self->height > 0);
  return self->items[self->height - 1];
}

#ifdef TEST
void test_ValueStack_initAndFree();
void test_ValueStack_pushPop();
void test_ValueStack_peek();
#endif

#endif
