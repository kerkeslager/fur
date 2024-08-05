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

#ifdef TEST
void test_ValueStack_initAndFree();
#endif

#endif
