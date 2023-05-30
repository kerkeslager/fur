#ifndef THREAD_H
#define THREAD_H

#include <stdbool.h>

#include "instruction.h"
#include "value_stack.h"

typedef struct {
  InstructionList* instructionList;
  size_t pcIndex;
  ValueStack stack;
  bool panic;
} Thread;

void Thread_init(Thread*, InstructionList*);
void Thread_free(Thread*);

Value Thread_run(Thread*);

inline static void Thread_clearPanic(Thread* self) {
  assert(self->panic); // No reason to call this if not in panic

  self->panic = false;
  self->pcIndex = InstructionList_count(self->instructionList);
}

#ifdef TEST

void test_Thread_clearPanic_setsPanicFalse();
void test_Thread_clearPanic_setsPCIndexToEnd();

#endif

#endif
