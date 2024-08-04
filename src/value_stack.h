#ifndef VALUE_STACK_H
#define VALUE_STACK_H

typedef struct {
  size_t height;
  size_t capacity;
  Value* items;
} ValueStack;

#endif
