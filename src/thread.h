#ifndef THREAD_H
#define THREAD_H

#include "frame_stack.h"
#include "message_queue.h"
#include "value_stack.h"

struct Thread;
typedef struct Thread Thread;

struct Thread {
  uint8_t* ip;
  ValueStack stack;
  Obj* heap;
  FrameStack frames;
  MessageQueue messages;
  Thread* next; // for ThreadQueue
};

#endif
