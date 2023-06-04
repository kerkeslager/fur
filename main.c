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

int main() {
  Compiler compiler;
  Compiler_init(&compiler, true /* Init in REPL mode */);
  InstructionList byteCode;
  InstructionList_init(&byteCode);
  Thread thread;
  Thread_init(&thread, &byteCode);

  for(;;) {
    char* buffer = readline("> ");

    if (buffer && *buffer) {
      add_history(buffer);

      bool success = Compiler_compile(&compiler, &byteCode, (const char*)buffer);

      if(success) {
        Value result = Thread_run(&thread);

        if(thread.panic) {
          Thread_clearPanic(&thread);
        } else {
          Value_println(result);
        }

        /*
         *  TODO
         *  We can't call free(buffer); here because it's the source that
         *  all our symbols point to, so for the moment we're just leaking memory.
         */
      } else {
        if(isColorAllowed()) {
          fprintf(stderr, ANSI_COLOR_RED "Error in compilation\n" ANSI_COLOR_RESET);
        } else {
          fprintf(stderr, "Error in compilation\n");
        }
      }
    }
  }

  Thread_free(&thread);
  InstructionList_free(&byteCode);
  Compiler_free(&compiler);

  return 0;
}
