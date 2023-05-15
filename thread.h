#ifndef THREAD_H
#define THREAD_H

#include "instruction.h"
#include "stack.h"

typedef struct {
  Instruction* pc;
  Stack stack;
} Thread;

void Thread_init(Thread*, Instruction*);
void Thread_free(Thread*);

void Thread_run(Thread*);

#ifdef TEST

#endif

#endif
