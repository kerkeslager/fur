#include <stdlib.h>

#include "stack.h"

void Stack_init(Stack* self) {
  /*
   * We initialize items with a small array, because the vast majority of
   * stack usages will use items. REPL sessions will typically only use
   * a few, and this adds to thread size, so we initialize it with a small
   * number (8) to keep minimum per-thread memory low.
   */
  self->items = malloc(sizeof(Value) * 8);
  self->top = self->items - 1;
  self->maxTop = self->items + 7;

  /*
   * REPL sessions may never open a scope, so we initialize scopes with NULL.
   * This additionally reduces minimum per-thread memory usage.
   */
  self->currentScope = self->items;
  self->scopes = NULL;
  self->scopeCount = 0;
  self->scopeCapacity = 0;
}

void Stack_free(Stack* self) {
  free(self->items);
  if(self->scopes != NULL) free(self->scopes);
}

void Stack_openScope(Stack* self) {
  if(self->scopeCount == self->scopeCapacity) {
    // TODO Handle this
    assert(self->scopeCapacity < UINT8_MAX);

    uint16_t newCapacity = (uint16_t)(self->scopeCapacity) * 1.25;

    if(self->scopeCapacity == 0) {
      self->scopeCapacity = 8;
    } else {
      if(newCapacity > UINT8_MAX) newCapacity = UINT8_MAX;
    }

    self->scopeCapacity = newCapacity;
    self->scopes = realloc(self->scopes, sizeof(Value*) * self->scopeCapacity);

    // TODO Handle this
    assert(self->scopes != NULL);
  }

  self->scopes[self->scopeCount] = self->currentScope;
  self->scopeCount++;
  self->currentScope = self->top + 1;
}

void Stack_closeScope(Stack* self) {
  /*
   * At least one value must have been placed on the stack during the scope,
   * otherwise the scope has nothing to return.
   */
  assert(self->currentScope <= self->top);

  for(Value* v = self->currentScope; v < self->top; v++) {
    Value_unreference(v);
  }

  *(self->currentScope) = *(self->top);
  self->top = self->currentScope;
  self->scopeCount--;
  self->currentScope = self->scopes[self->scopeCount];
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

void test_Stack_scopes() {
  Stack stack;
  Stack_init(&stack);

  Stack_push(&stack, Value_fromInteger(1));

  Stack_openScope(&stack);
  Stack_push(&stack, Value_fromInteger(2));
  Stack_push(&stack, Value_fromInteger(3));
  Stack_push(&stack, Value_fromInteger(4));
  Stack_push(&stack, Value_fromInteger(5));
  Stack_closeScope(&stack);

  assert(Value_asInteger(Stack_pop(&stack)) == 5);
  assert(Value_asInteger(Stack_pop(&stack)) == 1);
  assert(Stack_isEmpty(&stack));

  Stack_free(&stack);
}

#endif
