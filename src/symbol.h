#ifndef SYMBOL_H
#define SYMBOL_H

typedef struct {
  uint8_t length;
  char name[];
} Symbol;

#endif
