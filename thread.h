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
  self->panic = false;
}

#ifdef TEST

#endif

#endif
