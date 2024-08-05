#include "frame_stack.h"

#ifdef TEST

#include <assert.h>

void test_FrameStack_initAndFree() {
  FrameStack stack;
  Frame stackItems[10];
  Test_init(stackItems);

  FrameStack_init(&stack);

  assert(stack.height == 0);
  assert(stack.capacity == FRAME_STACK_INIT_ALLOCATE);
  assert(stack.items == stackItems);
  assert(Test_getAllocationCount() == 1);
  assert(Test_getAllocation(0) == sizeof(Frame) * FRAME_STACK_INIT_ALLOCATE);

  FrameStack_free(&stack);

  assert(Test_getFreeCount() == 1);
  assert(Test_getFree(0) == stackItems);
}

void test_FrameStack_pushPop() {
  FrameStack stack;
  Frame stackItems[10];
  Test_init(stackItems);

  FrameStack_init(&stack);

  uint8_t instruction;
  Frame f;
  Frame_init(&f, &instruction, 42);

  FrameStack_push(&stack, f);
  assert(stack.height == 1);

  Frame result = FrameStack_pop(&stack);

  assert(result.rp == f.rp);
  assert(result.stackIndex == f.stackIndex);
  assert(stack.height == 0);

  FrameStack_free(&stack);
}
#endif
