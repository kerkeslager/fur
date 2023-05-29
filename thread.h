#ifndef THREAD_H
#define THREAD_H

#include <stdbool.h>

#include "instruction.h"
#include "value_stack.h"

typedef struct {
  uint8_t* pc;
  ValueStack stack;
  bool panic;
} Thread;

void Thread_init(Thread*, uint8_t*);
void Thread_free(Thread*);

void Thread_run(Thread*, InstructionList*);

#ifdef TEST

#endif

#endif
