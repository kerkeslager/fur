#ifndef SYMBOL_H
#define SYMBOL_H

#include "common.h"

struct Symbol {
  uint8_t length;
  char name[];
};

#endif
