#ifndef VALUE_STACK_H
#define VALUE_STACK_H

#include "stack.h"
#include "value.h"

STACK_DECLARE(Value);

inline static void ValueStack_unary(ValueStack* self, Value (*apply)(Value)) {
  assert(self->top >= 1);
  self->items[self->top - 1] = apply(self->items[self->top - 1]);
}

inline static void ValueStack_binary(ValueStack* self, Value (*apply)(Value, Value)) {
  assert(self->top >= 2);
  self->top--;
  self->items[self->top - 1] = apply(self->items[self->top - 1], self->items[self->top]);
}

#ifdef TEST

void test_ValueStack_unary();
void test_ValueStack_binary();

#endif

#endif
