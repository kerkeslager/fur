#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "error.h"

#define ANSI_COLOR_RED    "\x1b[31m"
#define ANSI_COLOR_RESET  "\x1b[0m"

void printError(size_t line, const char* fmt, ...) {
  // Support the NO_COLOR flag, see https://no-color.org/ for explanation.
  const char* noColorEnv = getenv("NO_COLOR");
  bool isColorAllowed = noColorEnv == NULL || noColorEnv[0] == '\0';

  if(isColorAllowed) fprintf(stderr, ANSI_COLOR_RED);

  fprintf(stderr, "Error (line %zu): ", line);

  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);

  fprintf(stderr, "\n");

  if(isColorAllowed) fprintf(stderr, ANSI_COLOR_RESET);
}

#undef ANSI_COLOR_RED
#undef ANSI_COLOR_RESET
