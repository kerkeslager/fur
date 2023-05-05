#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

void sayHello() {
  printf("Hello, world");
}

#ifdef TEST

void test_false() {
  assert(false);
}

#endif
