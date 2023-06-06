#ifndef ERROR_H
#define ERROR_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#define ANSI_COLOR_RED    "\x1b[31m"
#define ANSI_COLOR_RESET  "\x1b[0m"

// TODO Get rid of this, now that we have printError
// Support the NO_COLOR flag, see https://no-color.org/ for explanation.
inline static bool isColorAllowed() {
  char* noColorEnv = getenv("NO_COLOR");
  return noColorEnv == NULL || noColorEnv[0] == '\0';
}

static void printError(size_t line, const char* fmt, ...) {
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

#ifdef TEST

#endif

#endif
