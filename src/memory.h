#ifndef MEMORY_H
#define MEMORY_H

#include <assert.h>
#include <stdlib.h>

#ifdef TEST
  #define TEST_MAX_ALLOCATIONS 16
  static size_t _allocationCount;
  static size_t _freeCount;
  static size_t _allocationList[TEST_MAX_ALLOCATIONS];
  static void* _freeList[TEST_MAX_ALLOCATIONS];

  static void* _allocationResult;

  inline static void Test_init(void* allocationResult) {
    _allocationCount = 0;
    _freeCount = 0;
    _allocationResult = allocationResult;
  }

  inline static void Test_setAllocationResult(void* ptr) {
    _allocationResult = ptr;
  }

  inline static size_t Test_getAllocationCount() { return _allocationCount; }
  inline static size_t Test_getAllocation(size_t index) {
    assert(index < _allocationCount);
    assert(index < TEST_MAX_ALLOCATIONS);
    return _allocationList[index];
  }

  inline static size_t Test_getFreeCount() { return _freeCount; }
  inline static void* Test_getFree(size_t index) {
    assert(index < _freeCount);
    assert(index < TEST_MAX_ALLOCATIONS);
    return _freeList[index];
  }

  inline static void* Memory_allocate(size_t size) {
    assert(_allocationCount < TEST_MAX_ALLOCATIONS);
    _allocationList[_allocationCount++] = size;
    return _allocationResult;
  }

  inline static void Memory_free(void* ptr) {
    assert(_freeCount < _allocationCount);
    assert(_freeCount < TEST_MAX_ALLOCATIONS);
    _freeList[_freeCount++] = ptr;
  }
#else
  inline static void* Memory_allocate(size_t size) {
    void* result = malloc(size);

    // TODO Handle this
    assert(result != NULL);

    return result;
  }

  inline static void Memory_free(void* ptr) {
    assert(ptr != NULL);
    free(ptr);
  }
#endif

#endif