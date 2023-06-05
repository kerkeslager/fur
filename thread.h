#ifndef THREAD_H
#define THREAD_H

#include <stdbool.h>

#include "instruction.h"
#include "stack.h"

typedef struct {
  ByteCode* byteCode;
  size_t pcIndex;
  Stack stack;
  bool panic;
} Thread;

void Thread_init(Thread*, ByteCode*);
void Thread_free(Thread*);

Value Thread_run(Thread*);

inline static void Thread_clearPanic(Thread* self) {
  assert(self->panic); // No reason to call this if not in panic

  self->panic = false;
  self->pcIndex = ByteCode_count(self->byteCode);
}

#ifdef TEST

void test_Thread_run_executesIntegerMathOps();
void test_Thread_run_integerComparison();

void test_Thread_clearPanic_setsPanicFalse();
void test_Thread_clearPanic_setsPCIndexToEnd();

#endif

#endif
