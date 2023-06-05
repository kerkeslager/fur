#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>

#include "compiler.h"
#include "error.h"
#include "thread.h"
#include "value.h"
#include "value_stack.h"

#include <stdio.h>

typedef struct {
  const char** items;
  size_t count;
  size_t capacity;
} BufferList;

void BufferList_init(BufferList* self) {
  self->count = 0;
  self->capacity = 16;
  self->items = malloc(sizeof(char*) * self->capacity);
}

void BufferList_free(BufferList* self) {
  for(size_t i = 0; i < self->count; i++) {
    free((void*)(self->items[i]));
  }
  free(self->items);
}

void BufferList_append(BufferList* self, const char* buffer) {
  if(self->count == self->capacity) {
    self->capacity *= 2;
    self->items = realloc(self->items, self->capacity * sizeof(char*));
  }

  self->items[self->count++] = buffer;
}

int main() {
  Compiler compiler;
  Compiler_init(&compiler, true /* Init in REPL mode */);
  ByteCode byteCode;
  ByteCode_init(&byteCode);
  Thread thread;
  Thread_init(&thread, &byteCode);
  BufferList bufferList;
  BufferList_init(&bufferList);

  for(;;) {
    const char* buffer = readline("> ");

    if (buffer && *buffer) {
      add_history(buffer);

      bool success = Compiler_compile(&compiler, &byteCode, buffer);

      if(success) {
        Value result = Thread_run(&thread);

        if(thread.panic) {
          Thread_clearPanic(&thread);
        } else {
          Value_println(result);
        }
      } else {
        if(isColorAllowed()) {
          fprintf(stderr, ANSI_COLOR_RED "Error in compilation\n" ANSI_COLOR_RESET);
        } else {
          fprintf(stderr, "Error in compilation\n");
        }
      }
    }

    /*
     * Save off buffers because they contain the memory locations to which
     * all of our symbols point.
     */
    if(buffer != NULL) BufferList_append(&bufferList, buffer);
  }

  BufferList_free(&bufferList);
  Thread_free(&thread);
  ByteCode_free(&byteCode);
  Compiler_free(&compiler);

  return 0;
}
