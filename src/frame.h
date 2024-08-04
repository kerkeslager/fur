#ifndef FRAME_H
#define FRAME_H

typedef struct {
  uint8_t* rp; // Return pointer
  size_t stackIndex;
} Frame;

#endif
