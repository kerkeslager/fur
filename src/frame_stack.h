#ifndef FRAME_STACK_H
#define FRAME_STACK_H

#include "frame.h"
#include "memory.h"

#include <assert.h>
#include <stdbool.h>

#define FRAME_STACK_INIT_ALLOCATE 8

typedef struct {
  size_t height;
  size_t capacity;
  Frame* items;
} FrameStack;

inline static void FrameStack_init(FrameStack* self) {
  self->height = 0;
  self->capacity = FRAME_STACK_INIT_ALLOCATE;
  self->items = Memory_allocate(sizeof(Frame) * self->capacity);
}

inline static void FrameStack_free(FrameStack* self) {
  Memory_free(self->items);
}

inline static void FrameStack_push(FrameStack* self, Frame v) {
  if(self->height == self->capacity) {
    assert(false); // TODO Implement resizing
  }

  assert(self->height < self->capacity);

  self->items[self->height++] = v;
}

inline static Frame FrameStack_pop(FrameStack* self) {
  assert(self->height > 0);
  return self->items[--self->height];
}

#ifdef TEST
void test_FrameStack_initAndFree();
void test_FrameStack_pushPop();
#endif

#endif
