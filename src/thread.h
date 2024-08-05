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

inline static void Thread_init(Thread* self, uint8_t* ip) {
  self->ip = ip;
  ValueStack_init(&(self->stack));
  self->heap = NULL;
  FrameStack_init(&(self->frames));
  MessageQueue_init(&(self->messages));
  self->next = NULL;
}

inline static void Thread_free(Thread* self) {
  ValueStack_free(&(self->stack));
  // TODO Free the heap
  FrameStack_free(&(self->frames));
  // TODO Free the message queue
  self->next = NULL;
}

#ifdef TEST
void test_Thread_initAndFree();
#endif

#endif
