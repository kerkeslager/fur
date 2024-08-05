#ifndef MEMORY_H
#define MEMORY_H

#include <assert.h>
#include <stdlib.h>

inline void* Memory_allocate(size_t size) {
  void* result = malloc(size);

  // TODO Handle this
  assert(result != NULL);

  return result;
}

inline void Memory_free(void* ptr) {
  assert(ptr != NULL);
  free(ptr);
}

#endif
