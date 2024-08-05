#include "thread.h"

#ifdef TEST
void test_Thread_initAndFree() {
  int allocationLocation;
  Test_init(&allocationLocation);
  uint8_t instruction;

  Thread thread;
  Thread_init(&thread, &instruction);
  assert(thread.ip == &instruction);
  assert(thread.heap == NULL);
  assert(thread.frames.height == 0);
  assert(thread.messages.first == NULL);
  assert(thread.next == NULL);

  Thread_free(&thread);
  assert(Test_getAllocationCount() == Test_getFreeCount());
}
#endif
