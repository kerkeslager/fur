#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>

#include "compiler.h"
#include "thread.h"
#include "value.h"
#include "value_stack.h"

int main() {
  for(;;) {
    char* buffer = readline("> ");

    if (buffer && *buffer) {
      add_history(buffer);

      InstructionList byteCode;
      InstructionList_init(&byteCode);

      Compiler compiler;
      Compiler_init(&compiler, true /* Init in REPL mode */);

      bool success = Compiler_compile(&compiler, &byteCode, (const char*)buffer);

      if(success) {
        Thread thread;
        Thread_init(&thread, byteCode.items);
        Thread_run(&thread);

        Value result = ValueStack_pop(&(thread.stack));
        Value_println(result);

        Thread_free(&thread);
        InstructionList_free(&byteCode);
        free(buffer);
      } else {
        fprintf(stderr, "Error in compilation\n");
      }
    }
  }

  return 0;
}
