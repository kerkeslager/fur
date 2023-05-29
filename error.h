#ifndef ERROR_H
#define ERROR_H

#include <stdbool.h>

#define ANSI_COLOR_RED    "\x1b[31m"
#define ANSI_COLOR_RESET  "\x1b[0m"

// Support the NO_COLOR flag, see https://no-color.org/ for explanation.
inline static bool isColorAllowed() {
  char* noColorEnv = getenv("NO_COLOR");
  return noColorEnv == NULL || noColorEnv[0] == '\0';
}

#ifdef TEST

#endif

#endif
