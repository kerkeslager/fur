#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>

#include "compiler.h"
#include "error.h"
#include "thread.h"
#include "value.h"
#include "value_stack.h"

int main() {
  Compiler compiler;
  Compiler_init(&compiler, true /* Init in REPL mode */);

  for(;;) {
    char* buffer = readline("> ");

    if (buffer && *buffer) {
      add_history(buffer);

      InstructionList byteCode;
      InstructionList_init(&byteCode);


      bool success = Compiler_compile(&compiler, &byteCode, (const char*)buffer);

      if(success) {
        Thread thread;
        Thread_init(&thread, &byteCode);
        Value result = Thread_run(&thread);

        if(thread.panic) {
          Thread_clearPanic(&thread);
        } else {
          Value_println(result);
        }

        Thread_free(&thread);
        InstructionList_free(&byteCode);
        free(buffer);
      } else {
        if(isColorAllowed()) {
          fprintf(stderr, ANSI_COLOR_RED "Error in compilation\n" ANSI_COLOR_RESET);
        } else {
          fprintf(stderr, "Error in compilation\n");
        }
      }

    }
  }

  Compiler_free(&compiler);

  return 0;
}
