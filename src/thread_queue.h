#ifndef THREAD_QUEUE_H
#define THREAD_QUEUE_H

#include "thread.h"

typedef struct {
  Thread* first;
  Thread* last;
} ThreadQueue;

#endif
