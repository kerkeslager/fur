#ifndef FRAME_H
#define FRAME_H

#include <stdint.h>
#include <stdlib.h>

typedef struct {
  uint8_t* rp; // Return pointer
  size_t stackIndex;
} Frame;

inline static void Frame_init(Frame* self, uint8_t* rp, size_t stackIndex) {
  self->rp = rp;
  self->stackIndex = stackIndex;
}

#ifdef TEST
void test_Frame_init();
#endif

#endif
