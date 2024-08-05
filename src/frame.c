#include "frame.h"

#ifdef TEST
#include <assert.h>

void test_Frame_init() {
  Frame frame;
  uint8_t rp;
  size_t stackIndex;
  Frame_init(&frame, &rp, 42);

  assert(frame.rp == &rp);
  assert(frame.stackIndex == 42);
}
#endif
