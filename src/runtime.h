#ifndef RUNTIME_H
#define RUNTIME_H

#include "symbol_set.h"
#include "thread_queue.h"

typedef struct {
  ModuleDictionary modules;
  SymbolSet symbols;
  ThreadQueue threads;
  size_t threadCount;
  size_t pthreadCount;
} Runtime;

#endif
