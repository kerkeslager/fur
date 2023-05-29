#ifndef THREAD_H
#define THREAD_H

#include <stdbool.h>

#include "instruction.h"
#include "value_stack.h"

typedef struct {
  InstructionList* instructionList;
  uint8_t* pc;
  ValueStack stack;
  bool panic;
} Thread;

void Thread_init(Thread*, InstructionList*);
void Thread_free(Thread*);

void Thread_run(Thread*);

inline static void Thread_clearPanic(Thread* self) {
  self->panic = false;
}

#ifdef TEST

#endif

#endif
