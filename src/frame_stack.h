#ifndef FRAME_STACK_H
#define FRAME_STACK_H

#include "frame.h"

typedef struct {
  size_t height;
  size_t capacity;
  Frame* items;
} FrameStack;

#endif
